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
//TODO: Check for coding style guidelines
//TODO: add better comments to tests
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
    assert(currentFreeHead->index == 9);    //Index will be 1 higher than the list size, since we take from the front
    assert(i==8);

    //Test the conditions of our first list
    assert(newList != NULL);  
    assert(manager.numHeads == 1);
    List* currentHead = &manager.heads[0];
    assert(newList == currentHead); 
    //Define pointers to iterate over the head lists in manager 
    List* prevList = newList;
    List* prevHead;
    //Check the initial conditions of our head lists
    for(int i=1; i<10; i++){
        newList = List_create();
        assert(manager.numHeads == i+1);    
        prevHead = currentHead;
        currentHead = &manager.heads[i];
        assert(newList == currentHead);       
        assert(prevList == prevHead);     
        assert(newList != NULL);    
        assert(newList != prevList);    
        assert(newList->next == NULL);
        assert(newList->prev == NULL);
        assert(newList->size == 0);
        assert(newList->index == i);
        prevList = newList;
    }
    

    //There should be no more free heads
    assert(manager.freeHeads == 0);

    //All of the heads should be taken
    assert(manager.numHeads == 10);

    //Attempting to create more than 10 lists will return a NULL pointer
    newList = List_create();
    assert(newList == NULL);
    assert(manager.numHeads == 10);     //Test the number of heads is still 10

    //Test that our out of bounds variables have been defined
    enum ListOutOfBounds start = LIST_OOB_START;
    enum ListOutOfBounds end = LIST_OOB_ENDS;
    assert(*(int*)manager.outOfBoundsStart == start);
    assert(*(int*)manager.outOfBoundsEnds == end);

    //------------------------Tests for List_insert_after(), List_count(), List_first(), List_last(), List_next(), List_prev() & List_curr()---------------------------------------------------------------------------//
    

    //Test for all heads to cover edge cases
    for(int i=0; i<10; i++){
        //Size of list is 0 tests
        currentHead = &manager.heads[i];
        assert(List_count(currentHead) == 0);
        assert(List_first(currentHead) == NULL);    
        assert(List_last(currentHead) == NULL);     
        assert(List_next(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        assert(List_prev(currentHead) == NULL);
        assert(currentHead->currentItem == manager.outOfBoundsStart);

    //Inserting an item into an empty list 
        //The head, tail, and current Nodes should all be the address of the item
        int testInt = 1;
        assert(List_insert_after(currentHead, &testInt) == 0);  
        assert(currentHead->head->item == &testInt);
        assert(currentHead->tail->item == &testInt);
        assert(currentHead->current->item == &testInt);
        assert(currentHead->currentItem == &testInt);
        assert(currentHead->size == 1);     //Size should be 1
        assert(List_count(currentHead) == 1);

        //The head, tail, and current Nodes should all be the same pointer
        assert(currentHead->head == currentHead->tail);
        assert(currentHead->head == currentHead->current);

        //List_next() tests
        assert(List_next(currentHead) == NULL);     //Current == tail condition
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == &testInt);   //Item is before the start of the list
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        currentHead->currentItem = &testInt;
        int headSize = currentHead->size;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        currentHead->size = headSize;

        //List_prev() tests
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        assert(List_prev(currentHead) == NULL);     //Current == head condition
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_prev(currentHead) == NULL);   //Item is before the start of the list condition
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_prev(currentHead) == currentHead->tail->item);     //Item is beyond the end of the list condition
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->currentItem == currentHead->current->item);
        headSize = currentHead->size;
        currentHead->size = 0;
        currentHead->currentItem = &testInt;
        assert(List_prev(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsStart);
        currentHead->size = headSize;

        //List_first() test
        assert(List_first(currentHead) == &testInt);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testInt);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //The head, tail, and current Nodes should have no connections (i.e. next, and prev should be NULL)
        assert(currentHead->current->prev == NULL);
        assert(currentHead->current->next == NULL);

    //Inserting an item after the head should make the item the tail, and the current item of the list, keeping head the same
        float testFloat = 3.14;
        assert(List_insert_after(currentHead, &testFloat) == 0);
        assert(currentHead->currentItem == &testFloat);
        assert(currentHead->current->item == &testFloat);
        assert(currentHead->current == currentHead->tail);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->size == 2);
        assert(List_count(currentHead) == 2);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testFloat);
        assert(currentHead->current->item == &testFloat);

        //List_first() test
        assert(List_first(currentHead) == &testInt);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_next() test 
        assert(List_next(currentHead) == &testFloat);   //Should be the node after head, which is the float
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem = &testFloat);
        currentHead->currentItem = manager.outOfBoundsStart;
        assert(List_next(currentHead) == &testInt);   //Item is before the start of the list
        currentHead->currentItem = manager.outOfBoundsEnds;
        assert(List_next(currentHead) == NULL);     //Item is beyond the end of the list
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        headSize = currentHead->size;
        currentHead->size = 0;
        assert(List_next(currentHead) == NULL);     //List size is 0
        assert(currentHead->currentItem == manager.outOfBoundsEnds);
        currentHead->size = headSize;

        //List_last() test
        assert(List_last(currentHead) == &testFloat);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

    //Inserting an item after the head again should make the item just the current item of the list, keeping both head and tail the same
        char* testString = "TEST";
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

        //List_next() test 
        assert(List_next(currentHead) == currentHead->tail->item);  //Should be the node after this, the tail
        assert(currentHead->currentItem == currentHead->tail->item);
        assert(currentHead->current == currentHead->tail);
        currentHead->current = currentHead->current->prev->prev;
        currentHead->currentItem = currentHead->current->item;
        assert(List_next(currentHead) == &testString);   //Should be the node after float, which is the string
        assert(currentHead->currentItem == currentHead->current->item);
        assert(currentHead->currentItem = &testString);

        //List_first() test
        assert(List_first(currentHead) == &testInt);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testFloat);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

    //Inserting an item after the tail (current pointer is at tail) should make the item both the tail and current item of the list, keeping head the same
        char testChar = 'T';
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

        //List_first() test
        assert(List_first(currentHead) == &testInt);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

        //List_last() test
        assert(List_last(currentHead) == &testChar);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

    //Inserting an item at the head (current pointer is at LIST_OOB_START) should make both the head, and the current pointer the item, keeping tail the same
        int testInt2 = 2;
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
        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);
        //List_last() test
        assert(List_last(currentHead) == &testChar);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);

    //Inserting an item after the tail (current pointer is at LIST_OOB_ENDS) should make the item both the tail and current item of the list, keeping head the same
        int testInt3 = 3;
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
        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);
        //List_last() test
        assert(List_last(currentHead) == &testInt3);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);
    
    //Inserting an item after the tail (current pointer is at tail) should make the item both the tail and current item of the list, keeping head the same
        int testInt4 = 4;
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
        //List_last() test
        assert(List_last(currentHead) == &testInt4);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);
        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);
        
    //If the freeNodes list is not singleton when inserting an item, the next free node should become the new head of the freeNodes list (inserting after head)
        Node* freeNodesNext = manager.freeNodes->next;
        int testInt5 = 5;
        assert(List_insert_after(currentHead, &testInt5) == 0);
        assert(currentHead->size == 8);
        assert(List_count(currentHead) == 8);
        assert(currentHead->head->prev == NULL);
        assert(currentHead->tail->next == NULL);
        assert(currentHead->currentItem == &testInt5);
        assert(currentHead->current->item == &testInt5);
        assert(currentHead->head != currentHead->current);
        assert(currentHead->tail != currentHead->head);
        assert(currentHead->tail != currentHead->current);
        assert(manager.freeNodes == freeNodesNext);
        assert(manager.freeNodes->prev == NULL);
        //List_last() test
        assert(List_last(currentHead) == &testInt4);
        assert(currentHead->tail == currentHead->current);
        assert(currentHead->tail->item == currentHead->currentItem);
        //List_first() test
        assert(List_first(currentHead) == &testInt2);
        assert(currentHead->head == currentHead->current);
        assert(currentHead->head->item == currentHead->currentItem);

    //DO LIST TESTS HERE

    //If the freeNodes list is singleton when inserting an item, the freeNodes list should sieze to exist after the operation (inserting after the head)
        currentHead->current = currentHead->head;
        currentHead->currentItem = currentHead->current->item;
        int testInt6 = 6;
        if(i==9){
            while(manager.numFreeNodes != 0){
                assert(List_insert_after(currentHead, &testInt6)==0);
            }
            assert(manager.numFreeNodes == 0);
        }
    
    //Inserting an item when there are no more Nodes available should return -1
        int testInt7 = 7;
        if(i==9){
            assert(List_insert_after(currentHead, &testInt7) == -1);
        }
    }

    //TODO: Use List_first(), List_last(), List_next(), List_prev(), List_size(), etc functions to test other functions from here on
//---------------------------------------------Tests for List_insert_after()---------------------------------------------------------------------------//

}