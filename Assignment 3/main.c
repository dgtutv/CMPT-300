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
List* processes;    //The list of all of our processes
struct PCB* runningProcess;     //A pointer to the currently running process
int currIndex;      //The list index for round robin schedualer (which index am I on?)
struct PCB* init;   //A pointer to the process that runs at startup

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

//------------------------------------------------Helper Functions-------------------------------------------//

//List uses this function to see if two items are equivalent
bool compareProcesses(void* process, void* ID){
    return(((struct PCB*)process)->ID == (int)ID);
}


//----------------------------------------------Round Robin Schedualer---------------------------------------//
void roundRobin(){
    struct PCB* nextProcess;        //Pointer to the process that will run next
    //If the current running process is of high priority
    if(runningProcess->priority == high){
        //If the medium queue is not empty, take the next process from there
        if(List_count(mediumQueue) > 0){
            nextProcess = List_trim(mediumQueue);
        }
        //If the medium queue is empty, and the low queue is not, take the process from the low queue
        else if(List_count(lowQueue) > 0){
            nextProcess = List_trim(lowQueue);
        }
        //If the medium and low queues are empty, take the process from the high queue
        else if(List_count(highQueue) > 0){
            nextProcess = List_trim(highQueue);
        }
        //If all queues are empty, do nothing
        else{
            return;
        }
        //Add the current process to the high queue
        List_prepend(highQueue, runningProcess);
    }
    //If the current running process is of medium priority
    else if(runningProcess->priority == medium){
        //If the low queue is not empty, take the next process from there
        if(List_count(lowQueue) > 0){
            nextProcess = List_trim(lowQueue);
        }
        //If the low queue is empty, and the high queue is not, take the process from the high queue
        else if(List_count(highQueue) > 0){
            nextProcess = List_trim(highQueue);
        }
        //If the low and high queues are empty, take the process from the medium queue
        else if(List_count(mediumQueue) > 0){
            nextProcess = List_trim(mediumQueue);
        }
        //If all queues are empty, do nothing
        else{
            return;
        }
        //Add the current process to the medium queue
        List_prepend(mediumQueue, runningProcess);
    }
    //If the current running process is of low priority
    else{
        //If the high queue is not empty, take the next process from there
        if(List_count(highQueue) > 0){
            nextProcess = List_trim(highQueue);
        }
        //If the high queue is empty, and the medium queue is not, take the process from the medium queue
        else if(List_count(mediumQueue) > 0){
            nextProcess = List_trim(mediumQueue);
        }
        //If the high and medium queues are empty, take the process from the low queue
        else if(List_count(lowQueue) > 0){
            nextProcess = List_trim(lowQueue);
        }
        //If all queues are empty, do nothing
        else{
            return;
        }
        //Add the current process to the low queue
        List_prepend(lowQueue, runningProcess);
    }
    //Set the runnign process to ready, and make the next process the running process
    runningProcess->state = ready;
    nextProcess->state = running;
    runningProcess = nextProcess;
}

//------------------------------------------The functions for our commands--------------------------------------------//

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
    List_append(processes, process);
    //If the currently running process is the init process, make this the currently runnning process, and make the init process' state ready
    if(runningProcess->ID == 0){
        runningProcess->state = ready;
        runningProcess = process;
        process->state = running;
    }
    return(process);
}

//Copies the currently running process and puts it on the ready queue corresponding to the original process' priority
//Attempting to Fork the init process should fail
//Returns a pointer to the new process on success, returns NULL on failure
struct PCB* fork(){
    //Attempting to fork the init process should fail
    if(runningProcess->ID == 0){
        return(NULL);
    }
    struct PCB* process = malloc(sizeof(struct PCB));
    process->priority = runningProcess->priority;
    process->state = ready;
    process->ID = currID;
    currID++;
    if(runningProcess->priority == low){
        List_prepend(lowQueue, process);
    }
    else if(runningProcess->priority == medium){
        List_prepend(mediumQueue, process);
    }
    else{
        List_prepend(highQueue, process);
    }
    List_append(processes, process);
    return(process);
}

//Kills the named process and removes it from the system
//Returns 1 on success, 0 on failure
//If the named process is the init process, end program iff there are no other processes
bool kill(int ID){

    //If the named process is the init process
    if(ID == 0){
        //If there are no other processes, end the program
        if(List_count(processes) == 1){
            exit(EXIT_SUCCESS);
        }
        //Otherwise, return failure
        else{
            return(0);
        }
    }

    //Find the process with the given ID, if it does not exist, report failure
    struct PCB* processToKill = List_search(processes, &compareProcesses, ID);
    if(processToKill == NULL){
        return(0);
    }

    //If the named process is the running process, set the next running process
    if(processToKill == runningProcess){
        roundRobin();
        //If after round robin, the running is still the named one, set the running process to init
        if(processToKill == runningProcess){
            runningProcess = init;
            init->state = running;
        }
    }
    
    //Find the priority queue in which the named process may reside
    List* queue;
    if(processToKill->priority == high){
        queue = highQueue;
    }
    else if(processToKill->priority == medium){
        queue = mediumQueue;
    }
    else{
        queue = lowQueue;
    }

    //Search through the priority queue, and remove the process if it is present
    if(List_search(queue, &compareProcesses, ID) != NULL){
        List_remove(queue);
    }

    //Search through the blocked queue, and remove the process if it is present
    if(List_search(blockedQueue, &compareProcesses, ID) != NULL){
        List_remove(blockedQueue);
    }

    //Search through the send queue, and remove the process if it is present
    if(List_search(sendQueue, &compareProcesses, ID) != NULL){
        List_remove(sendQueue);
    }

    //Search through the recv queue, and remove the process if it is present
    if(List_search(recvQueue, &compareProcesses, ID) != NULL){
        List_remove(recvQueue);
    }

    //Remove the process from the processes list
    List_remove(processes);
    return(1);       //Report success
}

//-------------------------------------Function to handle OS command requests----------------------------------//
void commands(char input){
    //Handle create request
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
            printf("Created a new process with ID %d and added it to the %s priority ready queue\n", newProcess->ID, priority);
            return;
        }
        return;
    }

    //Handle fork request
    else if(input == 'F'){
        struct PCB* newProcess;
        newProcess = fork();
        if(newProcess == NULL){
            printf("Failed to fork process!\n");
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
            printf("Forked process ID %d, new process has ID %d and has been added to the %s priority ready queue\n", runningProcess->ID, newProcess->ID, priority);
            return;
        }
        return;
    }

    //Handle invalid requests
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
    processes = List_create();

    //Create our init process
    init = malloc(sizeof(struct PCB));
    init->priority = high;
    init->state = running;     //The state should be running when the OS starts execution
    init->ID=0;
    List_append(processes, init);
    runningProcess = init;
    printf("Created the init process with process ID 0, high priority, and state set to running\n");

    char buffer;
    while(1){
        printf("%s","user@OS:~$ ");
        while((buffer = getchar()) != '\n' && buffer != EOF){   //Get user input
        	commands(buffer);   //Handle the input
        }
        //Mention which process is currently running
        char* priority;
        if(runningProcess->priority == 0){
            priority="high";
        }
        else if(runningProcess->priority == 1){
            priority="medium";
        }
        else if(runningProcess->priority == 2){
            priority="low";
        }
        printf("Process with ID %d and %s priority is currently running\n", runningProcess->ID, priority);
    }
}