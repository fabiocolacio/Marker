#include "scroll-extension.h"

static void
simple_callback (WebKitWebExtension *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data)
{
    g_print ("Page %d created for %s\n",
             webkit_web_page_get_id (web_page),
             webkit_web_page_get_uri (web_page));
}

G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
    g_signal_connect (extension, "page-created",
                      G_CALLBACK (simple_callback),
                      NULL);
}