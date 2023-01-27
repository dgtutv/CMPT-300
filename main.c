#include "list.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


//My implementation of FREE_FN from list.c
void freeItem(void* item){
    return;     //I will test with static memory for simplicity
}

//My implementation of COMPARATOR_FN (I interpret a match as the items having the same address)
int compareItem(void* item, void* reference){
    //If there is a match, return 1
    if(item == reference){
        return(1);
    }
    //Otherwise, return 0
    return(0);
}

int main(){
    extern Manager manager; //our manager from list.c

//----------------------------------------------Tests for List_create()---------------------------------------------------------------------------//
    List* newList = List_create();
    //Define pointers to iterate over the node lists in manager
    Node* currentFreeNode = manager.freeNodes;      
    //Check the initial conditions of our node lists
    int i=98;
    assert(currentFreeNode == &manager.nodes[0]);
    assert(currentFreeNode->item == NULL);
    assert(currentFreeNode->index == 0);
    assert(currentFreeNode != NULL);
    Node* prevFreeNode = currentFreeNode;
    currentFreeNode=currentFreeNode->next;
    assert(prevFreeNode == currentFreeNode->prev); 
    assert(currentFreeNode == &manager.nodes[99]);
    assert(currentFreeNode->item == NULL);
    assert(currentFreeNode->index == 99);
    assert(currentFreeNode != NULL);
    prevFreeNode = currentFreeNode;
    currentFreeNode = currentFreeNode->next;
    while(currentFreeNode->next != NULL){
        assert(prevFreeNode == currentFreeNode->prev); 
        if(i>0){
            assert(currentFreeNode == prevFreeNode->next);
        }
        assert(currentFreeNode == &manager.nodes[i]);
        assert(currentFreeNode->item == NULL);
        assert(currentFreeNode->index == i);
        assert(currentFreeNode != NULL);
        prevFreeNode = currentFreeNode;
        i--;
        prevFreeNode = currentFreeNode;
        currentFreeNode = currentFreeNode->next;
    }
    assert(currentFreeNode->index == 1);
    assert(i==1);   //Check that we have iterated over 100 Nodes

    //Check our freeHeads list does not run out of bounds
    List* currentFreeHead = &manager.freeHeads[0];
    List* prevFreeHead = NULL;
    i = 0;
    while(currentFreeHead->next != NULL){
        assert(prevFreeHead == currentFreeHead->prev);
        if(i>0){
            assert(currentFreeHead == prevFreeHead->next);
        }
        prevFreeHead = currentFreeHead;
        currentFreeHead = currentFreeHead->next;
        i++;
    }
    assert(currentFreeHead->index == 1);    //Index will be 1 since we add after the head
    assert(i==8);

    //Test the conditions of our initial list
    assert(newList != NULL);  
    assert(manager.numFreeHeads == LIST_MAX_NUM_HEADS-1);
    List* currentHead = &manager.heads[0];
    assert(newList == currentHead); 
    //Define pointers to iterate over the head lists in manager 
    List* prevList = newList;
    List* prevHead;
    //Check the initial conditions of our head lists
    for(int i=1; i<10; i++){
        newList = List_create();
        assert(manager.numFreeHeads == LIST_MAX_NUM_HEADS-i-1);    
        prevHead = currentHead;
        currentHead = &manager.heads[newList->index];
        assert(newList == currentHead);       
        assert(prevList == prevHead);     
        assert(newList != NULL);    
        assert(newList != prevList);    
        assert(newList->next == NULL);
        assert(newList->prev == NULL);
        assert(newList->size == 0);
        assert(newList->current == NULL);
        assert(newList->currentItem == manager.outOfBoundsStart);
        assert(newList->head == NULL);
        assert(newList->tail == NULL);
        prevList = newList;
    }
    

    //There should be no more free heads
    assert(manager.freeHeads == NULL);
    assert(manager.numFreeHeads == 0);

    //Attempting to create more than 10 lists will return a NULL pointer
    newList = List_create();
    assert(newList == NULL);

    //Test that our out of bounds variables have been defined
    enum ListOutOfBounds start = LIST_OOB_START;
    enum ListOutOfBounds end = LIST_OOB_ENDS;
    assert(*(int*)manager.outOfBoundsStart == start);
    assert(*(int*)manager.outOfBoundsEnds == end);

    //---------------------------------------Tests for all functions interacting with List_insert_after()---------------------------------------------------------------------------//
    //Here we test List_count(), List_first(), List_last(), List_next(), List_prev() and List_curr() on some lists filled by List_insert_after()
    //Also tests the majority of List_insert_after()
    
    //Our variables which will be used to test our functions for various scenarios
    int testInt0 = -1;
    int testInt1 = 1;
    int testInt2 = 2;
    int testInt3 = 3;
    int testInt4 = 4;
    int testInt5 = 5;
    int testInt6 = 6;
    int testInt7 = 7;
    float testFloat = 3.14;
    char testChar = 'T';
    char* testString = "TEST";
    Node* freeNodesNext = manager.freeNodes->next;
    int sizeReference;
    int insertCounter = 0;

    //Test for all heads to cover edge cases
    for(int i=0; i<10; i++){
        currentHead = &manager.heads[i];

        //List_curr() test
        assert(List_curr(currentHead) == manager.outOfBoundsStart);

        //List_count() test
        assert(List_count(currentHead) == 0);
        assert(currentHead->size == 0);
        currentHead->currentItem = &testInt0;
        assert(List_count(currentHead) == 0);
        assert(currentHead->size == 0);

        //List_first() test
        assert(List_first(currentHead) == NULL); 
        assert(currentHead->currentItem == NULL);  
        currentHead->currentItem = &testInt0;
        assert(List_first(currentHead) == NULL); 
        assert(currentHead->currentItem == NULL); 

        //List_last() test
        assert(List_last(currentHead) == NULL);  
        assert(currentHead->currentItem == NULL);  
        currentHead->currentItem = &testInt0;
        assert(List_last(currentHead) == NULL);  
        assert(currentHead->currentItem == NULL);

        //List_next() test  
        assert(List_next(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        currentHead->currentItem = &testInt0;
        assert(List_next(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsEnds);

        //List_prev() test
        assert(List_prev(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        currentHead->currentItem = &testInt0;
        assert(List_prev(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsStart);


    //Inserting an item into an empty list, the head, tail, and current Nodes should all be the address of the item

        //List_insert_after() test
        currentHead->current = NULL;
        currentHead->currentItem = NULL;
        assert(List_insert_after(currentHead, &testInt1) == 0);  
        assert(currentHead->head->item == &testInt1);
        assert(currentHead->tail->item == &testInt1);
        assert(currentHead->current->item == &testInt1);
        assert(currentHead->currentItem == &testInt1);
        assert(currentHead->head == currentHead->tail);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->current->prev == NULL);
        assert(currentHead->current->next == NULL);
        assert(currentHead->size == 1); 

        //List_curr() test
        assert(List_curr(currentHead) == &testInt1);

        //List_count() test     
        assert(List_count(currentHead) == 1);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == &testInt1);   //Item is before the start of the list
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = &testInt1;
        sizeReference = currentHead->size;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->current == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->current == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_first() test
        assert(List_first(currentHead) == &testInt1);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testInt1);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List traversal test
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == &testInt1);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == NULL);


    //Inserting an item after the head should make the item the tail, and the current item of the list, keeping head the same
        
        //List_insert_after() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_insert_after(currentHead, &testFloat) == 0);
        assert(currentHead->currentItem == &testFloat);
        assert(currentHead->current->item == &testFloat);
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testFloat);
        assert(currentHead->current->item == &testFloat);
        assert(currentHead->size == 2);

        //List_curr() test
        assert(List_curr(currentHead) == &testFloat);

        //List_count() test
        assert(List_count(currentHead) == 2);

        //List_first() test
        assert(List_first(currentHead) == &testInt1);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testFloat);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == NULL);



    //Inserting an item after the head again should make the item just the current item of the list, keeping both head and tail the same

        //List_insert_after() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_insert_after(currentHead, &testString) == 0);
        assert(currentHead->size == 3);
        assert(List_count(currentHead) == 3);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testString);
        assert(currentHead->current->item == &testString);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail != currentHead->current);
        assert(currentHead->head->next == currentHead->current);
        assert(currentHead->current->prev == currentHead->head);
        assert(currentHead->current->next == currentHead->tail);
        assert(currentHead->tail->prev == currentHead->current);

        //List_curr() test
        assert(List_curr(currentHead) == &testString);

        //List_first() test
        assert(List_first(currentHead) == &testInt1);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testFloat);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == NULL);




    //Inserting an item after the tail (current pointer is at tail) should make the item both the tail and current item of the list, keeping head the same
        
        //List_insert_after() test
        currentHead->current = currentHead->tail;
        currentHead->currentItem = currentHead->current->item;
        assert(List_insert_after(currentHead, &testChar) == 0);
        assert(currentHead->size == 4);
        assert(List_count(currentHead) == 4);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testChar);
        assert(currentHead->current->item == &testChar);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->head != currentHead->current);

        //List_curr() test
        assert(List_curr(currentHead) == &testChar);

        //List_first() test
        assert(List_first(currentHead) == &testInt1);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testChar);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == &testChar);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testChar);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == NULL);



    //Inserting an item at the head (current pointer is at LIST_OOB_START) should make both the head, and the current pointer the item, keeping tail the same
        
        //List_insert_after() test
        currentHead->current = NULL;
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_insert_after(currentHead, &testInt2) == 0);
        assert(currentHead->size == 5);
        assert(List_count(currentHead) == 5);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testInt2);
        assert(currentHead->current->item == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail != currentHead->current);

        //List_curr() test
        assert(List_curr(currentHead) == &testInt2);

        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testChar);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testInt1);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == &testChar);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testChar);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == &testInt2);
        assert(List_prev(currentHead) == NULL);


        

    //Inserting an item after the tail (current pointer is at LIST_OOB_ENDS) should make the item both the tail and current item of the list, keeping head the same
        
        //List_insert_after() test
        currentHead->current = currentHead->tail;
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_insert_after(currentHead, &testInt3) == 0);
        assert(currentHead->size == 6);
        assert(List_count(currentHead) == 6);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testInt3);
        assert(currentHead->current->item == &testInt3);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail == currentHead->current);

        //List_curr() test
        assert(List_curr(currentHead) == &testInt3);

        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testInt3);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testInt1);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == &testChar);
        assert(List_next(currentHead) == &testInt3);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testInt3);
        assert(List_prev(currentHead) == &testChar);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == &testInt2);
        assert(List_prev(currentHead) == NULL);
    
    //Inserting an item after the tail (current pointer is at tail) should make the item both the tail and current item of the list, keeping head the same
        
        //List_insert_after() test
        currentHead->current = currentHead->tail;
        currentHead->currentItem = currentHead->current->item;
        assert(List_insert_after(currentHead, &testInt4) == 0);
        assert(currentHead->size == 7);
        assert(List_count(currentHead) == 7);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testInt4);
        assert(currentHead->current->item == &testInt4);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail == currentHead->current);

        //List_curr() test
        assert(List_curr(currentHead) == &testInt4);
        
        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testInt4);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testInt1);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == &testChar);
        assert(List_next(currentHead) == &testInt3);
        assert(List_next(currentHead) == &testInt4);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testInt4);
        assert(List_prev(currentHead) == &testInt3);
        assert(List_prev(currentHead) == &testChar);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == &testInt2);
        assert(List_prev(currentHead) == NULL);

    //If the freeNodes list is not singleton when inserting an item, the next free node should become the new head of the freeNodes list (inserting after head)
        
        //List_insert_after() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        freeNodesNext = manager.freeNodes->next;
        assert(List_insert_after(currentHead, &testInt5) == 0);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testInt5);
        assert(currentHead->current->item == &testInt5);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail != currentHead->current);
        assert(manager.freeNodes == freeNodesNext);
        assert(manager.freeNodes->prev == NULL);

        //List_curr() test
        assert(List_curr(currentHead) == &testInt5);

        //List_count() test
        assert(currentHead->size == 8);
        assert(List_count(currentHead) == 8);

        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testInt4);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

        //List_next() test
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
        assert(currentHead->current == currentHead->head);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->head->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == currentHead->head);
        sizeReference = currentHead->size;
        currentHead->currentItem = &testInt1;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List_prev() test
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        sizeReference = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt1;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        assert(currentHead->current == NULL);
        currentHead->size = sizeReference;

        //List traversal test
        List_first(currentHead);
        assert(List_next(currentHead) == &testInt5);
        assert(List_next(currentHead) == &testInt1);
        assert(List_next(currentHead) == &testString);
        assert(List_next(currentHead) == &testFloat);
        assert(List_next(currentHead) == &testChar);
        assert(List_next(currentHead) == &testInt3);
        assert(List_next(currentHead) == &testInt4);
        assert(List_next(currentHead) == NULL);
        assert(List_prev(currentHead) == &testInt4);
        assert(List_prev(currentHead) == &testInt3);
        assert(List_prev(currentHead) == &testChar);
        assert(List_prev(currentHead) == &testFloat);
        assert(List_prev(currentHead) == &testString);
        assert(List_prev(currentHead) == &testInt1);
        assert(List_prev(currentHead) == &testInt5);
        assert(List_prev(currentHead) == &testInt2);
        assert(List_prev(currentHead) == NULL);

    //If the freeNodes list is singleton when inserting an item, the freeNodes list should sieze to exist after the operation (inserting after the head initially)
        sizeReference = 8;
        if(i==9){
            Node* currentNode = currentHead->head;
            while(manager.numFreeNodes != 0){
                insertCounter++;
                sizeReference++;
                currentHead->current = currentNode;
                currentHead->currentItem = currentNode->item;

                //List_insert_after() test
                assert(List_insert_after(currentHead, &testInt6)==0);  
                currentNode = currentNode->next;

                //List_curr() test
                assert(List_curr(currentHead) == &testInt6);

                //List_count() test
                assert(List_count(currentHead) == currentHead->size);   
                assert(currentHead->size == sizeReference);

                //List_first() test
                assert(List_first(currentHead) == &testInt2);
                assert(currentHead->head == currentHead->current);
                assert(currentHead->head->item == currentHead->currentItem);

                //List_last() test
                assert(List_last(currentHead) == &testInt4);
                assert(currentHead->tail == currentHead->current);
                assert(currentHead->tail->item == currentHead->currentItem);

                //List_next() test
                assert(List_next(currentHead) == NULL);     //Current == tail condition
                assert(currentHead->currentItem == manager.outOfBoundsEnds);
                assert(currentHead->current == NULL);
                currentHead->currentItem = manager.outOfBoundsStart;
                assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
                assert(currentHead->current == currentHead->head);
                assert(currentHead->currentItem == currentHead->current->item);
                assert(currentHead->currentItem == currentHead->head->item);
                assert(currentHead->currentItem == currentHead->current->item);
                currentHead->currentItem = manager.outOfBoundsEnds;
                assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
                assert(currentHead->currentItem == manager.outOfBoundsEnds);
                assert(currentHead->current == currentHead->head);
                sizeReference = currentHead->size;
                currentHead->currentItem = &testInt1;
                currentHead->size = 0;
                assert(List_next(currentHead) == NULL);     //List size is 0
                assert(currentHead->currentItem == manager.outOfBoundsEnds);
                assert(currentHead->current == NULL);
                currentHead->size = sizeReference;

                //List_prev() test
                currentHead->current = currentHead->head;
                currentHead->currentItem = currentHead->current->item;
                assert(List_prev(currentHead) == NULL);     //Current == head condition
                assert(currentHead->currentItem == manager.outOfBoundsStart);
                assert(currentHead->current == NULL);
                currentHead->currentItem = manager.outOfBoundsStart;
                assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
                assert(currentHead->currentItem == manager.outOfBoundsStart);
                assert(currentHead->current == NULL);
                currentHead->currentItem = manager.outOfBoundsEnds;
                assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
                assert(currentHead->current == currentHead->tail);
                assert(currentHead->currentItem == currentHead->current->item);
                assert(currentHead->currentItem == currentHead->tail->item);
                assert(currentHead->currentItem == currentHead->current->item);
                sizeReference = currentHead->size;
                currentHead->size = 0;
                currentHead->currentItem = &testInt1;
                assert(List_prev(currentHead) == NULL);     //List size is 0
                assert(currentHead->currentItem == manager.outOfBoundsStart);
                assert(currentHead->current == NULL);
                currentHead->size = sizeReference;

            }
            assert(manager.numFreeNodes == 0);

            //List traversal test
            List_first(currentHead);
            for(int i=0; i<insertCounter; i++){
                assert(List_next(currentHead) == &testInt6);
            }
            assert(List_next(currentHead) == &testInt5);
            assert(List_next(currentHead) == &testInt1);
            assert(List_next(currentHead) == &testString);
            assert(List_next(currentHead) == &testFloat);
            assert(List_next(currentHead) == &testChar);
            assert(List_next(currentHead) == &testInt3);
            assert(List_next(currentHead) == &testInt4);
            assert(List_next(currentHead) == NULL);
            assert(List_prev(currentHead) == &testInt4);
            assert(List_prev(currentHead) == &testInt3);
            assert(List_prev(currentHead) == &testChar);
            assert(List_prev(currentHead) == &testFloat);
            assert(List_prev(currentHead) == &testString);
            assert(List_prev(currentHead) == &testInt1);
            assert(List_prev(currentHead) == &testInt5);
            for(int i=0; i<insertCounter; i++){
                assert(List_prev(currentHead) == &testInt6);
            }
            assert(List_prev(currentHead) == &testInt2);
            assert(List_prev(currentHead) == NULL);
        }
    
    //Inserting an item when there are no more Nodes available should return -1
        if(i==9){
            //List_insert_after() test
            assert(List_insert_after(currentHead, &testInt7) == -1);

            //List_curr() test
            assert(List_curr(currentHead) == manager.outOfBoundsStart);

            //List_count() test
            assert(List_count(currentHead) == currentHead->size);
            assert(currentHead->size == sizeReference);

            //List_first() test
            assert(List_first(currentHead) == &testInt2);
            assert(currentHead->head == currentHead->current);
            assert(currentHead->head->item == currentHead->currentItem);

            //List_last() test
            assert(List_last(currentHead) == &testInt4);
            assert(currentHead->tail == currentHead->current);
            assert(currentHead->tail->item == currentHead->currentItem);

            //List_next() test
            assert(List_next(currentHead) == NULL);     //Current == tail condition
            assert(currentHead->currentItem == manager.outOfBoundsEnds);
            assert(currentHead->current == NULL);
            currentHead->currentItem = manager.outOfBoundsStart;
            assert(List_next(currentHead) == currentHead->head->item);   //Item is before the start of the list
            assert(currentHead->current == currentHead->head);
            assert(currentHead->currentItem == currentHead->current->item);
            assert(currentHead->currentItem == currentHead->head->item);
            assert(currentHead->currentItem == currentHead->current->item);
            currentHead->currentItem = manager.outOfBoundsEnds;
            assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
            assert(currentHead->currentItem == manager.outOfBoundsEnds);
            assert(currentHead->current == currentHead->head);
            sizeReference = currentHead->size;
            currentHead->currentItem = &testInt1;
            currentHead->size = 0;
            assert(List_next(currentHead) == NULL);     //List size is 0
            assert(currentHead->currentItem == manager.outOfBoundsEnds);
            assert(currentHead->current == NULL);
            currentHead->size = sizeReference;

            //List_prev() test
            currentHead->current = currentHead->head;
            currentHead->currentItem = currentHead->current->item;
            assert(List_prev(currentHead) == NULL);     //Current == head condition
            assert(currentHead->currentItem == manager.outOfBoundsStart);
            assert(currentHead->current == NULL);
            currentHead->currentItem = manager.outOfBoundsStart;
            assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
            assert(currentHead->currentItem == manager.outOfBoundsStart);
            assert(currentHead->current == NULL);
            currentHead->currentItem = manager.outOfBoundsEnds;
            assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
            assert(currentHead->current == currentHead->tail);
            assert(currentHead->currentItem == currentHead->current->item);
            assert(currentHead->currentItem == currentHead->tail->item);
            assert(currentHead->currentItem == currentHead->current->item);
            sizeReference = currentHead->size;
            currentHead->size = 0;
            currentHead->currentItem = &testInt1;
            assert(List_prev(currentHead) == NULL);     //List size is 0
            assert(currentHead->currentItem == manager.outOfBoundsStart);
            assert(currentHead->current == NULL);
            currentHead->size = sizeReference;

            //List traversal test
            List_first(currentHead);
            for(int i=0; i<insertCounter; i++){
                assert(List_next(currentHead) == &testInt6);
            }
            assert(List_next(currentHead) == &testInt5);
            assert(List_next(currentHead) == &testInt1);
            assert(List_next(currentHead) == &testString);
            assert(List_next(currentHead) == &testFloat);
            assert(List_next(currentHead) == &testChar);
            assert(List_next(currentHead) == &testInt3);
            assert(List_next(currentHead) == &testInt4);
            assert(List_next(currentHead) == NULL);
            assert(List_prev(currentHead) == &testInt4);
            assert(List_prev(currentHead) == &testInt3);
            assert(List_prev(currentHead) == &testChar);
            assert(List_prev(currentHead) == &testFloat);
            assert(List_prev(currentHead) == &testString);
            assert(List_prev(currentHead) == &testInt1);
            assert(List_prev(currentHead) == &testInt5);
            for(int i=0; i<insertCounter; i++){
                assert(List_prev(currentHead) == &testInt6);
            }
            assert(List_prev(currentHead) == &testInt2);
            assert(List_prev(currentHead) == NULL);
        }
    }

//-------------------------------------------------List_search() tests-------------------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //current item before start of list
    currentHead = &manager.heads[1];
    void* reference = &testString;
    currentHead->currentItem = manager.outOfBoundsStart;
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);
    
    //current item after end of list
    currentHead->currentItem = manager.outOfBoundsEnds;
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);

    //current item 1 before reference items
    List_first(currentHead);
    for(int i=0; i<2; i++){
        List_next(currentHead);
    }
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item many before reference item
    reference = &testChar;
    List_first(currentHead);
    List_next(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item at reference item
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item 1 after reference item
    List_next(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);

    //current item many after target item
    currentHead = &manager.heads[9];
    reference = &testInt5;
    List_last(currentHead);
    List_prev(currentHead);
    List_prev(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);

    //current item at head of list (and reference item is not)
    List_first(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item at head of list (and reference item is in head)
    reference = &testInt2;
    List_first(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item at tail of list (and reference item is not)
    List_last(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);
    
    //current item at tail of list (and reference item is in tail)
    List_last(currentHead);
    reference = &testInt4;
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //current item at head of list, but list does not contain the reference item
    currentHead = &manager.heads[2];
    reference = &testInt6;
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);


//---------------------------------------------------List_remove() tests-----------------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //Current item is before the start of the list
    currentHead = &manager.heads[0];
    currentHead->currentItem = manager.outOfBoundsStart;
    sizeReference = List_count(currentHead);
    assert(List_remove(currentHead) == NULL);
    assert(currentHead->size == sizeReference);
    assert(manager.numFreeNodes == 0);
    
    //Current item is after the end of the list
    currentHead->currentItem = manager.outOfBoundsEnds;
    sizeReference = List_count(currentHead);
    assert(List_remove(currentHead) == NULL);
    assert(currentHead->size == sizeReference);
    assert(manager.numFreeNodes == 0);

    //Standard case
    reference = &testInt1;
    List_first(currentHead);
    List_next(currentHead);
    List_next(currentHead);
    Node* currentNode = currentHead->current;
    sizeReference = List_count(currentHead);
    assert(List_remove(currentHead) == reference);
    assert(List_count(currentHead)== sizeReference-1);
    assert(manager.numFreeNodes == 1);
    assert(manager.freeNodes == currentNode);
    assert(currentNode->prev == NULL);
    assert(currentNode->next == NULL);
    assert(currentNode->item == NULL);
    List_first(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == NULL);

    //Current item is the head of the list
    reference = &testInt2;
    List_first(currentHead);
    currentNode = currentHead->head;
    sizeReference = List_count(currentHead);
    assert(List_remove(currentHead) == reference);
    assert(List_count(currentHead)== sizeReference-1);
    assert(manager.numFreeNodes == 2);
    assert(manager.freeNodes->next == currentNode);
    assert(currentNode->prev == manager.freeNodes);
    assert(currentNode->next == NULL);
    assert(currentNode->item == NULL);
    assert(List_first(currentHead) == &testInt5);
    assert(currentHead->head != currentNode);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->head->next->item == &testString);

    //Current item is the tail of the list
    reference = &testInt4;
    List_last(currentHead);
    currentNode = currentHead->tail;
    sizeReference = List_count(currentHead);
    assert(List_remove(currentHead) == reference);
    assert(List_count(currentHead)== sizeReference-1);
    assert(manager.numFreeNodes == 3);
    assert(manager.freeNodes->next == currentNode);
    assert(currentNode->prev == manager.freeNodes);
    assert(currentNode->item == NULL);
    assert(List_last(currentHead) == &testInt3);
    assert(currentHead->tail != currentNode);
    List_first(currentHead);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->tail->prev->item == &testChar);
    assert(currentHead->tail->next == NULL);

    //List is of size 1
    reference = &testInt5;
    for(int i=0; i<4; i++){
        //Empty all the List's elements but one
        List_last(currentHead);
        assert(List_remove(currentHead) != NULL); 
    }  
    assert(List_count(currentHead) == 1);
    assert(manager.numFreeNodes == 7);
    assert(List_first(currentHead) == List_last(currentHead));
    currentNode = currentHead->current;
    assert(List_remove(currentHead) == reference);
    assert(List_count(currentHead) == 0);
    assert(List_curr(currentHead) == manager.outOfBoundsEnds);
    assert(List_first(currentHead) == NULL);
    assert(List_last(currentHead) == NULL);
    assert(currentHead->head == NULL);
    assert(currentHead->tail == NULL);
    assert(currentHead->current == NULL);
    assert(manager.numFreeNodes == 8);
    assert(manager.freeNodes->next == currentNode);

    //List is empty
    assert(List_remove(currentHead) == NULL);
    assert(List_count(currentHead) == 0);
    assert(List_curr(currentHead) == NULL);
    assert(List_first(currentHead) == NULL);
    assert(List_last(currentHead) == NULL);
    assert(currentHead->head == NULL);
    assert(currentHead->tail == NULL);
    assert(currentHead->current == NULL);
    assert(manager.numFreeNodes == 8);
    assert(manager.freeNodes->next == currentNode);

//-----------------------------------------------------------List_search() special case tests--------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //empty list
    reference = &testString;
    assert(List_search(currentHead, &compareItem, reference) == NULL);

    
    currentHead=&manager.heads[1];
    reference = List_first(currentHead);
    while(List_count(currentHead)>1){
        //Remove all but one Node from the list
        List_last(currentHead);
        List_remove(currentHead);
    }

    //List is of size 1 and contains the reference item, with current after the end of the list
    assert(currentHead->currentItem == manager.outOfBoundsEnds);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->currentItem == manager.outOfBoundsEnds);

    //List is of size 1 and contains the reference item, with current at the reference item
    List_prev(currentHead);
    assert(List_curr(currentHead) == reference);
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //List is of size 1 and contains the reference item, with current before the start of the list
    List_prev(currentHead);
    assert(currentHead->currentItem == manager.outOfBoundsStart);
    assert(List_search(currentHead, &compareItem, reference) == reference);
    assert(List_curr(currentHead) == reference);

    //List is of size 1 and does not contain the reference item, with current before the start of the list
    reference = &testInt0;
    List_prev(currentHead);
    assert(currentHead->currentItem == manager.outOfBoundsStart);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->currentItem == manager.outOfBoundsEnds);

    //List is of size 1 and does not contain the reference item, with current at the reference item
    List_first(currentHead);
    assert(List_curr(currentHead) ==& testInt2);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->currentItem == manager.outOfBoundsEnds);

    //List is of size 1 and does not contain the reference item, with current after the end of the list
    assert(currentHead->currentItem == manager.outOfBoundsEnds);
    assert(List_search(currentHead, &compareItem, reference) == NULL);
    assert(currentHead->currentItem == manager.outOfBoundsEnds);

//-------------------------------------------List_insert_after() special case tests--------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //List is of size 1, and current is after the list
    assert(List_insert_after(currentHead, &testInt7)==0);
    assert(List_curr(currentHead) == &testInt7);
    assert(List_curr(currentHead) == List_last(currentHead));
    assert(List_count(currentHead) == 2);

    //List is of size 1, and we want to insert a NULL item
    List_remove(currentHead);
    List_first(currentHead);
    assert(List_insert_after(currentHead, NULL)==0);
    assert(List_curr(currentHead) == List_last(currentHead));
    assert(List_count(currentHead) == 2);

    //List is empty, and we want to insert a item with value of NULL
    currentHead = &manager.heads[0];
    List_first(currentHead);
    assert(List_insert_after(currentHead, NULL)==0);
    assert(List_curr(currentHead) == List_last(currentHead));
    assert(List_count(currentHead) == 1);

//----------------------------------------------List_insert_before() tests--------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    List_remove(currentHead);   //Make our list empty again

    //List size is 0, current is before list
    assert(currentHead->size == 0); 
    reference = &testFloat;
    List_prev(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->tail);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == NULL);
    assert(currentHead->size == 1); 
    List_remove(currentHead);

    //List size is 0, current is after list
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->tail);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == NULL);
    assert(currentHead->size == 1); 
    List_remove(currentHead);

    //List size is 0, current is before list, trying to insert NULL item
    reference = NULL;
    List_prev(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->tail);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == NULL);
    assert(currentHead->size == 1); 
    List_remove(currentHead);

    //List size is 0, current is after list, trying to insert NULL item
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->tail);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == NULL);
    assert(currentHead->size == 1); 

    //List size is 1, current is before list
    reference = &testFloat;
    currentHead->currentItem = manager.outOfBoundsStart;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == NULL);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is 1, current is at the node
    List_first(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == NULL);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is 1, current is after list
    List_next(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == NULL);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->tail == currentHead->current);
    assert(currentHead->current->next == NULL);
    assert(currentHead->current->prev == currentHead->head);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is 1, current is before list, trying to insert NULL item
    reference = NULL;
    currentHead->currentItem = manager.outOfBoundsStart;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == NULL);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is 1, current is at the node, trying to insert NULL item
    List_first(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == reference);
    assert(currentHead->tail->item == NULL);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->current->prev == NULL);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is 1, current is after list, trying to insert NULL item
    List_next(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->item == NULL);
    assert(currentHead->tail->item == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->currentItem == reference);
    assert(currentHead->tail == currentHead->current);
    assert(currentHead->current->next == NULL);
    assert(currentHead->current->prev == currentHead->head);
    assert(currentHead->size == 2); 
    List_remove(currentHead);

    //List size is >1, current is before list
    reference = &testInt0;
    currentHead= &manager.heads[6];
    Node* oldHead = currentHead->head;
    Node* oldTail = currentHead->tail;
    Node* oldTailPrev = currentHead->tail->prev;
    List_first(currentHead);
    List_prev(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->tail != currentHead->head);
    assert(currentHead->head != oldHead);
    assert(currentHead->head->next == oldHead);
    assert(oldHead->prev == currentHead->head);
    assert(currentHead->tail == oldTail);
    assert(currentHead->tail->prev == oldTail->prev);
    List_remove(currentHead);

    //List size is >1, current is at head
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    List_first(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->tail != currentHead->head);
    assert(currentHead->head != oldHead);
    assert(currentHead->head->next == oldHead);
    assert(oldHead->prev == currentHead->head);
    assert(currentHead->tail == oldTail);
    assert(currentHead->tail->prev == oldTail->prev);
    List_remove(currentHead);

    //List size is >1, current is at a node
    List_next(currentHead);
    List_next(currentHead);
    List_next(currentHead);
    Node* oldNode = currentHead->current;
    Node* oldNodePrev = oldNode->prev;
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail == oldTail);
    assert(currentHead->current->next == oldNode);
    assert(currentHead->current->prev == oldNodePrev);
    assert(currentHead->tail->prev == oldTailPrev);
    assert(oldNodePrev->next = currentHead->current);
    assert(oldNode->prev == currentHead->current);
    List_remove(currentHead);

    //List size is >1, current is at tail
    List_last(currentHead);
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail == oldTail);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->current->prev == oldTailPrev);
    assert(oldTailPrev->next = currentHead->current);
    assert(currentHead->tail->prev == currentHead->current);
    List_remove(currentHead);

    //List size is >1, current is after list
    List_last(currentHead);
    List_next(currentHead);
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail != oldTail);
    assert(currentHead->tail == currentHead->current);
    assert(currentHead->current->prev == oldTail);
    assert(oldTail->next = currentHead->current);
    assert(oldTail->prev == oldTailPrev);
    List_remove(currentHead);

    //List size is >1, current is before list, trying to insert NULL item
    reference = NULL;
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    List_first(currentHead);
    List_prev(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->tail != currentHead->head);
    assert(currentHead->head != oldHead);
    assert(currentHead->head->next == oldHead);
    assert(oldHead->prev == currentHead->head);
    assert(currentHead->tail == oldTail);
    assert(currentHead->tail->prev == oldTail->prev);
    List_remove(currentHead);

    //List size is >1, current is at head, trying to insert NULL item
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    List_first(currentHead);
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == currentHead->current);
    assert(currentHead->tail != currentHead->head);
    assert(currentHead->head != oldHead);
    assert(currentHead->head->next == oldHead);
    assert(oldHead->prev == currentHead->head);
    assert(currentHead->tail == oldTail);
    assert(currentHead->tail->prev == oldTail->prev);
    List_remove(currentHead);

    //List size is >1, current is at a node, trying to insert NULL item
    List_next(currentHead);
    List_next(currentHead);
    List_next(currentHead);
    oldNode = currentHead->current;
    oldNodePrev = oldNode->prev;
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail == oldTail);
    assert(currentHead->current->next == oldNode);
    assert(currentHead->current->prev == oldNodePrev);
    assert(currentHead->tail->prev == oldTailPrev);
    assert(oldNodePrev->next = currentHead->current);
    assert(oldNode->prev == currentHead->current);
    List_remove(currentHead);

    //List size is >1, current is at tail, trying to insert NULL item
    List_last(currentHead);
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail == oldTail);
    assert(currentHead->current->next == currentHead->tail);
    assert(currentHead->current->prev == oldTailPrev);
    assert(oldTailPrev->next = currentHead->current);
    assert(currentHead->tail->prev == currentHead->current);
    List_remove(currentHead);

    //List size is >1, current is after list, trying to insert NULL item    List_last(currentHead);
    List_next(currentHead);
    oldHead = currentHead->head;
    oldTail = currentHead->tail;
    oldTailPrev = currentHead->tail->prev;
    assert(List_insert_before(currentHead, reference) == 0);
    assert(currentHead->head->prev == NULL);
    assert(currentHead->tail->next == NULL);
    assert(currentHead->currentItem == reference);
    assert(currentHead->current->item == reference);
    assert(currentHead->head == oldHead);
    assert(currentHead->tail != oldTail);
    assert(currentHead->tail == currentHead->current);
    assert(currentHead->current->prev == oldTail);
    assert(oldTail->next = currentHead->current);
    assert(oldTail->prev == oldTailPrev);
    List_remove(currentHead);

//---------------------------------------------------List_trim() tests--------------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested
    List* trimList = &manager.heads[3];
    List* referenceList = &manager.heads[4];
    assert(trimList->tail->item == referenceList->tail->item);
    assert(trimList->head->item == referenceList->head->item);
    assert(trimList->size == referenceList->size);
    void* trimItem;
    void* referenceItem;

    //Full list scenario
    trimItem = List_trim(trimList);
    List_last(referenceList);
    referenceItem = List_remove(referenceList);
    assert(trimList->size == referenceList->size);
    assert(trimList->tail->item == referenceList->tail->item);
    assert(trimList->tail->next == NULL);
    assert(trimList->head->prev == NULL);
    assert(trimItem == referenceItem);
    List_last(referenceList);
    for(int i=0; i<List_count(trimList); i++){
        List_prev(referenceList);
        List_prev(trimList);
        assert(trimList->currentItem == referenceList->currentItem);
    }

    //Emptying a list to its last node
    while(List_count(referenceList)>1){
        trimItem = List_trim(trimList);
        List_last(referenceList);
        referenceItem = List_remove(referenceList);
        assert(trimItem == referenceItem);
        assert(trimList->size == referenceList->size);
        assert(trimList->tail->item == referenceList->tail->item);
        assert(trimList->tail->next == NULL);
        assert(trimList->head->prev == NULL);
    }

    //Singleton list scenario
    trimItem = List_trim(trimList);
    List_last(referenceList);
    referenceItem = List_remove(referenceList);
    assert(trimItem == referenceItem);
    assert(trimList->size == 0);
    assert(trimList->tail == NULL);
    assert(trimList->head == NULL);
    assert(trimList->current == NULL);
    assert(trimList->currentItem == manager.outOfBoundsEnds);

    //Empty list scenario
    trimItem = List_trim(trimList);
    assert(trimItem == NULL);
    assert(trimList->size == 0);
    assert(trimList->tail == NULL);
    assert(trimList->head == NULL);
    assert(trimList->current == NULL);
    assert(trimList->currentItem == manager.outOfBoundsEnds);

//-----------------------------------------------------List_free() tests--------------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //Empty list
    currentHead = &manager.heads[0];
    while(List_trim(currentHead)!=NULL);
    assert(currentHead->size == 0);
    List_free(currentHead, &freeItem);
    assert(currentHead->head == NULL);
    assert(currentHead->tail == NULL);
    assert(currentHead->current == NULL);
    assert(currentHead->currentItem == NULL);
    assert(currentHead->size == 0);
    currentFreeHead = manager.freeHeads;
    bool foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == currentHead){
            foundHead = true;
            break;
        }
    }
    assert(foundHead == true);

    //Singleton list
    currentHead = List_create();
    List_insert_after(currentHead, &testInt0);
    assert(currentHead->size == 1);
    List_free(currentHead, &freeItem);
    assert(currentHead->head == NULL);
    assert(currentHead->tail == NULL);
    assert(currentHead->current == NULL);
    assert(currentHead->currentItem == NULL);
    assert(currentHead->size == 0);
    currentFreeHead = manager.freeHeads;
    foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == currentHead){
            foundHead = true;
            break;
        }
        currentFreeHead = currentFreeHead->next;
    }
    assert(foundHead == true);

    //List size > 1
    currentHead = &manager.heads[5];
    assert(currentHead->size == 8);
    List_free(currentHead, &freeItem);
    assert(currentHead->head == NULL);
    assert(currentHead->tail == NULL);
    assert(currentHead->current == NULL);
    assert(currentHead->currentItem == NULL);
    assert(currentHead->size == 0);
    currentFreeHead = manager.freeHeads;
    foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == currentHead){
            foundHead = true;
            break;
        }
        currentFreeHead = currentFreeHead->next;
    }
    assert(foundHead == true);

//---------------------------------------------------List_concat() tests--------------------------------------------------------------------------------//
    //Each comment at this indentation level covers a possible case for the function being tested

    //pList1 & pList2 are both empty
    assert(manager.numFreeHeads > 1);
    List* list1 = List_create();
    List* list2 = List_create();
    Node* prevList1current = list1->current;
    Node* prevList1currentItem = list1->currentItem;
    List_concat(list1, list2);
    assert(list1->head == NULL);
    assert(list1->tail == NULL);
    assert(list1->current == prevList1current);
    assert(list1->currentItem == prevList1currentItem);
    assert(list1->size == 0);
    assert(list2->head == NULL);
    assert(list2->tail == NULL);
    assert(list2->current == NULL);
    assert(list2->currentItem == NULL);
    assert(list2->size == 0);
    currentFreeHead = manager.freeHeads;
    foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == list2){
            foundHead = true;
        }
        assert(currentFreeHead != list1);
        currentFreeHead = currentFreeHead->next;
    }

    //pList1 is singleton, pList2 is empty
    List_insert_after(list1, &testInt0);
    list2 = List_create();
    prevList1current = list1->current;
    prevList1currentItem = list1->currentItem;
    List_concat(list1, list2);
    assert(list1->head->item == list1->tail->item);
    assert(list1->tail == list1->current);
    assert(list1->current == prevList1current);
    assert(list1->currentItem == prevList1currentItem);
    assert(list1->size == 1);
    assert(list2->head == NULL);
    assert(list2->tail == NULL);
    assert(list2->current == NULL);
    assert(list2->currentItem == NULL);
    assert(list2->size == 0);
    currentFreeHead = manager.freeHeads;
    foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == list2){
            foundHead = true;
        }
        assert(currentFreeHead != list1);
        currentFreeHead = currentFreeHead->next;
    }

    //pList1 is empty, pList2 is singleton
    List_trim(list1);
    assert(List_count(list1) == 0);
    list2 = List_create();
    List_insert_after(list2, &testInt0);
    prevList1current = list1->current;
    prevList1currentItem = list1->currentItem;
    Node* prevList2Head = list2->head;
    Node* prevList2Tail = list2->tail;
    List_concat(list1, list2);
    assert(list1->head->item == list1->tail->item);//
    assert(list1->tail == list1->head);
    assert(list1->current == prevList1current);
    assert(list1->currentItem == prevList1currentItem);
    assert(list1->size == 1);
    assert(list2->head == NULL);
    assert(list2->tail == NULL);
    assert(list2->current == NULL);
    assert(list2->currentItem == NULL);
    assert(list2->size == 0);
    currentFreeHead = manager.freeHeads;
    foundHead = false;
    assert(currentFreeHead != NULL);
    while(currentFreeHead!=NULL){
        if(currentFreeHead == list2){
            foundHead = true;
        }
        assert(currentFreeHead != list1);
        currentFreeHead = currentFreeHead->next;
    }

    //pList1 & pList2 are both singleton
    //pList1->size>1, pList2 is empty
    //pList1 is empty, pList2->size>1
    //pList1 & pList2 both have size>1
}

