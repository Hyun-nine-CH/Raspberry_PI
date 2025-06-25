#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define PORT 9999
#define MAX_CLIENTS 5
//#define BUFSIZ 1024
#define MAX_ROOM_NAME 32
#define MAX_NICKNAME 32
#define MAX_ROOMS 10
#define MAX_MSG 1024

typedef struct {
    int sock;
    pid_t pid;
    char nickname[MAX_NICKNAME];
    char room[MAX_ROOM_NAME];
    int active;
} Client;

typedef struct {
    char name[MAX_ROOM_NAME];
} ChatRoom;

Client clients[MAX_CLIENTS];
int client_pipes[MAX_CLIENTS][2];
int client_count = 0;
ChatRoom rooms[MAX_ROOMS];
int room_count = 0;
int ssock;

void daemonServer() {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    setsid();
    chdir("/");
    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
}

void broadcast_to_room(const char *room, const char *msg, int sender_idx) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].active && strcmp(clients[i].room, room) == 0 && i != sender_idx) {
            write(client_pipes[i][1], msg, strlen(msg));
        }
    }
}

void handle_sigchld(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        for (int i = 0; i < client_count; ++i) {
            if (clients[i].pid == pid) {
                clients[i].active = 0;
                break;
            }
        }
    }
}

void handle_command(int idx, const char *msg) {
    char command[16], arg1[64], arg2[BUFSIZ];
    memset(command, 0, sizeof(command));
    memset(arg1, 0, sizeof(arg1));
    memset(arg2, 0, sizeof(arg2));
    int parsed = sscanf(msg, "%s %s %[^\n]", command, arg1, arg2);
    if (parsed < 2) {
        write(client_pipes[idx][1], "명령어 파싱 error\n", strlen("명령어 파싱 error\n"));
        return;
    }

    if (strcmp(command, "/add") == 0) {
        if (room_count >= MAX_ROOMS) {
            write(client_pipes[idx][1], "방 개수 초과\n", strlen("방 개수 초과\n"));
            return;
        }
        strncpy(rooms[room_count++].name, arg1, MAX_ROOM_NAME);
        rooms[room_count - 1].name[MAX_ROOM_NAME - 1] = '\0';
        write(client_pipes[idx][1], "방 생성 완료\n", strlen("방 생성 완료\n"));

    } else if (strcmp(command, "/join") == 0) {
        for (int i = 0; i < room_count; ++i) {
            if (strcmp(rooms[i].name, arg1) == 0) {
                strncpy(clients[idx].room, arg1, MAX_ROOM_NAME);
                clients[idx].room[MAX_ROOM_NAME - 1] = '\0';
                write(client_pipes[idx][1], "방 입장 완료\n", strlen("방 입장 완료\n"));
                return;
            }
        }
        write(client_pipes[idx][1], "해당 방 없음\n", strlen("해당 방 없음\n"));

    } else if (strcmp(command, "/leave") == 0) {
        clients[idx].room[0] = '\0';
        write(client_pipes[idx][1], "방 나감\n", strlen("방 나감\n"));

    } else if (strcmp(command, "/list") == 0) {
        char list[MAX_MSG] = "방 목록:\n";
        for (int i = 0; i < room_count; ++i) {
            strcat(list, rooms[i].name);
            strcat(list, "\n");
        }
        write(client_pipes[idx][1], list, strlen(list));

    } else if (strcmp(command, "/users") == 0) {
        char list[MAX_MSG] = "현재 방 사용자:\n";
        for (int i = 0; i < client_count; ++i) {
            if (clients[i].active && strcmp(clients[i].room, clients[idx].room) == 0) {
                strcat(list, clients[i].nickname[0] ? clients[i].nickname : "익명");
                strcat(list, "\n");
            }
        }
        write(client_pipes[idx][1], list, strlen(list));

    } else if (strcmp(command, "/nick") == 0) {
        strncpy(clients[idx].nickname, arg1, MAX_NICKNAME);
        clients[idx].nickname[MAX_NICKNAME - 1] = '\0';
        write(client_pipes[idx][1], "닉네임 설정됨\n", strlen("닉네임 설정됨\n"));

    } else if (strcmp(command, "!whisper") == 0) {
        for (int i = 0; i < client_count; ++i) {
            if (strcmp(clients[i].nickname, arg1) == 0 && clients[i].active) {
                write(client_pipes[i][1], arg2, strlen(arg2));
                return;
            }
        }
        write(client_pipes[idx][1], "대상 닉네임 없음\n", strlen("대상 닉네임 없음\n"));

    } else {
        write(client_pipes[idx][1], "알 수 없는 명령어입니다.\n", strlen("알 수 없는 명령어입니다.\n"));
    }
}

void handle_sigusr1(int sig) {
    char buf[BUFSIZ];
    for (int i = 0; i < client_count; ++i) {
        int n = read(client_pipes[i][0], buf, BUFSIZ);
        if (n > 0) {
            buf[n] = '\0';
            if (buf[0] == '/') {
                handle_command(i, buf);
            } else {
                char out[BUFSIZ];
                snprintf(out, sizeof(out), "%s: %s", clients[i].nickname[0] ? clients[i].nickname : "익명", buf);
                broadcast_to_room(clients[i].room, out, i);
            }
        }
    }
}

void handle_sigusr2(int sig) {
    syslog(LOG_INFO, "자식 프로세스 종료 (PID: %d)", getpid());
    exit(0);
}

void graceful_shutdown(int sig) {
    syslog(LOG_INFO, "서버 종료 전달");
    for (int i = 0; i < client_count; ++i) {
        kill(clients[i].pid, SIGUSR2);
    }
    close(ssock);
    closelog();
    exit(0);
}

int main() {
    daemonServer();

    signal(SIGCHLD, handle_sigchld);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGINT, graceful_shutdown);
    signal(SIGTERM, graceful_shutdown);

    openlog("Server", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "채팅 서버 시작");

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if (ssock < 0) {
        syslog(LOG_ERR, "소켓 생성 실패: %s", strerror(errno));
        exit(1);
    }

    int opt = 1;
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(ssock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        syslog(LOG_ERR, "bind 실패: %s", strerror(errno));
        exit(1);
    }

    listen(ssock, MAX_CLIENTS);
    syslog(LOG_INFO, "Port %d에서 클라이언트 연결 대기 중...", PORT);

    while (1) {
        int csock = accept(ssock, (struct sockaddr*)&cli_addr, &cli_len);
        if (client_count >= MAX_CLIENTS) {
            syslog(LOG_WARNING, "클라이언트 최대 연결 수 초과");
            close(csock);
            sleep(1);
            continue;
        }

        if (csock < 0) {
            syslog(LOG_WARNING, "accept 실패: %s", strerror(errno));
            continue;
        }

        if (pipe2(client_pipes[client_count], O_NONBLOCK) < 0) {
            syslog(LOG_ERR, "pipe 생성 실패: %s", strerror(errno));
            close(csock);
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
        syslog(LOG_INFO, "클라이언트 접속: %s:%d", ip, ntohs(cli_addr.sin_port));

        int my_idx = client_count;
        pid_t pid = fork();
        if (pid == 0) {
            signal(SIGUSR2, handle_sigusr2);
            close(ssock);
            close(client_pipes[my_idx][0]);

            char buf[BUFSIZ];
            while (1) {
                memset(buf, 0, BUFSIZ);
                int n = read(csock, buf, BUFSIZ);
                if (n <= 0) break;

                write(client_pipes[my_idx][1], buf, n);
                kill(getppid(), SIGUSR1);

                int m = read(client_pipes[my_idx][0], buf, BUFSIZ);
                if (m > 0) write(csock, buf, m);
            }

            close(client_pipes[my_idx][1]);
            close(csock);
            exit(0);
        } else {
            close(client_pipes[my_idx][1]);
            close(csock);
            clients[my_idx].sock = csock;
            clients[my_idx].active = 1;
            clients[my_idx].room[0] = '\0';
            clients[my_idx].nickname[0] = '\0';
            clients[my_idx].pid = pid;
            syslog(LOG_INFO, "클라이언트 접속: PID=%d", pid);
            client_count++;
        }
    }

    close(ssock);
    closelog();
    return 0;
}
