/*
Header file for AVL tree set or tree map implementation

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

// for polymorphism
#include "poly.h"

// include guard
#ifndef AVL_H
#define AVL_H

// a definition for NULL may be needed
#ifndef NULL
#define NULL ((void*)0)
#endif

// casting polymorphism
#define AVL_POLYTREE(value) ((AVL_Tree*)value.ref)

// function pointer type for comparator used to sort keys in set
// takes pointers to the keys to compare
// returns zero if *key1 == *key2, negative value if *key1 < *key2, and positive if *key1 > *key2
typedef int (*AVL_Comparator)(POLY_Polymorphic key1, POLY_Polymorphic key2);

// function pointer type for destroyer used to clean up keys and values
// takes the item 
typedef void (*AVL_Destroyer)(POLY_Polymorphic item);

// represents a node in an AVL tree
typedef struct AVL_Node
{
	struct AVL_Node *parent;
	struct AVL_Node *left;
	struct AVL_Node *right;
	int height;
	POLY_Polymorphic key;
	POLY_Polymorphic value;
} AVL_Node;

// represents an AVL tree
typedef struct AVL_Tree
{
	AVL_Node *root;
	unsigned long size;
	AVL_Comparator comparator;
	AVL_Destroyer kfree;
	AVL_Destroyer vfree;
} AVL_Tree;

// represents an inorder iterator for an AVL tree
typedef struct AVL_Iterator
{
	AVL_Tree* tree;
	AVL_Node* current;
} AVL_Iterator;

// initialize a tree
// takes a pointer to the memory to initialize, the functions used to destroy keys, destroy values, and compare keys
// kfree and vfree
// returns a pointer to the tree
AVL_Tree *AVL_Initialize(AVL_Tree *tree, AVL_Destroyer kfree, AVL_Destroyer vfree, AVL_Comparator comparator);

// remove all items from a tree and free associate memory
// takes a pointer to the tree
void AVL_Clear(AVL_Tree *tree);

// get the value associated with a key
// takes a pointer to the tree to search and the key
// returns the value or POLY_DEFAULT if not found
POLY_Polymorphic AVL_Get(AVL_Tree *tree, POLY_Polymorphic key);

// set the value associated with a key
// takes a pointer to the tree to search and the key and value
// value may be NULL
void AVL_Set(AVL_Tree *tree, POLY_Polymorphic key, POLY_Polymorphic value);

// insert a key into a tree with no associated value
// takes a pointer to the tree to search and the key
void AVL_Insert(AVL_Tree *tree, POLY_Polymorphic key);

// delete a key from a tree
// takes a pointer to the tree to search and the key
void AVL_Delete(AVL_Tree *tree, POLY_Polymorphic key);

// determines whether a tree contains a key
// takes a pointer to the tree to search and the key
// returns 1 if the tree contains the key, returns 0 otherwise
int AVL_Contains(AVL_Tree *tree, POLY_Polymorphic key);

// gets the size of a tree
// takes a pointer to the tree
// returns the number of items in the tree
unsigned long AVL_Size(AVL_Tree *tree);

// initializes an iterator for a tree
// takes a pointer to the memory to initialize and the tree to iterate over
// returns an iterator for that tree
AVL_Iterator* AVL_InitializeIterator(AVL_Tree *tree, AVL_Iterator *iterator);

// gets the next element from an iterator
// takes a pointer to the iterator
// returns 0 if the end has been reached, 1 otherwise
int AVL_Next(AVL_Iterator *iterator);

// gets the key of the current element of an iterator
// takes a pointer to the iterator
// returns the key
POLY_Polymorphic AVL_Key(AVL_Iterator *iterator);

// gets the value of the current element of an iterator
// takes a pointer to the iterator
// returns the value
POLY_Polymorphic AVL_Value(AVL_Iterator *iterator);

// resets an iterator to the beginning
// takes a pointer to the iterator
void AVL_Reset(AVL_Iterator *iterator);

// a function to do a deep comparison of two trees, note values are ignored, only keys are considered
// the key comparator for the first tree will be used to compare keys between the trees
// takes pointers to the two trees to compare
// returns the result of the comparison
int AVL_DeepComparator(POLY_Polymorphic key1, POLY_Polymorphic key2);

// a function to clear a tree and free the pointer to the tree
// takes a pointer to the tree to destroy
void AVL_Destroy(POLY_Polymorphic item);

#endif
