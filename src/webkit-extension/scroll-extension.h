#ifndef __SCROLL_EXTENSION_H__
#define __SCROLL_EXTENSION_H__

#include <webkit2/webkit-web-extension.h>

static void
simple_callback (WebKitWebExtension *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data);
                           
G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension);

#endif