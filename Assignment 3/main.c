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
//Returns -1 on failure, returns the ID of the process created otherwise
//Priorities: 0=high, 1=medium, 2=low
int create(int priority){
    if(priority < 0 || priority > 2){
        return -1;  //Report failure if an invalid priority level is passed
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

    //While the program is running, take input
    int retVal;
    int processPriority;
    int numBytes=0;
    while(1){
        //clear numBytes
        numBytes=0;

        //Clear the buffer
        char buffer;

        //Await keyboard input
        printf("user@simulation $ ");
        while(numBytes == 0){
            numBytes=read(0, buffer, 1);
        }
        if(){
            scanf("Enter the desired priority of the process (0=high, 1=medium, 2=low):\n", &processPriority);
            retVal = create(processPriority);
            if(retVal == -1){
                printf("Failed to create a new process!\n");
            }
            else{
                printf("Created a new process with ID %d\n", retVal);
            }
        }
        else{
            printf("That command does not exist!\n");
        }
    }
}