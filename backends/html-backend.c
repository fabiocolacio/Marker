#include <string.h>

#include <marker/marker-backend.h>

char *
marker_backend_render (const char *text)
{
  return strdup (text);
}
