#include "list.h"
#include <stdio.h>
#include <stdlib.h>

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

//Our manager for our list
Manager manager;

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create(){
    if(manager.nodes == 0){//If this is the first time List_create is called, setup our Manager, List and Node structs

        //Setup our outOfBounds Items
        Item outOfBoundsStart;
        Item outOfBoundsEnds;
        enum ListOutOfBounds start = LIST_OOB_START;
        enum ListOutOfBounds end = LIST_OOB_ENDS;
        outOfBoundsStart.item = &start;
        outOfBoundsEnds.item = &end;
        manager.outOfBoundsStart = &outOfBoundsStart;
        manager.outOfBoundsEnds = &outOfBoundsEnds;

        //Setup our nodes
        Node nodeArr[LIST_MAX_NUM_NODES];
        manager.nodes = nodeArr;
        manager.freeNodes = manager.nodes;
        manager.freeNodes->next = &nodeArr[1];
        manager.freeNodes->index = 0;
        Node* curr = &manager.nodes[0];
        for(int i=1; i<LIST_MAX_NUM_NODES-1; i++){
            Item currentItem;   //Create a new item to be linked with each Node
            curr=curr->next;
            curr->index=i;
            curr->next = &manager.nodes[i+1];
            curr->prev = &manager.nodes[i-1];
            currentItem.parent = curr;
            curr->child = &currentItem;

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
        head->next->prev = 0;  
        head->next = 0;
        head->prev = 0;
        return head;
    }

    else{
        return 0;
    }
}

// Returns the number of items in pList.
int List_count(List* pList){
    int lastIndex = pList->tail->index;
    int firstIndex = pList->head->index;
    return(abs(lastIndex-firstIndex+1));
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    if(pList->head == 0){    //If the list is empty
        pList->current->item = 0; //Set the current item to NULL
        return(0);  //Return NULL
    }
    void* returnVal = pList->head->child->item;  //Pointer to first item in pList
    pList->current = pList->head->child;   //Make the current item the first item
    return(returnVal);
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    if(pList->head == 0){    //If the list is empty
        pList->current->item = 0; //Set the current item to NULL
        return(0);  //Return NULL
    }
    void* returnVal = pList->tail->child->item;  //Pointer to last item in pList
    pList->current = pList->tail->child;   //Make the current item the last item
    return(returnVal);
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    if(pList->current->parent == pList->tail){    //If operation advances current item beyond the end of the pList
        pList->current = manager.outOfBoundsEnds;   //Set current item to be beyond end of pList
        return(0);  //Return a NULL pointer
    }
    pList->current = pList->current->parent->next->child; //Advance pList's current item by one
    return(pList->current->item);  //Return a pointer to the new current item
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    if(pList->current->parent == pList->head){    //If operation backs up the current item beyond the start of the pList
        pList->current = manager.outOfBoundsStart;   //Set current item to be before the start of pList
        return(0);  //Return a NULL pointer
    }
    pList->current = pList->current->parent->prev->child; //Backs up pList's current item by one
    return(pList->current->item);  //Return a pointer to the new current item
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    return(pList->current->item);
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
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //Make the next available node (after the one we are taking) the head of freeNodes list
        manager.freeNodes->next->prev = 0;            
        manager.freeNodes = manager.freeNodes->next;

        newNode->child->item = pItem;      //Make the newNode's item the item provided

        //If the current pointer is before the start of the pList, insert the newNode at the start of the list
        if(pList->current == manager.outOfBoundsStart){
            newNode->next = pList->head;
            pList->head->prev = newNode;
            pList->head = newNode;
        }

        //If the current pointer is beyond the end of the pList OR current is the end of the list, insert the newNode at the end of the list
        else if(pList->current == manager.outOfBoundsEnds || pList->current->parent == pList->tail){
            newNode->prev = pList->tail;
            pList->tail->next = newNode;
            pList->tail = newNode;
        }

        else{
            //Add the new item directly after the current item
            pList->current->parent->next->prev=newNode;
            newNode->next = pList->current->parent->next;
            newNode->prev = pList->current->parent;
            pList->current->parent->next = newNode;     
        }
        pList->current = newNode->child;    //Make item the current item
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
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //Make the next available node (after the one we are taking) the head of freeNodes list
        manager.freeNodes->next->prev = 0;            
        manager.freeNodes = manager.freeNodes->next;

        newNode->child->item = pItem;      //Make the newNode's item the item provided

        //If the current pointer is before the start of the pList OR current is the start of the list, insert the newNode at the start of the list
        if(pList->current == manager.outOfBoundsStart || pList->current->parent == pList->head){
            newNode->next = pList->head;
            pList->head->prev = newNode;
            pList->head = newNode;
        }

        //If the current pointer is beyond the end of the pList, insert the newNode at the end of the list
        else if(pList->current == manager.outOfBoundsEnds){     
            newNode->prev = pList->tail;
            pList->tail->next = newNode;
            pList->tail = newNode;
        }

        else{
            //Add the new item directly before the current item
            pList->current->parent->prev->next=newNode;
            newNode->next = pList->current->parent;
            newNode->prev = pList->current->parent->prev;
            pList->current->parent->prev = newNode;     
        }
        pList->current = newNode->child;    //Make item the current item
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
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //Make the next available node (after the one we are taking) the head of freeNodes list
        manager.freeNodes->next->prev = 0;            
        manager.freeNodes = manager.freeNodes->next;

        newNode->child->item = pItem;      //Make the newNode's item the item provided

        //Add the newNode (with the new item) to the end of pList
        pList->tail->next = newNode;
        newNode->prev = pList->tail;
        pList->tail = newNode;

        pList->current = newNode->child;    //Make the new item the current one
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
        Node* newNode = manager.freeNodes;        //Access a new Node to be added into our list

        //Make the next available node (after the one we are taking) the head of freeNodes list
        manager.freeNodes->next->prev = 0;            
        manager.freeNodes = manager.freeNodes->next;

        newNode->child->item = pItem;      //Make the newNode's item the item provided

        //Add the newNode (with the new item) to the end of pList
        pList->head->prev = newNode;
        newNode->next = pList->head;
        pList->head = newNode;

        pList->current = newNode->child;    //Make the new item the current one
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
    
    //If current is the tail, call List_trim to take over the function
    else if(pList->current->parent == pList->tail){
        return(List_trim(pList));
    }
    //If the current pointer is before the start of pList, or beyond the end of pList, return NULL
    else if(pList->current == manager.outOfBoundsStart || pList->current == manager.outOfBoundsEnds){
        return(0);
    }
    else{
        void* returnVal = pList->current->item;     //Get the current item to be returned
        pList->current->item = 0;   //Reset the current item to NULL       

        Node* oldNode = pList->current->parent;

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
            pList->current = pList->current->parent->next->child;       //Make the next item the current one
        }

        //Otherwise, take the old node out of the pList normally
        else{
            oldNode->prev->next = oldNode->next;
            oldNode->next->prev = oldNode->prev;
            pList->current = pList->current->parent->next->child;       //Make the next item the current one
        }     
        
        //Move the old node into freeNodes
        manager.freeNodes->prev = oldNode;
        oldNode->prev = 0;
        oldNode->next = manager.freeNodes;
        manager.freeNodes = oldNode;
        
        return(returnVal);
    }
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList);

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2);

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn);

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
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg);
