/*Program Name: UnixLS
Author: Daniel Todd
Description: The purpose of this program is to emulate the UNIX ls command with flags -i, -l, and -R
Course: CMPT 300 - Operating Systems*/

/*Known problems:
1. Does not scale the amount of columns per row relative to the amount of files in the directory we are reading
2. Does not work correctly when a file is specified at the command line*/

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
#include <grp.h>
#include <time.h>
#include <pwd.h>
#include <sys/ioctl.h>

/*-------------------------------------------------------------Structs----------------------------------------------------------------------*/
//Stores essential information about a file
typedef struct{
    char* name;
    bool isDirectory;
    bool canBeRan;
    bool isHidden;
    bool isSymbolicLink;
    int numOfHardLinks;
    char* ownerName;
    char* groupName;
    long long sizeOfFile;   //The size of the file in bytes
    char dateTimeOfMostRecentChange[100];
    ino_t iNodeNumber;
    DIR* linkStream;        //If this file is a symbolic link, this stores the directory stream for the directory the link points to, otherwise stores NULL
    mode_t permissions;     
    int ownerUserID;        //The user ID for the owner of the file
    int groupID;            //The group ID for the group associated with this file
} File;

//Stores essential information about a directory
typedef struct{
    List* files;
    DIR* parent;            //The directory stream for the parent of this directory, set to NULL if root.
    File* directoryFile;    //The file information for this directory
    DIR* directoryStream;   //The pointer to the dirent struct for the directory
} Directory;

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
void* doNothing(){}

/*Checks if a given string has spaces present in it
Returns true if spaces detected, false if not*/
bool hasSpace(char* string){
    for(int i=0; i<strlen(string); i++){
        if(string[i] == ' '){
            return true;
        }
    }
    return false;
}

/*Surrounds the string in single quotes if a space is present in the string
Returns the modified string, returns the original string if no space is present*/
char* addQuotes(char* string){
    if(hasSpace(string)){
        char* newString = malloc(strlen(string)+3);
        newString[0] = '\'';
        newString[strlen(string)+1] = '\'';
        newString[strlen(string)+2] = '\0';
        int j=0;
        for(int i=1; i<strlen(string)+1; i++){
            newString[i] = string[j];
            j++;
        }
        return(newString);
    }
    return(string);
}

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

    //This is likely then a file, or empty directory
    if(directoryStream == NULL){
        //Search for the file in the current working directory
        char currentWorkingDirectory[1024];
        if(getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) == NULL){
            printf("UnixLs: directoryReader: Could not find the current working directory\n");
        }
        Directory* returnDirectory = directoryReader(currentWorkingDirectory);

        //If the file exists in the working directory, return it
        if(returnDirectory != NULL){
            return returnDirectory;
        }

        //If the file does not exist, throw an error
        else{
            printf("UnixLs: DirectoryReader: Cannot open \'%s\', no such file or directory found\n", directoryName);
            return(NULL);
        }
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
        currentFile->iNodeNumber = directoryEntry->d_ino;
        currentFile->name = directoryEntry->d_name;

        //Gather information about the file
        struct stat* fileInformation = malloc(sizeof(struct stat));
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", directoryName, currentFile->name);
        if(lstat(filePath, fileInformation) == -1){
            printf("UnixLs: directoryReader: failed to obtain information about file \"%s\"\n", currentFile->name);
            return(NULL);
        }

        //If the file is a symbolic link, get the directory stream to the directory that the link points to
        if(S_ISLNK(fileInformation->st_mode)){
            currentFile->isSymbolicLink = true;
            char linkBuffer[1024];
            int numOfBytes = readlink(currentFile->name, linkBuffer, sizeof(linkBuffer));
            if(numOfBytes == -1){
                printf("UnixLs: directoryReader: failed to read the link pertaining to file \"%s\"\n", currentFile->name);
                return(NULL);
            }
            linkBuffer[numOfBytes] = '\0';  //Set the linkBuffer string to end at the correct location after calling readlink() on it
            DIR* linkDirectoryStream = opendir(linkBuffer);
            if(linkDirectoryStream == NULL){
                printf("UnixLs: directoryReader: failed to obtain the directory stream for symbolic link \"%s\"\n", currentFile->name);
                return(NULL);
            }
            currentFile->linkStream = linkDirectoryStream;
        }

        //Store information about the file, given by lstat
        currentFile->permissions = fileInformation->st_mode;
        currentFile->ownerUserID = fileInformation->st_uid;
        currentFile->groupID = fileInformation->st_gid;
        currentFile->sizeOfFile = fileInformation->st_size;
        currentFile->name = addQuotes(currentFile->name);
        if(currentFile->name[0] == '.'){
            currentFile->isHidden = true;
        } 
        else{
            currentFile->isHidden = false;
        }
        currentFile->numOfHardLinks = fileInformation->st_nlink;
        if((fileInformation->st_mode & S_IXUSR) == S_IXUSR){
            currentFile->canBeRan = true;
        }
        else{
            currentFile->canBeRan = false;
        }
        strftime(currentFile->dateTimeOfMostRecentChange, sizeof(currentFile->dateTimeOfMostRecentChange), "%b %d %H:%M", localtime(&fileInformation->st_mtime));
        if(S_ISDIR(fileInformation->st_mode)){
            currentFile->isDirectory = true;
        } 
        else{
            currentFile->isDirectory = false;
        }

        //Get the name of the group from the group ID
        struct group* currentGroup = getgrgid(currentFile->groupID);
        if(currentGroup == NULL){
            printf("UnixLs: directoryReader: Could not get group for group ID %d", currentFile->groupID);
            return(NULL);
        }
        currentFile->groupName = currentGroup->gr_name;
        
        //Get the name of the owner of the file from the owner's ID
        struct passwd* currentPassword = getpwuid(currentFile->ownerUserID);
        if(currentPassword == NULL){
            perror("getpwuid");
            exit(EXIT_FAILURE);
        }
        currentFile->ownerName = currentPassword->pw_name;

        //Store the file in the directories' list
        List_append(currentDirectory->files, currentFile);

        //If this is the file for the directory itself, store it in the Directory struct
        if(strcmp(currentFile->name, ".")){
            currentDirectory->directoryFile = currentFile;
        }
    }
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

/*Prints to screen ls output when no flags are specified*/
void ls(){
    //Iterate over our directories
    Directory* currentDirectory;
    for(int j=0; j<List_count(directories); j++){
        //Set the current directory
        if(j == 0){
            currentDirectory = List_first(directories);
        }
        else{
            currentDirectory = List_next(directories);     
        }

        //If there is more than one directory to iterate over, print the name of the directory before any input
        if(List_count(directories) > 1){
            printf("%s:\n", currentDirectory->directoryFile->name);
        }

        //Print the names of all the files
        File* currentFile = List_first(currentDirectory->files);
        for(int i=0; i<List_count(currentDirectory->files); i++){
            if(i==0){
                currentFile = List_first(currentDirectory->files);
            }
            else{
                currentFile = List_next(currentDirectory->files);
            }

            //If there is a carriage return character present, remove it
            if(strlen(currentFile->name) > 0 && currentFile->name[strlen(currentFile->name)-1] == '\r'){
                currentFile->name[strlen(currentFile->name)-1] = '\0';
            }
            if(!currentFile->isHidden && currentFile->canBeRan && !currentFile->isDirectory){
                printf("\033[1;32m%s\033[0m\n", currentFile->name);    //Make the text green and bold if it can be ran
            }
            else if(!currentFile->isHidden && currentFile->isDirectory){
                printf("\033[1;34m%s\033[0m\n", currentFile->name);     //Make the text blue and bold if it is a folder
            }
            else if(!currentFile->isHidden){
                printf("%s\n", currentFile->name);
            }
        }
    }
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

    //Get directory information for the files/directories provided by the user
    Directory* returnDirectory;
    if(List_count(baseFileNames) > 0){
        for(int i=0; i<List_count(baseFileNames); i++){
            if(i==0){
                returnDirectory = directoryReader(List_first(baseFileNames));
            }
            else{
                returnDirectory = directoryReader(List_next(baseFileNames));
            }
            if(returnDirectory == NULL){
                printf("UnixLs: Directory with name %s does not exist\n", (char*)List_curr(baseFileNames));
            }  
        }
    }

    //If no file or directory is provided by the user, get directory information for the directory of the program
    char currentWorkingDirectory[1024];
    if(getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) == NULL){
        printf("UnixLs: Could not find the current working directory\n");
    } 
    returnDirectory = directoryReader(currentWorkingDirectory);


    //If there are no flags set, print the standard ls output
    if(!rFlag && !lFlag && !iFlag){
        ls();
    }
    else if(iFlag && !rFlag && !lFlag){

    }
}