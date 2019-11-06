/*
 * scroll-extension.c
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

#include <string.h>
#include <math.h>

#include "scroll-extension.h"

#define TAG "cursor_pos"

/**TODO: Clear user_data when closing window.
 * I did not found any signal to connect
 *  to a cleaning function!
**/



static void
document_loaded_cb (WebKitWebPage *web_page,
                    gpointer       user_data)
{
    WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement *posmarker = webkit_dom_document_get_element_by_id(document, TAG);
  
    if (posmarker){
      webkit_dom_element_scroll_into_view(posmarker, FALSE);
    } 
}

static gboolean
send_request_cb (WebKitWebPage     *web_page,
                 WebKitURIRequest  *request,
                 WebKitURIResponse *redirected_response,
                 gpointer           user_data)
{
  WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
  WebKitDOMElement *posmarker = webkit_dom_document_get_element_by_id(document, TAG);
  
  if (posmarker){
    webkit_dom_element_scroll_into_view(posmarker, FALSE);
  } 

  return TRUE;
}

static void
page_created_cb (WebKitWebExtension *extension,
                 WebKitWebPage      *web_page,
                 gpointer            user_data)
{
    /** create a new position index for each thread.**/
    g_signal_connect (web_page, "document-loaded",
                      G_CALLBACK (document_loaded_cb),
                      0);
    /** g_signal_connect (web_page, "send-request",
     *G_CALLBACK (send_request_cb),
     * 0); **/
}


G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension)
{
  g_print("scroll 2.0 extension initialized\n");
  g_signal_connect (extension, "page-created",
                    G_CALLBACK (page_created_cb),
                    NULL);
}
