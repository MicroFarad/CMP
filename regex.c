/*
Source file for regular expression compiler

Copyright (C) 2016 Kyle Gagner
All Rights Reserved
*/

#include <stdlib.h>
#include <stdio.h>
#include "regex.h"
#include "avl.h"
#include "list.h"

// INTERNAL MACROS

#define POLYNFA(value)   ((NFA_Node*)value.ref)
#define POLYDFA(value)   ((DFA_Node*)value.ref)
#define POLYTOKEN(value) ((Token)value.integer)
#define POLYFRAG(value)  ((NFA_Fragment*)value.ref)

// INTERNAL TYPES

// represents a single node in a nondeterministic finite state automaton
typedef struct NFA_Node
{
	AVL_Tree transitions;
	AVL_Tree epsilons;
	unsigned long accepts;
	unsigned long identifier;
	struct NFA_Node *next;
} NFA_Node;

// represents a single node in a deterministic finite state automaton
typedef struct DFA_Node
{
	AVL_Tree transitions;
	unsigned long accepts;
	unsigned long identifier;
	struct DFA_Node *next;
	AVL_Tree *children;
	struct DFA_Node *parent;
	struct DFA_Node *surrogate;
} DFA_Node;

// represents a regex token
typedef enum
{
	LPAREN,
	CONCATENATION,
	ALTERNATION,
	KLEENE_STAR,
	OPTION,
	REPETITION
} Token;

// represents an nfa fragment with defined start and end
typedef struct
{
	NFA_Node *start;
	NFA_Node *end;
} NFA_Fragment;

// INTERNAL ROUTINES

// comparator for NFA
int NFA_Comparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	unsigned long i1 = POLYNFA(key1)->identifier;
	unsigned long i2 = POLYNFA(key2)->identifier;
	if(i1 < i2) return -1;
	if(i1 > i2) return 1;
	return 0;
}

// creates a uniquely numbered NFA node
NFA_Node *NFA_CreateState(unsigned long *unique, NFA_Node **last)
{
	NFA_Node *node = malloc(sizeof(NFA_Node));
	AVL_Initialize(&node->transitions, NULL, AVL_Destroy, UNICODE_CharComparator);
	AVL_Initialize(&node->epsilons, NULL, NULL, NFA_Comparator);
	node->accepts = 0;
	node->identifier = (*unique)++;
	node->next = NULL;
	if(*last) (*last)->next = node;
	*last = node;
	return node;
}

// comparator for DFA
int DFA_Comparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	unsigned long i1 = POLYDFA(key1)->identifier;
	unsigned long i2 = POLYDFA(key2)->identifier;
	if(i1 < i2) return -1;
	if(i1 > i2) return 1;
	return 0;
}

// creates a uniquely numbered DFA node
DFA_Node *DFA_CreateState(unsigned long *unique, DFA_Node **last)
{
	DFA_Node *node = malloc(sizeof(DFA_Node));
	AVL_Initialize(&node->transitions, NULL, NULL, UNICODE_CharComparator); // DESTROYER?
	node->parent = NULL;
	node->identifier = (*unique)++;
	node->next = NULL;
	if(*last) (*last)->next = node;
	*last = node;
	node->children = NULL;
	return node;
}

// comparator for DFA nodes used for binning nodes by their connection topology in state simplification
int DFA_BinComparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	unsigned long a1 = POLYDFA(key1)->accepts;
	unsigned long a2 = POLYDFA(key2)->accepts;
	if(a1 < a2) return -1;
	if(a1 > a2) return 1;
	AVL_Tree *t1 = &POLYDFA(key1)->transitions;
	AVL_Tree *t2 = &POLYDFA(key2)->transitions;
	unsigned long s1 = AVL_Size(t1);
	unsigned long s2 = AVL_Size(t2);
	if(s1 < s2) return -1;
	if(s1 > s2) return 1;
	AVL_Iterator iter1, iter2;
	AVL_InitializeIterator(t1, &iter1);
	AVL_InitializeIterator(t2, &iter2);
	while(AVL_Next(&iter1) && AVL_Next(&iter2))
	{
		UNICODE_Char k1 = UNICODE_POLYCHAR(AVL_Key(&iter1));
		UNICODE_Char k2 = UNICODE_POLYCHAR(AVL_Key(&iter2));
		if(k1 < k2) return -1;
		if(k1 > k2) return 1;
		if(POLYDFA(AVL_Value(&iter1))->parent && POLYDFA(AVL_Value(&iter2))->parent) //???
		{
			int v1 = POLYDFA(AVL_Value(&iter1))->parent->identifier;
			int v2 = POLYDFA(AVL_Value(&iter2))->parent->identifier;
			if(v1 < v2) return -1;
			if(v1 > v2) return 1;
		}
	}
	return 0;
}

// find the largest accepts value of a set of NFA states
unsigned long GetAccepts(AVL_Tree *states)
{
	AVL_Iterator iter;
	AVL_InitializeIterator(states, &iter);
	unsigned long accepts = 0;
	while(AVL_Next(&iter))
	{
		unsigned long t = POLYNFA(AVL_Key(&iter))->accepts;
		if(t>accepts) accepts = t;
	}
	return accepts;
}

// finds the mapping from a set of NFA states to a DFA state, or creates the mapping if it doesn't exist and queues unexplored states
DFA_Node *MapStates(AVL_Tree *map, AVL_Tree *states, unsigned long *unique, DFA_Node **last, LIST_List *unexplored)
{
	if(AVL_Contains(map, POLY_REF(states)))
		return POLYDFA(AVL_Get(map, POLY_REF(states)));
	else
	{
		DFA_Node *node;
		AVL_Set(map, POLY_REF(states), POLY_REF(node = DFA_CreateState(unique, last)));
		LIST_InsertHead(unexplored, POLY_REF(states));
		node->accepts = GetAccepts(states);
		return node;
	}
}

// finds the epsilon closure of a set of states
AVL_Tree *EpsilonClosure(AVL_Tree *states)
{
	AVL_Tree *result = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, NFA_Comparator);
	AVL_Iterator iter;
	AVL_InitializeIterator(states, &iter);
	while(AVL_Next(&iter)) AVL_Insert(result, AVL_Key(&iter));
	AVL_Tree *frontier = states;
	int intermediate = 0;
	while(AVL_Size(frontier))
	{
		AVL_Tree *next = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, NFA_Comparator);
		AVL_Iterator outer;
		AVL_InitializeIterator(frontier, &outer);
		while(AVL_Next(&outer))
		{
			AVL_Iterator inner;
			AVL_InitializeIterator(&POLYNFA(AVL_Key(&outer))->epsilons, &inner);
			while(AVL_Next(&inner))
			{
				if(!AVL_Contains(result, AVL_Key(&inner)))
				{
					AVL_Insert(result, AVL_Key(&inner));
					AVL_Insert(next, AVL_Key(&inner));
				}
			}
		}
		if(intermediate)
		{
			AVL_Clear(frontier);
			free(frontier);
		}
		intermediate = 1;
		frontier = next;
	}
	return result;
}

// using the epsilon closure, find all mappings from transitions to the set of states they may reach
AVL_Tree *TransitionSets(AVL_Tree *states)
{
	AVL_Tree intermediate;
	AVL_Initialize(&intermediate, NULL, NULL, UNICODE_CharComparator);
	AVL_Iterator outer;
	// WORK IN PROGRESS WITH EPSILON CLOSURE HERE
	AVL_InitializeIterator(EpsilonClosure(states), &outer);
	while(AVL_Next(&outer))
	{
		AVL_Iterator inner;
		AVL_InitializeIterator(&POLYNFA(AVL_Key(&outer))->transitions, &inner);
		while(AVL_Next(&inner))
		{
			AVL_Tree *set;
			if(AVL_Contains(&intermediate, AVL_Key(&inner)))
				set = AVL_POLYTREE(AVL_Get(&intermediate, AVL_Key(&inner)));
			else
				AVL_Set(&intermediate, AVL_Key(&inner), POLY_REF(set = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, NFA_Comparator)));
			AVL_Iterator sub;
			AVL_InitializeIterator(AVL_POLYTREE(AVL_Value(&inner)), &sub);
			while(AVL_Next(&sub)) AVL_Insert(set, AVL_Key(&sub));
		}
	}
	AVL_Tree *result = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, UNICODE_CharComparator);
	AVL_InitializeIterator(&intermediate, &outer);
	while(AVL_Next(&outer)) AVL_Set(result, AVL_Key(&outer), POLY_REF(EpsilonClosure(AVL_POLYTREE(AVL_Value(&outer)))));
	// YOU NEED TO FREE SETS IN INTERMEDIATE?
	AVL_Clear(&intermediate);
	return result;
}

void NFA_Debug(NFA_Node *start, char *name)
{
	FILE *fp = fopen(name, "w");
	fprintf(fp, "digraph\n{\n{\nnode [shape = circle]\n");
	NFA_Node *current = start;
	while(current)
	{
		fprintf(fp, "%d\n", current->identifier);
		current = current->next;
	}
	fprintf(fp, "}\n");
	current = start;
	while(current)
	{
		AVL_Iterator outer;
		AVL_InitializeIterator(&current->epsilons, &outer);
		while(AVL_Next(&outer))
			fprintf(fp, "%d -> %d [label = \"_\"]\n", current->identifier, POLYNFA(AVL_Key(&outer))->identifier);
		AVL_InitializeIterator(&current->transitions, &outer);
		while(AVL_Next(&outer))
		{
			AVL_Iterator inner;
			AVL_InitializeIterator(AVL_POLYTREE(AVL_Value(&outer)), &inner);
			while(AVL_Next(&inner))
				fprintf(fp, "%d -> %d [label = \"%c\"]\n", current->identifier, POLYNFA(AVL_Key(&inner))->identifier, UNICODE_POLYCHAR(AVL_Key(&outer)));
		}
		current = current->next;
	}
	fprintf(fp, "}");
	fclose(fp);
}

void DFA_Debug(DFA_Node *start, char *name)
{
	FILE *fp = fopen(name, "w");
	fprintf(fp, "digraph\n{\n{\nnode [shape = circle]\n");
	DFA_Node *current = start;
	while(current)
	{
		fprintf(fp, "%d\n", current->identifier);
		current = current->next;
	}
	fprintf(fp, "}\n");
	current = start;
	while(current)
	{
		AVL_Iterator iter;
		AVL_InitializeIterator(&current->transitions, &iter);
		while(AVL_Next(&iter))
			fprintf(fp, "%d -> %d [label = \"%c\"]\n", current->identifier, POLYDFA(AVL_Value(&iter))->identifier, UNICODE_POLYCHAR(AVL_Key(&iter)));
		current = current->next;
		printf("Debug 2.3\n");
	}
	fprintf(fp, "}");
	fclose(fp);
}

// converts an NFA to a DFA
DFA_Node* Convert(NFA_Node *start, unsigned long *unique, DFA_Node **last)
{
	AVL_Tree *initial = malloc(sizeof(AVL_Tree));
	AVL_Initialize(initial, NULL, NULL, NFA_Comparator);
	AVL_Insert(initial, POLY_REF(start));
	AVL_Tree *closure = EpsilonClosure(initial); // EPSILON CLOSURE BEHAVIOR CHANGED TO NOT FREE, ACTION NEEDED?
	//AVL_Clear(initial); LOOK AT THIS LINE
	LIST_List unexplored;
	LIST_Initialize(&unexplored);
	AVL_Tree map;
	AVL_Initialize(&map, AVL_Destroy, NULL, AVL_DeepComparator);
	DFA_Node *first = MapStates(&map, closure, unique, last, &unexplored);
	while(LIST_Size(&unexplored))
	{
		AVL_Tree *states = AVL_POLYTREE(LIST_TakeTail(&unexplored));
		DFA_Node *node = MapStates(&map, states, unique, last, &unexplored);
		AVL_Tree *transitions = TransitionSets(states);
		AVL_Iterator iter;
		AVL_InitializeIterator(transitions, &iter);
		while(AVL_Next(&iter))
			AVL_Set(&node->transitions, AVL_Key(&iter), POLY_REF(MapStates(&map, AVL_POLYTREE(AVL_Value(&iter)), unique, last, &unexplored)));
		AVL_Clear(transitions);
		free(transitions);
	}
	return first;
}

// pushes nfa fragment to stack representing transition
void ConstructTransition(unsigned long *unique, NFA_Node **last, UNICODE_Char c, LIST_List *stack)
{
	NFA_Fragment *fragment = malloc(sizeof(NFA_Fragment));
	fragment->start = NFA_CreateState(unique, last);
	fragment->end = NFA_CreateState(unique, last);
	// CHECK FOR MEMORY LEAKS LATER:
	AVL_Tree *set = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, NFA_Comparator);
	AVL_Insert(set, POLY_REF(fragment->end));
	AVL_Set(&fragment->start->transitions, UNICODE_CHARPOLY(c), POLY_REF(set));	
	LIST_InsertHead(stack, POLY_REF(fragment));
}

// pops nfa fragments from stack and pushes result of combining on an operator
void ConstructOperator(Token t, unsigned long *unique, NFA_Node **last, LIST_List *stack)
{
	switch(t)
	{
		case CONCATENATION:
		{
			NFA_Fragment *right = POLYFRAG(LIST_TakeHead(stack));
			NFA_Fragment *left = POLYFRAG(LIST_PeekHead(stack));
			AVL_Size(&left->end->epsilons);
			AVL_Insert(&left->end->epsilons, POLY_REF(right->start));
			AVL_Size(&left->end->epsilons);
			left->end = right->end;
			free(right);
			break;
		}
		case ALTERNATION:
		{
			NFA_Fragment *right = POLYFRAG(LIST_TakeHead(stack));
			NFA_Fragment *left = POLYFRAG(LIST_PeekHead(stack));
			NFA_Node *start = NFA_CreateState(unique, last);
			NFA_Node *end = NFA_CreateState(unique, last);
			AVL_Insert(&start->epsilons, POLY_REF(right->start));
			AVL_Insert(&start->epsilons, POLY_REF(left->start));
			AVL_Size(&start->epsilons);
			AVL_Insert(&right->end->epsilons, POLY_REF(end)); 
			AVL_Insert(&left->end->epsilons, POLY_REF(end));
			left->start = start;
			left->end = end;
			break;
		}
		case KLEENE_STAR:
		{
			NFA_Fragment *left = POLYFRAG(LIST_PeekHead(stack));
			NFA_Node *node = NFA_CreateState(unique, last);
			AVL_Insert(&node->epsilons, POLY_REF(left->start));
			AVL_Insert(&left->end->epsilons, POLY_REF(node));
			AVL_Size(&node->epsilons);
			AVL_Size(&left->end->epsilons);
			left->start = node;
			left->end = node;
			break;
		}
		case OPTION:
		{
			NFA_Fragment *left = POLYFRAG(LIST_PeekHead(stack));
			AVL_Insert(&left->start->epsilons, POLY_REF(left->end));
			break;
		}
		case REPETITION:
		{
			NFA_Fragment *left = POLYFRAG(LIST_PeekHead(stack));
			NFA_Node *start = NFA_CreateState(unique, last);
			NFA_Node *end = NFA_CreateState(unique, last);
			AVL_Insert(&start->epsilons, POLY_REF(left->start));
			AVL_Insert(&start->epsilons, POLY_REF(end));
			AVL_Insert(&left->end->epsilons, POLY_REF(end)); 
			left->start = start;
			left->end = end;
			break;
		}
		default:
			break;
	}
}

// defines operator precedence for tokens
int OperatorPrecedence(Token t)
{
	switch(t)
	{
		case ALTERNATION:
			return 0;
		case CONCATENATION:
			return 1;
		case KLEENE_STAR:
		case OPTION:
		case REPETITION:
			return 2;
		default:
			return -1;
	}
}

// pops operators of equal or lesser precedence than a given token, then pushes that token (constructs nfa fragments)
void PopThenPush(Token t, unsigned long *unique, NFA_Node **last, LIST_List *nfastack, LIST_List *tokenstack)
{
	int precedence = OperatorPrecedence(t);
	while(LIST_Size(tokenstack)&&OperatorPrecedence(POLYTOKEN(LIST_PeekHead(tokenstack))) >= precedence)
		ConstructOperator(POLYTOKEN(LIST_TakeHead(tokenstack)), unique, last, nfastack);
	LIST_InsertHead(tokenstack, POLY_INTEGER(t));
}

// uses the shunting yard algorithm to create an NFA from a regular expression
NFA_Node *ConstructNFA(unsigned long *unique, NFA_Node **last, unsigned long accepts, UNICODE_Char *expression)
{
	LIST_List tokenstack;
	LIST_Initialize(&tokenstack);
	LIST_List nfastack;
	LIST_Initialize(&nfastack);
	UNICODE_Char c;
	int cat = 0;
	while(c = *expression++)
	{
		int ncat = 0;
		switch(c)
		{
			case '(':
				if(cat) PopThenPush(CONCATENATION, unique, last, &nfastack, &tokenstack);
				LIST_InsertHead(&tokenstack, POLY_INTEGER(LPAREN));
				break;
			case ')':
				while(LIST_Size(&tokenstack) && POLYTOKEN(LIST_PeekHead(&tokenstack)) != LPAREN)
					ConstructOperator(POLYTOKEN(LIST_TakeHead(&tokenstack)), unique, last, &nfastack);
				LIST_TakeHead(&tokenstack);
				ncat = 1;
				break;
			case '.':
				PopThenPush(CONCATENATION, unique, last, &nfastack, &tokenstack);
				break;
			case '|':
				PopThenPush(ALTERNATION, unique, last, &nfastack, &tokenstack);
				break;
			case '*':
				PopThenPush(KLEENE_STAR, unique, last, &nfastack, &tokenstack);
				ncat = 1;
				break;
			case '?':
				PopThenPush(OPTION, unique, last, &nfastack, &tokenstack);
				ncat = 1;
				break;
			case '+':
				PopThenPush(REPETITION, unique, last, &nfastack, &tokenstack);
				ncat = 1;
				break;
			case '\\':
				if(cat) PopThenPush(CONCATENATION, unique, last, &nfastack, &tokenstack);
				ConstructTransition(unique, last, *expression++, &nfastack);
				ncat = 1;
				break;
			default:
				if(cat) PopThenPush(CONCATENATION, unique, last, &nfastack, &tokenstack);
				ConstructTransition(unique, last, c, &nfastack);
				ncat = 1;
				break;
		}
		cat = ncat;
	}
	while(LIST_Size(&tokenstack))
		ConstructOperator(POLYTOKEN(LIST_TakeHead(&tokenstack)), unique, last, &nfastack);
	NFA_Fragment *final = POLYFRAG(LIST_TakeHead(&nfastack));
	final->end->accepts = accepts;
	NFA_Node *result = final->start;
	free(final);
	return result;
}

// an AVL comparator for uint32s
int UnsignedLongComparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	if(key1.uint32 < key2.uint32) return -1;
	if(key1.uint32 > key2.uint32) return 1;
	return 0;
}

// simplifies a DFA in place
unsigned long SimplifyStates(DFA_Node *start)
{
	AVL_Tree bins;
	AVL_Initialize(&bins, NULL, NULL, DFA_BinComparator);
	DFA_Node *current = start;
	while(current)
	{
		if(AVL_Contains(&bins, POLY_REF(current)))
		{
			DFA_Node *parent = POLYDFA(AVL_Get(&bins, POLY_REF(current)));
			if(!parent->children) parent->children = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, DFA_Comparator);
			AVL_Insert(parent->children, POLY_REF(current));
			current->parent = parent;
		}
		else
		{
			AVL_Set(&bins, POLY_REF(current), POLY_REF(current));
			AVL_Initialize(current->children = malloc(sizeof(AVL_Tree)), NULL, NULL, DFA_Comparator);
			current->parent = current;
		}
		current = current->next;
	}
	int rebinned = 1;
	AVL_Iterator outer;
	AVL_InitializeIterator(&bins, &outer);
	while(rebinned)
	{
		rebinned = 0;
		AVL_Reset(&outer);
		while(AVL_Next(&outer))
		{
			DFA_Node *parent = POLYDFA(AVL_Key(&outer));
			AVL_Tree *rebins = parent->children;
			parent->children = AVL_Initialize(malloc(sizeof(AVL_Tree)), NULL, NULL, DFA_Comparator);
			unsigned long osize = AVL_Size(rebins);
			AVL_Iterator inner;
			AVL_InitializeIterator(rebins, &inner);
			while(AVL_Next(&inner))
			{
				DFA_Node *child = POLYDFA(AVL_Key(&inner));
				if(AVL_Contains(&bins, POLY_REF(child)))
				{
					DFA_Node *surrogate = POLYDFA(AVL_Get(&bins, POLY_REF(child)));
					AVL_Insert(surrogate->children, POLY_REF(child));
					child->surrogate = surrogate;
				}
				else
				{
					AVL_Set(&bins, POLY_REF(child), POLY_REF(child));
					AVL_Initialize(child->children = malloc(sizeof(AVL_Tree)), NULL, NULL, DFA_Comparator);
					child->surrogate = child;
				}
			}
			AVL_Reset(&inner);
			while(AVL_Next(&inner))
			{
				DFA_Node *child = POLYDFA(AVL_Key(&inner));
				child->parent = child->surrogate;
			}
			if(AVL_Size(parent->children) != osize) rebinned = 1;
			AVL_Clear(rebins);
			free(rebins);
			if(rebinned) break;
		}
	}
	AVL_Insert(&bins, POLY_REF(start));
	start->children = start->parent->children;
	start->parent->parent = start;
	start->parent->children = NULL;
	start->parent = start;
	current = start;
	DFA_Node **fix = NULL;
	unsigned long unique = 0;
	while(current)
	{
		if(current->parent == current)
		{
			if(fix) *fix = current;
			fix = &current->next;
			current->identifier = unique;
			AVL_Iterator inner;
			AVL_InitializeIterator(&current->transitions, &inner);
			while(AVL_Next(&inner))
				AVL_Set(&current->transitions, AVL_Key(&inner), POLY_REF(POLYDFA(AVL_Value(&inner))->parent));
			if(current->children) AVL_Clear(current->children);
			if(current->children) free(current->children);
			unique++;
		}
		current = current->next;
	}
	*fix = NULL;
	return unique;
}

// EXTERNAL ROUTINES

REGEX_Machine *REGEX_CreateMachine(REGEX_Expressions *expressions)
{
	unsigned long uniquenfa = 0;
	NFA_Node *lastnfa = NULL;
	NFA_Node *start = NFA_CreateState(&uniquenfa, &lastnfa);
	for(unsigned long n = 0; n < expressions->expressions_count; n++)
	{
		NFA_Node *nfa = ConstructNFA(&uniquenfa, &lastnfa, expressions->expressions[n].accepts, expressions->expressions[n].expression);
		AVL_Insert(&start->epsilons, POLY_REF(nfa));
	}
	unsigned long uniquedfa = 0;
	DFA_Node *lastdfa = NULL;
	DFA_Node *dfa = Convert(start, &uniquedfa, &lastdfa);
	REGEX_Machine *result = malloc(sizeof(REGEX_Machine));
	result->states = malloc(sizeof(REGEX_State)*(result->states_count = SimplifyStates(dfa)));
	DFA_Node *current = dfa;
	for(unsigned long n = 0; n < result->states_count; n++)
	{
		REGEX_State *state = &result->states[n];
		state->transitions_count = AVL_Size(&current->transitions);
		REGEX_Transition *transitions = state->transitions = malloc(sizeof(REGEX_Transition)*state->transitions_count);
		AVL_Iterator iter;
		AVL_InitializeIterator(&current->transitions, &iter);
		for(unsigned short i = 0; i < state->transitions_count; i++)
		{
			AVL_Next(&iter);
			transitions[i].on = UNICODE_POLYCHAR(AVL_Key(&iter));
			transitions[i].to = POLYDFA(AVL_Value(&iter))->identifier;
		}
		state->accepts = current->accepts;
		current = current->next;
	}
	return result;
}

void REGEX_DestroyMachine(REGEX_Machine *machine)
{
	for(unsigned long n = 0; n < machine->states_count; n++)
	{
		REGEX_State *state = &machine->states[n];
		free(state->transitions);
	}
	free(machine->states);
	free(machine);
}
