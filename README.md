# Unix V6 File System
We created a C program to implement a modified version of the V6 filesystem.

## Compilation Instructions:

To execute the program, the following steps are to be followed:
1) Execute this command to compile the code. "gcc FileSystem.c -o FileSystem -std=c99" (Tested on the UTD Unix server.)
2) To run the executable file, please run this command "./FileSystem"

## Documentation:

Using this program, we can do the following operations on our file system:

1) Openfs - It opens a file system. If a file system with that name does not exist, a file system with the name is created.
2) Initfs - It initializes the filesystem by initializing the inodes and chaining the data blocks. It creates the root directory and sets up our filesystem to be used for further operations.
3) cpin - It makes a copy of an existing file on our external machine onto our v6 file system.
4) cpout - It makes a copy of an existing file in our v6 file system on our external machine.
5) rm - It deletes a file from our v6 file system.
6) mkdir - It makes a directory in our v6 file system.
7) cd - it changes our working directory to the one specified.
8) q- it saves all the changes and quits the program.
