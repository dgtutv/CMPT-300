#include "list.c"
#include <stdio.h>
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
    List* nullList = 0;

    //Create 10 lists, should all be defined, and unique
    List* newList = List_create();
    assert(newList != nullList);    //Test our list is not undefined
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
        assert(newList != nullList);    //Test our list is not undefined
        assert(newList != prevList);    //Test our list is also not the same as the previously passed list 
        prevList = newList;
    }

    //There should be no more free heads
    assert(manager.freeHeads == 0);

    //All of the heads should be taken
    assert(manager.numHeads == 10);

    //Attempting to create more than 10 lists will return a NULL pointer
    newList = List_create();
    assert(newList == nullList);
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
}