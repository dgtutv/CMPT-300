/*Program Name: UnixLS
Author: Daniel Todd
Description: The purpose of this program is to emulate the UNIX ls command with flags -i, -l, and -R
Course: CMPT 300 - Operating Systems*/

/*------------------------------------------------------------Includes-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------Global Variables-------------------------------------------------------------------*/

int argumentCount;      //The number of command line arguments supplied
List* arguments;        //A list of the command line arguments supplied

//Booleans representing whether or not a certain flag was specified by the user
bool iFlag;
bool lFlag;
bool rFlag;

/*--------------------------------------------------------------Functions-------------------------------------------------------------------*/

/*Processes a given command-line argument, and sets global booleans iFlag, lFlag, and rFlag accordingly
Returns 0 on failure, 1 on success*/
int argumentHandler(char* argument){
    //TODO: throw an error if a flag is repeated
    //TODO: throw an error if an invalid flag is entered
}

/*----------------------------------------------------------------Main----------------------------------------------------------------------*/
int main(int argc, char* argv[]){
    //Instanstiate some global variables
    argumentCount = argc;
    List_create(arguments);
    iFlag = false;
    lFlag = false;
    rFlag = false;

    //Take the flags for ls using command line arguments
    if(argc > 1 && argc < 4){   
        char* flags0 = argv[1];
        List_append(arguments, flags0);
    }
    if(argc > 1 && argc < 4){
        char* flags1 = argv[2];
        List_append(arguments, flags1);
    }
    if(argc > 2 && argc < 4){
        char* flags2 = argv[3];
        List_append(arguments, flags2);
    }
    if(argc > 4){
        printf("ERROR: Too many arguments specified, only 3 or less arguments are allowed\n");
        return(0);
    }

    //Call argumentHandler for each of our arguments to set the flag booleans accordingly
    if(argc > 1){
        
    }

    //Call functions associated to flags specified by the user in the command-line arguments
}