#include <stdio.h>
#include <syscall.h>
#include "list.h"
/*-------------------------------------------------------------Structs----------------------------------------------------------------------*/
typedef struct File File;
typedef struct Directory Directory;

//Stores essential information about a file
struct File{
    char* fileName;
    char* fileType;
    bool isDirectory;
    bool canBeRan;
    bool isHiddenFile;
    bool userCanRead;
    bool userCanWrite;
    bool userCanExecute;
    int numOfHardLinks;
    char* fileOwner;
    char* fileGroupName;
    int sizeOfFile;
    char* dateTimeOfMostRecentChange;

};

//Stores essential information about a directory
struct Directory{
    List* files;
    Directory* parent;
    File* directoryFile;    //The file information for this directory
};

/*--------------------------------------------------------------Functions-------------------------------------------------------------------*/
/*----------------------------------------------------------------Main----------------------------------------------------------------------*/
void main(){
    printf("Makefile test\n");
}