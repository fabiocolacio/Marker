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

/**TODO: Clear user_data when closing window.
 * I did not found any signal to connect to a cleaning function!
**/

struct {
  glong horizontal;
  glong vertical;
  gboolean vertical_lock;
}typedef Scroll;

enum
{
  VERTICAL_SCROLL = 0,
  HORIZONTAL_SCROLL = 1
};

static void
document_loaded_cb (WebKitWebPage *web_page,
                    gpointer       user_data)
{
    WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
    WebKitDOMElement *body = WEBKIT_DOM_ELEMENT (webkit_dom_document_get_body (document));

    const Scroll *pos = user_data;

    if (body){
      if (pos->vertical_lock)
      {
          webkit_dom_element_set_scroll_top (body, webkit_dom_element_get_scroll_height(body));
      }else
      {
        webkit_dom_element_set_scroll_top (body, pos->vertical);
      }
      webkit_dom_element_set_scroll_left (body, pos->horizontal);
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
    if (strstr (uri, ".md") != NULL || strstr(uri, ".sd"))
    {
      WebKitDOMDocument *document = webkit_web_page_get_dom_document (web_page);
      WebKitDOMDOMWindow * view = webkit_dom_document_get_default_view(document);

      WebKitDOMElement *body = WEBKIT_DOM_ELEMENT (webkit_dom_document_get_body (document));
      Scroll * pos = user_data;

      if (body)
      {
        pos->vertical = webkit_dom_element_get_scroll_top (body);
        pos->horizontal = webkit_dom_element_get_scroll_left (body);

        glong height = webkit_dom_element_get_scroll_height(body);
        glong offset = webkit_dom_dom_window_get_inner_height(view) + pos->vertical;
        pos->vertical_lock = offset >= height;
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
    Scroll *pos = g_malloc (sizeof(Scroll));
    pos->horizontal = 0;
    pos->vertical = 0;
    pos->vertical_lock = FALSE;

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
