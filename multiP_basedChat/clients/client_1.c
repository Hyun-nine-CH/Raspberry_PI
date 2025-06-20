#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define PORT 9999
// #define BUFSIZ 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char send_buf[BUFSIZ], recv_buf[BUFSIZ];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // 로컬 테스트용

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    initscr();
    cbreak();
    noecho();
    WINDOW *chat_win = newwin(20, 80, 0, 0);
    WINDOW *input_win = newwin(3, 80, 20, 0);
    scrollok(chat_win, TRUE);
    box(chat_win, 0, 0);
    box(input_win, 0, 0);
    wrefresh(chat_win);
    wrefresh(input_win);

    while (1) {
        memset(send_buf, 0, BUFSIZ);
        mvwgetnstr(input_win, 1, 1, send_buf, BUFSIZ - 1);
        write(sock, send_buf, strlen(send_buf));

        memset(recv_buf, 0, BUFSIZ);
        read(sock, recv_buf, BUFSIZ);
        wprintw(chat_win, "Server: %s\n", recv_buf);
        wrefresh(chat_win);
        werase(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
    }

    endwin();
    close(sock);
    return 0;
}

