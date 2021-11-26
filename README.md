# Unix V6 File System
### We implemented a c program to open a file,read,write,making a directory,changing the directory,deleting the file from v6 file system ,creating a new file in v6 file system and filling up the contents in that from external file(cpin),creating an external file and make the external file contents equal to v6 file contents9(cpout) and finally to quit the file system we have created
## Some unix calls used:
### lseek(),read(),write(),open().
## Compilation and Running:
### We executed the c program on command prompt by connecting to global protect(vpn to access the campus server).We should compiler the code by using gcc compiler.The command is gcc filename(cc modv6.c).For running we use the command ./a.out.
### After the program runs then we can enter whatever command the user wishes.The commands that can be chosen by the user:
## open_fs :  open_fs foo.txt

## init_fs :  initfs 10 2

## cpin :  cpin Path(any path on your desktop) transfer1.txt

## cpout : cpout destination path(choose the path on your desktop for external file) transfer.txt

## rm : rm transfer.txt

## cd : cd name(name of the directory)

## mkdir : mkdir name(name of the directory)

## q(quit) : q used for quitting
