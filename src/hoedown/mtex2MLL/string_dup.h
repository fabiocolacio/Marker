/*
// string_dup.h
//
// Copyright (c) 2014 Stephen Mathieson
// MIT licensed
*/

#ifndef HAVE_STRINGDUP
#define HAVE_STRINGDUP

/**
 * Drop-in replacement for string_dup(3) from libc.
 *
 * Creates a copy of `str`.  Free when done.
 *
 * Returns a pointer to the newly allocated
 * copy of `str`, or `NULL` on failure.
 */

char *
string_dup(const char *str);

#endif /* HAVE_STRINGDUP */
