#ifndef __MARKER_STRING_H__
#define __MARKER_STRING_H__

int
marker_string_ends_with(const char* str,
                        const char* sub_str);

char*
marker_string_alloc(const char* str);

char*
marker_string_prepend(const char* str,
                      const char* addition,
                      char*       buffer,
                      size_t      buffer_size);

char*
marker_string_append(const char* str, 
                     const char* addition,
                     char*       buffer,
                     size_t      buffer_size);

int
marker_string_buffer_set(const char* str,
                         char*  buffer,
                         size_t buffer_size);
                         
char*
marker_string_rfind(const char* str,
                    const char* sub);

char*
marker_string_escape(const char* str);

char*
marker_string_filename_get_path(const char* filename);

#endif

