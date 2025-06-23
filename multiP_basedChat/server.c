#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>

#define PORT 9999
#define MAX_CLIENTS 5
#define BUFSIZ 1024

int client_pipes[MAX_CLIENTS][2];
//pid_t child_pids[MAX_CLIENTS];
int client_count = 0;

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

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigusr1(int sig) {
    char msg[BUFSIZ];
    for(int i=0; i<client_count; i++) {
        int n=read(client_pipes[i][0], msg, BUFSIZ);
        if(n>0) {
            for(int j=0; j<client_count; j++) {
                if(i!=j) {
                    write(client_pipes[j][1], msg, n);
                }
            }
        }
    }
}

int main() {
    daemonServer();

    signal(SIGCHLD, handle_sigchld);
    signal(SIGUSR1, handle_sigusr1);

    openlog("Server", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "채팅 서버 시작");

    int ssock, csock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len=sizeof(cli_addr);

    ssock=socket(AF_INET, SOCK_STREAM, 0);
    if(ssock == -1) {
        syslog(LOG_ERR, "소켓 생성 실패: %s", strerror(errno));
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(ssock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {    
        syslog(LOG_ERR, "bind 싫패: %s", strerror(errno));
        exit(1);
    }

    listen(ssock, 5);
    syslog(LOG_INFO, "Port %d에서 클라이언트 연결 대기 중...", PORT);

    while(1) {
        csock=accept(ssock, (struct sockaddr*)&cli_addr, &cli_len);
        if(csock < 0) {
            syslog(LOG_WARNING, "accept 실패: %s", strerror(errno));
            continue;
        }

        if(pipe(client_pipes[client_count]) < 0) {
            syslog(LOG_ERR, "pipe 생성 실패");
            close(csock);
            continue;
        }
//        syslog(LOG_INFO, "클라이언트 접속: %s", inet_ntoa(cli_addr.sin_addr));

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        syslog(LOG_INFO, "클라이언트 접속: %s:%d", ip, ntohs(cli_addr.sin_port));

        pid_t pid = fork();
        if(pid==0) {
            close(ssock);
            close(client_pipes[client_count][0]);

            char buf[BUFSIZ], recv_buf[BUFSIZ];
            while(1) {
                memset(buf, 0, BUFSIZ);
                int n = read(csock, buf, BUFSIZ);
                if(n<=0) break;

                write(client_pipes[client_count][1], buf, n);
                kill(getppid(), SIGUSR1);

                int m=read(client_pipes[client_count][0], recv_buf, BUFSIZ);
                if(m>0) write(csock, recv_buf, m);
            }

            close(client_pipes[client_count][1]);
            close(csock);
            exit(0);
        }else{
            close(client_pipes[client_count][1]);
            close(csock);
            client_count++;
        }
    }
    close(ssock);
    closelog();
    return 0;
}
