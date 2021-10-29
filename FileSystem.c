#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#define BLOCK_SIZE 1024
#define INODE_SIZE 64
#define FREE_ARRAY_SIZE 251

typedef struct {
    int isize;
    int fsize;
    int unsigned nfree;
    unsigned int free[251]; 
    char flock;
    char ilock;
    char fmod;
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

typedef struct { 
    unsigned int inode; 
    char filename[28];
} dir_type;  //32 Bytes long

superblock_type superblock;
int fd;
int icount = 1;
int no_of_blocks;

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

void blockWriter(int blockNumber, void * buffer, int size){

    lseek(fd, blockNumber*BLOCK_SIZE, SEEK_SET);
    write(fd, buffer, size);
    
}

void blockWriter_withOffset(int blockNumber, int offset, void * buffer, int size){
    lseek(fd, blockNumber*BLOCK_SIZE + offset, SEEK_SET);
    write(fd, buffer, size);
}

void allocate_blocks(int blocknumber){
    if(superblock.nfree == FREE_ARRAY_SIZE){
        // write to block
        blockWriter(blocknumber, &superblock.nfree, sizeof(superblock.nfree));
        blockWriter_withOffset(blocknumber, 2, &superblock.free, sizeof(superblock.free));
        superblock.nfree = 0;
    }

    superblock.free[superblock.nfree] = blocknumber;
    superblock.nfree++;
}

void getFreeDataBlock(){

    superblock.nfree--;
    if(superblock.nfree == 0){
        int blocknumber = superblock.free[0];
        lseek(fd, blocknumber*BLOCK_SIZE, SEEK_SET);
        read(fd, superblock.nfree, sizeof(superblock.nfree));

        lseek(fd, blocknumber*BLOCK_SIZE + sizeof(superblock.nfree), SEEK_SET);
        read(fd, superblock.free, sizeof(superblock.free));
        return blocknumber;
    }
    return superblock.free[superblock.nfree];
}

int getInode(){
    icount++;
    if(icount > no_of_blocks - 1){
        return -1;
    }
    return icount;
}

void initfs(int n1, int n2){

    char fillerBlock[BLOCK_SIZE] = {0};
    fd = open("foo.txt", O_RDWR | O_CREAT, 0600);
    // printf("%d \n", fd);

    blockWriter(n1 - 1, fillerBlock, BLOCK_SIZE);
    superblock.nfree = 5;
    blockWriter(1, &superblock, sizeof(superblock));

    superblock.isize = n2;

    superblock.nfree = 0;
    for(int blocknumber = 2 + superblock.isize; blocknumber < n1; blocknumber++){
        // allocate blocks
    }



    // if((INODE_SIZE*n2)%BLOCK_SIZE)
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE;
    // else   
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE + 1;



}

int main(){
    int n1 = 10;
    int n2 = 3;
    superblock_type temp;

    // temp.nfree = 5;
    // printf("%lu\n", sizeof(temp.nfree));

    temp.nfree = 6;
    for(int i = 0; i < temp.nfree; i++){
        temp.free[i] = i;
    }

    // printf("%d \n", temp.free[1]);

    fd = open("foo.txt", O_RDWR | O_CREAT, 0600);


    int x1, x2;
    x1 = 1000;
    blockWriter(5, &temp.nfree, 2);
    // blockWriter(5, &x1, sizeof(x1));
    blockWriter_withOffset(5, 2, &temp.free, 2*FREE_ARRAY_SIZE);

    superblock_type temp2;


    lseek(fd, BLOCK_SIZE*5, SEEK_SET);
    // read(fd, &x2, sizeof(x2));
    // printf("%d \n", x2);

    read(fd, &temp2.nfree, 2);
    printf("%d\n", temp2.nfree);

    lseek(fd, BLOCK_SIZE*5 + 2, SEEK_SET);
    read(fd, &temp2.free, 2*FREE_ARRAY_SIZE);

    printf("%d \n", temp2.free[2]);


    // return 0;

    // initfs(n1, n2);

    // open_fs("foo.txt");
    // lseek(fd, BLOCK_SIZE, SEEK_SET);
    // read(fd, &temp, sizeof(temp));

    // printf("nfree value is %d \n", temp.nfree);

}

