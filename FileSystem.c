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
inode_type root;

int fd;
int icount = 1;
int no_of_blocks;
int no_of_inodes;

int open_fs(char *file_name){
    int fd = open(file_name, O_RDWR, 0600);

    if(fd == -1){
        //call initfs
        return 1;
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

void inode_writer(int iNumber, inode_type inode){
    int blocknumber = 2 + (INODE_SIZE*iNumber/BLOCK_SIZE);
    int offset = ((INODE_SIZE*iNumber)%BLOCK_SIZE) - 64;

    blockWriter_withOffset(blocknumber, offset, &inode, sizeof(inode));
}

inode_type inode_reader(int iNumber, inode_type inode){
    int blocknumber = 2 + (INODE_SIZE*iNumber/BLOCK_SIZE);
    int offset = ((INODE_SIZE*iNumber)%BLOCK_SIZE) - 64;

    lseek(fd, blocknumber*BLOCK_SIZE + offset, SEEK_SET);
    read(fd, &inode, sizeof(inode));

    return inode;
}

void initialize_inodes(int iNumber){
    inode_type inode;
    inode.flags = 0;
    inode_writer(iNumber, inode);
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

int getFreeDataBlock(){

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

    unsigned short int compare_flag = 1 << 15;
    inode_type inode;

    for(int i = 2; i <= no_of_inodes; i++){

        inode = inode_reader(i, inode);

        if(!(inode.flags & compare_flag)){

            inode.flags |= 1 << 15;
            inode_writer(i, inode);
            return i;
        
        }
    }
    return -1;
        
}

void create_root(){
    int block_number = getFreeDataBlock();
    dir_type d[32];

    strcpy(d[0].filename, ".");
    d[0].inode = 1;

    strcpy(d[1].filename, "..");
    d[1].inode = 1;

    printf("The . directory is : \n");
    printf("%s \n", d[0].filename);
    printf("%d \n", sizeof(d));

    blockWriter(block_number, d, sizeof(d));
    // blockWriter(block_number, d[0].filename, sizeof(d[0].filename));

    // printf("The root directory is at block number %d \n", block_number);

    root.addr[0] = block_number;

    root.flags |= 1 << 15;

    inode_writer(1, root);

    // root.flags;

}

void initfs(int n1, int n2){
    // n1 is the file system size in number of blocks and n2 is the number of blocks devoted to the i-nodes.

    char fillerBlock[BLOCK_SIZE] = {0};
    fd = open("foo.txt", O_RDWR | O_CREAT, 0600);
    // printf("%d \n", fd);

    blockWriter(n1 - 1, fillerBlock, BLOCK_SIZE);
    superblock.nfree = 5;
    blockWriter(1, &superblock, sizeof(superblock));

    superblock.isize = n2;

    no_of_inodes = BLOCK_SIZE*n2/INODE_SIZE;

    for(int i = 2; i <= no_of_inodes; i++){
        // Initialize the inodes and write to file system
        initialize_inodes(i);
    }

    superblock.nfree = 0;
    for(int blocknumber = 2 + superblock.isize; blocknumber < n1; blocknumber++){
        allocate_blocks(blocknumber);
    }

    create_root();




    // if((INODE_SIZE*n2)%BLOCK_SIZE)
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE;
    // else   
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE + 1;

}

int main(){
    int n1 = 10;
    int n2 = 2;
    inode_type temp_root;

    initfs(n1, n2);

    lseek(fd, 2 * BLOCK_SIZE, SEEK_SET);
    read(fd, &temp_root, sizeof(temp_root));

    int a = 1 << 15;

    if(temp_root.flags & a){
    printf("The inode is allocated! \n");
    }

    else{
        printf("The inode is unallocated! \n");
    }

    printf("The number of inodes is: %d \n", no_of_inodes);

    for(int i = 1; i <= 31; i++){
        printf("Inode %d is allocated \n", getInode());
    }

    printf("Inode %d is allocated \n", getInode());

    // inode_type test;
    // test = inode_reader(1, test);
    // printf("Test flag: %d", test.flags);
    // getInode();
    // getInode();

    // printf("The root's addr[0] is %d \n", temp_root.addr[0]);



    // dir_type temp_dir[32];

    // lseek(fd, temp_root.addr[0]*BLOCK_SIZE, SEEK_SET);
    // read(fd, temp_dir, sizeof(temp_dir));

    // printf("%s \n", temp_dir[0].filename);
    










    // superblock_type temp;

    // // temp.nfree = 5;
    // // printf("%lu\n", sizeof(temp.nfree));

    // temp.nfree = 6;
    // for(int i = 0; i < temp.nfree; i++){
    //     temp.free[i] = i*i;
    // }

    // // printf("%d \n", temp.free[1]);

    // fd = open("foo.txt", O_RDWR | O_CREAT, 0600);


    // int x1, x2;
    // x1 = 1000;
    // blockWriter(5, &temp.nfree, 2);
    // // blockWriter(5, &x1, sizeof(x1));
    // blockWriter_withOffset(5, 2, &temp.free, 2*FREE_ARRAY_SIZE);

    // superblock_type temp2;


    // lseek(fd, BLOCK_SIZE*5, SEEK_SET);
    // // read(fd, &x2, sizeof(x2));
    // // printf("%d \n", x2);

    // read(fd, &temp2.nfree, 2);
    // printf("%d\n", temp2.nfree);

    // lseek(fd, BLOCK_SIZE*5 + 2, SEEK_SET);
    // read(fd, &temp2.free, 2*FREE_ARRAY_SIZE);

    // printf("%d \n", temp2.free[5]);


    // return 0;

    // initfs(n1, n2);

    // open_fs("foo.txt");
    // lseek(fd, BLOCK_SIZE, SEEK_SET);
    // read(fd, &temp, sizeof(temp));

    // printf("nfree value is %d \n", temp.nfree);

}

