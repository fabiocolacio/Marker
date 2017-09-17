#ifndef __MARKER_STRING_H__
#define __MARKER_STRING_H__

int
marker_string_ends_with(const char* str,
                        const char* sub_str);

char*
marker_string_alloc(const char* str);

char*
marker_string_prepend(const char* str,
                      const char* addition);

char*
marker_string_append(const char* str, 
                     const char* addition);

#endif

