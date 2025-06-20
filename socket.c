#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int g_pfd[2];
static int ssockfd;

void sigHandler(int signo) {
    char buf[BUFSIZ];
    close(g_pfd[1]);
    int n = read(g_pfd[0], buf, BUFSIZ);
    write(ssock, buf, n);
    close(g_pfd[1]);
}

int main(int argc, chat** argv) {
    int ssockfd;
    struct sockaddr_in servaddr;
    int clntlen = sizeof(clntaddr);

    if(argv < 3) {
        fprintf(strerr, "usage : %s IP_ADDR PORT_NO\n", argv[0]);
        return -1;
    }

    ssockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(ssockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    // mam inet_pton
    inet_pton(AF_INET, argv[1], servaddr.sin_addr.s_addr);

//    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[2]));
//    bind(ssockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

//    listen(ssockfd, 8);

    while(1) {

        int pfd[2], pid;
//        int pfd1[2], pfd2[2], pid;
        int csockfd = accept(ssockfd, (struct sockaddr*)&clntaddr, clntlen);
        //pipe(pfd1);
        //pipe(pfd2);

        pipe(g_pfd);

        if((pid = fork()) < 0) {
            perror("fork( )");
            signal(SIGUSR1, sigHandler);
            close(g_pfd[1]);
            perror("fork( )");
        }else if (pid == 0) {
            close(pfd[0]);
            do {
                fgets(buf, BUFSIZ, stdin);
                write(g_pfd[1], buf, strlen(buf+1));
                kill(ppid(), SIGUSR1);
            }while(strcmp(buf, "quit"))
                //            signal(SIGUSR1, sigHandler);
        }else{            // pid > 0
        }
    }
    close(ssockfd);
    return 0;
}

