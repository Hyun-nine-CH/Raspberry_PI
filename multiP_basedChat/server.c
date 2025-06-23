#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#define PORT 9999
#define BUFSIZ 1024

void daemonServer() {
    pid_t pid=fork();
    if(pid<0) exit(1);
    if(pid>0) exit(0);

    setsid();
    chdir("/");
    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if(fd!=-1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if(fd>2) close(fd);
    }
}

void client(int csock) {
    char buf[BUFSIZ];
    while(1) {
        memset(buf, 0, BUFSIZ);
        int n = read(csock, buf, BUFSIZ);
        if(n<=0) break;

        syslog(LOG_INFO, "클라이언트 메시지 수신: %s", buf);
        write(csock, buf, strlen(buf));
    }
    syslog(LOG_INFO, "클라이언트 연결 종료");
    close(csock);
    exit(0);
}

int main() {
    daemonServer();

    openlog("Server", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "채팅 서버 시작");

    int ssock, csock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len=sizeof(cli_addr);

    ssock=socket(AF_INET, SOCK_STREAM, 0);
    if(ssock == -1) {
        syslog(LOG_ERR, "소켓 생성 실패: %s", strerror(errno));
//        perror("socket"); 데몬프로세스로 닫아놓음
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(ssock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {    
//    perror("bind"); 데몬프로세스로 닫아놓음
        syslog(LOG_ERR, "bind 싫패: %s", strerror(errno));
        exit(1);
    }

    listen(ssock, 5);
//    printf("Server listening on port %d...\n", PORT);
    syslog(LOG_INFO, "Port %d에서 클라이언트 연결 대기 중...", PORT);

    while(1) {
        csock=accept(ssock, (struct sockaddr*)&cli_addr, &cli_len);
        if(csock < 0) {
            syslog(LOG_WARNING, "accept 실패: %s", strerror(errno));
            continue;
        }
        syslog(LOG_INFO, "클라이언트 접속: %s", inet_ntoa(cli_addr.sin_addr));

        pid_t pid = fork();
        if(pid==0) {
            close(ssock);
            client(csock);
        }else{
            close(csock);
        }
    }
    close(ssock);
    closelog();
    return 0;
}
