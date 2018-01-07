#include "scroll-extension.h"
/** 
 * TODO: Clear user_data when closing window. 
 * I did not found any signal to connect to a cleaning function!
**/

static void
restore_scroll_position(WebKitWebPage   *web_page,
                gpointer        user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    if (body){
        glong *pos = user_data;
        webkit_dom_element_set_scroll_top(body, *pos);
    }
}

static gboolean 
store_scroll_position(WebKitWebPage     *web_page,
                       WebKitURIRequest  *request,
                       WebKitURIResponse *redirected_response,
                       gpointer           user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    if (body){
        glong *pos = user_data;
        *pos = webkit_dom_element_get_scroll_top(body); 
    }
    return FALSE;
}

static void
simple_callback (WebKitWebExtension *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data)
{
    /** create a new position index for each thread.**/
    glong * pos = malloc(sizeof(glong));
    *pos = 0;

    g_signal_connect(web_page, "document-loaded",
                     G_CALLBACK(restore_scroll_position), 
                     pos);
    g_signal_connect (web_page, "send-request",
                             G_CALLBACK (store_scroll_position),
                             pos);
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
    g_signal_connect (extension, "page-created",
                      G_CALLBACK (simple_callback),
                      NULL);
}