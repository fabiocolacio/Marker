/*
 * scroll-extension.c
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>

#include "scroll-extension.h"

/** 
 * TODO: Clear user_data when closing window. 
 * I did not found any signal to connect to a cleaning function!
**/

enum
{
  VERTICAL_SCROLL = 0,
  HORIZONTAL_SCROLL = 1
};

int
marker_string_ends_with (const char *str,
                         const char *sub_str)
{
  size_t str_len = strlen (str);
  size_t sub_len = strlen (sub_str);
  if (strcmp (sub_str, &str[str_len - sub_len]) == 0)
  {
    return 1;
  }
  return 0;
}

static void
document_loaded_cb (WebKitWebPage *web_page,
                    gpointer       user_data)
{
    WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement *body = WEBKIT_DOM_ELEMENT (webkit_dom_document_get_body (document));
    
    const glong *pos = user_data;
    
    if (body){
      webkit_dom_element_set_scroll_top (body, pos[0]);
      webkit_dom_element_set_scroll_left (body, pos[1]);     
    } else {
      g_error ("Error restoring scroll position!\n");
    }
}

static gboolean 
send_request_cb (WebKitWebPage     *web_page,
                 WebKitURIRequest  *request,
                 WebKitURIResponse *redirected_response,
                 gpointer           user_data)
{
    const gchar *uri = webkit_uri_request_get_uri (request);
    if (marker_string_ends_with (uri, ".md"))
    { 
      WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
      WebKitDOMElement *body = WEBKIT_DOM_ELEMENT (webkit_dom_document_get_body (document));
      glong * pos = user_data;
      
      if (body)
      {
        pos[VERTICAL_SCROLL] = webkit_dom_element_get_scroll_top (body);
        pos[HORIZONTAL_SCROLL] = webkit_dom_element_get_scroll_left (body);       
      }
      else
      {
        g_error ("Error restoring scroll position!\n");
      }
    }    
    
    return FALSE;
}

static void
page_created_cb (WebKitWebExtension *extension,
                 WebKitWebPage      *web_page,
                 gpointer            user_data)
{
    /** create a new position index for each thread.**/
    glong *pos = g_malloc (2 * sizeof (glong));
    pos[VERTICAL_SCROLL] = 0;
    pos[HORIZONTAL_SCROLL] = 0;

    g_signal_connect (web_page, "document-loaded",
                      G_CALLBACK (document_loaded_cb), 
                      pos);
    g_signal_connect (web_page, "send-request",
                      G_CALLBACK (send_request_cb),
                      pos);
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
  g_signal_connect (extension, "page-created",
                    G_CALLBACK (page_created_cb),
                    NULL);
}
