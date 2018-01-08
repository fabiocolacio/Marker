#include "click-extension.h"

static void 
on_click(WebKitDOMEventTarget * target,
      gpointer               user_data)
{
    WebKitDOMElement* body =  target;
    if (body)
    {    
        g_print("clicked: %s\n", webkit_dom_element_get_tag_name(body));
    }
}

static GCallback click_callback = G_CALLBACK(on_click);

static void
enable_click_callback(WebKitDOMElement * element)
{
    if (!element)
    {
        return;
    }
    webkit_dom_event_target_add_event_listener(element, "click", 
                                                click_callback,
                                                FALSE,
                                                NULL);

    WebKitDOMHTMLCollection* children = webkit_dom_element_get_children (element);
    if (children)
    {
        gulong n = webkit_dom_html_collection_get_length (children);
        gulong i = 0;
        for (;i<n;i++)
        {
            enable_click_callback(webkit_dom_html_collection_item(children, i));
        }
    }
}

static void
disable_click_callback(WebKitDOMElement * element)
{
    if (!element)
    {
        return;
    }

    webkit_dom_event_target_remove_event_listener(element,
                                                    "click",
                                                    click_callback,
                                                    FALSE);
    WebKitDOMHTMLCollection* children = webkit_dom_element_get_children (element);
    if (children)
    {
        gulong n = webkit_dom_html_collection_get_length (children);
        gulong i = 0;
        for (;i<n;i++)
        {
            enable_click_callback(webkit_dom_html_collection_item(children, i));
        }
    }
}

static void
create_body(WebKitWebPage   *web_page,
                gpointer        user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    if (body){
        /*retatch body click listener*/
        enable_click_callback(body);        
    }
}

static gboolean 
remove_body(WebKitWebPage     *web_page,
                       WebKitURIRequest  *request,
                       WebKitURIResponse *redirected_response,
                       gpointer           user_data)
{
    WebKitDOMDocument * document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement* body = webkit_dom_document_get_body (document);
    if (body){
        /*detach body click listener*/          
        disable_click_callback(body);          
    }
    return FALSE;
}

static void
initialize (WebKitWebExtension *extension,
                           WebKitWebPage      *web_page,
                           gpointer            user_data)
{
    g_signal_connect(web_page, "document-loaded",
                     G_CALLBACK(create_body), 
                     NULL);
    g_signal_connect(web_page, "send-request",
                     G_CALLBACK(remove_body),
                     NULL);
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
    g_signal_connect (extension, "page-created",
                      G_CALLBACK (initialize),
                      NULL);
}