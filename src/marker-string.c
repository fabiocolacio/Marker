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

char*
marker_string_prepend(const char* str,
                      const char* addition)
{
  size_t str_len = strlen(str);
  size_t add_len = strlen(addition);
  size_t len = str_len + add_len + 1;
  char* new_str = malloc(len);
  memset(new_str, 0, len);
  strcat(new_str, addition);
  strcat(new_str, str);
  return new_str;
}

char*
marker_string_append(const char* str,
                     const char* addition)
{
  return marker_string_prepend(addition, str);
}

