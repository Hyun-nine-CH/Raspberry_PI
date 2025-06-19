#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *str = "The quick brown fox jumps over the lazy dog and a mischievous cat chases a tiny mouse under the old, creaky wooden bridge, enjoying the warm, sunny afternoon by the calm, flowing river.";

    const char *tok = (argc<2)?"ous":argv[1];

    printf(strstr(str, tok)?"O\n":"X\n");

    return 0;
}
