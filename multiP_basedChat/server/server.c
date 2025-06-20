#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 9999
// #define BUFSIZ 1024

void client(int csock) {
    char buf[BUFSIZ];
    while(1) {
        memset(buf, 0, BUFSIZ);
        int n = read(csock, buf, BUFSIZ);
        if(n<=0) break;
        printf("Client: %s", buf);
        write(csock, buf, strlen(buf));
    }
    close(csock);
    exit(0);
}

int main() {
    int ssock, csock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len=sizeof(cli_addr);

    ssock=socket(AF_INET, SOCK_STREAM, 0);
    if(ssock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(ssock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {    
    perror("bind");
    exit(1);
    }

    listen(ssock, 5);
    printf("Server listening on port %d...\n", PORT);

    while(1) {
        csock=accept(ssock, (struct sockaddr*)&cli_addr, &cli_len);
        if(csock < 0) continue;

        pid_t pid = fork();
        if(pid==0) {
            close(ssock);
            client(csock);
        }else{
            close(csock);
        }
    }
    close(ssock);
    return 0;
}
