#include "scroll-extension.h"

/** 
 * TODO: Clear user_data when closing window. 
 * I did not found any signal to connect to a cleaning function!
**/

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

static void
restore_scroll_position(WebKitWebPage   *web_page,
                        gpointer         user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = WEBKIT_DOM_ELEMENT(webkit_dom_document_get_body (document));
    
    const glong *pos = user_data;
    
    g_print("restore_scroll_position()\n");
    
    if (body){
        webkit_dom_element_set_scroll_top(body, *pos);     
    } else {
      g_print("  couldn't get body!\n");
    }
    
    g_print("  scroll position: %d\n", *pos);
}

static gboolean 
store_scroll_position(WebKitWebPage      *web_page,
                       WebKitURIRequest  *request,
                       WebKitURIResponse *redirected_response,
                       gpointer           user_data)
{
    const gchar *uri = webkit_uri_request_get_uri(request);
    if (marker_string_ends_with(uri, ".md"))
    {
      g_print("uri: %s\n", uri);
      
      WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
      WebKitDOMElement* body = WEBKIT_DOM_ELEMENT(webkit_dom_document_get_body (document));
      glong * pos = user_data;
      
      g_print("store_scroll_position()\n");
      if (body){
          *pos = webkit_dom_element_get_scroll_top(body);        
      } else {
      g_print("  couldn't get body!\n");
      }
      
      
      g_print("  scroll position: %d\n", *pos);
    }
    
    return FALSE;
}

static void
initialize (WebKitWebExtension                *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data)
{
    /** create a new position index for each thread.**/
    glong * pos = g_malloc(sizeof(glong));
    *pos = 0;

    g_signal_connect(web_page, "document-loaded",
                     G_CALLBACK(restore_scroll_position), 
                     pos);
    g_signal_connect(web_page, "send-request",
                     G_CALLBACK(store_scroll_position),
                     pos);
    
    g_print("initialize()\n");
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
    g_signal_connect (extension, "page-created",
                      G_CALLBACK (initialize),
                      NULL);
    g_print("webkit_web_extension_initialize()\n");
}
