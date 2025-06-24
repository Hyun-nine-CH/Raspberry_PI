#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 9999
//#define BUFSIZ 1024
#define MAX_ROOM_NAME 32
#define MAX_NICKNAME 32

int sock;
WINDOW *chat_win, *input_win;

typedef struct {
    pid_t pid;
    char room[MAX_ROOM_NAME];
    char nickname[MAX_NICKNAME];
} ClientInfo;

ClientInfo self_info = {0, "", ""};

void cleanup_and_exit(int sig) {
    endwin();
    close(sock);
    printf("\n클라이언트 종료.\n");
    exit(0);
}

void process_command(const char *input) {
    // 명령어이든 일반 메시지이든 모두 서버로 전송
    write(sock, input, strlen(input));
}

int main() {
    struct sockaddr_in serv_addr;
    char send_buf[BUFSIZ], recv_buf[BUFSIZ];

    signal(SIGINT, cleanup_and_exit);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    fcntl(sock, F_SETFL, O_NONBLOCK); // 비차단 소켓

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect");
            exit(1);
        }
    }

    initscr();
    cbreak();
    echo();
    curs_set(1);

    chat_win = newwin(20, 80, 0, 0);
    input_win = newwin(3, 80, 20, 0);
    scrollok(chat_win, TRUE);
    box(chat_win, 0, 0);
    box(input_win, 0, 0);
    wrefresh(chat_win);
    wrefresh(input_win);

    while (1) {
        memset(send_buf, 0, BUFSIZ);
        mvwgetnstr(input_win, 1, 1, send_buf, BUFSIZ - 1);
        process_command(send_buf);

        memset(recv_buf, 0, BUFSIZ);
        int n = read(sock, recv_buf, BUFSIZ);
        if (n > 0) {
            recv_buf[n] = '\0';
            wprintw(chat_win, "%s\n", recv_buf);
            wrefresh(chat_win);
        } else if (n == 0) {
            wprintw(chat_win, "서버와 연결 종료됨\n");
            wrefresh(chat_win);
            break;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
        }

        werase(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
    }

    cleanup_and_exit(0);
    return 0;
}

