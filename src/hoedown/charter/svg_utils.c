#include "svg_utils.h"
#include <stdio.h>

char*
txt_align_to_char(txtAlign a)
{
    switch(a)
    {
        case TXT_MIDDLE:
            return "middle";
        case TXT_RIGHT:
            return "end";
        default:
            return "begin";
    }
}

char*
txt_style_to_char(txtStyle a)
{
   switch(a)
    {
        case TXT_BOLD:
            return "bold";
        default:
            return "regular";
    }
}


void
svg_header(char*  buffer,
           unsigned int width,
           unsigned int height)
{
    sprintf(buffer,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%u\" height=\"%u\">\n",
            width, height);
}

void
svg_footer(char* buffer)
{
    sprintf(buffer, "%s</svg>\n", buffer);
}

void
clip_region(char*  buffer,
             double x,
             double y,
             double width,
             double height,
             char*  id)
{
    sprintf(buffer, "%s<defs>"
            "<clipPath id=\"%s\">"
            "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n"
            "</clipPath>"
            "</defs>",
            buffer, id, x, y, width, height);
}

void
rect(char*  buffer,
     double x,
     double y,
     double width,
     double heigth,
     char*  fill,
     char*  stroke,
     double stroke_width,
     char*  clip_id)
{
    rect_alpha(buffer,x , y, width, heigth, fill, 1.0, stroke, stroke_width, clip_id);
}

void
rect_alpha(char*  buffer,
           double x,
           double y,
           double width,
           double heigth,
           char*  fill,
           double fill_alpha,
           char*  stroke,
           double stroke_width,
           char*  clip_id)
{
    if (clip_id)
    {
        sprintf(buffer, "%s<rect clip-path=\"url(#%s)\" ",
                buffer, clip_id);
    }else
    {
        sprintf(buffer, "%s<rect ", buffer);
    }
    sprintf(buffer,
            "%s x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "style=\"fill:%s; fill-opacity:%.2f; stroke:%s; stroke-width:%.2f;\" />\n",
            buffer, x, y, width, heigth, fill, fill_alpha, stroke, stroke_width);
}

void
line(char*  buffer,
     double x1,
     double y1,
     double x2,
     double y2,
     char*  color,
     double line_width,
     char*  clip_id)
{
    styled_line(buffer, x1, y1, x2, y2, color, line_width, "", clip_id);
}

void
styled_line(char*  buffer,
             double x1,
             double y1,
             double x2,
             double y2,
             char*  color,
             double line_width,
             char*  style,
             char*  clip_id)
{
    if (clip_id)
    {
       sprintf(buffer, "%s<line clip-path=\"url(#%s)\" ", buffer, clip_id);
    } else
    {
      sprintf(buffer, "%s<line ", buffer);
    }
    sprintf(buffer,
            "%s x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" %s style=\"stroke: %s;stroke-width:%.2f\"/>\n",
                buffer, x1, y1, x2, y2, style, color, line_width);
}


void
poly_line(char*         buffer,
          double*       xs,
          double*       ys,
          unsigned int  n,
          char*         color,
          double        line_width,
          char*         style,
          char*         clip_id)
{
    if (clip_id)
    {
        sprintf(buffer, "%s<polyline clip-path=\"url(#%s)\"", buffer, clip_id);
    }else
    {
        sprintf(buffer, "%s<polyline ", buffer);
    }
    sprintf(buffer, "%s style=\"fill:none; stroke:%s; stroke-width:%.2f;\" %s points=\"",
                buffer, color, line_width, style);

    unsigned int i;
    for (i = 0;i < n; i++)
    {
        sprintf(buffer, "%s%.2f,%.2f ", buffer, xs[i], ys[i]);
    }
    sprintf(buffer, "%s\" />", buffer);
}


void
text_transform(char*      buffer,
               double     x,
               double     y,
               txtAlign   anchor,
               txtStyle   style,
               char*      transform,
               char*      txt,
               char*      clip_id)
{
    if (clip_id)
    {
        sprintf(buffer, "%s<text clip-path=\"url(#%s)\" ", buffer, clip_id);
    }else
    {
        sprintf(buffer, "%s<text ", buffer);
    }
    if (transform)
    {
        sprintf(buffer, "%s transform=\"%s\" ", buffer, transform);
    }

    sprintf(buffer,
            "%s x=\"%.2f\" y=\"%.2f\" text-anchor=\"%s\"  font-weight=\"%s\">%s</text>\n",
            buffer, x, y, txt_align_to_char(anchor), txt_style_to_char(style), txt);
}

void
text(char*      buffer,
     double     x,
     double     y,
     txtAlign   anchor,
     txtStyle   style,
     char*      txt,
     char*      clip_id)
{
    text_transform(buffer, x, y, anchor, style, NULL, txt, clip_id);
}

void
bold_text(char*     buffer,
          double    x,
          double    y,
          txtAlign  anchor,
          char*     txt,
          char*     clip_id)
{
    text(buffer, x, y, anchor, TXT_BOLD, txt, clip_id);
}

void
regular_text(char*     buffer,
             double    x,
             double    y,
             txtAlign  anchor,
             char*     txt,
             char*     clip_id)
{
    text(buffer, x, y, anchor, TXT_NORMAL, txt, clip_id);
}


void
circle(char*  buffer,
       double x,
       double y,
       double r,
       char*  color,
       char*  clip_id)
{

    sprintf(buffer, "%s<circle", buffer);
    if (clip_id)
    {
        sprintf(buffer, "%s clip-path=\"url(#%s)\" ",
                        buffer, clip_id);
    }
    sprintf(buffer, "%s cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"%s\" />",
                    buffer, x, y, r, color);

}
