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
#define MAX_ROOM_NAME 32
#define MAX_NICKNAME 32

int sock;
WINDOW *chat_win, *input_win;

void cleanup_and_exit(int sig) {
    endwin();
    close(sock);
    printf("\n클라이언트 종료.\n");
    exit(0);
}

void setup_windows() {
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
}

void handle_input_output() {
    char send_buf[BUFSIZ], recv_buf[BUFSIZ];

    while (1) {
        memset(send_buf, 0, BUFSIZ);
        mvwgetnstr(input_win, 1, 1, send_buf, BUFSIZ - 1);

        // 입력값 끝에 개행 추가해서 서버가 개별 명령으로 인식하도록 처리
        strncat(send_buf, "\n", BUFSIZ - strlen(send_buf) - 1);
        write(sock, send_buf, strlen(send_buf));

        memset(recv_buf, 0, BUFSIZ);
        int n = read(sock, recv_buf, BUFSIZ - 1);
        if (n > 0) {
            recv_buf[n] = '\0';
            wprintw(chat_win, "%s", recv_buf);
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
}

int main() {
    struct sockaddr_in serv_addr;

    signal(SIGINT, cleanup_and_exit);
    signal(SIGUSR2, cleanup_and_exit);  // 서버 종료 시 우아하게 종료

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
        }
    }

    setup_windows();
    handle_input_output();
    cleanup_and_exit(0);
    return 0;
}
