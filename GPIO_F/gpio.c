#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>   // write(), read(), close()

int main(int argc, char** argv)
{
    char buf[BUFSIZ];
    int fd = -1;
    memset(buf, 0, BUFSIZ);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return 1;
    }

    printf("GPIO Set : %s\n", argv[1]);

    fd = open("/dev/gpioled", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // 커널 모듈에 데이터 쓰기
    if (write(fd, argv[1], strlen(argv[1])) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    // 커널 모듈로부터 데이터 읽기
    if (read(fd, buf, sizeof(buf) - 1) < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("Read data : %s\n", buf);
    close(fd);
    return 0;
}

