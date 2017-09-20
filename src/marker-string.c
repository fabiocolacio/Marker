#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
                      const char* addition,
                      char*       buffer,
                      size_t      buffer_size)
{
  size_t str_len = strlen(str);
  size_t add_len = strlen(addition);
  size_t len = str_len + add_len + 1;
  
  
  if (buffer)
  {
    size_t bf = buffer_size;
    memset(buffer, 0, buffer_size);
    
    if (add_len <= bf)
    {
      memcpy(buffer, addition, add_len);
      bf -= add_len;
      
      if (str_len <= bf)
      {
        memcpy(&buffer[add_len], str, str_len + 1);
      }
      else
      {
        memcpy(&buffer[str_len], str, bf);
        buffer[buffer_size - 1] = '\0';
      }
    }
    else
    {
      memcpy(&buffer[str_len], str, bf);
      buffer[buffer_size - 1] = '\0';
    }
    return NULL;
  }
 
  char* new_str = malloc(len);
  memset(new_str, 0, len);
  strcat(new_str, addition);
  strcat(new_str, str);
  return new_str;
}

char*
marker_string_append(const char* str,
                     const char* addition,
                     char*       buffer,
                     size_t      buffer_size)
{
  return marker_string_prepend(addition, str, buffer, buffer_size);
}

int
marker_string_buffer_set(const char* str,
                         char*       buffer,
                         size_t      buffer_size)
{
  size_t str_len = strlen(str);
  if (str_len >= buffer_size)
  {
    memset(buffer, 0, buffer_size);
    memcpy(buffer, str, buffer_size);
    buffer[buffer_size - 1] = '\0';
    return 1;
  }
  memcpy(buffer, str, str_len + 1);
  return 0;
}

char*
marker_string_rfind(const char* str,
                    const char* sub)
{
  size_t str_len = strlen(str);
  size_t sub_len = strlen(sub);
  const char* ptr;
  
  for (int i = str_len; i > 0; --i)
  {
    ptr = &str[i];
    if (memcmp(ptr, sub, sub_len) == 0)
    {
      return ptr;
    }
  }
  return '\0';
}

