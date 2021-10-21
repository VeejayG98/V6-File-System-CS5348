#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#define BLOCK_SIZE 1024
#define INODE_SIZE 64

typedef struct {
    int isize;
    int fsize;
    int nfree;
    unsigned int free[254];
    int flock;
    int ilock;
    unsigned int fmod;
    unsigned int time;
} superblock_type;

typedef struct {
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0;
    unsigned int size1;
    unsigned int addr[9];
    unsigned int actime;
unsigned int modtime;
} inode_type;

superblock_type superblock;

int open_fs(char *file_name){
    int fd = open(file_name, O_RDWR, 0600);

    if(fd == -1){
        //call initfs
    }
    else{
        
        lseek(fd, BLOCK_SIZE, SEEK_SET);
        read(fd, &superblock, sizeof(superblock));

        //Indicate success in opening/creating the filesystem
        return 1;

    }
}

