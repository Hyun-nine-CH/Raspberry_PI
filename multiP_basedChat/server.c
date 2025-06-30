/* server.c - 멀티프로세스 TCP 채팅 서버 (fork + pipe + sigaction + non-blocking + 브로드캐스트 + graceful shutdown) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9999
#define MAX_CLIENTS 10
#define BUFSIZE 1024

typedef struct {
    pid_t pid;
    int pipe_parent[2]; // [0]=read, [1]=write (자식→부모)
    int pipe_child[2];  // [0]=read, [1]=write (부모→자식)
    int csock;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int client_count = 0;
int listen_sock;

void cleanup_and_exit(int status);

void broadcast_message(const char *msg, pid_t sender_pid) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].pid != sender_pid) {
            write(clients[i].pipe_child[1], msg, strlen(msg));
        }
    }
}

void handle_usr1(int sig) {
    char buf[BUFSIZE];
    for (int i = 0; i < client_count; ++i) {
        int fd = clients[i].pipe_parent[0];
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        while (1) {
            memset(buf, 0, BUFSIZE);
            int n = read(fd, buf, BUFSIZE);
            if (n > 0) {
                printf("[수신 - PID %d]: %s", clients[i].pid, buf);
                fflush(stdout);
                broadcast_message(buf, clients[i].pid);
            } else {
                break;
            }
        }
    }
}

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_shutdown(int sig) {
    fprintf(stderr, "\n[서버 종료 요청: SIG %d]\n", sig);
    cleanup_and_exit(0);
}

void set_signal(int signo, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(signo, &sa, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
}

void cleanup_and_exit(int status) {
    for (int i = 0; i < client_count; ++i) {
        kill(clients[i].pid, SIGTERM);
        waitpid(clients[i].pid, NULL, 0);
        close(clients[i].pipe_parent[0]);
        close(clients[i].pipe_child[1]);
        close(clients[i].csock);
    }
    close(listen_sock);
    fprintf(stderr, "[서버 종료]\n");
    exit(status);
}

int main() {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    set_signal(SIGCHLD, handle_sigchld);
    set_signal(SIGINT, handle_shutdown);
    set_signal(SIGTERM, handle_shutdown);
    set_signal(SIGUSR1, handle_usr1);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_sock, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }

    printf("[서버 시작 - 포트 %d]\n", PORT);

    while (1) {
        int csock = accept(listen_sock, (struct sockaddr*)&cliaddr, &clilen);
        if (csock < 0) {
            perror("accept");
            continue;
        }

        if (client_count >= MAX_CLIENTS) {
            fprintf(stderr, "[최대 클라이언트 수 초과]\n");
            close(csock);
            continue;
        }

        int pipe_p[2], pipe_c[2];
        if (pipe(pipe_p) < 0 || pipe(pipe_c) < 0) {
            perror("pipe");
            close(csock);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(csock);
            continue;
        } else if (pid == 0) {
            // 자식: 클라이언트 통신 담당
            close(pipe_p[0]); // 부모가 읽는 쪽 닫음
            close(pipe_c[1]); // 부모가 쓰는 쪽 닫음

            char buf[BUFSIZE];
            while (1) {
                memset(buf, 0, BUFSIZE);
                int n = read(csock, buf, BUFSIZE);
                if (n <= 0) break;
                write(pipe_p[1], buf, n);
                kill(getppid(), SIGUSR1);

                // 부모가 보낸 메시지도 읽어서 클라이언트로 전달
                memset(buf, 0, BUFSIZE);
                int m = read(pipe_c[0], buf, BUFSIZE);
                if (m > 0) write(csock, buf, m);
            }

            close(pipe_p[1]);
            close(pipe_c[0]);
            close(csock);
            exit(0);
        }

        // 부모
        close(pipe_p[1]); // 자식 쓰는 쪽 닫음
        close(pipe_c[0]); // 자식 읽는 쪽 닫음

        clients[client_count].pid = pid;
        clients[client_count].pipe_parent[0] = pipe_p[0];
        clients[client_count].pipe_child[1] = pipe_c[1];
        clients[client_count].csock = csock;
        client_count++;

        printf("[클라이언트 연결 - PID %d]\n", pid);
    }

    return 0;
}
