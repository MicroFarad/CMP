/*
Header file for linked list implementation

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

#include "poly.h"

// include guard
#ifndef LIST_H
#define LIST_H

// a definition for NULL may be needed
#ifndef NULL
#define NULL ((void*)0)
#endif

// represents a node in a linked list
typedef struct LIST_Node
{
	struct LIST_Node* prev;
	struct LIST_Node* next;
	POLY_Polymorphic value;
} LIST_Node;

// represents a linked list
typedef struct LIST_List
{
	LIST_Node* first;
	LIST_Node* last;
	unsigned long size;
} LIST_List;

// represents an iterator for a list
typedef struct LIST_Iterator
{
	LIST_List* list;
	LIST_Node* current;
} LIST_Iterator;

// initialize a list
// takes a pointer to the list to initialize and the function used to destroy values, may be NULL
// returns a pointer to the list
LIST_List* LIST_Initialize(LIST_List* list);

// remove all items from a list and free memory
// takes a pointer to the list
void LIST_Clear(LIST_List* list);

// insert at the head of a list
// takes a pointer to the list and the value to insert
void LIST_InsertHead(LIST_List* list, POLY_Polymorphic value);

// insert at the tail of a list
// takes a pointer to the list and the value to insert
void LIST_InsertTail(LIST_List* list, POLY_Polymorphic value);

// take a value from the head of a list
// takes a pointer to the list
// returns the value removed
POLY_Polymorphic LIST_TakeHead(LIST_List* list);

// take a value from the tail of a list
// takes a pointer to the list
// returns the value removed
POLY_Polymorphic LIST_TakeTail(LIST_List* list);

// get a value from the head of a list without removing it
// takes a pointer to the list
// returns the value
POLY_Polymorphic LIST_PeekHead(LIST_List* list);

// get a value from the tail of a list without removing it
// takes a pointer to the list
// returns the value
POLY_Polymorphic LIST_PeekTail(LIST_List* list);

// finds the size of a list
// takes a pointer to the list
// returns the number of elements in the list
unsigned long LIST_Size(LIST_List* list);

// initialize an iterator for a list
// takes a pointer to the iterator to initialize and a pointer to the list upon which to iterate
// returns a pointer to the iterator
LIST_Iterator* LIST_InitializeIterator(LIST_List* list, LIST_Iterator* iterator);

// gets the next element from an iterator
// takes a pointer to the iterator
// returns 0 if the end has been reached, 1 otherwise
int LIST_Next(LIST_Iterator* iterator);

// gets the current element from an iterator
// takes a pointer to the iterator
// returns the value
POLY_Polymorphic LIST_Peek(LIST_Iterator* iterator);

// resets an iterator to the beginning
// takes a pointer to the iterator
void LIST_Reset(LIST_Iterator* iterator);

#endif
