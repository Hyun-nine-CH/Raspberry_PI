#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
//	close(0);
//	int n;
	char str[BUFSIZ];
	int fd1, fd2;
	int n = read(0, str, BUFSIZ);
	fcntl(0, F_SETFL, O_NONBLOCK);
	write(1, str, n);
//	scanf("%d", &n);
//	printf("Hello, world(%s)\n", str);
	fd1 = open("sample1.txt", O_WRONLY | O_CREAT);
	fd2 = open("sample2.txt", O_WRONLY | O_CREAT);
	printf("%d %d\n", fd1, fd2);
	close(fd1);
	close(fd2);
	return 0;
}
