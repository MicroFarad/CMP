/*
Source file for Unicode text processing

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

#include "unicode.h"

int UNICODE_CharComparator(POLY_Polymorphic key1, POLY_Polymorphic key2)
{
	if(UNICODE_POLYCHAR(key1) <  UNICODE_POLYCHAR(key1)) return -1;
	else if(UNICODE_POLYCHAR(key1) >  UNICODE_POLYCHAR(key1)) return 1;
	else return 0;
}
