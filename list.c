/*
Source file for linked list implementation

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

#include <stdlib.h>
#include "list.h"

LIST_List* LIST_Initialize(LIST_List* list)
{
	list->first = NULL;
	list->last = NULL;
	list->size = 0;
	return list;
}

// helper function frees memory of node and its key
// takes a pointer to the list and the node to destroy
void LIST_DestroyNode(LIST_List* list, LIST_Node* node)
{
	free(node);
}

// remove all items from a list and free memory
// takes a pointer to the list
void LIST_Clear(LIST_List* list)
{
	LIST_Node* node = list->first;
	while(node)
	{
		LIST_Node* next = node->next;
		LIST_DestroyNode(list, node);
		node = next;
	}
	list->size = 0;
}

// helper function inserts a node between two other nodes or at either end of a list
// takes a pointer to the list, the value to insert, and pointers to the previous and next nodes
void LIST_Insert(LIST_List* list, POLY_Polymorphic value, LIST_Node* prev, LIST_Node* next)
{
	LIST_Node* node = (LIST_Node*)malloc(sizeof(LIST_Node));
	node->value = value;
	node->prev = prev;
	node->next = next;
	if(prev) prev->next = node;
	else list->first = node;
	if(next) next->prev = node;
	else list->last = node;
	list->size++;
}

void LIST_InsertHead(LIST_List* list, POLY_Polymorphic value)
{
	LIST_Insert(list, value, NULL, list->first);
}

void LIST_InsertTail(LIST_List* list, POLY_Polymorphic value)
{
	LIST_Insert(list, value, list->last, NULL);
}

// helper function deletes a node
// takes a pointer to the list and a pointer to the node
void LIST_Delete(LIST_List* list, LIST_Node* node)
{
	if(node->prev) node->prev->next = node->next;
	else list->first = node->next;
	if(node->next) node->next->prev = node->prev;
	else list->last = node->prev;
	LIST_DestroyNode(list, node);
	list->size--;
}

POLY_Polymorphic LIST_TakeHead(LIST_List* list)
{
	if(!list->first) return POLY_DEFAULT;
	POLY_Polymorphic value = list->first->value;
	LIST_Delete(list, list->first);
	return value;
}

POLY_Polymorphic LIST_TakeTail(LIST_List* list)
{
	if(!list->last) return POLY_DEFAULT;
	POLY_Polymorphic value = list->last->value;
	LIST_Delete(list, list->last);
	return value;
}

POLY_Polymorphic LIST_PeekHead(LIST_List* list)
{
	if(!list->first) return POLY_DEFAULT;
	return list->first->value;
}

POLY_Polymorphic LIST_PeekTail(LIST_List* list)
{
	if(!list->last) return POLY_DEFAULT;
	return list->last->value;
}

unsigned long LIST_Size(LIST_List* list)
{
	return list->size;
}

LIST_Iterator* LIST_InitializeIterator(LIST_List* list, LIST_Iterator* iterator)
{
	iterator->list = list;
	iterator->current = NULL;
	return iterator;
}

int LIST_Next(LIST_Iterator* iterator)
{
	if(iterator->current)
	{
		iterator->current = iterator->current->next;
	}
	else
	{
		iterator->current = iterator->list->first;
	}
	if(iterator->current) return 1;
	else return 0;
}

POLY_Polymorphic LIST_Peek(LIST_Iterator* iterator)
{
	if(iterator->current) return iterator->current->value;
	else return POLY_DEFAULT;
}

void LIST_Reset(LIST_Iterator* iterator)
{
	iterator->current = NULL;
}

