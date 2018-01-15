#ifndef __SVG_UTILS_H__
#define __SVG_UTILS_H__

enum
{
    TXT_LEFT = 0,
    TXT_MIDDLE = 1,
    TXT_RIGHT = 2
}typedef txtAlign;


enum{
    TXT_NORMAL  = 0,
    TXT_BOLD = 1
} typedef txtStyle;

void
svg_header(char*  buffer,
           unsigned int width,
           unsigned int height);

void
svg_footer(char* buffer);

void
clip_region(char*  buffer,
            double x,
            double y,
            double width,
            double height,
            char*  id);

void
rect(char*  buffer,
     double x,
     double y,
     double width,
     double heigth,
     char*  fill,
     char*  stroke,
     double stroke_width,
     char*  clip_id);

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
           char*  clip_id);

void
styled_line(char*  buffer,
             double x1,
             double y1,
             double x2,
             double y2,
             char*  color,
             double line_width,
             char*  style,
             char*  clip_id);

void
line(char*  buffer,
     double x1,
     double y1,
     double x2,
     double y2,
     char*  color,
     double line_width,
     char*  clip_id);

void
poly_line(char*         buffer,
          double*       xs,
          double*       ys,
          unsigned int  n,
          char*         color,
          double        line_width,
          char*         style,
          char*         clip_id);

void
text_transform(char*      buffer,
               double     x,
               double     y,
               txtAlign   anchor,
               txtStyle   style,
               char*      transform,
               char*      text,
               char*      clip_id);

void
text(char*      buffer,
     double     x,
     double     y,
     txtAlign   anchor,
     txtStyle   style,
     char*      text,
     char*      clip_id);

void
bold_text(char*     buffer,
          double    x,
          double    y,
          txtAlign  anchor,
          char*     text,
          char*     clip_id);

void
regular_text(char*     buffer,
             double    x,
             double    y,
             txtAlign  anchor,
             char*     text,
             char*     clip_id);

void
circle(char*  buffer,
       double x,
       double y,
       double r,
       char*  color,
       char*  clip_id);

#endif
