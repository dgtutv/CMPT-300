/*Program Name: UnixLS
Author: Daniel Todd
Description: The purpose of this program is to emulate the UNIX ls command with flags -i, -l, and -R
Course: CMPT 300 - Operating Systems*/

/*------------------------------------------------------------Includes-----------------------------------------------------------------------*/
#include <stdio.h>
#include <syscall.h>
#include "list.h"
#include <string.h>

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
List* baseFileNames;     //A list of the names of files or directories, optionally provided by the user at the command-line

//Booleans representing whether or not a certain flag was specified by the user
bool iFlag;
bool lFlag;
bool rFlag;

/*--------------------------------------------------------------Functions-------------------------------------------------------------------*/

/*Processes a given command-line argument, and sets global booleans iFlag, lFlag, and rFlag accordingly
Returns false on failure, true on success*/
bool argumentHandler(char* argument){
    //If a hyphen is provided, treat the argument as a set of flags
    char currentChar;
    bool hasHyphen = false;
    bool flagSet = false;   //Boolean to tell if there has been a valid flag following a hyphen
    for(int i=0; i<strlen(argument); i++){
        currentChar = argument[i];

        //Set the flags accordingly, ensuring format of input is correct
        if(currentChar == '-' && !hasHyphen && List_count(baseFileNames) == 0){
            hasHyphen = true;
            continue;
        }
        else {
            if(currentChar == '-' && hasHyphen){
                printf("UnixLs: invalid format, character \'-\' provided too many times\n");
                return(false);
            }
            if(currentChar == '-' && List_count(baseFileNames) > 0){
                printf("UnixLs: invalid format, any options must come before file names\n");
                return(false);
            }
        }
        if(currentChar == 'i' && hasHyphen){
            iFlag = true;
            flagSet = true;
        }
        else if(currentChar == 'R' && hasHyphen){
            rFlag = true;
            flagSet = true;
        }
        else if(currentChar == 'l' && hasHyphen){
            lFlag = true;
            flagSet = true;
        }
        else if(hasHyphen){
            printf("UnixLs: invalid format, \'%c\' is not an option\n", currentChar);
            return(false);
        }
    }

    //If there is a hyphen provided, but no flags have been set, return an error
    if(hasHyphen && !flagSet){
        printf("UnixLs: cannot access \'-\', no such file or directory\n");
        return(false);
    }
   
    //If there is no hyphen provided, treat the argument as a file or directory
    else if(!hasHyphen){
        List_append(baseFileNames, argument);
    }

    return(true);
}

/*----------------------------------------------------------------Main----------------------------------------------------------------------*/
int main(int argc, char* argv[]){
    //Instanstiate some global variables
    argumentCount = argc;
    arguments = List_create();
    baseFileNames = List_create();
    iFlag = false;
    lFlag = false;
    rFlag = false;

    //Take the flags for ls using command line arguments
    for(int i=1; i<argc; i++){
        List_append(arguments, argv[i]);
    }

    //Call argumentHandler for each of our arguments to set the flag booleans accordingly
    if(argc > 1){
        char* currentArgument = List_first(arguments);
        bool returnValue;
        while(currentArgument != NULL){
            returnValue = argumentHandler(currentArgument);
            if(returnValue == false){
                printf("UnixLs: One or more of the provided arguments is invalid\n");
                return(0);
            }
            currentArgument = List_next(arguments);
        }
    }

    //For now, print out all the set flags, and specified files or directories for testing purposes
    if(rFlag){printf("The \'R\' option has been set\n");}
    if(lFlag){printf("The \'l\' option has been set\n");}
    if(iFlag){printf("The \'i\' option has been set\n");}
    if(List_count(baseFileNames) > 0){
        printf("Files specified: %*s\n", 25-17, (char*)List_first(baseFileNames));
        for(int i=1; i<List_count(baseFileNames); i++){
            printf("%*s\n", 25, (char*)List_next(baseFileNames));
        }
    }

    //Call functions associated to flags specified by the user in the command-line arguments
}