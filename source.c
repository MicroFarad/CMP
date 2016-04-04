#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regex.h"

int main(int argc, char **argv)
{
	FILE *fp = fopen("test_regex.txt", "r");
	REGEX_Expressions expr;
	expr.expressions_count = 1;
	expr.expressions = malloc(sizeof(REGEX_Expression)*expr.expressions_count);
	char buf[512];
	for(unsigned long n = 0; n < expr.expressions_count; n++)
	{
		fscanf(fp, "%s\n", buf);
		expr.expressions[n].accepts = n+1;
		expr.expressions[n].expression = malloc(sizeof(UNICODE_Char)*(strlen(buf)+1));
		expr.expressions[n].expression[strlen(buf)] = 0;
		for(int i = 0; buf[i]; i++)
		{
			expr.expressions[n].expression[i] = buf[i];
		}
	}
	REGEX_Machine *machine = REGEX_CreateMachine(&expr);
	printf("Print out state machine...\n");
	for(unsigned long n = 0; n < machine->states_count; n++)
	{
		printf("%d %d   ", n, machine->states[n].accepts);
		for(unsigned short i = 0; i < machine->states[n].transitions_count; i++)
		{
			printf("%c %d   ", machine->states[n].transitions[i].on, machine->states[n].transitions[i].to);
		}
		printf("\n");
	}
}
