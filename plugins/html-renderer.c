#include <glib.h>
#include <glib/gprintf.h>

#include <marker-renderer.h>

char *
marker_renderer_render (const char *text)
{
    return g_strdup (text);
}
