#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
 
#define PORT 9999
#define MAX_ROOM_NAME 32
#define MAX_NICKNAME 32
#define MAX_MSG 1024
#define MAX_CLIENTS 5

int sock;

typedef struct {
    pid_t pid;
    char room[MAX_ROOM_NAME];
    char nickname[MAX_NICKNAME];
} ClientInfo;

ClientInfo self_info = {0, "", ""};

void cleanup_and_exit(int sig) {
    close(sock);
    printf("\n클라이언트 종료.\n");
    exit(0);
}

void process_command(const char *input) {
    ssize_t n = write(sock, input, strlen(input));
    if (n > 0) {
        printf("[전송됨] %s\n", input);
    } else {
        fprintf(stderr, "서버로 메시지 전송 실패: %s\n", strerror(errno));
    }
}

int main() {
    struct sockaddr_in serv_addr;
    char send_buf[MAX_MSG], recv_buf[MAX_MSG];

    signal(SIGINT, cleanup_and_exit);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect");
            exit(1);
        } else {
            printf("[정보] 서버에 비동기 연결 시도 중 (EINPROGRESS)\n");
        }
    }

    printf("채팅 클라이언트 시작 (종료: Ctrl+C)\n");

    while (1) {
        memset(send_buf, 0, MAX_MSG);
        printf("입력 > ");
        fflush(stdout);
        if (fgets(send_buf, MAX_MSG, stdin) == NULL) break;

        send_buf[strcspn(send_buf, "\n")] = 0;
        if (strlen(send_buf) > 0) process_command(send_buf);

        memset(recv_buf, 0, MAX_MSG);
        int n = read(sock, recv_buf, MAX_MSG);
        if (n > 0) {
            recv_buf[n] = '\0';
            printf("서버: %s\n", recv_buf);
        } else if (n == 0) {
            printf("서버와 연결 종료됨\n");
            break;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
        }
    }

    cleanup_and_exit(0);
    return 0;
}
