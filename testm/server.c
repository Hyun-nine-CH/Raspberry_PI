#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 9999
#define MAX_CLIENTS 2
#define BUF_SIZE 1024

int client_socks[MAX_CLIENTS]; // 자식들과 통신하는 socket
int pipes[MAX_CLIENTS][2];     // 부모 <-> 자식 파이프
pid_t child_pids[MAX_CLIENTS];
int client_count = 0;

void sigusr1_handler(int sig) {
    char buf[BUF_SIZE];
    for (int i = 0; i < client_count; i++) {
        int n = read(pipes[i][0], buf, BUF_SIZE);
        if (n > 0) {
            // 받은 메시지를 다른 자식에게 전달
            for (int j = 0; j < client_count; j++) {
                if (j != i) {
                    write(client_socks[j], buf, n);
                }
            }
        }
    }
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGCHLD, sigchld_handler);

    int ssock = socket(AF_INET, SOCK_STREAM, 0);
    if (ssock < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(ssock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(ssock, MAX_CLIENTS);
    printf("서버 시작, 포트 %d 에서 대기 중...\n", PORT);

    while (client_count < MAX_CLIENTS) {
        int csock = accept(ssock, (struct sockaddr*)&cli_addr, &cli_len);
        if (csock < 0) continue;

        pipe(pipes[client_count]);

        pid_t pid = fork();
        if (pid == 0) {
            close(pipes[client_count][0]); // 자식은 읽기 파이프 닫음
            char buf[BUF_SIZE];
            while (1) {
                memset(buf, 0, BUF_SIZE);
                int n = read(csock, buf, BUF_SIZE);
                if (n <= 0) break;
                write(pipes[client_count][1], buf, n);
                kill(getppid(), SIGUSR1);
            }
            close(pipes[client_count][1]);
            close(csock);
            exit(0);
        } else {
            close(pipes[client_count][1]); // 부모는 쓰기 파이프 닫음
            client_socks[client_count] = csock;
            child_pids[client_count] = pid;
            client_count++;
        }
    }

    // 부모는 대기
    while (1) pause();
    return 0;
}

