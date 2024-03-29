/*
 * CS 5348 - PROJECT.2 PART.2
 * TEAM - 11
 * GOVINDRAJ, VARASIDDHI JAYASURYAA - vxg190049
 * KALANTARIAN, LEVON - lxk016400
 * GODISHALA, SREEJA - sxg200037
 *
 *------------------------------------------------------------------
 *
 * All the team members worked collectively to implement each command.
 * Each one of us got familiar with the read, write, open, lseek system calls by implementing small programs on our own and
 * used that knowledge to understand the working of the three commands to be implemented.
 * For implementing the initfs command we went through the class notes and the slides, understood the multiple steps involved in the 
   file system initialization process such as initializing the data blocks, inodes, adding and getting the free data blocks and verifing the 
   file system initialization without the ninode value.
 *
 *-------------------------------------------------------------------

 OPENFS
 ------

 * Takes the path of the filesystem to open as input. 
 * Using this path, we open the filesystem. If the filesystem is not present, then we create the file system using the flag O|CREAT passed into the open() system call.
 * If the file system already exists, we read in the superblock, and get the number of inodes present and set the current directory to the root directory.
 
 INITFS
 ------

 * The initfs command takes two inputs- No of blocks allocated for the filesystem, and No of blocks allocated just for the inodes.
 * Using the two inputs, we write the value of isize of the filesystem. 
 * We initialize all the inodes of the filesystem. Set all the inodes to unallocated by setting the first bit (MSB) of the inodes to 0.
 * We take all the data blocks of the filesystem and chain them together.
 * We then proceed to create the root directory of the file system. We allocate a free data block for the root directory and allocate inode number 1 to the root.
 * We write two entries into the directory (. and ..) and set all the addr[i] to 0 until a data block is allocated to it.
 * We set the 15th bit of the root inode to 1 to indicate it is allocated.
 * We set the 14th bit of the root inode to 1 to indicate it is a directory.
 
 MKDIR
 ------

 * It takes in the filename (of the directory to make) as the input.
 * We pass the folder name to an address resolver to remove any absolute or relative paths. This takes us to the correct directory to make the folder.
 * We then go through each addr[i] to get the directory entries. If the directory entry is empty, its strlen(filename) will be 0.
 * We use this to find an empty space within the directory entries. If there is a existing folder with the same name, then the directory is not created.
 * After finding an available directory entry, we allocate an inode and data block to the new folder.
 * We set ".." to point to the inode of the parent directory and "." to itself.
 

 CD
 ------

 * It takes in the filename (of the directory we want to access) as the input.
 * We use a compare flag to check if the file is a directory or not. Using the current directory's inode, we traverse through the addr[] array and through each directory entry to find the directory we wish to access.
 * If the file is found, it's inode flags is tested with the compare flag to ensure that the file is a directory. Once the file is found, we set the fileFound variable to 1, so further looping does not occur.
 * For our implementation of cd in the main() function, we actually use a wrapper function called internal_cd().
 * This function resolves the address provided (whether it be relative or absolute) and uses cd() to access each directory within the address provided.
 * eg- /user/jay- we use internal_cd() to resolve this address, so we first cd() into user and then cd() into jay.
 

 CPIN
 ------

 * It takes in two parameters as its input- source_path- which is the path of the file on our local machine, and filename- which is the path of the file (including the filename) where we want to copy into our v6 file system.
 * We pass the folder name to an address resolver to remove any absolute or relative paths. This takes us to the correct directory to copy the file into.
 * We then go through each addr[i] to get the directory entries. If the directory entry is empty, its strlen(filename) will be 0.
 * We use this to find an empty space within the directory entries. This empty space will be the entry for the copied file.
 * After finding an available directory entry, we allocate an inode to the copied file. We also set the spaceAvailable vairable to 1 once space is found to prevent further looping.
 * We then open the source file and read it's bytes in chunks (equal to 1024 bytes) and write it into the copied file within the v6 file system.
 

 CPOUT
 ------ 

 * It takes in two parameters as its input- dest_path- which is the path of the file on our local machine where we will copy into, and filename- which is the path of the file (including the filename) where we want to copy out from our v6 file system.
 * We pass the folder name to an address resolver to remove any absolute or relative paths. This takes us to the correct directory to copy the file from the v6 file system.
 * We then go through each addr[i] to get the directory entries. If the directory entry is empty, its strlen(filename) will be 0.
 * We use this to find the file which we wish to copy. Once the file is found, the fileFound variable is set to 1 to prevent further looping.
 * After finding the file, we read the contents of the file in chunks of 1024 bytes and write those read bytes into the destination address of our local machine.
 

 RM
 ------ 

 * It takes in the filename (of the file we want to delete) as the input.
 * Using the current directory's inode, we traverse through the addr[] array and through each directory entry to find the file we wish to delete.
 * Once the file is found, we use a compare flag to ensure that the file is a regular file and not a directory. We set the fileFound variable to 1 to prevent further looping.
 * We then go through the addr[i] of the file to free the data blocks allocated to it.
 * We set the filename of that file to "" so that it's strlen would be equal to 0.
 * We de-allocate the inode by toggling the 15th bit to 0.
 
  
 LS
 ------ 

 * This function goes traverses through the current directory's addr[] array and goes through each directory entry and prints out each file's name and inode number allocated to it.

*/


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
int actualDir_iNumber;

char pwd[100];
char before_pwd[100];

int open_fs(char *file_name);
void blockWriter(int blockNumber, void *buffer, int size);
int blockReader(int blockNumber, void *buffer, int size);
void blockReader_withOffset(int blockNumber, int offset, void *buffer, int size);
void inode_writer(int iNumber, inode_type inode);
inode_type inode_reader(int iNumber, inode_type inode);
void initialize_inodes(int iNumber);
void allocate_blocks(int blocknumber);
int getFreeDataBlock();
int getFreeInode();
void create_root();
void initfs(int n1, int n2);
char* addressResolver(char *name);
void mkdir(char *name);
int cd(char *name);
void internal_cd(char *name);
void cpin(char *source_path, char *filename);
void cpout(char *dest_path, char *filename);
void ls();
void rm(char *filename);
void quit();


int open_fs(char *file_name){
    fd = open(file_name, O_RDWR);

    if(fd == -1){
        fd = open(file_name, O_RDWR | O_CREAT, 0600);
        printf("File system does not exist! \n It has been created! Please initialize it \n");
    }

    else{
        blockReader(1, &superblock, sizeof(superblock));
        root = inode_reader(1, root);
        currDir_iNumber = 1;
        no_of_inodes = BLOCK_SIZE*superblock.isize/INODE_SIZE;
    }

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
    dir_type d[32];

    for(int i = 0; i < 32; i++){
        strcpy(d[i].filename, "");
    }

    strcpy(d[0].filename, ".");
    d[0].inode = 1;

    strcpy(d[1].filename, "..");
    d[1].inode = 1;

    blockWriter(block_number, d, sizeof(d));

    root.addr[0] = block_number;
    root.nlinks = 1;
    for(int i = 1; i < 9; i++){
        root.addr[i] = 0;
    }
    
    root.flags |= 1 << 15; //Root is allocated
    root.flags |= 1 <<14; //It is a directory
    root.actime = time(NULL);
    root.modtime = time(NULL);
    
    root.gid = 0;
    root.uid = 0;

    root.size0 = 0;
    root.size1 = 2 * sizeof(dir_type);
    
    inode_writer(1, root);
}

// Function to initialize the file system
void initfs(int n1, int n2){

    // n1 is the file system size in number of blocks and n2 is the number of blocks devoted to the i-nodes.

    char fillerBlock[BLOCK_SIZE] = {0};

    blockWriter(n1 - 1, fillerBlock, BLOCK_SIZE); //Writing to last block.

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
}

char* addressResolver(char *name){
    char* address = strdup(name);
    char* token = strtok(address, "/");

    char* new_address = strdup("");
    char* prev = "";

    while(token != NULL){
        
        if(strlen(prev) != 0){
            strcat(new_address, prev);
            strcat(new_address, "/");
        }
        prev = token;
        token = strtok(NULL, "/");        
    }

    actualDir_iNumber = currDir_iNumber;

    if(address[0] == '/'){
        currDir_iNumber = 1;
    }

    token = strtok(new_address, "/");

    while(token != NULL){
        if(cd(token) == -1){
            printf("Error! Folder not found!\n");
            return "";
        }
        token = strtok(NULL, "/");
    }

    return prev;
}

void mkdir(char *name){

    name = addressResolver(name);

    if(strlen(name) == 0){
        return;
    }

    int iNumber;
    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    int currDirBlockNumber;

    dir_type currDir[32];
    int spaceAvailable = 0;

    for(int j = 0; j < 9; j++){

        if(spaceAvailable){
            break;
        }

        if(currDir_inode.addr[j] == 0){
            if( (currDir_inode.addr[j] = getFreeDataBlock()) == -1){
                printf("Storage space full! No free Data blocks are available! \n");
                return;
            }
        }

        currDirBlockNumber = currDir_inode.addr[j];
        lseek(fd, currDir_inode.addr[j] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(int i = 0; i < 32; i++){

            if(strcmp(currDir[i].filename, name) == 0){
                printf("Folder already exists! \n");
                return;
            }

            if(strlen(currDir[i].filename) == 0){
                iNumber = getFreeInode();

                if(iNumber == -1){
                    printf("Storage space full! No free inodes are available! \n");
                    return;
                }

                strcpy(currDir[i].filename, name);
                currDir[i].inode = iNumber;
                spaceAvailable = 1;
                break;
            }
        }
    }

    int blockNumber = getFreeDataBlock();

    if(blockNumber == -1){
        printf("Storage space full! No free Data blocks are available! \n");
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

    inode_type inode = inode_reader(iNumber, inode);
    inode.nlinks = 1;
    inode.flags |= 1 <<14; //It is a directory

    inode.actime = time(NULL);
    inode.modtime = time(NULL);

    inode.gid = 0;
    inode.uid = 0;

    inode.size0 = 0;
    inode.size1 = 2 * sizeof(dir_type);
    inode.addr[0] = blockNumber;
    for(int i = 1; i < 9; i++){
        inode.addr[i] = 0;
    }

    inode_writer(iNumber, inode);

    currDir_inode.size1 += sizeof(dir_type);
    inode_writer(currDir_iNumber, currDir_inode);
    blockWriter(currDirBlockNumber, currDir, sizeof(currDir));  

    currDir_iNumber = actualDir_iNumber;  

}

int cd(char *name){

    unsigned short int compare_flag = 1 << 14; //To check if the file is a directory or not
    inode_type inode;

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    int currDirBlockNumber;

    dir_type currDir[32];

    int new_iNumber = 0;
    int fileFound = 0;
    
    for(int j = 0; j < 9; j++){

        if(fileFound){
            break;
        }

        if(currDir_inode.addr[j] == 0){
            break;
        }

        currDirBlockNumber = currDir_inode.addr[j];
        lseek(fd, currDir_inode.addr[j] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(int i = 0; i < 32; i++){
        
            if(strcmp(currDir[i].filename, name) == 0){

                inode = inode_reader(currDir[i].inode, inode);

                if(inode.flags & compare_flag){
                    new_iNumber = currDir[i].inode;
                    fileFound = 1;
                    break;
                }
            }
        }
    }

    if(new_iNumber == 0){
        return -1;
    }
    
    inode.actime = time(NULL);
    inode_writer(new_iNumber, inode);
    currDir_iNumber = new_iNumber;

    return 1;
}

void internal_cd(char *name){
    // addressResolver(name);
    char* address = strdup(name);
    char* token = strtok(address, "/");

    if(address[0] == '/'){
        currDir_iNumber = 1;
        strcpy(pwd, "/");
    }


    while(token != NULL){

        if(cd(token) == -1){
            printf("Error! Folder not found!\n");
        }
        
        token = strtok(NULL, "/");
    }
}

void cpin(char *source_path, char *filename){

    filename = addressResolver(filename);

    int source, blockNumber, iNumber, bytesRead;

    if(strlen(filename) == 0){
        printf("Invalid filename! \n");
        return;
    }

    if((source = open(source_path, O_RDWR)) == -1){
        printf("File does not exist! \n");
        return;
    }

    if((iNumber = getFreeInode()) == -1){
        printf("Storage space full! No free inodes are available! \n");
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

    do
    {
        bytesRead = read(source, tempBuffer, BLOCK_SIZE);
        new_file.size1 += bytesRead;
        
        if((blockNumber = getFreeDataBlock()) == -1){
            printf("Storage space full! No free Data blocks are available! \n");
            return;
        }

        new_file.addr[i] = blockNumber;
        blockWriter(blockNumber, tempBuffer, bytesRead);

    } while (bytesRead == BLOCK_SIZE);

    inode_writer(iNumber, new_file);

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    dir_type currDir[32];
    int currDirBlockNumber;
    int spaceAvailable = 0;

    for(int j = 0; j < 9; j++){

        if(spaceAvailable){
            break;
        }
        
        if(currDir_inode.addr[j] == 0){
            if( (currDir_inode.addr[j] = getFreeDataBlock()) == -1){
                printf("Storage space full! No free Data blocks are available! \n");
                return;
            }
        }

        currDirBlockNumber = currDir_inode.addr[j];

        lseek(fd, currDir_inode.addr[j] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(i = 0; i < 32; i++){
            if(strlen(currDir[i].filename) == 0){
                strcpy(currDir[i].filename, filename);
                currDir[i].inode = iNumber;
                spaceAvailable = 1;
                break;
            }
        }
    }
    currDir_inode.size1 += sizeof(dir_type);
    inode_writer(currDir_iNumber, currDir_inode);
    blockWriter(currDirBlockNumber, currDir, sizeof(currDir));  

    currDir_iNumber = actualDir_iNumber;  

}

void cpout(char *dest_path, char *filename){

    filename = addressResolver(filename);

    if(strlen(filename) == 0){
        printf("Invalid filename! \n");
        return;
    }

    int dest, currDirBlockNumber, j, bytesRead;
    inode_type file;
    dest = open(dest_path, O_RDWR|O_CREAT, 0600);

    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);

    dir_type currDir[32];
    unsigned short int compare_flag = 1 << 14;


    char temp_buffer[BLOCK_SIZE];
    int fileFound = 0;

    for(int k = 0; k < 9; k++){

        if(fileFound){
            break;
        }

        if(currDir_inode.addr[k] == 0){
            break;
        }

        currDirBlockNumber = currDir_inode.addr[k];
        lseek(fd, currDir_inode.addr[k] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(int i = 0; i < 32; i++){

            if(strcmp(currDir[i].filename, filename) == 0){
            
                file = inode_reader(currDir[i].inode, file);

                if(file.flags & compare_flag){
                    printf("It is not a file!");
                    return;
                }

                for(j = 0; j < file.size1/BLOCK_SIZE; j++){
                    
                    bytesRead = blockReader(file.addr[j], temp_buffer, BLOCK_SIZE);
                    
                    write(dest, temp_buffer, bytesRead);
                }
                bytesRead = blockReader(file.addr[j], temp_buffer, file.size1 % BLOCK_SIZE);
                write(dest, temp_buffer, bytesRead);
                
                fileFound = 1;
                break;

            }
        }
    }

    if(!fileFound){
        printf("File not found! \n");
        close(dest);
        remove(dest_path);
        return;
    }

    currDir_iNumber = actualDir_iNumber;  

}

void ls(){
    int currDirBlockNumber;
    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    dir_type currDir[32];

    for(int j = 0; j < 9; j++){

        if(currDir_inode.addr[j] == 0){
            break;
        }

        currDirBlockNumber = currDir_inode.addr[j];
        lseek(fd, currDir_inode.addr[j] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(int i = 0; i < 32; i++){
            if(strlen(currDir[i].filename) != 0){
                printf("%s \t %d \n", currDir[i].filename, currDir[i].inode);
            }
        }
    }

}

void rm(char *filename){

    filename = addressResolver(filename);

    int currDirBlockNumber, j, blockNumber;
    inode_type file;

    if(strlen(filename) == 0){
        printf("Invalid filename! \n");
        return;
    }
    
    inode_type currDir_inode = inode_reader(currDir_iNumber, currDir_inode);
    dir_type currDir[32];
    unsigned short int compare_flag = 1 << 14;
    int fileFound = 0;

    for(int k = 0; k < 9; k++){

        if(fileFound){
            break;
        }

        if(currDir_inode.addr[k] == 0){
            break;
        }

        currDirBlockNumber = currDir_inode.addr[k];
        lseek(fd, currDir_inode.addr[k] * BLOCK_SIZE, SEEK_SET);
        read(fd, &currDir, sizeof(currDir));

        for(int i = 0; i < 32; i++){

            if(strcmp(currDir[i].filename, filename) == 0){

                file = inode_reader(currDir[i].inode, file);

                if(file.flags & compare_flag){
                    printf("It is not a file!");
                    return;
                }

                for(j = 0; j < file.size1/BLOCK_SIZE; j++){
                    allocate_blocks(file.addr[j]);
                }
                file.flags &= ~(1 << 15); //Unallocating the inode

                strcpy(currDir[i].filename,"");
                inode_writer(currDir[i].inode, file);
                blockWriter(currDirBlockNumber, currDir, sizeof(currDir));  
                currDir_inode.size1 -= file.size1;

                fileFound = 1;
                break;
            }
        }
    }

    if(!fileFound){
        printf("File not found! \n");
        return;
    }

    currDir_iNumber = actualDir_iNumber;  

}

// Function to quit the program
void quit(){
    close(fd);
    exit(0);
}

// The main function

int main(){
    int no_of_Blocks, no_of_inode_blocks;
    char *args;
    char *arg1, *arg2;
    char command[1024];

    printf("Please open a file system. \n");

    while(1){

        printf("Enter a command: \n");

        scanf(" %[^\n]s", command);

        args = strtok(command, " ");

        if(strcmp(args, "openfs") == 0){
            args = strtok(NULL, " ");
            open_fs(args);
        }

        else if(strcmp(args, "initfs") == 0){
            arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");

            if((!arg1) || (!arg2)){
                printf("Please provide 2 parameters- n1 and n2");
            }

            else{

                no_of_Blocks = atoi(arg1);
                no_of_inode_blocks = atoi(arg2);

                initfs(no_of_Blocks, no_of_inode_blocks);
            
            }

        }

        else if(strcmp(args, "q") == 0){
            quit();
        }

        else if(strcmp(args, "ls") == 0){
            ls();
        }

        else if(strcmp(args, "mkdir") == 0){
            args = strtok(NULL, " ");
            mkdir(args);
        }

        else if(strcmp(args, "cd") == 0){
            args = strtok(NULL, " ");
            internal_cd(args);
        }

        else if(strcmp(args, "cpin") == 0){
            arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");

            if((!arg1) || (!arg2)){
                printf("Please provide 2 parameters- source path and destination path");
            }

            cpin(arg1, arg2);

        }

        else if(strcmp(args, "cpout") == 0){
            arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");

            if((!arg1) || (!arg2)){
                printf("Please provide 2 parameters- destination path and source path");
            }

            cpout(arg1, arg2);
        }

        else if(strcmp(args, "rm") == 0){
            args = strtok(NULL, " ");
            rm(args);
        }
        

    }
    
}

