#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

int listDir(char *arg) {
    DIR *pdir;
    struct dirent *dirt;
    struct stat statBuf;
    struct passwd *username;
    struct group *groupname;
    struct tm *t;
    int i = 0, count = 0;
    char *dirname[255], buf[255], permission[11], mtime[20];

    memset(dirName, 0, sizeof(dirName));
    memset(&dirt, 0, sizeof(dirt));
    memset(&statBuf, 0, sizeof(statBuf));

    if((pdir = opendir(arg)) <= 0) {
            perror("opendir");
            return -1;
}

chdir(arg);
getcwd(buf, 255);
printf("\n%s: Directory\n", arg);

while((dirt = readdir(pdir)) != NULL) {
    lstat(dirt->d_name, &statBuf);
    if(S_ISDIR(statBuf.st_mode))
            permission[0]='d';
    else if(S_ISLNK(statBuf.st_mode))
            permission[0]='l';
    else if(S_ISCHR(statBuf.st_mode))
            permission[0]='c';
    else if(S_ISCHR(statBuf.st_mode))
            permission[0]='b';
    else if(S_ISCHR(statBuf.st_mode))
            permission[0]='s';
    else if(S_ISCHR(statBuf.st_mode))
            permission[0]='p';
    else
            permission[0]='-';

    permission[1] = statBuf.st_mode&S_IRUSR? 'r' : '-';
    permission[2] = statBuf.st_mode&S_IWUSR? 'w' : '-';
    permission[3] = statBuf.st_mode&S_IXUSR? 'x' :
                    statBuf.st_mode&S_ISUID? 'S' : '-';

    permission[4] = statBuf.st_mode&S_IRGRP? 'r' : '-';
    permission[5] = statBuf.st_mode&S_IWGRP? 'w' : '-';
    permission[6] = statBuf.st_mode&S_IXGRP? 'x' :
                    statBuf.st_mode&S_ISGID? 'S' : '-';

    permission[7] = statBuf.st_mode&S_IROTH? 'r' : '-';
    permission[8] = statBuf.st_mode&S_IWOTH? 'w' : '-';
    permission[9] = statBuf.st_mode&S_IXOTH? 'x' : '-';
    
    if(statBuf.st_mode & S_IXOTH) {
        permission[9] = statBuf.st_mode&S_ISVTX? 't' : 'x';
       }else{
            permission[9]= statBuf.st_mode&S_ISVTX? 'T' : '-';
       }
       permission[10]='\0';
       if(S_ISDIR(statBuf.st_mode)==1) {
            if(
       }
}
