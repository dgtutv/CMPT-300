#include "list.c"
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void commands(char input){
    if(input == 'C'){
        int processPriority;
        printf("%s","Enter the desired priority of the process (0=high, 1=medium, 2=low):");
        scanf(" %d", &processPriority);
        printf("Created a new process with ID %d with %d priority\n", 2, processPriority);
        return;
    }
    printf("%s","That command does not exist!\n");
    return;
}
int main(){
    char buffer;
    while(1){
        //Get user input
        printf("%s","user@OS:~$ ");
        while((buffer = getchar()) != '\n' && buffer != EOF){
        	//Handle the input
        	commands(buffer);
        }

        
    }
}
