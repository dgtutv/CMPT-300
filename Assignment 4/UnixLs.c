/*Program Name: UnixLS
Author: Daniel Todd
Description: The purpose of this program is to emulate the UNIX ls command with flags -i, -l, and -R
Course: CMPT 300 - Operating Systems*/

#define _DEFAULT_SOURCE     //Defines some necessary macros

/*------------------------------------------------------------Includes-----------------------------------------------------------------------*/
#include <stdio.h>
#include <syscall.h>
#include "list.h"
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

/*-------------------------------------------------------------Structs----------------------------------------------------------------------*/
typedef struct File File;
typedef struct Directory Directory;

//Stores essential information about a file
struct File{
    char* fileName;
    bool isDirectory;
    bool canBeRan;
    bool isHiddenFile;
    bool isSymbolicLink;
    int numOfHardLinks;
    char* fileOwner;
    char* fileGroupName;
    int sizeOfFile;
    char* dateTimeOfMostRecentChange;
    ino_t iNodeNumber;
    DIR* linkStream;        //If this file is a symbolic link, this stores the directory stream for the directory the link points to, otherwise stores NULL
    mode_t permissions;     
    int ownerUserID;        //The user ID for the owner of the file
    int groupID;            //The group ID for the group associated with this file
};

//Stores essential information about a directory
struct Directory{
    List* files;
    DIR* parent;            //The directory stream for the parent of this directory, set to NULL if root.
    File* directoryFile;    //The file information for this directory
    DIR* directoryStream;   //The pointer to the dirent struct for the directory
};

/*-----------------------------------------------------------Global Variables-------------------------------------------------------------------*/

int argumentCount;      //The number of command line arguments supplied
List* arguments;        //A list of the command line arguments supplied
List* baseFileNames;    //A list of the names of files or directories, optionally provided by the user at the command-line
List* directories;      //A list of all the directories accessed.
//TODO: closedir() on all the directories accessed at the end of the program.

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

/*Gets information about the directory provided by the directoryName parameter
Returns a pointer to the directory on success, returns NULL on failure*/
Directory* directoryReader(const char* directoryName){
    //Get a pointer to the directory with opendir
    DIR* directoryStream = opendir(directoryName);
    if(directoryStream == NULL){
        printf("UnixLs: Cannot open \'%s\', no such file or directory found\n", directoryName);
        return(NULL);
    }

    //Set information about our directory
    Directory* currentDirectory = malloc(sizeof(Directory));
    currentDirectory->files = List_create();
    currentDirectory->directoryStream = directoryStream;
    currentDirectory->parent = opendir("..");
    List_append(directories, currentDirectory);

    //Read all of the entries in the directory
    struct dirent* directoryEntry;
    while((directoryEntry = readdir(directoryStream)) != NULL){
        //Store information we can get from the directory entry about the current file
        File* currentFile = malloc(sizeof(File));
        List_append(currentDirectory->files, currentFile);
        currentFile->iNodeNumber = directoryEntry->d_ino;
        currentFile->fileName = directoryEntry->d_name;
        if(directoryEntry->d_type == DT_DIR){
            currentFile->isDirectory = true;
        }    

        //Gather information about the file
        struct stat* fileInformation = malloc(sizeof(struct stat));
        if(lstat(currentFile->fileName, fileInformation) == -1){
            printf("UnixLs: directoryReader: failed to obtain information about file \"%s\"\n", currentFile->fileName);
            return(NULL);
        }

        //If the file is a symbolic link, get the directory stream to the directory that the link points to
        if(S_ISLNK(fileInformation->st_mode)){
            currentFile->isSymbolicLink = true;
            char linkBuffer[1024];
            int numOfBytes = readlink(currentFile->fileName, linkBuffer, sizeof(linkBuffer));
            if(numOfBytes == -1){
                printf("UnixLs: directoryReader: failed to read the link pertaining to file \"%s\"\n", currentFile->fileName);
                return(NULL);
            }
            linkBuffer[numOfBytes] = '\0';  //Set the linkBuffer string to end at the correct location after calling readlink() on it
            DIR* linkDirectoryStream = opendir(linkBuffer);
            if(linkDirectoryStream == NULL){
                printf("UnixLs: directoryReader: failed to obtain the directory stream for symbolic link \"%s\"\n", currentFile->fileName);
                return(NULL);
            }
            currentFile->linkStream = linkDirectoryStream;
        }

        //Store information about the file, given by lstat
        currentFile->permissions = fileInformation->st_mode;
        currentFile->ownerUserID = fileInformation->st_uid;
        currentFile->groupID = fileInformation->st_gid;

        //TODO: canBeRan, isHiddenFile, userCanRead, userCanWrite, userCanExecute, numOfHardLinks, fileOwner, fileGroupName, sizeOfFile, dateTimeOfMostRecentChange
    }

    //The first entry in the directory is the directory itself
    currentDirectory->directoryFile = List_first(currentDirectory->files);
}

/*Decodes the permissions given in the parameter from mode_t type to the string format provided by ls
Returns a string*/
char* decodePermissions(mode_t permissions){
    //Use ternary operators on indices of the return string to format it
    char* returnString = malloc(10);

    //Owner permissions
    returnString[0] = (permissions & S_IRUSR) ? 'r' : '-';
    returnString[1] = (permissions & S_IWUSR) ? 'w' : '-';
    returnString[2] = (permissions & S_IXUSR) ? 'x' : '-';

    //Group permissions
    returnString[3] = (permissions & S_IRGRP) ? 'r' : '-';
    returnString[4] = (permissions & S_IWGRP) ? 'w' : '-';
    returnString[5] = (permissions & S_IXGRP) ? 'x' : '-';

    //Other permissions
    returnString[6] = (permissions & S_IROTH) ? 'r' : '-';
    returnString[7] = (permissions & S_IWOTH) ? 'w' : '-';
    returnString[8] = (permissions & S_IXOTH) ? 'x' : '-';  

    //Return our decoded permissions
    returnString[9] = '\0';
    return(returnString);  
}

/*----------------------------------------------------------------Main----------------------------------------------------------------------*/
int main(int argc, char* argv[]){
    //Instanstiate some global variables
    argumentCount = argc;
    arguments = List_create();
    baseFileNames = List_create();
    directories = List_create();
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

    //Get directory information for the files/directories provided by the user
    //If no file or directory is provided by the user, get directory information for the directory of the program

}