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

//TODO: Create tests for each function
int main(){
    extern Manager manager; //our manager from list.c

//----------------------------------------------Tests for List_create()---------------------------------------------------------------------------//

    //Create 10 lists, should all be defined, and unique
    List* newList = List_create();
    assert(newList != NULL);    //Test our list is not undefined
    assert(manager.numHeads == 1);      //Test our numHeads integer is correct
    List* currentHead = &manager.heads[0];
    assert(newList == currentHead);       //Test our newly defined list is the same list present in our heads linked list
    List* prevList = newList;
    List* prevHead;
    for(int i=1; i<10; i++){
        newList = List_create();
        assert(manager.numHeads == i+1);    //Test our numHeads integer is correct
        prevHead = currentHead;
        currentHead = &manager.heads[i];
        assert(newList == currentHead);       //Test our newly defined list is the same list present in our heads linked list
        assert(prevList == prevHead);      //Test our previously defined list is still present in our heads linked list
        assert(newList != NULL);    //Test our list is not undefined
        assert(newList != prevList);    //Test our list is also not the same as the previously passed list 
        prevList = newList;
    }

    //There should be no more free heads
    assert(manager.freeHeads == 0);

    //All of the heads should be taken
    assert(manager.numHeads == 10);

    //Attempting to create more than 10 lists will return a NULL pointer
    newList = List_create();
    assert(newList == NULL);
    assert(manager.numHeads == 10);     //Test the number of heads is still 10\

    //Check that all of the Nodes are currently free
    Node* currentNode = manager.freeNodes;
    Node* prevNode;
    for(int i=0; i<100; i++){
        assert(currentNode == &manager.nodes[i]);
        prevNode = currentNode;
        currentNode = currentNode->next;
        if(i==99){
            assert(currentNode == 0);        //Check that there are only 100 free Nodes
        }
        else{
            assert(currentNode->prev == prevNode);   //Check for correct double linking of Nodes in freeHeads list
        }
    }

    //---------------------------------------------Tests for List_insert_after()---------------------------------------------------------------------------//

    //Test for all heads to cover edge cases
    for(int i=0; i<10; i++){
        currentHead = &manager.heads[i];
    //Inserting an item into an empty list 
        //The head, tail, and current Nodes should all be the address of the item
        int* testNum = (int*)malloc(sizeof(int));
        *testNum = 1;
        assert(List_insert_after(currentHead, testNum) == 0);  
        assert(currentHead->head->item == testNum);
        assert(currentHead->tail->item == testNum);
        assert(currentHead->current->item == testNum);
        assert(currentHead->currentItem = testNum);
        assert(currentHead->size == 1);     //Size should be 1
        //The head, tail, and current Nodes should all be the same pointer
        assert(currentHead->head == currentHead->tail);
        assert(currentHead->head == currentHead->current);
        //The head, tail, and current Nodes should have no connections (i.e. next, and prev should be NULL)
        assert(currentHead->current->prev == NULL);
        assert(currentHead->current->next == NULL);
    //Inserting an item after the head should make the item the tail, and the current item of the list, keeping head the same
        //Inserting an item after the head again should make the item just the current item of the list, keeping both head and tail the same
        //Inserting an item after the tail (current pointer is at tail) should make the item both the tail and current item of the list, keeping head the same
        //Inserting an item at the head (current pointer is at LIST_OOB_START) should make both the head, and the current pointer the item keeping tail the same
        //Inserting an item after the tail (current pointer is at LIST_OOB_ENDS) should make the item both the tail and current item of the list, keeping head the same
        //Inserting an item when there are no more Nodes available should return NULL
    }
    

    
}