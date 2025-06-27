/* server.c - 멀티프로세스 TCP 채팅 서버 (pipe + fork + signal + graceful shutdown) */

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
    int pipefd[2]; // [0]=read, [1]=write
    int csock;
} ClientInfo;

ClientInfo *clients = NULL;
int client_count = 0;
int listen_sock;

void cleanup_and_exit(int status);

void sigchld_handler(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void sigint_handler(int sig) {
    fprintf(stderr, "\n[서버 종료 시도: 시그널 %d]\n", sig);
    cleanup_and_exit(0);
}

void broadcast_message(const char *msg, int sender_pid) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].pid != sender_pid) {
            if (write(clients[i].csock, msg, strlen(msg)) < 0) {
                perror("[오류] 클라이언트로 메시지 전송 실패");
            }
        }
    }
}

void handle_usr1(int sig) {
    char buf[BUFSIZE];
    for (int i = 0; i < client_count; ++i) {
        memset(buf, 0, BUFSIZE);
        int n = read(clients[i].pipefd[0], buf, BUFSIZE);
        if (n > 0) {
            printf("[메시지 수신 - PID %d]: %s", clients[i].pid, buf);
            broadcast_message(buf, clients[i].pid);
        }
    }
}

void cleanup_and_exit(int status) {
    // 자식 프로세스 종료
    for (int i = 0; i < client_count; ++i) {
        if (kill(clients[i].pid, SIGTERM) == 0) {
            waitpid(clients[i].pid, NULL, 0);
        }
        close(clients[i].pipefd[0]);
        close(clients[i].csock);
    }
    // 리스닝 소켓 종료
    close(listen_sock);
    // 동적 메모리 해제
    free(clients);
    fprintf(stderr, "[서버 정상 종료]\n");
    exit(status);
}

int main() {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    clients = calloc(MAX_CLIENTS, sizeof(ClientInfo));
    if (!clients) {
        perror("[오류] 클라이언트 배열 할당 실패");
        exit(1);
    }

    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGUSR1, handle_usr1);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("[오류] 소켓 생성 실패");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("[오류] 바인드 실패");
        exit(1);
    }

    if (listen(listen_sock, MAX_CLIENTS) < 0) {
        perror("[오류] 리슨 실패");
        exit(1);
    }

    printf("[서버 시작 - 포트 %d]\n", PORT);

    while (1) {
        int csock = accept(listen_sock, (struct sockaddr*)&cliaddr, &clilen);
        if (csock < 0) {
            perror("[오류] accept 실패");
            continue;
        }

        if (client_count >= MAX_CLIENTS) {
            fprintf(stderr, "[경고] 최대 클라이언트 수 도달\n");
            close(csock);
            continue;
        }

        int pipefd[2];
        if (pipe(pipefd) < 0) {
            perror("[오류] 파이프 생성 실패");
            close(csock);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("[오류] fork 실패");
            close(csock);
            close(pipefd[0]);
            close(pipefd[1]);
            continue;
        } else if (pid == 0) {
            // 자식 프로세스
            close(pipefd[0]); // read end
            char buf[BUFSIZE];
            while (1) {
                memset(buf, 0, BUFSIZE);
                int n = read(csock, buf, BUFSIZE);
                if (n <= 0) break;
                write(pipefd[1], buf, n);
                kill(getppid(), SIGUSR1);
            }
            close(pipefd[1]);
            close(csock);
            exit(0);
        }

        // 부모 프로세스
        close(pipefd[1]); // write end
        clients[client_count].pid = pid;
        clients[client_count].pipefd[0] = pipefd[0];
        clients[client_count].csock = csock;
        client_count++;

        printf("[클라이언트 연결 - PID %d]\n", pid);
    }

    return 0;
}

