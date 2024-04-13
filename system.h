/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * system.h
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define ARRAY_SIZE(a) ( (sizeof (a)) / (sizeof (a[0])) )

#define UNUSED(s)				\
	do {					\
		(void)(s);			\
	} while (0)

#define TRACE(s)				\
	do {					\
		fprintf(stderr,			\
			"error: %s:%d: %s\n",	\
			__FILE__,		\
			__LINE__,		\
			safe_strlen(s) ?	\
			(s) : "^");		\
	} while (0)

#define EXIT(s)					\
	do {					\
		TRACE((s));			\
		assert( 0 );			\
		exit(-1);			\
	} while (0)

#define FREE(p)					\
	do {					\
		if ((p)) {			\
			free((void *)(p));	\
			(p) = NULL;		\
		}				\
	} while (0)

void us_sleep(uint64_t us);

void file_delete(const char *pathname);

void safe_sprintf(char *buf, size_t len, const char *format, ...);

size_t safe_strlen(const char *s);

size_t page_size(void);  /* size of kernel's internal page size*/

void *memory_align(void *p, size_t n);  /* give a number >= p and a multiple of n*/

#endif /* _SYSTEM_H_ */

/*each thread shud have a stack, & we use this to make sure the stack memory must be page aligned(i.e it should start and end at a page boundary)
why should stack memory be page aligned? - requirement by CPU, it expects stack of size N*logical_page_size which is page aligned
stack and heap grows in opposite direction to make sure it does not interfere and keeps a buffer between them
*/