/*
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

static int g_var = 1;
void *inc_function(void *);
void *dec_function(void *);

int main(int argc, char **argv) {
    pthread_t ptInc, ptDec;
    pthread_create(&ptInc, NULL, inc_function, NULL);
    pthread_create(&ptDec, NULL, dec_function, NULL);
    
    pthread_join(ptInc, NULL);
    pthread_join(ptDec, NULL);
    return 0;
}

void *inc_function(void * arg) {
    printf("Inc : %d < Before\n", g_var);
    g_var++;
    printf("Inc : %d > After\n", g_var);

    return NULL;
}

void *dec_function(void * arg) {
    printf("Dec : %d < Before\n", g_var);
    g_var--;
    printf("Dec : %d > After\n", g_var);

    return NULL;
}
*/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

static int g_var = 1;

void *inc_function(void *);
void *dec_function(void *);

int main(int argc, char **argv) {
    pthread_t ptInc, ptDec;

    pthread_create(&ptInc, NULL, inc_function, NULL);
    pthread_create(&ptDec, NULL, dec_function, NULL);

    pthread_join(ptInc, NULL);
    pthread_join(ptDec, NULL);

    return 0;
}

void *inc_function(void * arg) {
    printf("Inc : %d < Before\n", g_var);

    // 약간의 지연을 줘서 dec_function이 중간에 실행될 기회를 줌
    usleep(100000);  // 100ms

    g_var++;
    printf("Inc : %d > After\n", g_var);

    return NULL;
}

void *dec_function(void * arg) {
    printf("Dec : %d < Before\n", g_var);
    usleep(200000);  // 200ms
    g_var--;
    printf("Dec : %d > After\n", g_var);

    return NULL;
}
