/*
Header file for regular expression compiler

Copyright (C) 2016 Kyle Gagner
All Rights Reserved
*/

#include "unicode.h"

#ifndef REGEX_H
#define REGEX_H

typedef struct
{
	UNICODE_Char *expression;
	unsigned long accepts;
} REGEX_Expression;

typedef struct
{
	REGEX_Expression *expressions;
	unsigned long expressions_count;
} REGEX_Expressions;

typedef struct
{
	UNICODE_Char on;
	unsigned long to;
} REGEX_Transition;

typedef struct
{
	REGEX_Transition *transitions;
	unsigned short transitions_count;
	unsigned long accepts;
} REGEX_State;

typedef struct
{
	REGEX_State *states;
	unsigned long states_count;
} REGEX_Machine;

REGEX_Machine *REGEX_CreateMachine(REGEX_Expressions *expressions);

void REGEX_DestroyMachine(REGEX_Machine *machine);

#endif
