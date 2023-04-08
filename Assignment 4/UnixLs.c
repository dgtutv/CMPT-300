/*Program Name: UnixLS
Author: Daniel Todd
Description: The purpose of this program is to emulate the UNIX ls command with flags -i, -l, and -R
Course: CMPT 300 - Operating Systems*/

/*Known problems:
1. When the -R flag is specified, the full path is shown, rather than the path relative to the folder being recursively called
2. Not a problem, but would prefer to sort alphabetically if given the extra time*/

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
#include <libgen.h>

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
    char* linkPath;        //If this file is a symbolic link, this stores the path to the file the link points to
    mode_t permissions;     
    int ownerUserID;        //The user ID for the owner of the file
    int groupID;            //The group ID for the group associated with this file
    bool isZip;
} File;

//Stores essential information about a directory
typedef struct{
    List* files;
    DIR* parent;            //The directory stream for the parent of this directory, set to NULL if root.
    File* directoryFile;    //The file information for this directory
    DIR* directoryStream;   //The pointer to the dirent struct for the directory
    char* name;             //The name of the directory
} Directory;

/*-----------------------------------------------------------Global Variables-------------------------------------------------------------------*/

int argumentCount;      //The number of command line arguments supplied
List* arguments;        //A list of the command line arguments supplied
List* baseFileNames;    //A list of the names of files or directories, optionally provided by the user at the command-line
List* directories;      //A list of all the directories accessed.
List* openedDirectories;        //A list to keep track of all the open directory streams
//TODO: closedir() on all the directories accessed at the end of the program.

//Booleans representing whether or not a certain flag was specified by the user
bool iFlag;
bool lFlag;
bool rFlag;

/*--------------------------------------------------------------Functions-------------------------------------------------------------------*/
void* doNothing(){}

/*Gets the filename from a full or local path
Returns a string, returns NULL upon failure*/
char* getNameFromPath(char* path){
    char* reverseString = malloc(strlen(path)+1);
    int forwardIterator = 0;

    //Iterate through the string backwards, store chars after the last '\' in the reverseString
    for(int i=strlen(path); i > 0; --i){
        if(path[i] == '/'){
            break;
        }
        reverseString[forwardIterator] = path[i];
        forwardIterator++;
    }
    reverseString[forwardIterator] = '\0';
    int reverseStringSize = forwardIterator;
    
    //Reverse reverseString into returnString
    char* returnString = malloc(sizeof(reverseString));    
    forwardIterator = 0;
    for(int i = reverseStringSize-1; i > 0; --i){
        returnString[forwardIterator] = reverseString[i];
        forwardIterator++;
    }
    returnString[forwardIterator] = '\0';

    free(reverseString);
    return(returnString);
}

/*Counts the number of digits in an integer*/
int numDigits(int num){
    int count = 0;
    while(num != 0) {
        num /= 10;
        ++count;
    }
    return count;
}

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

bool compareDirectories(void* directory1, void* directory2){
    if(strcmp(((Directory*)directory1)->directoryFile->name,  ((Directory*)directory2)->directoryFile->name) == 0){
        return(true);
    }
    return(false);
}

/*Gets information about the directory provided by the directoryName parameter
Returns a pointer to the directory on success, returns NULL on failure*/
Directory* directoryReader(char* directoryName){
    //Get a pointer to the directory with opendir
    DIR* directoryStream = opendir(directoryName);
    List_append(openedDirectories, directoryStream);

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
    if(currentDirectory->files == NULL){
        printf("UnixLs: directoryReader(): Failed to allocate a new list for currentDirectory->files\n");
    }
    currentDirectory->directoryStream = directoryStream;
    currentDirectory->parent = opendir("..");

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

        //If the file is a symbolic link, get the path to the file that the link points to
        if(S_ISLNK(fileInformation->st_mode)){
            currentFile->isSymbolicLink = true;
            currentFile->linkPath = malloc(1024);
            int numOfReturnedBytes = readlink(filePath, currentFile->linkPath, 1024);
            if(numOfReturnedBytes == -1){
                printf("UnixLs: directoryReader: failed to read the link pertaining to file \"%s\"\n", currentFile->name);
                return(NULL);
            }
            currentFile->linkPath[numOfReturnedBytes] = '\0';       //Set the path to end at the correct location after calling readlink() on it    
        }
        else{
            currentFile->isSymbolicLink = false;
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
        strftime(currentFile->dateTimeOfMostRecentChange, sizeof(currentFile->dateTimeOfMostRecentChange), "%b %d %Y %H:%M", localtime(&fileInformation->st_mtime));
        
        //Check if the file is a directory
        char* fullPath = realpath(filePath, NULL);
        if(S_ISDIR(fileInformation->st_mode)){
            currentFile->isDirectory = true;
            //If the -R flag is set, recursively call directory reader on each directory
            if(rFlag && strcmp(currentFile->name, ".") != 0 && strcmp(currentFile->name, "..") != 0){
                directoryReader(fullPath);
            }
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
            printf("UnixLs: directoryReader: Could not get the name of the owner for the file %s", currentFile->name);
            return(NULL);
        }
        currentFile->ownerName = currentPassword->pw_name;

        //Check if the file is a zip file
        if(S_ISREG(fileInformation->st_mode)) {
            FILE* file = fopen(filePath, "rb");
            if (file == NULL) {
                printf("UnixLs: directoryReader: Could not open file %s", currentFile->name);
                return NULL;
            }
            unsigned char firstFourBytes[4];
            size_t read_count = fread(firstFourBytes, sizeof(unsigned char), 4, file);
            fclose(file);
            if(read_count == 4 && firstFourBytes[0] == 'P' && firstFourBytes[1] == 'K' && firstFourBytes[2] == 3 && firstFourBytes[3] == 4){
                currentFile->isZip = true;                //If the first 4 bytes are the same as the zip signature, the file is a zip file
            }
            else{
                currentFile->isZip = false;
            }
        }

        //Store the file in the directories' list
        List_append(currentDirectory->files, currentFile);

        //If this is the file for the directory itself, store it in the Directory struct
        if(strcmp(currentFile->name, ".") == 0){
            currentDirectory->directoryFile = currentFile;
            currentDirectory->directoryFile->name = directoryName;
            currentDirectory->name = getNameFromPath(fullPath);
        }

    }
    //If the directory is not already in the directories list, and it is not a hidden directory, store it there (redundancy check for -R flag)
    List_first(directories);
    if(List_search(directories, &compareDirectories, currentDirectory) == NULL && currentDirectory->name[0] != '.'){        //TODO: compare to directory name, dont show if directory name has . at start
        List_append(directories, currentDirectory);
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
            printf("%s:\n", currentDirectory->name);
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
            if(!currentFile->isHidden && currentFile->canBeRan && !currentFile->isDirectory && !currentFile->isSymbolicLink){
                printf("\033[1;32m%s\033[0m\n", currentFile->name);    //Make the text green and bold if it can be ran
            }
            else if(!currentFile->isHidden && currentFile->isDirectory && !currentFile->isSymbolicLink){
                printf("\033[1;34m%s\033[0m\n", currentFile->name);     //Make the text blue and bold if it is a folder
            }
            else if(!currentFile->isHidden && currentFile->isZip && !currentFile->isSymbolicLink){
                printf("\033[1;31m%s\033[0m\n", currentFile->name);    //Make the text red and bold if it is a zip folder
            }
            else if(!currentFile->isHidden && currentFile->isSymbolicLink){
                printf("\033[1;36m%s\033[0m\n", currentFile->name);       //Make the text turquoise and bold if it is a symbolic link
            }
            else if(!currentFile->isHidden){
                printf("%s\n", currentFile->name);
            }
        }
        //Print a newline if there are any more directories to be printed
        if(j != List_count(directories)-1 && List_count(directories) > 1){
            printf("\n");
        }
    }
}

/*Prints to screen ls output when the i flag is specified*/
void ls_i(){
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
            printf("%s:\n", currentDirectory->name);
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
            if(!currentFile->isHidden && currentFile->canBeRan && !currentFile->isDirectory && !currentFile->isSymbolicLink){
                printf("%ld \033[1;32m%s\033[0m\n", currentFile->iNodeNumber, currentFile->name);    //Make the text green and bold if it can be ran
            }
            else if(!currentFile->isHidden && currentFile->isDirectory && !currentFile->isSymbolicLink){
                printf("%ld \033[1;34m%s\033[0m\n", currentFile->iNodeNumber, currentFile->name);     //Make the text blue and bold if it is a folder
            }
            else if(!currentFile->isHidden && currentFile->isZip && !currentFile->isSymbolicLink){
                printf("%ld \033[1;31m%s\033[0m\n", currentFile->iNodeNumber, currentFile->name);    //Make the text red and bold if it is a zip folder
            }
            else if(!currentFile->isHidden && currentFile->isSymbolicLink){
                printf("%ld \033[1;36m%s\033[0m\n", currentFile->iNodeNumber, currentFile->name);       //Make the text turquoise and bold if it is a symbolic link
            }
            else if(!currentFile->isHidden){
                printf("%ld %s\n", currentFile->iNodeNumber, currentFile->name);
            }
        }
        //Print a newline if there are any more directories to be printed
        if(j != List_count(directories)-1 && List_count(directories) > 1){
            printf("\n");
        }
    }
}

/*Prints to screen ls output when the l flag is specified*/
void ls_l(){
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
            printf("%s:\n", currentDirectory->name);
        }

        //Get the max length of the owner name, group name, and file size
        int maxOwnerNameLength = 0;
        int maxGroupNameLength = 0;
        int maxSizeLength = 0;
        int maxNumHardLinksLength = 0;
        File* currentFile;
        for(int i=0; i<List_count(currentDirectory->files); i++){
            if(i == 0){
                currentFile = List_first(currentDirectory->files);
            }
            else{
                currentFile = List_next(currentDirectory->files);
            }
            if(!currentFile->isHidden){
                int ownerNameLength = strlen(currentFile->ownerName);
                int groupNameLength = strlen(currentFile->groupName);
                int sizeLength = numDigits(currentFile->sizeOfFile);
                int numHardLinksLength = numDigits(currentFile->numOfHardLinks);
                if(ownerNameLength > maxOwnerNameLength){
                    maxOwnerNameLength = ownerNameLength;
                }
                if(groupNameLength > maxGroupNameLength){
                    maxGroupNameLength = groupNameLength;
                }
                if(sizeLength > maxSizeLength){
                    maxSizeLength = sizeLength;
                }
                if(numHardLinksLength > maxNumHardLinksLength){
                    maxNumHardLinksLength = numHardLinksLength;
                }
            }
        }
        
        //Print file information to the terminal
        for(int i=0; i<List_count(currentDirectory->files); i++){
            if(i==0){
                currentFile = List_first(currentDirectory->files);
            }
            else{
                currentFile = List_next(currentDirectory->files);
            }

            if(!currentFile->isHidden){
                //If there is a carriage return character present, remove it
                if(strlen(currentFile->name) > 0 && currentFile->name[strlen(currentFile->name)-1] == '\r'){
                    currentFile->name[strlen(currentFile->name)-1] = '\0';
                }

                //Print the first character on the permissions section
                if(currentFile->isDirectory){
                    printf("d");
                }
                else if(currentFile->isSymbolicLink){
                    printf("l");
                }
                else{
                    printf("-");
                }

                //Print the permissions of the file
                printf("%s ", decodePermissions(currentFile->permissions));

                //Print the # of hard links to the file
                printf("%*d ", maxNumHardLinksLength, currentFile->numOfHardLinks);

                //Print the name of the owner of the file
                printf("%*s ", maxOwnerNameLength, currentFile->ownerName);

                //Print the name of the group the file belongs to
                printf("%*s ", maxGroupNameLength, currentFile->groupName);

                //Print the size of the file in bytes
                printf("%*lld ", maxSizeLength, currentFile->sizeOfFile);

                //Print the date and time of most recent change to contents of the file
                printf("%s  ", currentFile->dateTimeOfMostRecentChange);

                //Print the names of all the files
                if(!currentFile->isHidden && currentFile->canBeRan && !currentFile->isDirectory && !currentFile->isSymbolicLink){
                    printf("\033[1;32m%s\033[0m\n", currentFile->name);    //Make the text green and bold if it can be ran
                }
                else if(!currentFile->isHidden && currentFile->isDirectory && !currentFile->isSymbolicLink){
                    printf("\033[1;34m%s\033[0m\n", currentFile->name);     //Make the text blue and bold if it is a folder
                }
                else if(!currentFile->isHidden && currentFile->isZip && !currentFile->isSymbolicLink){
                    printf("\033[1;31m%s\033[0m\n", currentFile->name);    //Make the text red and bold if it is a zip folder
                }

                //If the file is a symbolic link, print the text as bold and turqoise, also show the path to the file it points to
                else if(!currentFile->isHidden && currentFile->isSymbolicLink){
                    printf("\033[1;36m%s\033[0m -> %s\n", currentFile->name, currentFile->linkPath);    
                }

                else{
                    printf("%s\n", currentFile->name);
                }
            }
        }
        //Print a newline if there are any more directories to be printed
        if(j != List_count(directories)-1 && List_count(directories) > 1){
            printf("\n");
        }
    }
}
/*Prints to screen ls output when the l flag is specified*/
void ls_li(){
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
            printf("%s:\n", currentDirectory->name);
        }

        //Get the max length of the owner name, group name, and file size
        int maxOwnerNameLength = 0;
        int maxGroupNameLength = 0;
        int maxSizeLength = 0;
        int maxNumHardLinksLength = 0;
        File* currentFile;
        for(int i=0; i<List_count(currentDirectory->files); i++){
            if(i == 0){
                currentFile = List_first(currentDirectory->files);
            }
            else{
                currentFile = List_next(currentDirectory->files);
            }
            if(!currentFile->isHidden){
                int ownerNameLength = strlen(currentFile->ownerName);
                int groupNameLength = strlen(currentFile->groupName);
                int sizeLength = numDigits(currentFile->sizeOfFile);
                int numHardLinksLength = numDigits(currentFile->numOfHardLinks);
                if(ownerNameLength > maxOwnerNameLength){
                    maxOwnerNameLength = ownerNameLength;
                }
                if(groupNameLength > maxGroupNameLength){
                    maxGroupNameLength = groupNameLength;
                }
                if(sizeLength > maxSizeLength){
                    maxSizeLength = sizeLength;
                }
                if(numHardLinksLength > maxNumHardLinksLength){
                    maxNumHardLinksLength = numHardLinksLength;
                }
            }
        }
        
        //Print file information to the terminal
        for(int i=0; i<List_count(currentDirectory->files); i++){
            if(i==0){
                currentFile = List_first(currentDirectory->files);
            }
            else{
                currentFile = List_next(currentDirectory->files);
            }

            if(!currentFile->isHidden){
                //If there is a carriage return character present, remove it
                if(strlen(currentFile->name) > 0 && currentFile->name[strlen(currentFile->name)-1] == '\r'){
                    currentFile->name[strlen(currentFile->name)-1] = '\0';
                }

                //Print the i-node number of the file
                printf("%ld ", currentFile->iNodeNumber);

                //Print the first character on the permissions section
                if(currentFile->isDirectory){
                    printf("d");
                }
                else if(currentFile->isSymbolicLink){
                    printf("l");
                }
                else{
                    printf("-");
                }

                //Print the permissions of the file
                printf("%s ", decodePermissions(currentFile->permissions));

                //Print the # of hard links to the file
                printf("%*d ", maxNumHardLinksLength, currentFile->numOfHardLinks);

                //Print the name of the owner of the file
                printf("%*s ", maxOwnerNameLength, currentFile->ownerName);

                //Print the name of the group the file belongs to
                printf("%*s ", maxGroupNameLength, currentFile->groupName);

                //Print the size of the file in bytes
                printf("%*lld ", maxSizeLength, currentFile->sizeOfFile);

                //Print the date and time of most recent change to contents of the file
                printf("%s  ", currentFile->dateTimeOfMostRecentChange);

                //Print the names of all the files
                if(!currentFile->isHidden && currentFile->canBeRan && !currentFile->isDirectory && !currentFile->isSymbolicLink){
                    printf("\033[1;32m%s\033[0m\n", currentFile->name);    //Make the text green and bold if it can be ran
                }
                else if(!currentFile->isHidden && currentFile->isDirectory && !currentFile->isSymbolicLink){
                    printf("\033[1;34m%s\033[0m\n", currentFile->name);     //Make the text blue and bold if it is a folder
                }
                else if(!currentFile->isHidden && currentFile->isZip && !currentFile->isSymbolicLink){
                    printf("\033[1;31m%s\033[0m\n", currentFile->name);    //Make the text red and bold if it is a zip folder
                }
            
                //If the file is a symbolic link, print the text as bold and turqoise, also show the path to the file it points to
                else if(!currentFile->isHidden && currentFile->isSymbolicLink){
                    printf("\033[1;36m%s\033[0m -> %s\n", currentFile->name, currentFile->linkPath);    
                }

                else{
                    printf("%s\n", currentFile->name);
                }
            }
        }
        //Print a newline if there are any more directories to be printed
        if(j != List_count(directories)-1 && List_count(directories) > 1){
            printf("\n");
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
    openedDirectories = List_create();
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
        }
    }

    //If no file or directory is provided by the user, get directory information for the directory of the program
    else{
        char currentWorkingDirectory[1024];
        if(getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) == NULL){
            printf("UnixLs: Could not find the current working directory\n");
        } 
        returnDirectory = directoryReader(currentWorkingDirectory);
    }

    //Print ls output
    if(!lFlag && !iFlag){     //Standard ls (could be with -R flag)
        ls();
    }
    else if(iFlag && !lFlag){     //ls with -i flag (could be with -R flag)
        ls_i();
    }
    else if(lFlag && !iFlag){     //ls with -l flag (could be with -R flag)
        ls_l();
    }
    else if(lFlag && iFlag){      //ls with -l & -i flag (could be with -R flag)
        ls_li();    
    }

    //Close all of our opened directory streams
    DIR* currentDirectoryStream;
    for(int i=0; i<List_count(openedDirectories); i++){
        if(i==0){
            currentDirectoryStream = List_first(openedDirectories);
        }
        else{
            currentDirectoryStream = List_next(openedDirectories);
        }
        closedir(currentDirectoryStream);
    }
}