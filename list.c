#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//TODO: Remove unecessary code to improve efficiency

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

//Our manager for our list
static Manager manager;

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.

List* List_create(){
    if(manager.nodes == NULL){//If this is the first time List_create is called, setup our Manager, List and Node structs

        //Setup our outOfBounds Items
        enum ListOutOfBounds start = LIST_OOB_START;
        enum ListOutOfBounds end = LIST_OOB_ENDS;
        manager.outOfBoundsStart = &start;
        manager.outOfBoundsEnds = &end;

    //Setup our nodes
        Node nodeArr[LIST_MAX_NUM_NODES];
        manager.nodes = nodeArr;
        //Assign freeNodes to nodes, as all nodes are initially free
        manager.freeNodes = &nodeArr[0];
        //Setting up our first node
        manager.nodes[0].next = &manager.nodes[1];
        manager.nodes[0].index = 0;
        manager.nodes[0].prev = NULL;
        manager.nodes[0].item = NULL;
        Node* currentNode;
        //Setting up all nodes between the first and last
        for(int i=1; i<LIST_MAX_NUM_NODES-1; i++){
            currentNode=&manager.nodes[i];
            currentNode->index=i;
            currentNode->next = &manager.nodes[i+1];
            currentNode->prev = &manager.nodes[i-1];
            currentNode->item = NULL;
        }
        //Setting up our last node
        manager.nodes[LIST_MAX_NUM_NODES-1].index = LIST_MAX_NUM_NODES-1;
        manager.nodes[LIST_MAX_NUM_NODES-1].prev = currentNode;
        manager.nodes[LIST_MAX_NUM_NODES-1].next = NULL;
        manager.nodes[LIST_MAX_NUM_NODES-1].item = NULL;
        
    //Setup our heads
        List listArr[LIST_MAX_NUM_HEADS];
        manager.heads = listArr;
        //Assign freeHeads to heads, as all heads are initially free
        manager.freeHeads = &manager.heads[0];
        //Setup our first head
        manager.heads[0].next = &manager.heads[1];
        manager.heads[0].index = 0;
        manager.heads[0].prev = NULL;
        manager.heads[0].current = NULL;
        manager.heads[0].currentItem = NULL;
        manager.heads[0].head = NULL;
        manager.heads[0].tail = NULL;
        manager.heads[0].size = 0;
        List* currentList;
        //Setting up all heads between the first and last
        for(int i=1; i<LIST_MAX_NUM_HEADS-1; i++){
            currentList = &manager.heads[i];
            currentList->next = &manager.heads[i+1];
            currentList->index = i;
            currentList->prev = &manager.heads[i-1];
            currentList->current = NULL;
            currentList->currentItem = NULL;
            currentList->head = NULL;
            currentList->tail = NULL;
            currentList->size = 0;
        }
        //Setting up our last head
        manager.heads[LIST_MAX_NUM_HEADS-1].next = NULL;
        manager.heads[LIST_MAX_NUM_HEADS-1].index = LIST_MAX_NUM_HEADS-1;
        manager.heads[LIST_MAX_NUM_HEADS-1].prev = currentList;
        manager.heads[LIST_MAX_NUM_HEADS-1].current = NULL;
        manager.heads[LIST_MAX_NUM_HEADS-1].currentItem = NULL;
        manager.heads[LIST_MAX_NUM_HEADS-1].head = NULL;
        manager.heads[LIST_MAX_NUM_HEADS-1].tail = NULL;
        manager.heads[LIST_MAX_NUM_HEADS-1].size = 0;
    }

    if(manager.freeHeads != NULL){   //If possible, return a new head pointer (not possible when heads are all in use)
        //Set head to next available head (the start of our freeHeads list)
        List* head = manager.freeHeads;  

        //Fully disconnect head from freeHeads list
        //If the freeHeads list is of size 1, set the freeHeads list to NULL
        if(head->next == NULL){
            manager.freeHeads = NULL;
        }
        //Otherwise, disconnect just the head of the freeHeads (from the list side)
        else{
            head->next->prev = NULL;  
            manager.freeHeads = head->next;
        }
        //Disconnect the head from the head's side
        head->next = NULL;
        head->prev = NULL;
        head->size = 0;
        head->current = NULL;
        head->currentItem = NULL;
        head->head = NULL;
        head->tail = NULL;
        manager.numHeads++;         //Increment our numHeads counter
        return(head);
    }

    else{
        return NULL;
    }
}

// Returns the number of items in pList.
int List_count(List* pList){
    return(pList->size);
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    if(pList->size == 0){    //If the list is empty
        //Set the current item to NULL
        pList->current = NULL; 
        pList->currentItem = NULL;
        return(NULL);  //Return NULL
    }
    void* returnValue = pList->head->item;  //Pointer to first item in pList
    //Make the current item the first item
    pList->current = pList->head;
    pList->currentItem = pList->current->item;   
    //Return a pointer to the first item
    return(returnValue);
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    if(pList->size == 0){    //If the list is empty
        //Set the current item to NULL
        pList->current = NULL; 
        pList->currentItem = NULL;
        return(NULL);  //Return NULL
    }
    void* returnValue = pList->tail->item;  //Pointer to last item in pList
    //Make the current item the last item
    pList->current = pList->tail;   
    pList->currentItem = pList->current->item;
    //Return a pointer to the last item
    return(returnValue);
}
// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    //If the pList is empty
    if(pList->size == 0){  
        pList->currentItem = manager.outOfBoundsEnds;       //Set current item to be beyond end of pList
        pList->current = NULL;
        return(NULL);       //Return a NULL pointer
    }

    //Otherwise, if current is before start of list
    else if(pList->currentItem == manager.outOfBoundsStart){
        //Set the new current item to head
        pList->current = pList->head;   
        pList->currentItem = pList->head->item;
        return(pList->head->item);  //Return a pointer to the new current item     
    }

    //Otherwise, if the operation advances current item beyond the end of the pList
    else if(pList->current == pList->tail){  
        pList->currentItem = manager.outOfBoundsEnds;       //Set current item to be beyond end of pList
        pList->current = pList->tail;
        return(NULL);       //Return a NULL pointer
    }

    //Otherwise, if the current item is already beyond the end of the pList, do nothing to the pList and return NULL
    else if(pList->currentItem == manager.outOfBoundsEnds){ 
        return(NULL);
    }

    //Otherwise, advance pList's current item by one
    else{
        pList->current = pList->current->next;
        pList->currentItem = pList->current->item;
        return(pList->currentItem);  //Return a pointer to the new current item
    }
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    //If the pList is empty
    if(pList->size == 0){     
        pList->currentItem = manager.outOfBoundsStart;      //Set current item to be before the start of pList
        pList->current = NULL;
        return(NULL);   //Return a NULL pointer
    }

    //Otherwise, if current is after the end of the list
    else if(pList->currentItem == manager.outOfBoundsEnds){
        //Set the new current item to tail
        pList->current = pList->tail;
        pList->currentItem = pList->tail->item;
        return(pList->currentItem);  //Return a pointer to the new current item

    }

    //Otherwise, if the operation backs up the current item beyond the start of the pList
    else if(pList->current == pList->head){     
        pList->currentItem = manager.outOfBoundsStart;      //Set current item to be before the start of pList
        pList->current = pList->head;
        return(NULL);   //Return a NULL pointer
    }

    //Otherwise, if the current item is already before the start of the pList, do nothing to the pList and return NULL
    else if(pList->currentItem == manager.outOfBoundsStart){     
        return(NULL);   //Return a NULL pointer
    }

    //Otherwise, back up pList's current item by one
    else{
        pList->current = pList->current->prev;
        pList->currentItem = pList->current->item;
        return(pList->currentItem);  //Return a pointer to the new current item
    } 
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    if(pList->size == 0){return(NULL);}     //If the pList is empty, return NULL
    return(pList->currentItem);     //Otherwise, return the current item in pList
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem){
    if(manager.freeNodes == NULL){    //If there are no free nodes left to use
        return(-1);     //Report failure
    }
    else{   
        Node* newNode = manager.freeNodes;      //The pointer to our new node, taken as the first node from our freeNodes linked list
        newNode->item = pItem;      //Set the item of our new node

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == NULL){
            manager.freeNodes = NULL;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = NULL;            
            manager.freeNodes = manager.freeNodes->next;
        }

        //If the list is empty
        if(pList->size == 0){
            //Make the item the head and tail
            pList->tail = newNode;
            pList->head = newNode;
            //Ensure that the new head/tail (our new node) is not somehow still linked to another node
            newNode->next = NULL;
            newNode->prev = NULL;
        }

        //Otherwise, if the current item pointer is before the start of the pList
        else if(pList->currentItem == manager.outOfBoundsStart){
            //Insert the new node at the start of the list
            newNode->next = pList->head;
            newNode->prev = NULL;       //Ensure that the new head (our new node) is not somehow still linked to another node
            pList->head->prev = newNode;
            pList->head = newNode;
        }

        //Otherwise if the current item pointer is beyond the end of the pList OR current is the end of the list
        else if(pList->currentItem == manager.outOfBoundsEnds || pList->current == pList->tail){
            //Insert the newNode at the end of the list
            newNode->prev = pList->tail;
            newNode->next = NULL;       //Ensure that the new tail (our new node) is not somehow still linked to another node
            pList->tail->next = newNode;
            pList->tail = newNode;
        }

        //Otherwise
        else{
            //Add the new item directly after the current item
            pList->current->next->prev = newNode;
            newNode->next = pList->current->next;
            newNode->prev = pList->current;
            pList->current->next = newNode;
            
        }
        //Make item the current item
        pList->current = newNode;   
        pList->currentItem = newNode->item; 
        pList->size++;      //Increment the size of pList 

        return(0);  //Report success
    } 
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem){
    if(manager.freeNodes==NULL){    //If there are no free nodes left to use
        return(-1);     //Report failure
    }
    else{
        Node* newNode = manager.freeNodes;      //The pointer to our new node, taken as the first node from our freeNodes linked list
        newNode->item = pItem;      //Set the item of our new node

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == NULL){
            manager.freeNodes = NULL;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = NULL;            
            manager.freeNodes = manager.freeNodes->next;
        }

        //If the list is empty
        if(pList->size == 0){
            //Make the item the head and tail
            pList->tail = newNode;
            pList->head = newNode;
            //Ensure that the new head/tail (our new node) is not somehow still linked to another node
            newNode->next = NULL;
            newNode->prev = NULL;
        }

        //Otherwise, if the current pointer is before the start of the pList OR current is the start of the list
        if(pList->current == manager.outOfBoundsStart || pList->current == pList->head){
            //Insert the newNode at the start of the list
            newNode->next = pList->head;
            pList->head->prev = newNode;
            newNode->prev = NULL;       //Ensure that the new head (our new node) is not somehow still linked to another node
            pList->head = newNode;
        }

        //Otherwise, if the current item pointer is beyond the end of the pList
        else if(pList->currentItem == manager.outOfBoundsEnds){
            //Insert the newNode at the end of the list
            newNode->prev = pList->tail;
            newNode->next = NULL;       //Ensure that the new tail (our new node) is not somehow still linked to another node
            pList->tail->next = newNode;
            pList->tail = newNode;
        }
        
        //Otherwise,
        else{
            //Add the new item directly before the current item
            newNode->next = pList->current;
            newNode->prev = pList->current->prev;
            pList->current->prev->next = newNode;
            pList->current->prev = newNode;  
        }

        //Make item the current item
        pList->current = newNode;
        pList->currentItem = newNode->item;
        pList->size++;      //Increment the size of pList      
        return(0);  //Report success
    }
} 

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){
    if(manager.freeNodes==NULL){    //If there are no free nodes left to use
        return(-1);     //Report failure
    }
    else{
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list
        newNode->item = pItem;      //Make the newNode's item the item provided

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == NULL){
            manager.freeNodes = NULL;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = NULL;            
            manager.freeNodes = manager.freeNodes->next;
        }

        //If the list is empty, make the item the head and tail
        if(pList->size == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        //Add the newNode (with the new item) to the end of pList
        pList->tail->next = newNode;
        newNode->prev = pList->tail;
        pList->tail = newNode;

        //Make the new item the current one
        pList->current = newNode;
        pList->currentItem = newNode->item;
        pList->size++;      //Increment the size of pList
        return(0);  //Report success
    }
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    if(manager.freeNodes==NULL){    //If there are no free nodes left to use
        return(-1);     //Report failure
    }
    else{
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list
        newNode->item = pItem;      //Make the new Node's item the item provided

        //If the freeNodes list is singleton, take the head
        if(manager.freeNodes->next == NULL){
            manager.freeNodes = NULL;
        }

        //Otherwise, make the next available node (after the one we are taking) the head of freeNodes list
        else{
            manager.freeNodes->next->prev = NULL;            
            manager.freeNodes = manager.freeNodes->next;
        }

        //If the list is empty, make the item the head and tail
        if(pList->size == 0){
            pList->tail = newNode;
            pList->head = newNode;
        }

        //Add the newNode (with the new item) to the end of pList
        pList->head->prev = newNode;
        newNode->next = pList->head;
        pList->head = newNode;

        //Make the new item the current one
        pList->current = newNode;    
        pList->currentItem = newNode->item;
        pList->size++;      //Increment the size of pListx
        return(0);  //Report success
    }
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){
    //If the pList is empty, return NULL
    if(pList->size == 0){
        return(NULL);
    }

    //If the current pointer is before the start of pList, or beyond the end of pList, return NULL
    else if(pList->currentItem == manager.outOfBoundsStart || pList->currentItem == manager.outOfBoundsEnds){
        return(NULL);
    }
    else{
        pList->size--;      //Decrement the size of pList
        void* returnValue = pList->currentItem;     //Get the current item to be returned
        Node* oldNode = pList->current;     //Get the current Node to be deleted
        
        //If current is head and tail (the list is of size 1)
        if(oldNode == pList->head && oldNode == pList->tail){
            //Set head, tail, and current pointers to NULL to disconnect oldNode from the list
            pList->head = NULL;
            pList->tail = NULL;
            pList->current = NULL;
            pList->currentItem = manager.outOfBoundsStart;
            pList->size = 0;    //Ensure the size of pList is 0
        }

        //Otherwise, if current is head 
        else if(oldNode == pList->head){   
            //Disconnect the head and make the next item our new head
            oldNode->next->prev = NULL;
            pList->head = oldNode->next;
            //Make the new head our current item
            pList->current = pList->head;  
            pList->currentItem = pList->current->item;      
        }

        //Otherwise, if current is the tail
        else if(oldNode == pList->tail){
            //Disconnect the tail and make the previous item our old tail
            oldNode->prev->next = NULL;
            pList->tail = oldNode->prev;
            //Make the new tail our current item
            pList->current = pList->tail;   
            pList->currentItem = pList->current->item;  
        }

        //Otherwise
        else{
            //Take the node out of the list in the standard way for any doubly-linked list
            oldNode->prev->next = oldNode->next;
            oldNode->next->prev = oldNode->prev;
            //Make the next item the current item
            pList->current = oldNode->next;    
            pList->currentItem = pList->current->item;   
        }

        oldNode->prev = NULL;
        oldNode->item = NULL;
        //If the freeNodes list is empty
        if(manager.freeNodes == NULL){   
            //Make the old node the head of the freeNodes list    
            oldNode->next = NULL;
            manager.freeNodes = oldNode;
        }
        //Otherwise
        else{
            //Move the old node into the current freeNodes list
            manager.freeNodes->prev = oldNode;
            oldNode->next = manager.freeNodes;
            manager.freeNodes = oldNode;
        }
        return(returnValue);    //Return the item contained in the deleted node
    }
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){
    //If the pList is empty, return NULL
    if(pList->size == 0){
        return(NULL);
    }

    //Otherwise
    else{
        //Set current to the tail, pass control off to List_remove()
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
    if(pList2->size != 0){ 
        //If pList1 is empty, move pList2 to pList1
        if(pList1->size == 0){
            pList1->head = pList2->head;
            pList1->tail = pList2->tail;
            pList1->current = pList2->current;
            pList1->currentItem = pList2->currentItem;
        }

        //Otherwise, add pList2 to the end of pList1
        else{
            pList1->tail->next = pList2->head;
            pList2->head->prev = pList1->tail;
            pList1->tail = pList2->tail;
        }
        pList1->size += pList2->size;       //Update the size of pList1

        //Empty pList2
        pList2->head = NULL;
        pList2->tail = NULL;
        pList2->current = NULL;
        pList2->currentItem = NULL;
        pList2->size = 0;
    }
    List_free(pList2, List_remove);     //Delete pList2
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    void* itemToBeDeleted;
    //While pList remains non-empty
    while(pList->size != 0){
        //Set the current item to the tail
        pList->current = pList->tail;   
        pList->currentItem = pList->current->item;

        itemToBeDeleted = pList->currentItem;   //Store a pointer to the item to be deleted
        List_remove(pList);     //Free the Node
        (*pItemFreeFn)(itemToBeDeleted);     //Free the item in memory   
    }

    //Reset the list to its standard values
    pList->head = NULL;
    pList->tail = NULL;
    pList->current = NULL;
    pList->currentItem = NULL;
    pList->size = 0;    //Ensure the size of pList is 0

    //If the freeHeads list is empty, make the freed list the head of freeHeads
    if(manager.freeHeads == NULL){
        manager.freeHeads = pList;  
    }

    //Otherwise,
    else{
        //Add pList to the freeHeads list
        pList->next = manager.freeHeads;
        manager.freeHeads->prev = pList;
        manager.freeHeads = pList;
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
    //If pList is empty, or beyond the end of pList return NULL
    if(pList->size == 0 || pList->currentItem == manager.outOfBoundsEnds){
        return(NULL);
    }

    //Otherwise, if the current pointer is before the start of pList
    else if(pList->currentItem == manager.outOfBoundsStart){
        //Set the current item to the head of pList
        pList->current = pList->head;
        pList->currentItem = pList->current->item;
    }

    //While the current node is not NULL, test for the item in the current Node's position 
    while(pList->current != NULL){   
        pList->currentItem = pList->current->item;
        //If a match is found, return a pointer to the current item 
        if((*pComparator)(pList->currentItem, pComparisonArg) == 1){
            return(pList->currentItem);
        }

        //If not, iterate our current node and continue
        pList->current = pList->current->next;
    }

    //If no match was found, the current pointer is left beyond the end of the list, and a NULL pointer is returned
    pList->currentItem = manager.outOfBoundsEnds;
    return(NULL);
}
