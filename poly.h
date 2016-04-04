/*
Header file for polymorphism

Copyright (C) 2015 Kyle Gagner
All Rights Reserved
*/

#ifndef POLY_H
#define POLY_H

#define POLY_DEFAULT        ((POLY_Polymorphic){.ref=((void*)0)})
#define POLY_UINT8(value)   ((POLY_Polymorphic){.uint8=value})
#define POLY_SINT8(value)   ((POLY_Polymorphic){.sint8=value})
#define POLY_UINT16(value)  ((POLY_Polymorphic){.uint16=value})
#define POLY_SINT16(value)  ((POLY_Polymorphic){.sint16=value})
#define POLY_UINT32(value)  ((POLY_Polymorphic){.uint32=value})
#define POLY_SINT32(value)  ((POLY_Polymorphic){.sint32=value})
#define POLY_UINT64(value)  ((POLY_Polymorphic){.uint64=value})
#define POLY_SINT64(value)  ((POLY_Polymorphic){.sint64=value})
#define POLY_FLOAT32(value) ((POLY_Polymorphic){.float32=value})
#define POLY_FLOAT64(value) ((POLY_Polymorphic){.float64=value})
#define POLY_INTEGER(value) ((POLY_Polymorphic){.integer=value})
#define POLY_REF(value)     ((POLY_Polymorphic){.ref=value})

typedef union POLY_Polymorphic
{
	unsigned char      uint8;
	signed char        sint8;
	unsigned short     uint16;
	signed short       sint16;
	unsigned long      uint32;
	signed long        sint32;
	unsigned long long uint64;
	signed long long   sint64;
	float              float32;
	double             float64;
	int                integer;
	void*              ref;
} POLY_Polymorphic;

#endif
