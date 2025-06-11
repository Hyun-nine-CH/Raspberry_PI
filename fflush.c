#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Hello, "); 
    printf("world!");
//    fflush(stdout);
    fflush(NULL);
    _exit(1);
}
