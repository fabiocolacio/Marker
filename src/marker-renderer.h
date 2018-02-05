#ifndef __MARKER_RENDERER_H__
#define __MARKER_RENDERER_H__

/**
 * marker_renderer_render:
 *
 * @text: The UTF-8 encoded text of the active editor.
 *
 * Returns: A newly allocated buffer containing HTML data to be loaded by the preview.
 */
char                *marker_renderer_render                      (const char         *text);

#endif
