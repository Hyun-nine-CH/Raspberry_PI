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
#define BUFSIZ 1024

int sock;
WINDOW *chat_win, *input_win;

void cleanup_and_exit(int sig) {
    endwin();
    close(sock);
    printf("\n클라이언트 종료.\n");
    exit(0);
}

void process_command(const char *input) {
    if(strncmp(input, "/add", 5)==0) {
        write(sock, input, strlen(input));
    }else if(strncmp(input, "/join", 6)==0) {
        write(sock, input, strlen(input));
    }else if(strncmp(input, "/leave", 6)==0) {
        write(sock, input, strlen(input));
    }else if(strncmp(input, "/list", 5)==0) {
        write(sock, input, strlen(input));
    }else if(strncmp(input, "/users", 6)==0) {
        write(sock, input, strlen(input));
    }else if(strncmp(input, "!whisper", 9)==0) {
        write(sock, input, strlen(input));
    }else{
        write(sock, input, strlen(input));
}

int main() {
    struct sockaddr_in serv_addr;
    char send_buf[BUFSIZ], recv_buf[BUFSIZ];

    signal(SIGINT, cleanup_and_exit);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0) {
        perror("socket");
        exit(1);
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // 로컬 테스트용

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        if(errno != EINPROGRESS) {
            perror("connect");
            exit(1);
        }
    }

    echo();
    curs_set(1);
    initscr();
    cbreak();

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
//        write(sock, send_buf, strlen(send_buf));

        memset(recv_buf, 0, BUFSIZ);
        int n=read(sock, recv_buf, BUFSIZ);
        if(n>0) {
            recv_buf[n]='\n';
            wprintw(chat_win, "Server: %s\n", recv_buf);
            wrefresh(chat_win);
        }

        werase(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
    }
    cleanup_and_exit(0);
    return 0;
}
