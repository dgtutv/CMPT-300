#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

//Our manager for our list
static Manager manager;

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.

List* List_create(){
    if(manager.nodes == 0){//If this is the first time List_create is called, setup our Manager, List and Node structs

        //Setup our out of bounds variables
        manager.outOfBoundsStart = LIST_OOB_START;
        manager.outOfBoundsEnds = LIST_OOB_ENDS;

        //Setup our nodes
        Node nodeArr[LIST_MAX_NUM_NODES];
        manager.nodes = nodeArr;
        manager.freeNodes = manager.nodes;
        manager.freeNodes->next = &nodeArr[1];
        manager.freeNodes->index = 0;
        Node* curr = &manager.nodes[0];
        for(int i=1; i<LIST_MAX_NUM_NODES-1; i++){
            curr=curr->next;
            curr->index=i;
            curr->next = &manager.nodes[i+1];
            curr->prev = &manager.nodes[i-1];
        }
        curr->next->index = LIST_MAX_NUM_NODES-1;
        curr->next->prev = curr;
        
        //Setup our heads
        List listArr[LIST_MAX_NUM_HEADS];
        manager.heads = listArr;
        manager.freeHeads = manager.heads;
        manager.freeHeads->next = &manager.heads[1];
        manager.freeHeads->index = 0;
        List* currL = &manager.heads[0];
        for(int i=1; i<LIST_MAX_NUM_HEADS-1; i++){
            currL=currL->next;
            currL->index=i;
            currL->next = &manager.heads[i+1];
            currL->prev = &manager.heads[i-1];
        }
        currL->next->index = LIST_MAX_NUM_HEADS-1;
        currL->next->prev = currL;
    }

    if(manager.freeHeads != 0){   //If possible, return a new head pointer (not possible when heads are all in use)

        //Set head to next available head (the start of our freeHeads list)
        List* head = manager.freeHeads;  

        //Fully disconnect head from freeHeads list
        //If the freeHeads list is of size 1, set the freeHeads list to NULL
        if(head->next == 0){
            manager.freeHeads = 0;
        }
        //Otherwise, disconnect just the head of the freeHeads
        else{
            head->next->prev = 0;  
            manager.freeHeads = head->next;
        }
        manager.heads[manager.numHeads-1] = *head;   //Add the head to the heads array
        manager.numHeads++;         //Increment our numHeads counter
        head->size = 0;
        return(head);
    }

    else{
        return 0;
    }
}

// Returns the number of items in pList.
int List_count(List* pList){
    return(pList->size);
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    if(pList->head == 0){    //If the list is empty
        pList->current = 0; //Set the current Node to NULL
        pList->currentItem = 0; //Set the current item to NULL
        return(0);  //Return NULL
    }
    void* returnVal = pList->head->item;  //Pointer to first item in pList
    pList->currentItem = pList->head->item;   //Make the current item the first item
    pList->current = pList->head;       //Make the current Node the first Node
    return(returnVal);
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    if(pList->head == 0){    //If the list is empty
        pList->current = 0; //Set the current Node to NULL
        pList->currentItem = 0; //Set the current item to NULL
        return(0);  //Return NULL
    }
    void* returnVal = pList->tail->item;  //Pointer to last item in pList
    pList->current = pList->tail;   //Make the current Node the last Node
    pList->currentItem = pList->current->item;  //Make the current item the last item
    return(returnVal);
}
// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    //If operation advances current item beyond the end of the pList, or the pList is empty, set current item to be beyond end of pList, and Return a NULL pointer
    if(pList->current == pList->tail || (pList->head == 0 && pList->tail == 0) || pList->currentItem == &manager.outOfBoundsEnds){    
        pList->currentItem = &manager.outOfBoundsEnds;
        return(0);
    }
    pList->current = pList->current->next; //Advance pList's current Node by one
     pList->currentItem = pList->current->item; //Advance pList's current item by one
    return(pList->current->item);  //Return a pointer to the new current item
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    //If operation backs up the current item beyond the start of the pList, set current item to be before the start of pList, and return a NULL pointer
    if(pList->current == pList->head || (pList->head == 0 && pList->tail == 0) || pList->currentItem == &manager.outOfBoundsStart){       
        pList->currentItem = &manager.outOfBoundsStart;   
        return(0);
    }
    pList->current = pList->current->prev; //Backs up pList's current Node by one
    pList->currentItem = pList->current->item; //Backs up pList's current item by one
    return(pList->current->item);  //Return a pointer to the new current item
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    //If the pList is empty, return NULL
    if(pList->head == 0 && pList->tail == 0){return(0);}
    return(pList->currentItem);
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem){
    if(manager.freeNodes==0){    //If there are no free nodes left to use
        return(-1);
    }
    else{
        pList->size++;      //Increment the size of pList
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == 0){
            manager.freeNodes = 0;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = 0;            
            manager.freeNodes = manager.freeNodes->next;
        }

        newNode->item = pItem;      //Make the newNode's item the item provided

        //If the current pointer is before the start of the pList, insert the newNode at the start of the list
        if(pList->currentItem == &manager.outOfBoundsStart){
            newNode->next = pList->head;
            pList->head->prev = newNode;
            pList->head = newNode;
        }

        //If the current pointer is beyond the end of the pList OR current is the end of the list, insert the newNode at the end of the list
        else if(pList->currentItem == &manager.outOfBoundsEnds || pList->current == pList->tail){
            newNode->prev = pList->tail;
            pList->tail->next = newNode;
            pList->tail = newNode;
        }

        //If the list is empty, make the item the head, tail, and current
        else if(pList->head == 0 && pList->tail == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        else{
            //Add the new item directly after the current item
            pList->current->next->prev=newNode;
            newNode->next = pList->current->next;
            newNode->prev = pList->current;
            pList->current->next = newNode;     
        }
        pList->current = newNode;    //Make newNode the current Node
        pList->currentItem = pList->current->item;    //Update current item
        return(0);
    } 
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem){
    if(manager.freeNodes==0){    //If there are no free nodes left to use
        return(-1);
    }
    else{
        pList->size++;      //Increment the size of pList
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == 0){
            manager.freeNodes = 0;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = 0;            
            manager.freeNodes = manager.freeNodes->next;
        }

        newNode->item = pItem;      //Make the newNode's item the item provided

        //If the current pointer is before the start of the pList OR current is the start of the list, insert the newNode at the start of the list
        if(pList->currentItem == &manager.outOfBoundsStart || pList->current == pList->head){
            newNode->next = pList->head;
            pList->head->prev = newNode;
            pList->head = newNode;
        }

        //If the current pointer is beyond the end of the pList, insert the newNode at the end of the list
        else if(pList->currentItem == &manager.outOfBoundsEnds){     
            newNode->prev = pList->tail;
            pList->tail->next = newNode;
            pList->tail = newNode;
        }

        //If the list is empty, make the item the head, tail, and current
        else if(pList->head == 0 && pList->tail == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        else{
            //Add the new item directly before the current item
            pList->current->prev->next=newNode;
            newNode->next = pList->current;
            newNode->prev = pList->current->prev;
            pList->current->prev = newNode;     
        }
        pList->current = newNode;    //Make newNode the current Node
        pList->currentItem = pList->current->item;    //Update current item
        return(0);
    } 
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){
    if(manager.freeNodes==0){    //If there are no free nodes left to use
        return(-1);
    }
    else{
        pList->size++;      //Increment the size of pList
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == 0){
            manager.freeNodes = 0;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = 0;            
            manager.freeNodes = manager.freeNodes->next;
        }

        newNode->item = pItem;      //Make the newNode's item the item provided

        //If the list is empty, make the item the head, tail, and current
        if(pList->head == 0 && pList->tail == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        //Otherwise, add the newNode (with the new item) to the end of pList
        else{
            pList->tail->next = newNode;
            newNode->prev = pList->tail;
            pList->tail = newNode;
        }

        pList->current = newNode;    //Make newNode the current Node
        pList->currentItem = pList->current->item;    //Update current item
        return(0);
    }
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    if(manager.freeNodes==0){    //If there are no free nodes left to use
        return(-1);
    }
    else{
        pList->size++;      //Increment the size of pList
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == 0){
            manager.freeNodes = 0;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = 0;            
            manager.freeNodes = manager.freeNodes->next;
        }
        
        newNode->item = pItem;      //Make the newNode's item the item provided

        //If the list is empty, make the item the head, tail, and current
        if(pList->head == 0 && pList->tail == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        //Otherwise, add the newNode (with the new item) to the end of pList
        else{
            pList->head->prev = newNode;
            newNode->next = pList->head;
            pList->head = newNode;
        }

        pList->current = newNode;    //Make newNode the current Node
        pList->currentItem = pList->current->item;    //Update current item
        return(0);
    }
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){

    //If the pList is empty, return NULL
    if(pList->head == 0 && pList->tail == 0){
        return(0);
    }

    //If the current pointer is before the start of pList, or beyond the end of pList, return NULL
    else if(pList->currentItem == &manager.outOfBoundsStart || pList->currentItem == &manager.outOfBoundsEnds){
        return(0);
    }
    else{
        pList->size--;      //Decrement the size of pList
        void* returnVal = pList->current->item;     //Get the current item to be returned
        pList->current->item = 0;   //Reset the current item to NULL       

        Node* oldNode = pList->current;

        //If current is head and tail (the list is of size 1), simply set head and tail to NULL to disconnect oldNode from the list
        if(oldNode == pList->head && oldNode == pList->tail){
            pList->head = 0;
            pList->tail = 0;
            pList->current = 0;     //Make the current item set to NULL
        }

        //Otherwise, if current is head simply disconnect the head and make current->next the new head to diconnect oldNode from the list
        else if(oldNode == pList->head){      
            oldNode->next->prev = 0;
            pList->head = oldNode->next;
            pList->current = pList->current->next;       //Make the next Node the current one
            pList->currentItem = pList->current->next->item;       //Make the next item the current one
        }

        //Otherwise, if current is the tail, simply disconnect the tail and make current->prev the new tail to disconnect oldNode from the list
        else if(oldNode== pList->tail){
            oldNode->prev->next = 0;
            pList->tail = oldNode->prev;
            pList->current = pList->tail;    //Make the new last Node the current one
            pList->currentItem = pList->tail->item;    //Make the new last item the current one
        }

        //Otherwise, take the old node out of the pList normally
        else{
            oldNode->prev->next = oldNode->next;
            oldNode->next->prev = oldNode->prev;
            pList->current = pList->current->next;       //Make the next Node the current one
            pList->current->item = pList->current->next->item;       //Make the next item the current one
        }   

        oldNode->prev = 0;

        //If the freeNodes list is empty, make the oldNode the head of the freeNodes list
        if(manager.freeNodes == 0){       
            oldNode->next = 0;
            manager.freeNodes = oldNode;
        }

        //Otherwise, ove the old node into freeNodes
        else{
            manager.freeNodes->prev = oldNode;
            oldNode->next = manager.freeNodes;
            manager.freeNodes = oldNode;
        }
        return(returnVal);
    }
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){

    //If the pList is empty, return NULL
    if(pList->head == 0 && pList->tail == 0){
        return(0);
    }

    //Otherwise, set current to the tail, pass control off to List_remove()
    else{
        pList->current = pList->tail;
        pList->currentItem = pList->current->item;
        return(List_remove(pList));
    }
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2){
    //If pList2 is not empty
    if(pList2->head != 0 && pList2->tail != 0){
        pList1->size += pList2->size;       //Update the size of pList1
        //If pList1 is empty, move pList2 to pList1
        if(pList1->head == 0 && pList1->tail == 0){
            pList1->tail = pList2->tail;
            pList1->head = pList2->head;
            pList1->current = pList2->current;
        }
        //Otherwise, add pList2 to the end of pList1
        else{
            pList1->tail->next = pList2->head;
            pList2->head->prev = pList1->tail;
            pList1->tail = pList2->tail;
        }
    }

    List_free(pList2, List_remove);     //Delete pList2
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    //While pList remains non-empty, free its tail
    while(pList->head!=0 && pList->tail!=0){
        pList->current = pList->tail;
        pList->current->item = pList->tail->item;
        (*pItemFreeFn)(pList->current);     //Free the memory of the item
        List_remove(pList);     //Free the Node itself
    }
    pList->head = 0;
    pList->tail = 0;
    pList->current = 0;
    pList->size = 0;    //Reset the size of pList

    //If the freeHeads list is empty, make the freed list the head of freeHeads
    if(manager.freeHeads == 0){
        manager.freeHeads = pList;
    }
    //Otherwise, free the pList itself
    else{
        pList->next = manager.freeHeads;
        manager.freeHeads->prev = pList;
    }
}
// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){

    //If the current pointer is before the start of the pList, set the current pointer to the head of pList
    if(pList->current->item == &manager.outOfBoundsStart){
        pList->current = pList->head;
        pList->currentItem = pList->current->item;
    }

    //While the current node is not NULL, test for the item in the current Node's position 
    while(pList->current != 0){   

        //If a match is found, return a pointer to the current item 
        if((*pComparator)(pList->current->item, pComparisonArg) == 1){
            return(pList->current->item);
        }
        pList->current = pList->current->next;   //Iterate our current Node
        pList->currentItem = pList->current->item;   //Iterate our current item
    }
    //If no match was found, the current pointer is left beyond the end of the list, and a NULL pointer is returned
    pList->currentItem = &manager.outOfBoundsEnds;
    return(0);
}
