/*
Header file for Unicode text processing

Copyright (C) 2015-2016 Kyle Gagner
All Rights Reserved
*/

// for polymorphism
#include "poly.h"

// include guard
#ifndef UNICODE_H
#define UNICODE_H

#define UNICODE_CHARPOLY(value) ((POLY_Polymorphic){.uint16=value})
#define UNICODE_POLYCHAR(value) ((UNICODE_Char)value.uint16)

// represents a 16 bit Unicode character (UTF-16)
// note that this is neither a code point nor a grapheme
typedef unsigned short UNICODE_Char;

// function used to compare 16 bit characters for use with AVL tree implementation
// takes two polymorphic keys as uint16's to compare, data field is unused
// returns result of comparison
int UNICODE_CharComparator(POLY_Polymorphic key1, POLY_Polymorphic key2);

#endif
