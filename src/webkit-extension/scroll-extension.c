#include "scroll-extension.h"
static glong old_scroll = 0;


static void
document_loaded(WebKitWebPage   *web_page,
                gpointer        user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    webkit_dom_element_set_scroll_top(body, old_scroll);
}
static gboolean 
web_page_send_request(WebKitWebPage     *web_page,
                       WebKitURIRequest  *request,
                       WebKitURIResponse *redirected_response,
                       gpointer           user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    if (body){
        old_scroll = webkit_dom_element_get_scroll_top(body); 
    }
    return FALSE;
}


static void
simple_callback (WebKitWebExtension *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data)
{
    g_signal_connect(web_page, "document-loaded", G_CALLBACK(document_loaded), NULL);
    g_signal_connect_object (web_page, "send-request",
                             G_CALLBACK (web_page_send_request),
                             NULL, 0);
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
    g_signal_connect (extension, "page-created",
                      G_CALLBACK (simple_callback),
                      NULL);
}