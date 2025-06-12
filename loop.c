#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sigHd1(int no) {
    printf("%d\n", no);
}

int main(int argc, char **argv) {
    int i;
    signal(SIGINT, sigHd1);
    for(i=0; ; i++) {
        printf("%10d\r", i);
        fflush(NULL);
        sleep(1);
    }
    return 0;
}
