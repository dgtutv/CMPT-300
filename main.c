#include "list.c"
#include <stdio.h>
#include <assert.h>

int main(){
    extern Manager manager; //our manager from list.c

    //Tests for List_create()
    List* newList = List_create();
    List* nullList = 0;
    assert(newList != nullList);    //Test our list is not undefined

    //Tests for 
}