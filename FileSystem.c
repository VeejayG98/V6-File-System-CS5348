#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>

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
int currDir_iNumber;

int open_fs(char *file_name){
    fd = open(file_name, O_RDWR | O_CREAT, 0600);

    if(fd == -1){
        //call initfs
        return -1;
    }

    // This part of code is commented out for Part 2

    // else{
        
    //     lseek(fd, BLOCK_SIZE, SEEK_SET);
    //     read(fd, &superblock, sizeof(superblock));

    //     //Indicate success in opening/creating the filesystem
    //     return 1;

    // }
}

// Function to write to a block
void blockWriter(int blockNumber, void *buffer, int size){

    lseek(fd, blockNumber*BLOCK_SIZE, SEEK_SET);
    write(fd, buffer, size);
    
}

int blockReader(int blockNumber, void *buffer, int size){
    lseek(fd, blockNumber*BLOCK_SIZE, SEEK_SET);
    int bytesRead = read(fd, buffer, size);
    return bytesRead;
}

void blockReader_withOffset(int blockNumber, int offset, void *buffer, int size){
    lseek(fd, blockNumber*BLOCK_SIZE + offset, SEEK_SET);
    read(fd, buffer, size);
}

// Function to write to a block provided offset
void blockWriter_withOffset(int blockNumber, int offset, void *buffer, int size){
    lseek(fd, blockNumber*BLOCK_SIZE + offset, SEEK_SET);
    write(fd, buffer, size);
}

// Function to write inode
void inode_writer(int iNumber, inode_type inode){
    int blocknumber = 2 + (INODE_SIZE*iNumber/BLOCK_SIZE);
    int offset = ((INODE_SIZE*iNumber)%BLOCK_SIZE) - 64;

    blockWriter_withOffset(blocknumber, offset, &inode, sizeof(inode));
}

// Function to read inodes
inode_type inode_reader(int iNumber, inode_type inode){
    int blocknumber = 2 + (INODE_SIZE*iNumber/BLOCK_SIZE);
    int offset = ((INODE_SIZE*iNumber)%BLOCK_SIZE) - 64;

    lseek(fd, blocknumber*BLOCK_SIZE + offset, SEEK_SET);
    read(fd, &inode, sizeof(inode));

    return inode;
}

// Function to initialize inodes
void initialize_inodes(int iNumber){
    inode_type inode;
    inode.flags = 0;
    inode_writer(iNumber, inode);
}

// Function to chain the data blocks
void allocate_blocks(int blocknumber){
    if(superblock.nfree == FREE_ARRAY_SIZE){
        // write to block
        blockWriter(blocknumber, &superblock.nfree, sizeof(superblock.nfree));
        blockWriter_withOffset(blocknumber, sizeof(superblock.nfree), &superblock.free, sizeof(superblock.free));
        superblock.nfree = 0;
    }

    superblock.free[superblock.nfree] = blocknumber;
    superblock.nfree++;
}

// Function to grab a free data block
int getFreeDataBlock(){

    superblock.nfree--;
    if(superblock.free[superblock.nfree] == 0){
        printf("No Free Data blocks left to allocate! \n");
        superblock.nfree++;
        return -1;
    }

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

// Function to grab a free inode
int getFreeInode(){

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

// Function to create the root of the file system
void create_root(){
    int block_number = getFreeDataBlock();
    printf("Block Number %d is allocated \n", block_number);
    dir_type d[32];

    for(int i = 0; i < 32; i++){
        strcpy(d[i].filename, "");
    }

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
    root.nlinks = 1;
    
    root.flags |= 1 << 15; //Root is allocated
    root.flags |= 1 <<14; //It is a directory
    root.actime = time(NULL);
    root.modtime = time(NULL);
    
    root.gid = 0;
    root.uid = 0;

    root.size0 = 0;
    root.size1 = 2 * sizeof(dir_type);
    
    inode_writer(1, root);

    // root.flags;

}

// Function to initialize the file system
void initfs(int n1, int n2){
    // n1 is the file system size in number of blocks and n2 is the number of blocks devoted to the i-nodes.

    char fillerBlock[BLOCK_SIZE] = {0};

    // fd = open("foo.txt", O_RDWR | O_CREAT, 0600);
    // printf("%d \n", fd);

    blockWriter(n1 - 1, fillerBlock, BLOCK_SIZE); //Writing to last block.

    // superblock.nfree = 5;
    // blockWriter(1, &superblock, sizeof(superblock));

    superblock.isize = n2;

    no_of_inodes = BLOCK_SIZE*n2/INODE_SIZE;

    for(int i = 2; i <= no_of_inodes; i++){
        // Initialize the inodes and write to file system
        initialize_inodes(i);
    }

    superblock.nfree = 0;
    superblock.free[superblock.nfree] = 0;
    superblock.nfree++;
    for(int blocknumber = 2 + superblock.isize; blocknumber < n1; blocknumber++){
        allocate_blocks(blocknumber);
    }

    create_root();

    currDir_iNumber = 1;
    
    superblock.flock = 'f';
    superblock.ilock = 'i';
    superblock.fmod = 'm';
    superblock.time = time(NULL);

    blockWriter(1, &superblock, sizeof(superblock));

    // if((INODE_SIZE*n2)%BLOCK_SIZE)
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE;
    // else   
    //     superblock.isize = (INODE_SIZE*n2)/BLOCK_SIZE + 1;

}

void mkdir(char *name){

    int iNumber;
    // printf("The current directory is %d \n", currDir_iNumber);
    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    int currDirBlockNumber = currDir_inode.addr[0];

    dir_type currDir[32];

    lseek(fd, currDir_inode.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &currDir, sizeof(currDir));

    for(int i = 0; i < 32; i++){

        if(strcmp(currDir[i].filename, name) == 0){
            printf("Folder already exists! \n");
            return;
        }

        if(strlen(currDir[i].filename) == 0){
            iNumber = getFreeInode();

            if(iNumber == -1){
                printf("Storage space full! No free inodes are available!");
                return;
            }

            strcpy(currDir[i].filename, name);
            currDir[i].inode = iNumber;
            break;
        }
    }

    int blockNumber = getFreeDataBlock();

    if(blockNumber == -1){
        printf("Storage space full! No free Data blocks are available!");
        return;
    }
    
    dir_type d[32];

    for(int i = 0; i < 32; i++){
        strcpy(d[i].filename, "");
    }

    d[0].inode = iNumber;
    strcpy(d[0].filename, ".");
    
    d[1].inode = currDir_iNumber;
    strcpy(d[1].filename, "..");

    blockWriter(blockNumber, d, sizeof(d));

    // printf("The inode allocated is %d \n", iNumber);
    inode_type inode = inode_reader(iNumber, inode);
    inode.nlinks = 1;
    // printf("Before setting flag: %d \n", inode.flags);
    inode.flags |= 1 <<14; //It is a directory
    // printf("After setting flag: %d \n", inode.flags);

    inode.actime = time(NULL);
    inode.modtime = time(NULL);

    inode.gid = 0;
    inode.uid = 0;

    inode.size0 = 0;
    inode.size1 = 2 * sizeof(dir_type);
    inode.addr[0] = blockNumber;

    inode_writer(iNumber, inode);

    currDir_inode.size1 += sizeof(dir_type);
    inode_writer(currDir_iNumber, currDir_inode);
    blockWriter(currDirBlockNumber, currDir, sizeof(currDir));    

    // inode_type new_inode = inode_reader(iNumber, new_inode);
    // printf("After writing, flags: %d \n", new_inode.flags);

    // printf("The inode allocated is %d \n", iNumber);
    // inode_type inode = inode_reader(iNumber, inode);
    // inode.nlinks = 1;
    // printf("Before setting flag: %d \n", inode.flags);
    // inode.flags |= 1 <<14; //It is a directory
    // printf("After setting flag: %d \n", inode.flags);


    // inode_writer(iNumber, inode);

    

}

void cd(char *name){

    // printf("The folder to find is %s \n", name);
    unsigned short int compare_flag = 1 << 14; //To check if the file is a directory or not
    inode_type inode;

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    int currDirBlockNumber = currDir_inode.addr[0];

    dir_type currDir[32];

    lseek(fd, currDir_inode.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &currDir, sizeof(currDir));

    int new_iNumber = 0;
    for(int i = 0; i < 32; i++){
        // printf("%s \n", currDir[i].filename);
        // printf("The iNumber of the file is %d \n", currDir[i].inode);
        if(strcmp(currDir[i].filename, name) == 0){
            // printf("Yes! \n");

            inode = inode_reader(currDir[i].inode, inode);

            if(inode.flags & compare_flag){
                // printf("Another yes! \n");
                new_iNumber = currDir[i].inode;
                break;
            }
        }
    }

    if(new_iNumber == 0){
        printf("Folder not found! \n");
        return;
    }
    
    inode.actime = time(NULL);
    inode_writer(new_iNumber, inode);
    currDir_iNumber = new_iNumber;
    // printf("The new dir inode number is %d \n", currDir_iNumber);

}

void cpin(char *source_path, char *filename){
    int source, blockNumber, iNumber, bytesRead;

    if((source = open(source_path, O_RDWR)) == -1){
        printf("File does not exist! \n");
        return;
    }

    if((iNumber = getFreeInode()) == -1){
        printf("Storage space full! No free inodes are available!");
        return;
    }

    

    inode_type new_file = inode_reader(iNumber, new_file);

    new_file.nlinks = 1;
    new_file.uid = 0;
    new_file.gid = 0;

    new_file.actime = time(NULL);
    new_file.modtime = time(NULL);
    new_file.size1 = 0;
    
    char tempBuffer[BLOCK_SIZE];
    int i = 0;
    // unsigned int size = 0;

    do
    {
        bytesRead = read(source, tempBuffer, BLOCK_SIZE);
        printf("%s \n", tempBuffer);
        new_file.size1 += bytesRead;
        
        if((blockNumber = getFreeDataBlock()) == -1){
            printf("Storage space full! No free Data blocks are available!");
            return;
        }

        new_file.addr[i] = blockNumber;
        blockWriter(blockNumber, tempBuffer, bytesRead);

    } while (bytesRead == BLOCK_SIZE);

    inode_writer(iNumber, new_file);

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    dir_type currDir[32];
    int currDirBlockNumber = currDir_inode.addr[0];

    lseek(fd, currDir_inode.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &currDir, sizeof(currDir));

    for(i = 0; i < 32; i++){
        if(strlen(currDir[i].filename) == 0){
            strcpy(currDir[i].filename, filename);
            currDir[i].inode = iNumber;
        }
    }
    currDir_inode.size1 += sizeof(dir_type);
    inode_writer(currDir_iNumber, currDir_inode);
    blockWriter(currDirBlockNumber, currDir, sizeof(currDir));  

}

void cpout(char *dest_path, char *filename){

    int dest, currDirBlockNumber, j, bytesRead;
    inode_type file;
    dest = open(dest_path, O_RDWR|O_CREAT, 0600);

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    currDirBlockNumber = currDir_inode.addr[0];

    dir_type currDir[32];
    unsigned short int compare_flag = 1 << 14;

    lseek(fd, currDir_inode.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &currDir, sizeof(currDir));

    char temp_buffer[BLOCK_SIZE];

    for(int i = 0; i < 32; i++){

        if(strcmp(currDir[i].filename, filename) == 0){
            printf("The inode of the file is %d \n", currDir[i].inode);
            file = inode_reader(currDir[i].inode, file);

            if(file.flags & compare_flag){
                printf("It is not a file!");
                return;
            }

            for(j = 0; j < file.size1/BLOCK_SIZE; j++){
                
                bytesRead = blockReader(file.addr[j], temp_buffer, BLOCK_SIZE);
                printf("%s \n", temp_buffer);
                printf("Bytes to write %d \n", bytesRead);
                write(dest, temp_buffer, bytesRead);
            }
            bytesRead = blockReader(file.addr[j], temp_buffer, file.size1 % BLOCK_SIZE);
            printf("Bytes to write %d and addr is %d \n", bytesRead, j);
            write(dest, temp_buffer, bytesRead);
            
            break;

        }
    }
}

// Function to quit the program
void quit(){
    close(fd);
    exit(0);
}

// The main function
int main(){
    printf("Root flag is %d \n", root.flags);

    int n1 = 10;
    int n2 = 2;
    inode_type temp_root;

    open_fs("foo.txt");

    initfs(n1, n2);

    // quit();

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

    // int iNumber = getFreeInode();

    // printf("The inode allocated is %d \n", iNumber);
    // inode_type inode = inode_reader(iNumber, inode);
    // inode.nlinks = 1;
    // printf("Before setting flag: %d \n", inode.flags);
    // inode.flags |= 1 <<14; //It is a directory
    // printf("After setting flag: %d \n", inode.flags);

    // inode_writer(iNumber, inode);

    // inode_type new_inode = inode_reader(iNumber, new_inode);
    // printf("After writing, flags: %d \n", new_inode.flags);

    // int b = 1 << 14;

    // if(new_inode.flags & b){
    //     printf("Success!! \n");
    // }

    mkdir("Testing");

    inode_type new_inode = inode_reader(2, new_inode);
    printf("After writing, flags: %d \n", new_inode.flags);

    dir_type temp_dir[32];
    
    lseek(fd, temp_root.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &temp_dir, sizeof(temp_dir));

    printf("%s \n", temp_dir[2].filename);
    printf("Current Dir Inode: %d \n", currDir_iNumber);

    cd("Testing");

    mkdir("Batman");
    temp_root= inode_reader(currDir_iNumber, temp_root);

    lseek(fd, temp_root.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &temp_dir, sizeof(temp_dir));

    printf("%s \n", temp_dir[2].filename);
    printf("%d \n", temp_dir[2].inode);
    printf("Current Dir Inode: %d \n", currDir_iNumber);

    cpin("transfer.txt", "transfer.txt");

    lseek(fd, temp_root.addr[0] * BLOCK_SIZE, SEEK_SET);
    read(fd, &temp_dir, sizeof(temp_dir));

    printf("%s \n", temp_dir[3].filename);
    printf("%d \n", temp_dir[3].inode);
    printf("Current Dir Inode: %d \n", currDir_iNumber);

    cpout("transfer1.txt", "transfer.txt");

    inode_type file = inode_reader(temp_dir[3].inode, file);
    printf("The size of the file is %d \n", file.size1);
    char temp[BLOCK_SIZE];
    blockReader(file.addr[0], temp, file.size1);

    printf("%s \n", temp);

    // int dest = open("transfer1.txt", O_RDWR|O_CREAT, 0600);
    // write(dest, temp, file.size1);







    // for(int i = 1; i <= 31; i++){
    //     printf("Inode %d is allocated \n", getFreeInode());
    // }

    // printf("Inode %d is allocated \n", getFreeInode());

    // printf("Block Number %d is allocated \n", getFreeDataBlock());

    // for(int i = 1; i <= n1 - n2 - 2; i++){
    //     printf("Block Number %d is allocated \n", getFreeDataBlock());
    //     // printf("Hello \n");
    // }

    // int itest = getFreeInode();
    // inode_type test = inode_reader(itest, test);

    // printf("test %d flag %d and a %d\n", itest, test.flags, a);

    // mkdir("Testing");
    // mkdir("Testing");
    // printf("%d \n", temp_root.addr[0]);


    // lseek(fd, temp_root.addr[0] * BLOCK_SIZE, SEEK_SET);
    // read(fd, &temp_dir, sizeof(temp_dir));

    // if(temp_dir[4].filename[0]){
    //     printf("Yes! \n");
    // }
    // else{
    //     printf("No! \n");
    // // }
    // printf("%s \n", temp_dir[2].filename);

    // int b = 1 << 14;

    // inode_type new_inode = inode_reader(temp_dir[2].inode, new_inode);

    // // printf("%d \n", root.flags);
    // printf("%d \n", new_inode.flags);

    // if(new_inode.flags & b){
    // printf("It is a directory! \n");
    // }

    // else{
    //     printf("It is not! \n");
    // }

    // cd("Testing");
    
    // temp_root = inode_reader(currDir_iNumber, temp_root);

    // temp_dir[32];

    // lseek(fd, temp_root.addr[0] * BLOCK_SIZE, SEEK_SET);
    // read(fd, &temp_dir, sizeof(temp_dir));

    // printf("%s \n", temp_dir[0].filename);

    // printf("%d \n", strlen(temp_dir[1].filename));





    // inode_type test;
    // test = inode_reader(1, test);
    // printf("Test flag: %d", test.flags);
    // getFreeInode();
    // getFreeInode();

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

