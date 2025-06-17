#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 5100

int main(int argc, char *argv[]) {
    int ssock;
    struct sockaddr_in servaddr;
    char send_buf[BUFSIZ], recv_buf[BUFSIZ];
    fd_set readfd;
    int maxfd, n;

    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }

    // 소켓 생성
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    // 서버 연결
    if (connect(ssock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    printf("Connected to server. Type message (q to quit):\n");

    do {
        FD_ZERO(&readfd);                   // fd 집합 초기화
        FD_SET(0, &readfd);                 // 표준 입력 감시
        FD_SET(ssock, &readfd);             // 서버 소켓 감시

        maxfd = ssock + 1;

        select(maxfd, &readfd, NULL, NULL, NULL); // 입출력 이벤트 대기

        if (FD_ISSET(0, &readfd)) {         // 키보드 입력 감지
            memset(send_buf, 0, sizeof(send_buf));
            fgets(send_buf, BUFSIZ, stdin);
            write(ssock, send_buf, strlen(send_buf));
        }

        if (FD_ISSET(ssock, &readfd)) {     // 서버로부터 수신 데이터 감지
            memset(recv_buf, 0, sizeof(recv_buf));
            n = read(ssock, recv_buf, BUFSIZ - 1);
            if (n <= 0) {
                printf("Disconnected from server.\n");
                break;
            }
            printf(">> %s", recv_buf);
        }

    } while (strncmp(send_buf, "q", 1) != 0);

    close(ssock);
    return 0;
}

