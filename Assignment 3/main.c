#include "list.c"
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//----------------------------------------------Global variables---------------------------------------------//
int currID=1;   //This will keep track of which ID a process should be, where we increment ID for each new process created
List* lowQueue;     //The ready queue for low priority processes
List* mediumQueue;  //The ready queue for medium priority processes
List* highQueue;    //The ready queue for high priority processes
List* sendQueue;    //The queue of processes waiting on a send operation (i.e., waiting for a reply)
List* recvQueue;    //The queue of processes waiting on a receive operation
List* blockedQueue; //The queue of all blocked processes for our operating system


//---------------------------------------------OS data structures-------------------------------------------//

//The structure for a process control block
struct PCB{
    enum processState{running, ready, blocked}state;
    enum processPriority{high, medium, low}priority;
    int ID;
    char* message;      //A message our process holds
};

//The structure for a semaphore
struct sem{
    int val;    //The value of our semaphore
    List* waitingProcesses; //A list of all the processes waiting on this semaphore
};

//------------------------------------------The functions for our commands-----------------------------------//

//Creates a process and puts it on the appropiate ready queue
//Returns NULL on failure, returns a pointer to the thread otherwise
//Priorities: 0=high, 1=medium, 2=low
struct PCB* create(int priority){
    if(priority < 0 || priority > 2){
        return NULL;  //Report failure if an invalid priority level is passed
    }
    struct PCB* process = malloc(sizeof(struct PCB));
    process->priority = priority;
    process->state = ready;
    process->ID = currID;
    currID++;
    if(process->priority == low){
        List_prepend(lowQueue, process);
    }
    else if(process->priority == medium){
        List_prepend(mediumQueue, process);
    }
    else{
        List_prepend(highQueue, process);
    }
    return(process);
}

//Function to handle user-command requests
void commands(char input){
    if(input == 'C'){
        int processPriority;
        struct PCB* newProcess;
        printf("%s","Enter the desired priority of the process (0=high, 1=medium, 2=low):");
        scanf(" %d", &processPriority);
        newProcess = create(processPriority);
        if(newProcess == NULL){
            printf("Failed to create a new process!\n");
            return;
        }
        else{
            char* priority;
            if(newProcess->priority == 0){
                priority="high";
            }
            else if(newProcess->priority == 1){
                priority="medium";
            }
            else if(newProcess->priority == 2){
                priority="low";
            }
            printf("Created a new process with ID %d with %s priority\n", newProcess->ID, priority);
            return;
        }
        return;
    }
    printf("%s","That command does not exist!\n");
    return;
}

//-----------------------------------------------------Main--------------------------------------------------//

int main(int argc, char* argv[]){
    //Initialize each of our queues
    lowQueue = List_create();    
    mediumQueue = List_create();  
    highQueue = List_create();    
    sendQueue = List_create();    
    recvQueue = List_create();    
    blockedQueue = List_create(); 

    //Create our init process
    struct PCB init;
    init.priority = high;
    init.state = running;     //The state should be running when the OS starts execution
    init.ID=0;

    char buffer;
    while(1){
        printf("%s","user@OS:~$ ");
        while((buffer = getchar()) != '\n' && buffer != EOF){   //Get user input
        	commands(buffer);   //Handle the input
        }
    }
}