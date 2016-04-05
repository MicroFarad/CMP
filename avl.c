/*
Source file for AVL tree set or tree map implementation

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

#include <stdlib.h>
#include "avl.h"

AVL_Tree *AVL_Initialize(AVL_Tree *tree, AVL_Destroyer kfree, AVL_Destroyer vfree, AVL_Comparator comparator)
{
	tree->root = NULL;
	tree->size = 0;
	tree->comparator = comparator;
	tree->kfree = kfree;
	tree->vfree = vfree;
	return tree;
}

// helper function frees memory of node, its key and value, and its children recursively
// takes a pointer to the tree and the node to destroy
void AVL_DestroyNode(AVL_Tree *tree, AVL_Node *node)
{
	if(!node) return;
	AVL_DestroyNode(tree, node->left);
	AVL_DestroyNode(tree, node->right);
	if(tree->kfree) tree->kfree(node->key);
	if(tree->vfree) tree->vfree(node->value);
	free(node);
}

void AVL_Clear(AVL_Tree *tree)
{
	AVL_DestroyNode(tree, tree->root);
	tree->size = 0;
}

// helper function gets the node associated with a key
// takes a pointer to the tree to search and a pointer to the key
// returns a pointer to the node or NULL if not found
AVL_Node *AVL_GetNode(AVL_Tree *tree, POLY_Polymorphic key)
{
	int comparison;
	AVL_Node *current = tree->root;
	while(current && (comparison = tree->comparator(key, current->key)))
	{
		if(comparison > 0) current = current->right;
		else current = current->left;
	}
	return current;
}

POLY_Polymorphic AVL_Get(AVL_Tree *tree, POLY_Polymorphic key)
{
	AVL_Node *node = AVL_GetNode(tree, key);
	if(node) return node->value;
	else return POLY_DEFAULT;
}

// helper function recalculates the height value for a node
// takes a pointer to the node to recalculate
// returns the height
int AVL_RecalcHeight(AVL_Node *node)
{
	if(!node) return 0;
	if(node->right)
	{
		if(node->left)
			if(node->right->height > node->left->height) node->height = node->right->height + 1;
			else node->height = node->left->height + 1;
		else
			node->height = node->right->height + 1;
	}
	else
	{
		if(node->left)
			node->height = node->left->height + 1;
		else
			node->height = 1;
	}
	return node->height;
}

// helper function moves a node up a level by tree rotation
// takes a pointer to the tree structure and a pointer to the node
void AVL_Ascend(AVL_Tree *tree, AVL_Node *node)
{
	AVL_Node *parent = node->parent;
	AVL_Node *grandparent = parent->parent;
	if(grandparent)
	{
		if(parent == grandparent->right) grandparent->right = node;
		else grandparent->left = node;
	}
	else
	{
		tree->root = node;
	}
	if(node == parent->right)
	{
		parent->right = node->left;
		if(node->left) node->left->parent = parent;
		node->left = parent;
	}
	else
	{
		parent->left = node->right;
		if(node->right) node->right->parent = parent;
		node->right = parent;
	}
	node->parent = grandparent;
	parent->parent = node;
	AVL_RecalcHeight(parent);
	AVL_RecalcHeight(node);
	AVL_RecalcHeight(grandparent);
}

// helper function calculates node's balance factor (left height - right height)
// takes a pointer to the node to perform the calculation on
// returns the balance factor
int AVL_Balance(AVL_Node *node)
{
	if(node->left)
		if(node->right)
			return node->left->height - node->right->height;
		else
			return node->left->height;
	else
		if(node->right)
			return -node->right->height;
		else
			return 0;
}

// helper function repairs an AVL tree after insertion or deletion
// takes a pointer to the tree and a pointer to the node above which errors may occur
void AVL_Repair(AVL_Tree *tree, AVL_Node *node)
{
	AVL_Node *parent = node->parent;
	while(parent)
	{
		AVL_RecalcHeight(parent);
		if(AVL_Balance(parent) == 2)
		{
			if(AVL_Balance(node) == -1) AVL_Ascend(tree, node->right);
			AVL_Ascend(tree, parent->left);
			parent = parent->parent;
		}
		else if(AVL_Balance(parent) == -2)
		{
			if(AVL_Balance(node) == 1) AVL_Ascend(tree, node->left);
			AVL_Ascend(tree, parent->right);
			parent = parent->parent;
		}
		node = parent;
		parent = node->parent;
	}
}

void AVL_Set(AVL_Tree *tree, POLY_Polymorphic key, POLY_Polymorphic value)
{
	int comparison;
	AVL_Node *parent = tree->root;
	AVL_Node *next;
	AVL_Node *node;
	if(parent)
	{
		while(parent)
		{
			if(comparison = tree->comparator(key, parent->key))
			{
				if(comparison > 0) next = parent->right;
				else next = parent->left;
				if(next) parent = next;
				else break;
			}
			else
			{
				if(tree->vfree) tree->vfree(parent->value);
				parent->value = value;
				return;
			}
		}
	}
	tree->size++;
	node = malloc(sizeof(AVL_Node));
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;
	node->height = 1;
	node->key = key;
	node->value = value;
	if(parent)
	{
		if(comparison > 0)
		{
			parent->right = node;
		}
		else
		{
			parent->left = node;
		}
		node->parent = parent;
		AVL_Repair(tree, node);
	}
	else
	{
		tree->root = node;
	}
}

void AVL_Insert(AVL_Tree *tree, POLY_Polymorphic key)
{
	AVL_Set(tree, key, POLY_DEFAULT);
}

void AVL_Delete(AVL_Tree *tree, POLY_Polymorphic key)
{
	AVL_Node *container = AVL_GetNode(tree, key);
	AVL_Node *delete;
	AVL_Node *parent;
	if(!container) return;
	tree->size--;
	if(container->right)
	{
		if(container->left)
		{
			delete = container->left;
			while(delete->right) delete = delete->right;
			parent = delete->parent;
			if(delete == parent->right)
			{
				parent->right = delete->left;
				if(delete->left) delete->left->parent = parent;
			}
			else
			{
				parent->left = delete->left;
				if(delete->left) delete->left->parent = parent;
			}
			if(tree->kfree) tree->kfree(container->key);
			if(tree->vfree) tree->vfree(container->value);
			POLY_Polymorphic tmp;
			tmp = delete->key;
			delete->key = container->key;
			container->key = tmp;
			tmp = delete->value;
			delete->value = container->value;
			container->value = tmp;
		}
		else
		{
			delete = container;
			parent = delete->parent;
			if(parent)
				if(delete == parent->right) parent->right = delete->right;
				else parent->left = delete->right;
			else
				tree->root = delete->right;
			delete->right->parent = parent;
		}
	}
	else
	{
		if(container->left)
		{
			delete = container;
			parent = delete->parent;
			if(parent)
				if(delete == parent->right) parent->right = delete->left;
				else parent->left = delete->left;
			else
				tree->root = delete->left;
			delete->left->parent = parent;
		}
		else
		{
			delete = container;
			parent = delete->parent;
			if(parent)
				if(delete == parent->right) parent->right = NULL;
				else parent->left = NULL;
			else
				tree->root = NULL;
		}
	}
	delete->right = NULL;
	delete->left = NULL;
	AVL_DestroyNode(tree, delete);
	if(parent)
	{
		AVL_RecalcHeight(parent);
		if(parent->right) AVL_Repair(tree, parent->right);
		else if(parent->left) AVL_Repair(tree, parent->left);
		else AVL_Repair(tree, parent);
	}
}

int AVL_Contains(AVL_Tree *tree, POLY_Polymorphic key)
{
	return AVL_GetNode(tree, key) ? 1 : 0;
}

// gets the size of a tree
// takes a pointer to the tree
// returns the number of items in the tree
unsigned long AVL_Size(AVL_Tree *tree)
{
	return tree->size;
}

AVL_Iterator *AVL_InitializeIterator(AVL_Tree *tree, AVL_Iterator *iterator)
{
	iterator->tree = tree;
	iterator->current = NULL;
	return iterator;
}

int AVL_Next(AVL_Iterator *iterator)
{
	AVL_Node *node = iterator->current;
	if(node)
	{
		if(node->right)
		{
			node = node->right;
			while(node->left) node = node->left;
		}
		else if(node->parent)
		{
			if(node == node->parent->right)
			{
				while(node->parent && node == node->parent->right) node = node->parent;
				if(node->parent) node = node->parent;
				else node = NULL;
			}
			else
			{
				node = node->parent;
			}
		}
		else
		{
			node = NULL;
		}
	}
	else
	{
		node = iterator->tree->root;
		if(node)
		{
			while(node->left)
			{
				node = node->left;
			}
		}
	}
	if(iterator->current = node) return 1;
	else return 0;
}

POLY_Polymorphic AVL_Key(AVL_Iterator *iterator)
{
	if(iterator->current) return iterator->current->key;
	else return POLY_DEFAULT;
}

POLY_Polymorphic AVL_Value(AVL_Iterator *iterator)
{
	if(iterator->current) return iterator->current->value;
	else return POLY_DEFAULT;
}

void AVL_Reset(AVL_Iterator *iterator)
{
	iterator->current = NULL;
}

int AVL_DeepComparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	if (AVL_POLYTREE(key1)->size < AVL_POLYTREE(key2)->size) return -1;
	else if (AVL_POLYTREE(key1)->size > AVL_POLYTREE(key2)->size) return 1;
	AVL_Iterator iter1;
	AVL_InitializeIterator(AVL_POLYTREE(key1), &iter1);
	AVL_Iterator iter2;
	AVL_InitializeIterator(AVL_POLYTREE(key2), &iter2);
	int cmp = 0;
	while(AVL_Next(&iter1) && AVL_Next(&iter2))
		if((cmp = AVL_POLYTREE(key1)->comparator(AVL_Key(&iter1), AVL_Key(&iter2))) != 0) break;
	return cmp;
}

void AVL_Destroy(POLY_Polymorphic item)
{
	AVL_Clear(AVL_POLYTREE(item));
	free(AVL_POLYTREE(item));
}
