// List data type
// You may modify this file as needed; however,
// you may *NOT* modify the function prototypes or constant names.

#ifndef _LIST_H_
#define _LIST_H_
#include <stdbool.h>

#define LIST_SUCCESS 0
#define LIST_FAIL -1

typedef struct Node_s Node;
typedef struct Manager_s Manager;
typedef struct List_s List;
typedef struct Item_s Item;

//Structure for a node of a list
struct Node_s {
    Node* next;
    Node* prev;
    int index;  //The permanent index of the node (where it actually is in our array)
    void* item;
};

enum ListOutOfBounds {
    LIST_OOB_START,
    LIST_OOB_ENDS
};
//Structure to manage a list
struct List_s {
    Node* head;  //First node in list (NULL if this list is not in use)
    Node* tail;  //Last node in list (NULL if this list is not in use)
    Node* current;   //Current node being accessed in list (NULL if this list is not in use, head if list is empty), this will be of ListOutOfBounds type if before or after the list
    void* currentItem;      //Stores the current item of the List, will be the same as Node current item, if not out of bounds
    List* next;  //The next list in the freeHeads array (this is NULL if this list is in use)
    List* prev;  //The previous list in the freeHeads array
    int index;  //The permanent index of the list (where it actually is in our head array)
    int size;   //The size of the list
};

// Maximum number of unique lists the system can support
// (You may modify this, but reset the value to 10 when handing in your assignment)
#define LIST_MAX_NUM_HEADS 10

// Maximum total number of nodes (statically allocated) to be shared across all lists
// (You may modify this, but reset the value to 100 when handing in your assignment)
#define LIST_MAX_NUM_NODES 100

static Node nodeArr[LIST_MAX_NUM_NODES];    //An array of all of our nodes
static List headArr[LIST_MAX_NUM_HEADS];    //Array of all our heads


//Structure to manage our nodes and lists
struct Manager_s {
    Node* nodes;
    List* heads;
    Node* freeNodes;    //Linked list of free nodes
    int numHeads;       //Count of heads currently in use
    int numFreeNodes;   //Count of nodes available to use
    List* freeHeads;    //Linked list of all our free heads
    void* outOfBoundsStart; //This item is pointed to when the current item in list is before the start of the list
    void* outOfBoundsEnds;  //This item is pointed to when the current item in list is after the end of
};

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create();

// Returns the number of items in pList.
int List_count(List* pList);

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList);

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList); 

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList);

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList);

// Returns a pointer to the current item in pList.
void* List_curr(List* pList);

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem);

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem);

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem);

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem);

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList);

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

#endif