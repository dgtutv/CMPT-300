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

//--------Tests for List_create()---------//
    List* nullList = 0;

    //Create 10 lists, should all be defined, and unique
    List* newList = List_create();
    assert(newList != nullList);    //Test our list is not undefined
    List* prevList = newList;
    for(int i=1; i<10; i++){
        newList = List_create();
        assert(newList != nullList);    //Test our list is not undefined
        assert(newList != prevList);    //Test our list is also not the same as the previously passed list 
        prevList = newList;
    }

    //Attempting to create more than 10 lists will return a NULL pointer
    newList = List_create();
    assert(newList == nullList);
}