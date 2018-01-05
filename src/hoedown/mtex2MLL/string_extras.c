#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_extras.h"
#include "string_dup.h"

void mtex2MML_remove_last_char(char* str)
{
  size_t len = strlen(str);
  str[len - 1] = '\0';
}

void mtex2MML_remove_first_char(char* str)
{
  size_t len = strlen(str);
  memmove(str, str + 1, len);
}

char * mtex2MML_str_replace (char *string, const char *substr, const char *replacement)
{
  char *tok = NULL;
  char *newstr = NULL;
  char *oldstr = NULL;

  newstr = string_dup(string);

  while ( ( tok = strstr( newstr, substr ) ) ) {

    oldstr = newstr;
    newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );

    /* If failed to alloc mem, free old string and return NULL */
    if ( newstr == NULL ) {
      free (oldstr);
      return NULL;
    }

    memcpy ( newstr, oldstr, tok - oldstr );
    memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
    memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
    memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );

    free (oldstr);
  }

  return newstr;
}
