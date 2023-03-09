#include "list.c"
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//-----------------------------------------------OS data structures-------------------------------------------//

//The structure for a process control block
struct PCB{
    enum processState{running, ready, blocked}state;
    enum processPriority{low, medium, high}priority;
    int ID;
    char* message;      //A message our process holds
};

//The structure for a semaphore
struct sem{
    int val;    //The value of our semaphore
    List* waitingProcesses; //A list of all the processes waiting on this semaphore
};

//------------------------------------------The functions for our commands-----------------------------------//

bool create(int priority){
    
}

//-----------------------------------------------------Main--------------------------------------------------//

int main(int argc, char* argv[]){
    //Initialize each of our queues
    List* lowQueue = List_create();     //The ready queue for low priority processes
    List* mediumQueue = List_create();  //The ready queue for medium priority processes
    List* highQueue = List_create();    //The ready queue for high priority processes
    List* sendQueue = List_create();    //The queue of processes waiting on a send operation (i.e., waiting for a reply)
    List* recvQueue = List_create();    //The queue of processes waiting on a receive operation
    List* blockedQueue = List_create(); //The queue of all blocked processes for our operating system

    //Create our init process
    struct PCB init;
    init.priority = high;
    init.state = ready;     //The state should be ready when the OS starts execution
    init.ID=0;

    int currID=1;   //This will keep track of which ID a process should be, where we increment ID for each new process created


    printf("Program runs \n");
}