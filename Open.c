#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
extern int errno;

int main(){
        int wr;
        int fd = open("foo.txt", O_RDWR | O_CREAT, 777);
        printf("fd = %d \n", fd);

        wr = write(fd, "OS project is hard\n", strlen("OS project is hard\n"));

        close(fd);
}