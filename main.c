#include "list.c"
#include <stdio.h>
#include <assert.h>


//My implementation of FREE_FN from list.c
void freeItem(void* item){
    return;     //I will test with static memory for simplicity
}

int main(){
    extern Manager manager; //our manager from list.c
    //Tests for List_create()
    List* newList = List_create();
    List* nullList = 0;
    assert(newList != nullList);    //Test our list is not undefined

    //TODO: Create tests for each function
    //TODO: Implement FREE_FN
    //TODO: Implement COMPARATOR_FN
}