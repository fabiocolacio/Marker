/*
 * click-extension.c
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Marker; see the file LICENSE.md. If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include "click-extension.h"

static void 
on_click(WebKitDOMEventTarget * target,
      gpointer               user_data)
{
    WebKitDOMElement* element =  target;
    WebKitDOMDocument * document = webkit_dom_node_get_owner_document (element);
    if (element && document)
    {    
        gdouble offset = webkit_dom_element_get_offset_top (element) +
                         webkit_dom_element_get_client_top (element);
        glong doc_height = webkit_dom_html_document_get_height (document);

        gdouble ratio = offset / doc_height;
        g_print("position: %f, ratio: %f\n",offset, ratio);        
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
   
    WebKitDOMHTMLCollection* children = webkit_dom_element_get_children (element);
    if (children)
    {
        gulong n = webkit_dom_html_collection_get_length (children);
        if (n)
        {
            gulong i = 0;
            for (;i<n;i++)
            {
                WebKitDOMElement * child = webkit_dom_html_collection_item(children, i);
                enable_click_callback(child);
            }
        }
        else
        {
            webkit_dom_event_target_add_event_listener(element, "click", 
                                                        click_callback,
                                                        FALSE,
                                                        NULL);
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
   
    WebKitDOMHTMLCollection* children = webkit_dom_element_get_children (element);
    if (children)
    {
        gulong n = webkit_dom_html_collection_get_length (children);
        gulong i = 0;
        for (;i<n;i++)
        {
            WebKitDOMElement * child = webkit_dom_html_collection_item(children, i);
            webkit_dom_event_target_remove_event_listener(child,
                                                            "click",
                                                            click_callback,
                                                            FALSE);
            disable_click_callback(child);
        }
    }
}

static void
create_body(WebKitWebPage   *web_page,
                gpointer     user_data)
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
