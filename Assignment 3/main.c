#include "list.h"
#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

//TODO: fix bug where first part of message typed is only part picked up by program

//---------------------------------------------OS data structures-------------------------------------------//

//The structure for a process control block
struct PCB{
    enum processState{running, ready, blocked}state;
    enum processPriority{high, medium, low}priority;
    int ID;
    char* message;      //A message our process holds
};

//The structure for a semaphore
struct semaphore{
    int val;    //The value of our semaphore
    int ID;     //The ID of our semaphore
    List* blocked;      //A list of all the processes blocked on this semaphore
};

//The structure for a message on the messages queue
struct message{
    int recvID;     //The ID of the process receiving a message
    int sendID;     //The ID of the process which sent the message
    char* message;  //The message to be sent between the processes
};

//----------------------------------------------Global variables---------------------------------------------//
int currID=1;   //This will keep track of which ID a process should be, where we increment ID for each new process created
List* lowQueue;     //The ready queue for low priority processes
List* mediumQueue;  //The ready queue for medium priority processes
List* highQueue;    //The ready queue for high priority processes
List* sendQueue;    //The queue of processes waiting on a send operation (i.e., waiting for a reply)
List* recvQueue;    //The queue of processes waiting on a receive operation
List* blockedQueue; //The queue of all blocked processes for our operating system
List* processes;    //The list of all of our processes
List* messages;     //The list of all messages waiting to be received
struct semaphore semaphores[4];   //The list of all semaphores present in the current OS instance
struct PCB* runningProcess;     //A pointer to the currently running process
int currIndex;      //The list index for round robin schedualer (which index am I on?)
struct PCB* init;   //A pointer to the process that runs at startup

//------------------------------------------------Helper Functions-------------------------------------------//

//List uses this function to see if two items are equivalent
bool compareProcesses(void* process, void* ID){
    return(((struct PCB*)process)->ID == *(int*)ID);
}

//List uses this function to see if a message is meant for a process #ID
bool compareMessages(void* message, void* ID){
    return(((struct message*)message)->recvID == *(int*)ID);
}

//This function checks whether a blocked receiver process has a message ready for it, and returns the reciever process if so
void recvOnWait(){
    if(List_count(recvQueue)>0 && List_count(messages)>0){
        struct PCB* currentProcess = (struct PCB*)List_first(recvQueue);
        while(currentProcess != NULL){
            //See if there is a message for the current process
            List_first(messages);
            struct message* message = List_search(messages, &compareMessages, &currentProcess->ID);

            //If there is a message for the current process, receive the message
            if(message != NULL){
                currentProcess->message=message->message;
                printf("Process #%d now contains message \"%s\"\n", currentProcess->ID, currentProcess->message);
                //Remove the message from the messages queue
                List_remove(messages);

                //Remove the process from the blocked queues
                List_remove(recvQueue);
                List_remove(blockedQueue);
            }
            else{
                //End the loop when there are no more processes
                currentProcess = (struct PCB*)List_next(recvQueue);
            }  
        }
    }
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
    //Set the running process to ready, and make the next process the running process
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
    process->message = "";
    currID++;
    
    List_append(processes, process);
    //If the currently running process is the init process, make this the currently runnning process, and make the init process' state ready
    if(runningProcess->ID == 0){
        runningProcess->state = ready;
        runningProcess = process;
        process->state = running;
    }
    else{
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
            printf("ERROR: Could not kill init process, as other processes exist!\n");
            return(0);
        }
    }

    //Find the process with the given ID, if it does not exist, report failure
    List_first(processes);
    struct PCB* processToKill = List_search(processes, &compareProcesses, &ID);
    if(processToKill == NULL){
        printf("ERROR: Process %d could not be found!\n", ID);
        return(0);
    }
    //Remove the process from the processes list
    assert(((struct PCB*)List_remove(processes))->ID == processToKill->ID);

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
    char* priority;
    if(processToKill->priority == high){
        queue = highQueue;
        priority = "high";
    }
    else if(processToKill->priority == medium){
        queue = mediumQueue;
        priority = "medium";
    }
    else{
        queue = lowQueue;
        priority = "low";
    }

    //Search through the priority queue, and remove the process if it is present
    List_first(queue);
    if(List_search(queue, &compareProcesses, &ID) != NULL){
        List_remove(queue);
        printf("Process %d removed from the %s priority queue\n", processToKill->ID, priority);
    }

    //Search through the blocked queue, and remove the process if it is present
    List_first(blockedQueue);
    if(List_search(blockedQueue, &compareProcesses, &ID) != NULL){
        List_remove(blockedQueue);
        printf("Process %d removed from the blocked queue\n", processToKill->ID);
    }

    //Search through the send queue, and remove the process if it is present
    List_first(sendQueue);
    if(List_search(sendQueue, &compareProcesses, &ID) != NULL){
        List_remove(sendQueue);
        printf("Process %d removed from the send queue\n", processToKill->ID);
    }

    //Search through the recv queue, and remove the process if it is present
    List_first(recvQueue);
    if(List_search(recvQueue, &compareProcesses, &ID) != NULL){
        List_remove(recvQueue);
        printf("Process %d removed from the recieve queue\n", processToKill->ID);
    }

    //Free the killed process
    free(processToKill);

    return(1);       //Report success
}

//Sends a message to process with specified ID
//Blocks the current running process until a reply is received
//Returns message sent on success, NULL upon failure
struct message* send(int ID, char* string){
    struct PCB* sendingProcess = runningProcess;
    //Find the process with the given ID, if it does not exist, report failure
    List_first(processes);
    struct PCB* receivingProcess = List_search(processes, &compareProcesses, &ID);
    if(receivingProcess == NULL){
        printf("ERROR: Process #%d could not be found!\n", ID);
        return(NULL);
    }

    //Send our message
    struct message* newMessage = malloc(sizeof(struct message));
    newMessage->sendID = sendingProcess->ID;
    newMessage->recvID = ID;
    newMessage->message = string;
    List_append(messages, newMessage);
    
    
    //Run the next process
    roundRobin();
    //If after round robin, the running is still the named one, set the running process to init
    if(sendingProcess == runningProcess){
        runningProcess = init;
        init->state = running;
    }

    //Take our sendingProcess off its waiting queue (which it was put onto by roundRobin)
    List* queue;
    char* priority;
    if(sendingProcess->priority == high){
        queue = highQueue;
        priority = "high";
    }
    else if(sendingProcess->priority == medium){
        queue = mediumQueue;
        priority = "medium";
    }
    else{
        queue = lowQueue;
        priority = "low";
    }
    List_first(queue);
    if(List_search(queue, &compareProcesses, &sendingProcess->ID) != NULL){
        List_remove(queue);
    }

    //Make our currently running process wait for a reply
    List_prepend(sendQueue, sendingProcess);
    List_prepend(blockedQueue, sendingProcess);
    sendingProcess->state = blocked;
    printf("Placed process #%d on the blocked queue and the send queue\nProcess #%d is now blocked\n", sendingProcess->ID, sendingProcess->ID);

    //Report success
    return(newMessage);
}

//The currently running process receives a message
//If there is no message currently waiting for this process, then block this process until one is available
//Returns the message recieved on success, returns NULL when the process is blocked
struct message* receive(){
    struct PCB* receivingProcess = runningProcess;
    //See if there is a message for the currently running process
    List_first(messages);
    struct message* message = List_search(messages, &compareMessages, &receivingProcess->ID);

    //If there is a message for the currently running process, receive the message and return it 
    if(message != NULL){
        receivingProcess->message=message->message;
        printf("Process #%d now contains message \"%s\"\n", receivingProcess->ID, receivingProcess->message);       
        List_remove(messages);      //Remove the message from the messages queue
        return message;
    }
    //Otherwise run the next process
    roundRobin();
    //If after round robin, the running is still the named one, set the running process to init
    if(receivingProcess == runningProcess){
        runningProcess = init;
        init->state = running;
    }

    //Take our sendingProcess off its waiting queue (which it was put onto by roundRobin)
    List* queue;
    char* priority;
    if(receivingProcess->priority == high){
        queue = highQueue;
        priority = "high";
    }
    else if(receivingProcess->priority == medium){
        queue = mediumQueue;
        priority = "medium";
    }
    else{
        queue = lowQueue;
        priority = "low";
    }
    List_first(queue);
    if(List_search(queue, &compareProcesses, &receivingProcess->ID) != NULL){
        List_remove(queue);
    }

    //Make our currently running process wait for a message
    List_prepend(recvQueue, receivingProcess);
    List_prepend(blockedQueue, receivingProcess);
    receivingProcess->state = blocked;
    printf("Placed process #%d on the blocked queue and the receive queue\nProcess #%d is now blocked\n", receivingProcess->ID, receivingProcess->ID);
    
    //Report failure 
    return(NULL);
}

//Function unlocks sender specified by ID, and sends it the specified message
//Returns the ID of the unblocked sender on success, returns a NULL pointer on failure
struct PCB* reply(int ID, char* message){
    //Find the sender on the blocked queue and the send queue, and remove it from both
    List_first(blockedQueue);
    struct PCB* sender1 = (struct PCB*)List_search(blockedQueue, compareProcesses, &ID);
    List_first(sendQueue);
    struct PCB* sender2 = (struct PCB*)List_search(sendQueue, compareProcesses, &ID);
    List_remove(blockedQueue);
    List_remove(sendQueue);

    //If the sender is not found in the send queue, return an error
    if(sender2 == NULL){
        printf("ERROR: Process with ID %d could not be found!\n", ID);
        return(NULL);
    }

    //Find the priority queue in which the sender process will reside
    List* queue;
    char* priority;
    if(sender2->priority == high){
        queue = highQueue;
        priority = "high";
    }
    else if(sender2->priority == medium){
        queue = mediumQueue;
        priority = "medium";
    }
    else{
        queue = lowQueue;
        priority = "low";
    }

    //Unblock the sender, and give it the message
    sender2->state=ready;
    sender2->message=message;
    List_prepend(queue, (void*)sender2);
    printf("Process with ID %d was added to the %s priority ready queue \n", sender2->ID, priority);
    
    printf("Process #%d recieved message \"%s\" \n", sender2->ID, sender2->message);
    return(sender2);
}

//Fucntion initializes the named semaphore with the value given
//IDs are unique and can range from 0-4
//Returns a pointer to the semaphore on success, a NULL pointer on failure
struct semaphore* newSem(int ID, int val){
    //Check the given value is valid
    if(val < 0){
        printf("ERROR: Initial semaphore value must be greater than or equal to 0\n");
        return(NULL);
    }

    //Check the given semaphore ID
    if(ID<0 || ID>4){
        printf("ERROR: Semaphore ID must range from [0, 4]\n");
        return(NULL);
    }

    //Check the semaphore is not in use
    if(semaphores[ID].ID != -1){
        printf("ERROR: Semaphore with ID #%d is already in use!\n", ID);
        return(NULL);
    }

    //Initialize the semaphore
    semaphores[ID].ID = ID;
    semaphores[ID].val = val;
    return(&semaphores[ID]);
}

//Function executes the semaphore P operation on behalf of the running process
//It is assumed that semaphore ID range from 0-4
//Returns false on failure, true on success
//TODO: ensure that the init process does not get blocked
bool semP(int ID){
    //Check that the currently running process is not the INIT process
    if(runningProcess==init){
        printf("ERROR: Cannot call p() with the init process\n");
        return(false);
    }
    //Check the given semaphore ID
    if(ID<0 || ID>4){
        printf("ERROR: Semaphore ID must range from [0, 4]\n");
        return(false);
    }

    //Check the semaphore is in use
    if(semaphores[ID].ID == -1){
        printf("ERROR: Semaphore with ID #%d is not in use!\n", ID);
        return(false);
    }

    //Decrement the semaphore
    semaphores[ID].val--;

    //Check if the running procecss should be blocked
    if(semaphores[ID].val <= -1){
        semaphores[ID].val++;   //Reset the value of the semaphore 
        struct PCB* process = runningProcess;   //Create a pointer to the currently running process
        roundRobin();   //Run the next process

        //If after round robin, the running is still the named one, set the running process to init
        if(process == runningProcess){
            runningProcess = init;
            init->state = running;
        }

        //Take our process off its waiting queue (which it was put onto by roundRobin)
        List* queue;
        char* priority;
        if(process->priority == high){
            queue = highQueue;
            priority = "high";
        }
        else if(process->priority == medium){
            queue = mediumQueue;
            priority = "medium";
        }
        else{
            queue = lowQueue;
            priority = "low";
        }
        List_first(queue);
        if(List_search(queue, &compareProcesses, &process->ID) != NULL){
            List_remove(queue);
        }

        //Block the process that called P
        List_prepend(semaphores[ID].blocked, process);
        List_prepend(blockedQueue, process);
        process->state = blocked;
        printf("Placed process #%d on both the global, and semaphore #%d's blocked queue\nProcess #%d is now blocked\n", process->ID, ID, process->ID);

        //Report success
        return(true);
    }

    //If not, report success
    printf("Semaphore #%d decremented to %d by process #%d\n", ID, semaphores[ID].val, runningProcess->ID);
}

//Function executes the semaphore V operation on behalf of the running process
//It is assumed that semaphore ID range from 0-4
//Returns false on failure, true on success
bool semV(int ID){
    //Check the given semaphore ID
    if(ID<0 || ID>4){
        printf("ERROR: Semaphore ID must range from [0, 4]\n");
        return(false);
    }

    //Check the semaphore is in use
    if(semaphores[ID].ID == -1){
        printf("ERROR: Semaphore with ID #%d is not in use!\n", ID);
        return(NULL);
    }
    
    //Increment the semaphore
    semaphores[ID].val++;
    printf("Semaphore #%d incremented to %d by process #%d\n", ID, semaphores[ID].val, runningProcess->ID);
    
    //Check if the semaphore value is positive, if so unblock a process waiting on this semaphore
    if(semaphores[ID].val > 0){
        if(List_count(semaphores[ID].blocked)>0){
            struct PCB* processToUnblock = List_trim(semaphores[ID].blocked);   //Grab our blocked process

            //Find the priority queue in which the blocked process will reside
            List* queue;
            char* priority;
            if(processToUnblock->priority == high){
                queue = highQueue;
                priority = "high";
            }
            else if(processToUnblock->priority == medium){
                queue = mediumQueue;
                priority = "medium";
            }
            else{
                queue = lowQueue;
                priority = "low";
            }

            //Unblock our blocked process
            processToUnblock->state=ready;
            List_prepend(queue, (void*)processToUnblock);
            printf("Process with ID %d was unblocked\n", processToUnblock->ID);

            //Decrement the semaphore, as now we are acting as the unblocked portion of P()
            semaphores[ID].val--;       
            printf("Semaphore #%d decremented to %d by process #%d\n", ID, semaphores[ID].val, processToUnblock->ID);
            printf("Process #%d was added to the %s priority ready queue \n", processToUnblock->ID, priority);
        }
    }
    //Return success
    return(true);
}

//Function dumps complete state information of the process with the given ID to the screen
void procinfo(int ID){
    //Find the process with the given ID, if it does not exist, report failure
    List_first(processes);
    struct PCB* process = List_search(processes, &compareProcesses, &ID);
    if(process == NULL){
        printf("ERROR: Process #%d could not be found!\n", ID);
        return;
    }

    //Find the state of the process in string form
    char* state;
    if(process->state == running){
        state = "running";
    }
    else if(process->state == ready){
        state = "ready";
    }
    else{
        state = "blocked";
    }

    //Find the queue and priority of the process in string form
    char* priority;
    if(process->priority == high){
        priority = "high";
    }
    else if(process->priority == medium){
        priority = "medium";
    }
    else{
        priority = "low";
    }

    //Print the process state information
    printf("Process #%d state: %s\n", process->ID, state);
    printf("Process #%d priority: %s\n", process->ID, priority);
    if(strlen(process->message)==0){
        printf("Process #%d contains no message\n", process->ID);
    }
    else{
        printf("Process #%d contains message: \"%s\" \n", process->ID, process->message);
    } 
}

//-------------------------------------Function to handle OS command requests----------------------------------//
void commands(char input){
    //Handle create requests
    if(input == 'C'){
        int processPriority;
        struct PCB* newProcess;
        printf("%s","Enter the desired priority of the process (0=high, 1=medium, 2=low):");
        scanf(" %d", &processPriority);
        newProcess = create(processPriority);
        if(newProcess == NULL){
            printf("ERROR: Failed to create a new process!\n");
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

    //Handle fork requests
    else if(input == 'F'){
        struct PCB* newProcess;
        newProcess = fork();
        if(newProcess == NULL){
            printf("ERROR: Failed to fork process!\n");
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

    //Handle kill requests
    else if(input == 'K'){
        int ID;
        printf("Enter the ID of the process you would like to kill:\n");
        scanf(" %d", &ID);
        bool returnVal = kill(ID);
        if(returnVal){
            printf("Successfully killed process %d\n", ID);
        }
        else{
            printf("ERROR: Could not kill process %d\n", ID);
        }
        return;
    }

    //Handle exit requests
    else if(input == 'E'){
        int ID = runningProcess->ID;
        bool returnVal = kill(ID);
        if(returnVal){
            printf("Successfully killed process %d\n", ID);
        }
        else{
            printf("ERROR: Could not kill process %d\n", ID);
        }
        return;
    }

    //Handle quantum requests
    else if(input == 'Q'){
        struct PCB* prevProcess = runningProcess;
        char* priority;
        if(prevProcess->priority == 0){
            priority="high";
        }
        else if(prevProcess->priority == 1){
            priority="medium";
        }
        else if(prevProcess->priority == 2){
            priority="low";
        }
        roundRobin();
        printf("Process #%d placed on the %s priority queue\n", prevProcess->ID, priority);
        return;
    }

    //Handle send requests
    else if(input == 'S'){
        int ID;
        char* string = malloc(sizeof(char[40]));
        printf("Enter the ID of the process you would like to send to:\n");
        scanf(" %d", &ID);
        printf("Enter the message you would like to send to process #%d:\n", ID);
        scanf(" %s", string);
        struct message* message = send(ID, string);
        if(message != NULL){
            printf("Process #%d sent message \"%s\" to process #%d\n", message->sendID, message->message, message->recvID);
        }
        else{
            printf("ERROR: Could not send message %d\n", ID);
        }
        return;
    }

    //Handle receive requests
    else if(input == 'R'){
        struct message* message = receive();
        if(message != NULL){
            printf("Process #%d recieved message \"%s\" from process #%d\n", message->recvID, message->message, message->sendID);
        }
        return;
    }

    //Handle reply requests
    else if(input == 'Y'){
        int ID;
        char* msg = malloc(sizeof(char[40]));
        printf("Enter the ID of the process you would like to reply to:\n");
        scanf(" %d", &ID);
        printf("Enter the message you would like to reply to process #%d with:\n", ID);
        scanf(" %s", msg);
        struct PCB* response = reply(ID, msg);
        if(response == NULL){
            printf("ERROR: Could not reply to process #%d with message \"%s\" \n", ID, msg);
        }
        else{
            printf("Succesfully replied to process #%d with message \"%s\" \n", response->ID, response->message);
        }
        return;
    }

    //Handle new semaphore requests
    else if(input == 'N'){
        int ID;
        int val;
        printf("Enter the ID of the semaphore you would like to initialize:\n");
        scanf(" %d", &ID);
        printf("Enter the value of the semaphore you would like to initialize:\n");
        scanf(" %d", &val);
        struct semaphore* newSemaphore = newSem(ID, val);
        if(newSemaphore == NULL){
            printf("ERROR: Could not initialize semaphore #%d with value %d \n", ID, val);
        }
        else{
            printf("Successfully initialized semaphore #%d with value %d \n", ID, val);
        }
        return;
    }

    //Handle sempahore p requests
    else if(input == 'P'){
        int ID;
        struct PCB* curr = runningProcess;
        printf("Enter the ID of the semaphore you would like to call p() on:\n");
        scanf(" %d", &ID);
        bool passed = semP(ID);
        if(passed){
            printf("Process #%d successfully called p(%d)\n", curr->ID, ID);
        }
        else{
            printf("Process #%d failed to call p(%d)\n", curr->ID, ID);
        }
        return;
    }

    //Handle sempahore v requests
    else if(input == 'V'){
        int ID;
        struct PCB* curr = runningProcess;
        printf("Enter the ID of the semaphore you would like to call v() on:\n");
        scanf(" %d", &ID);
        bool passed = semV(ID);
        if(passed){
            printf("Process #%d successfully called v(%d)\n", curr->ID, ID);
        }
        else{
            printf("Process #%d failed to call v(%d)\n", curr->ID, ID);
        }
        return;
    }

    //Handle procinfo requests
    else if(input == 'I'){
        int ID;
        printf("Enter the ID of the process you would like information on:\n");
        scanf(" %d", &ID);
        procinfo(ID);
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
    messages = List_create();

    //Setup our uninitialized semaphores
    for(int i=0; i<5; i++){
        semaphores[i].ID = -1;  //Set all of our semaphore IDs to -1 (to indicate they have not been initialized)
        semaphores[i].blocked = List_create();
    }

    //Create our init process
    init = malloc(sizeof(struct PCB));
    init->priority = high;
    init->state = running;     //The state should be running when the OS starts execution
    init->ID=0;
    init->message="";
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
        recvOnWait();       //Check if there is a message to be received, and a receiver waiting
        printf("Process with ID %d and %s priority is currently running\n", runningProcess->ID, priority);
    }
}