/*
// string_dup.c
//
// Copyright (c) 2014 Stephen Mathieson
// MIT licensed
*/

#ifndef HAVE_STRINGDUP

#include <stdlib.h>
#include <string.h>
#include "string_dup.h"

char *
string_dup(const char *str) {
  if (!str) return NULL;
  int len = strlen(str) + 1;
  char *buf = malloc(len);
  if (buf) memcpy(buf, str, len);
  return buf;
}

#endif /* HAVE_STRINGDUP */
