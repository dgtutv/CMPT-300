#include <list.h>
#include <stdio.h>
#include <assert.h>

int main(){
    extern Manager manager; //our manager from list.c
    List* newList = List_create();
    List* nullList = 0;
    assert(newList != nullList);    //Test our list is not undefined
    //Issue with asserting that the list isnt empty
}