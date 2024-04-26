#include <stdio.h> //for printf
#include <stdlib.h>
#include <pthread.h> //for threads
#include <unistd.h> //
#include <stdbool.h>// use of boolean variables
#include "list.c"
/*------------------------------------------Network Header Files Include------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>

/*---------------------------------------END Network Header Files Include------------------------------------------*/

/*---------------------------------------Function to free items on a list------------------------------------------*/
void freeItem(void* item){
    free(item);
}

/*-----------------------------------------Network structs and Variables-------------------------------------------*/
struct addrinfo local_addrinfo; //struct containing our systems information
struct addrinfo *local_addrinfo_p, *p, *p2, *remote_addrinfo_p; //Our addrinfo pointers
struct addrinfo remote_addrinfo; //struct containing the information of the system we wish to talk to
int localSocketFD, remoteSocketFD;  //File directories for our sockets
bool sentEndRequest = false;
/*---------------------------------------END Network structs and Variables-----------------------------------------*/

/*------------------------------------------------Global Variables-------------------------------------------------*/
size_t BUFFER_SIZE = 256;   //Size specified by TA
bool user_close = false;
List *send_list;
List *print_list;
char *remoteMachineName, *localPortNumber, *remotePortNumber;
/*----------------------------------------------END Global Variables-----------------------------------------------*/

/*-----------------------------------------------------Threads-----------------------------------------------------*/
pthread_mutex_t send_list_mutex;
pthread_mutex_t print_list_mutex;
//The keyboard input thread, on receipt of input, adds the input to the list of messages
//that need to be sent to the remote s-talk client. 
//producer: for messages on the send list
void * Keylistener_thread_func(void * thread_id){

    long * id = (long *)thread_id;
    int numBytes=0;
    int bufLen=0;


    //run indefinitely as long as the user hasn't given '!' as input
    while(1){
        char message[0];
        //clear numBytes
        numBytes=0;

        //Clear the buffer
        char buffer[BUFFER_SIZE];

        //Await keyboard input
        while(numBytes == 0){
            numBytes=read(0, buffer, BUFFER_SIZE);
            if(numBytes == -1){
                printf("ERROR in keylistener!\n");
            }
        }

        //proceed to handle the keylistener functionality
        pthread_mutex_lock(&send_list_mutex);   //control access to the critical section with a mutex
        List_append(send_list, buffer);     //now that this thread has the lock we can manipulate the list
        pthread_mutex_unlock(&send_list_mutex);     //give back the lock so the other threads may proceed

        //check to see if the user wants to close the program
        //this is when they input the character '!'
        if((buffer[0] == '!') && (numBytes == 2)){
            user_close = true;
        }
    }
}

//The UDP input thread, on receipt of input from the remote s-talk client, will put the
//message onto the list of messages that need to be printed to the local screen. 
//producer: for the print list
void * UDP_input_thread_func(void* thread_id){
    int localStatus, localSocketFD;

    //Get our local address info
    memset(&local_addrinfo, 0, sizeof local_addrinfo); // make sure the struct is empty
    local_addrinfo.ai_family = AF_INET;		//Specified by TA tutorial
    local_addrinfo.ai_socktype = SOCK_DGRAM;        //UDP sockets
    local_addrinfo.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, localPortNumber, &local_addrinfo, &local_addrinfo_p) != 0){//If this fails, address could not be found
        printf("ERROR: cannot find address of local port\n");
        exit(EXIT_FAILURE);
    }

    //Loop through all of our results until we can grab a socket
    for(p = local_addrinfo_p; p!=NULL; p=p->ai_next){
        localSocketFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(localSocketFD != -1){  //If we got a valid socket file descriptor, attempt to bind to socket
            if(-1 == bind(localSocketFD, p->ai_addr, p->ai_addrlen)){
                //If the socket failed to bind, throw an error and close the file descriptor
                close(localSocketFD);
                printf("Local port failed to bind socket!\n");
                exit(EXIT_FAILURE);
            }
            else{
                break;
            }
        }
    }

    long * id = (long *)thread_id;
    int numBytes;
    int bufLen;

    while(1){
        //Clear our buffer everytime
        char buffer[BUFFER_SIZE];
        //Await UDP input
        numBytes = recvfrom(localSocketFD, buffer, BUFFER_SIZE, 0, (struct sockaddr *)p, sizeof(p));
        fflush(stdin);
        //End code at \0 termination
        bufLen = strlen(buffer);
        char message[]="";
        for(int i=0; i<bufLen; i++){
            if(buffer[i] == '\n' || buffer[i] == '\0'){
                break;
            }
            else{
                strncat(message, &buffer[i], 1);
            }
        }
        //Input is recieved, continue to write to list
        pthread_mutex_lock(&print_list_mutex);      //control access to the critical section with a mutex
        List_append(print_list, message);
        //give back the lock so other threads may proceed 
        pthread_mutex_unlock(&print_list_mutex);
    }
}

//The UDP output thread will take each message off this list and send it over the network
//to the remote client. 
//consumer: for the send list
void * UDP_output_thread_func(void * thread_id){
    int remoteStatus, remoteSocketFD;

    //Get our remote address info
    memset(&remote_addrinfo, 0, sizeof remote_addrinfo); // make sure the struct is empty
    remote_addrinfo.ai_family = AF_INET;		//Specified by TA tutorial
    remote_addrinfo.ai_socktype = SOCK_DGRAM;        //UDP sockets

    if(getaddrinfo(remoteMachineName, remotePortNumber, &remote_addrinfo, &remote_addrinfo_p) != 0){//If this fails, address could not be found
        printf("ERROR: cannot find address of local port\n");
        exit(EXIT_FAILURE);
    }

    for(p2 = remote_addrinfo_p; p2!=NULL; p2=p2->ai_next){
        remoteSocketFD = socket(p2->ai_family, p2->ai_socktype, p2->ai_protocol);
        if(remoteSocketFD != -1){  //If we got a valid socket file descriptor, the thread can start sending data
            break;
        }
    }
    
    int numberOfSentBytes;


    while(1){
        //Reset our message
        char* message;  //Do not need to allocate this to memory, as it has been allocated by keyListener already

        //control access to the critical section with a mutex
        pthread_mutex_lock(&send_list_mutex);
            if(List_count(send_list) > 0){
                //now that this thread has the lock we can manipulate the list
                message = List_trim(send_list);

                //Reset the number of sent bytes to 0
                numberOfSentBytes = 0;

                //Keep sending data until the whole message is sent, or an error occurs

                //send the message over the datagram sockets
                numberOfSentBytes += sendto(remoteSocketFD, message, strlen(message), 0, p2->ai_addr, p2->ai_addrlen);
                if(numberOfSentBytes == -1){
                    printf("sendto ERROR!\n");
                }

                if((strcmp(message, "!")) && (strlen(message)==2)){
                    pthread_mutex_unlock(&send_list_mutex);
                    sentEndRequest = true;
                    return;
                }
            }
        //give back the lock so the other threads may proceed
        pthread_mutex_unlock(&send_list_mutex); 
    }
}

//The screen output thread will take each message off this list and output it to the screen. 
//consumer: for the print list
void * printer_thread_func(void * thread_id){
    long * id = (long *)thread_id;

    while(1){
        //Clear our message
        char* message;

        //control access to the critical section with a mutex
        pthread_mutex_lock(&print_list_mutex);

            if(List_count(print_list) > 0){
                fflush(stdout);
                message = List_trim(print_list);
                if((message[0]=='!') && (strlen(message)==1)){
                    pthread_mutex_unlock(&print_list_mutex);
                    user_close = true;
                    sentEndRequest = true;
                    return;
                }
                fflush(stdout);
                printf("Message received: %s\n", message);
                fflush(stdout);
            }

        //give back the lock so other threads may proceed 
        pthread_mutex_unlock(&print_list_mutex);
    }
}

/*-----------------------------------------------------END Threads--------------------------------------------------*/

void main(int argc, char* argv[]){


    // Taking command-line arguments
    // We must ensure that there are four command line arguments which are detailed below
    // otherwise we cannot run the program
    if(argc != 4){
        printf("Correct command to run: ./s-talk [local port number] [remote machine name] [remote port number]\n");
        return;
    }
    localPortNumber = argv[1];
    remoteMachineName = argv[2];
    remotePortNumber = argv[3];
    printf("The local port number: %s, machine name: %s, and the remote port number: %s\n", localPortNumber, remoteMachineName, remotePortNumber);


    send_list = List_create();
    print_list = List_create();

    //producing thread id's for the four threads needed for this assignment
    pthread_t Keylistener_thread; //takes keyboard input and adds it to the to_be_sent list
    pthread_t UDP_input_thread; //gets messages from UDP_output and add them to the to_be_printed list
    pthread_t UDP_output_thread; //takes message off of the to_bo_sent list and gives them to UDP_input
    pthread_t printer_thread; //takes messages off of the to_be_printed list and outputs them to the screen

    //initialise our mutex variables for access to critical sections within the four threads
    pthread_mutex_init(&send_list_mutex, NULL);
    pthread_mutex_init(&print_list_mutex, NULL);

    //Create and instantiate the keyListener thread
    long returnValue;
    long keyListenerIdentifier = 1;
    returnValue = pthread_create(&Keylistener_thread, NULL, Keylistener_thread_func, (void *)&keyListenerIdentifier);
    if(returnValue){
        printf("pthread_create() ERROR for keyListener\n");
    }

    //Create and instantiate the printer thread
    long printerIndentifier = 2;
    returnValue = pthread_create(&printer_thread, NULL, printer_thread_func, (void *)&printerIndentifier);
    if(returnValue){
        printf("pthread_create() ERROR for printer");
        return;
    }

    //Create and instantiate the networkListener thread
    long networkListenerIdentifier = 3;
    returnValue = pthread_create(&UDP_input_thread, NULL, UDP_input_thread_func, (void *)&networkListenerIdentifier);
    if(returnValue){
        printf("pthread_create() ERROR for networkListener");
        return;
    }

    //Create and instantiate the networkTalker thread
    long networkTalkerIdentifier = 4;
    returnValue = pthread_create(&UDP_output_thread, NULL, UDP_output_thread_func, (void *)&networkTalkerIdentifier);
    if(returnValue){
        printf("pthread_create() ERROR for printer");
        return;
    }
    while(user_close == false);
    //clean up the mutexes and end the threads
    //only once the user decides they are done with s-talk
    pthread_cancel(Keylistener_thread);
    pthread_cancel(printer_thread);
    pthread_cancel(UDP_input_thread);
    if(sentEndRequest){
        pthread_cancel(UDP_output_thread);
    }
    pthread_mutex_destroy(&send_list_mutex);
    pthread_mutex_destroy(&print_list_mutex);
    close(localSocketFD);
    close(remoteSocketFD);
    List_free(send_list, &freeItem);
    List_free(print_list, &freeItem);

    freeaddrinfo(local_addrinfo_p);
    freeaddrinfo(remote_addrinfo_p);
    exit(EXIT_SUCCESS);

}