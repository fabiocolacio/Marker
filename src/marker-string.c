#include <string.h>
#include <stdlib.h>

#include "marker-string.h"

int
marker_string_ends_with(const char* str,
                        const char* sub_str)
{
  size_t str_len = strlen(str);
  size_t sub_len = strlen(sub_str);
  if (strcmp(sub_str, &str[str_len - sub_len]) == 0)
  {
    return 1;
  }
  return 0;
}

char*
marker_string_alloc(const char* str)
{
  size_t len = strlen(str) + 1;
  char* new_str = (char*) malloc(len);
  memcpy(new_str, str, len);
  return new_str;
}

