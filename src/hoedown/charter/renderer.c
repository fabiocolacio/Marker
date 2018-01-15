#include "renderer.h"
#include "svg_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define x_margin 35
#define y_margin 25

#define min_ticks 3
#define max_ticks 8

struct{
    double x_min;
    double y_min;
    double x_max;
    double y_max;
    double x0;
    double y0;
    double top;
    double left;
    double w;
    double h;
    axisMode x_mode;
    axisMode y_mode;
} typedef svg_plane;

struct
{
    double *        vals;
    double *        pos;
    char **         labels;
    unsigned int    n;
    int             t_exp;
    int             tick;
}typedef ticks;


const char *colormap[10] = {
    "#db5f57",
    "#dbae57",
    "#b9db57",
    "#69db57",
    "#57db94",
    "#57d3db",
    "#5784db",
    "#7957db",
    "#c957db",
    "#db579e"
};

const int tick_mul[3] = {5, 2, 1};


void
ticks_free(ticks t)
{
    if (t.n > 0)
    {
        if (t.vals != NULL)
            free(t.vals);
        if (t.pos != NULL)
            free(t.pos);

        if (t.labels != NULL)
        {
            int i;
            for (i = 0; i < t.n; i++)
            {
                free(t.labels[i]);
            }
            free(t.labels);
        }
    }
}

svg_plane
compute_plane(chart * c)
{
    int p_h = y_margin;
    int p_w = x_margin;
    int w = c->width -  (c->y_axis.label != NULL ? 3*p_w : 2.5*p_w);
    int h = c->height - (c->x_axis.label != NULL ? 3*p_h : 2*p_h);

    svg_plane p;

    p.x_mode = c->x_axis.mode;
    p.y_mode = c->y_axis.mode;

    p.h = h;
    p.w = w;
    p.top = p_h;
    p.left = (c->y_axis.label != NULL ? 2*p_w : 1.5*p_w);
    p.x0 =  (c->y_axis.label != NULL ? 2*p_w : 1.5*p_w);
    p.y0 = h+p_h;

    if (c->x_axis.mode == LINEAR)
    {
        p.x_max = chart_get_max_x(c);
        p.x_min = chart_get_min_x(c);
    } else
    {
        p.x_max = ceil(log10(chart_get_max_x(c)));
        p.x_min = floor(log10(chart_get_min_x(c)));
    }

    if (c->y_axis.mode == LINEAR)
    {
        p.y_max = chart_get_max_y(c);
        p.y_min = chart_get_min_y(c);
    } else
    {
        p.y_max = ceil(log10(chart_get_max_y(c)));
        p.y_min = floor(log10(chart_get_min_y(c)));
    }
    return p;
}

ticks
compute_ticks(double min,
              double max,
              double origin,
              double size,
              cbool  vertical,
              cbool  logmode)
{
    ticks r;
    r.labels = NULL;
    r.vals = NULL;
    r.pos = NULL;

    double range = max - min;
    int r_xp = (int)floor(log10(range));
    int r_np = r_xp;
    cbool done = FALSE;
    double tick;
    int t0 ;
    for (r_np = r_xp + 2; r_np >= r_xp-2; r_np --)
    {
        int i;
        for (i = 0; i < 3; i++)
        {
            tick = tick_mul[i]*pow(10, r_np);
            int count = (int)floor(range/tick);

            if (count >= min_ticks && count <= max_ticks)
            {
                t0 = floor(min/tick);

                r.n = count+3;
                r.tick = tick_mul[i];
                r.t_exp = r_np;
                done = TRUE;
                break;
            }
        }
        if (done)
        {
            break;
        }
    }

    if (!done)
    {
        return r;
    }
    double *vals = malloc(r.n*sizeof(double));
    double *pos = malloc(r.n*sizeof(double));

    char  **labels =malloc(sizeof(char*)*r.n);
    double dx = size/range;
    int i;
    for (i = 0; i< r.n; i++)
    {
        double v = r.tick*pow(10, r.t_exp)*(t0 + i-1);
        int e = (int)floor(log10(fabs(v)));
        double d = v/pow(10,e);

        vals[i] = v;
        char * l;
        if (logmode)
        {
            l = malloc(128*sizeof(char));
            memset(l, 0, 128);
            if (v - round(v) == 0)
            {
                sprintf(l, "<tspan>10<tspan  font-size=\"11\" baseline-shift=\"super\">%d</tspan></tspan>", (int)v);
            }
            if (vertical){
                pos[i] = origin - dx*((vals[i]) - min);
            }else
            {
                pos[i] = origin + dx*((vals[i]) - min);
            }
        }else{
            l = malloc(8*sizeof(char));
            memset(l, 0, 8);
            if (v == 0)
            {
                sprintf(l, "0");
            }
            else if (e > -2 && e < 2)
            {
                sprintf(l, "%.1f", v);
            }
            else
            {
                sprintf(l, "%.1fe%d", d, e);
            }
            if (vertical){
                pos[i] = origin - dx*(vals[i] - min);
            }else
            {
                pos[i] = origin + dx*(vals[i] - min);
            }
        }

        labels[i] = l;

    }
    r.vals = vals;
    r.pos = pos;
    r.labels = labels;

    return r;
}

ticks
compute_y_ticks(svg_plane plane, cbool logmode)
{
    return compute_ticks(plane.y_min, plane.y_max, plane.y0, plane.h, TRUE, logmode);
}

ticks
compute_x_ticks(svg_plane plane, cbool logmode)
{
    return compute_ticks(plane.x_min, plane.x_max, plane.x0, plane.w, FALSE, logmode);
}

void
axis_to_svg(char*       buffer,
            chart*      c,
            svg_plane   plane)
{
    int p_h = plane.top;
    int p_w = plane.left;
    int w = plane.w;
    int h = plane.h;
    rect(buffer, p_w, p_h, w, h, "none", "black", 0.5, NULL);

    if (c->x_axis.label != NULL)
    {
        double tx = p_w + plane.w/2;
        double ty = y_margin + plane.y0 +y_margin/2;
        regular_text(buffer, tx, ty, TXT_MIDDLE, c->x_axis.label, NULL);
    }
    if (c->y_axis.label != NULL)
    {
        double tx = 1;
        double ty = p_h+plane.h/2;
        char * transform = malloc(64*sizeof(char));
        memset(transform, 0, 64);

        sprintf(transform, "rotate(270,%.2f,%.2f) translate(0, 10)", tx, ty);

        text_transform(buffer, tx, ty, TXT_MIDDLE, TXT_NORMAL, transform, c->y_axis.label, NULL);
        free(transform);
    }
}

void
x_ticks_to_svg(char*        buffer,
               svg_plane    plane,
               ticks        t)
{
    if (!t.vals || plane.x_min == plane.x_max)
        return;
    unsigned int i;
    double ty = 20 + plane.y0;
    for (i=0;i<t.n;i++)
    {
        if (t.vals[i] < plane.x_min || t.vals[i] > plane.x_max)
        {
            continue;
        }
        line(buffer, t.pos[i], plane.y0+1, t.pos[i], plane.y0+6, "black", 1.0, NULL);
        regular_text(buffer, t.pos[i], ty, TXT_MIDDLE, t.labels[i], NULL);
    }
}

void
y_ticks_to_svg(char*     buffer,
               svg_plane plane,
               ticks     t)
{

    if (!t.vals || plane.y_min == plane.y_max)
        return;
    unsigned int i;
    double tx = plane.left - 10;
    for (i=0;i<t.n;i++)
    {
        if (t.vals[i] < plane.y_min || t.vals[i] > plane.y_max)
        {
            continue;
        }
        line(buffer, plane.left-6, t.pos[i], plane.left-1, t.pos[i], "black", 1.0, NULL);
        regular_text(buffer, tx, t.pos[i]+4, TXT_RIGHT, t.labels[i], NULL);
    }
}

double
get_x(double    x,
      svg_plane p,
      axis      a)
{
    if (a.mode == LOG){
        x = log10(x);
    }
    double dx = p.w/(p.x_max-p.x_min);
    x -= p.x_min;
    x *= dx;
    x += p.x0;
    return x;
}

double
get_y(double    y,
      svg_plane p,
      axis      a)
{
    if (a.mode == LOG){
        y = log10(y);
    }
    double dy = p.h/(p.y_max-p.y_min);
    y -= p.y_min;
    y *= dy;
    y = p.y0-y;
    return y;
}

cbool
is_marker(char c)
{
    switch (c)
    {
        case 'o':
            return TRUE;
        case 's':
            return TRUE;
        case 'x':
            return TRUE;
        case '+':
            return TRUE;
        default:
            return FALSE;
    }
}

void
draw_bar(char *     buffer,
         double     lw,
         char*      color,
         char*      lcolor,
         double     x,
         double     y,
         double     y0,
         double     w)
{
    rect(buffer, x-w/2, (y < y0 ? y : y0), w, fabs(y-y0), color, lcolor, lw, "plot-area");
}


void
draw_point(char*    buffer,
           char     style,
           char*    color,
           double   x,
           double   y)
{
    switch (style){
        case 'o':
            circle(buffer, x, y, 3, color, "plot-area");
            break;
        case 's':
            rect(buffer, x-3, y-3, 6, 6, color, "none", 0, "plot-area");
            break;
        case 'x':
            line(buffer, x-3, y-3, x+3, y+3, color, 1.5, "plot-area");
            line(buffer, x+3, y-3, x-3, y+3, color, 1.5, "plot-area");
            break;
        case '+':
            line(buffer, x-4.5, y, x+4.5, y, color, 1.5, "plot-area");
            line(buffer, x, y-4.5, x, y+4.5, color, 1.5, "plot-area");
            break;
    }
}

char*
get_style(lineStyle s)
{
    if (s == DOTTED)
        return "stroke-dasharray=\"1, 5\"";
    if (s == DASHED)
        return "stroke-dasharray=\"5, 5\"";
    return "";
}

void
legend_to_svg(char*     buffer,
              plotList* plots,
              svg_plane plane)
{
    if (plots == NULL || plots->plot == NULL)
        return;

    plotList * el = plots;
    unsigned int c = el->plot->label == NULL ? 0 : 1;
    while ((el = el->next) != NULL)
    {
        c += el->plot->label == NULL ? 0 : 1;
    }
    if (c == 0)
        return;
    int dh = 15;
    int dw = 40;
    int h = dh*c+6;
    int w = 100;
    double x = plane.left+plane.w-10-w;
    double y = plane.top+10;
    rect_alpha(buffer, x, y, w, h, "white", 0.5, "#333", 0.5, NULL);
    clip_region(buffer, x+dw, y, w-dw-2, h, "leg-area");
    el = plots;
    int ind= 0;
    while (el!= NULL)
    {
        if (el->plot->label != NULL)
        {
            plot * p = el->plot;
            char * color = p->color;
            if (!color)
            {
                color = malloc(8*sizeof(char));
                memcpy(color, colormap[ind%10], 7);
            }
            y+= dh;
            if (p->type == LINE && p->line_style != NOLINE)
            {
                styled_line(buffer, x+10, y-4, x+dw, y-4, color, p->line_width, get_style(p->line_style), NULL);
            }
            regular_text(buffer, x+dw+30, y, TXT_MIDDLE, p->label, "leg-area");

            if ((is_marker(p->marker_style) && p->type == LINE ) || p->type == SCATTER)
            {
                draw_point(buffer, p->marker_style, color, x+10+(dw-10)/2, y-4);
            }
            if (p->type == BAR)
            {
                barPref * pref = p->extra_data;
                double y0 = y;
                double w = dw-10;
                double xb = x+10+w/2;
                double yb = y - 8;
                draw_bar(buffer,p->line_width , color, pref->line_color, xb, yb, y0, w);
            }
            if (!p->color)
            {
                free(color);
            }
        }
        el = el->next;
        ind ++;
    }
}


void
line_plot_to_svg(char*          buffer,
                 chart*         c,
                 plot*          p,
                 svg_plane      plane,
                 unsigned int   index)
{
    unsigned int n = p->n;
    if (!n || p->y_data == NULL){
        return;
    }
    unsigned int i;
    char * color = p->color;

    if (!color)
    {
        color = malloc(8*sizeof(char));
        memcpy(color, colormap[index%10], 7);
    }

    if (p->line_style != NOLINE)
    {
        double *xs = malloc(n*sizeof(double));
        double *ys = malloc(n*sizeof(double));
        for (i=0; i< n;i++){
            double x = p->x_data == NULL ? i+1 : p->x_data[i];
            double y = p->y_data[i];

            xs[i] = get_x(x, plane, c->x_axis);
            ys[i] = get_y(y, plane, c->y_axis);
        }

        char * dash_style = get_style(p->line_style);
        poly_line(buffer, xs, ys, n, color, p->line_width, dash_style, "plot-area");
        free(xs);
        free(ys);
    }
    if (is_marker(p->marker_style))
    {


        for (i=0; i< n;i++){
            double x = p->x_data == NULL ? i+1 : p->x_data[i];
            double y = p->y_data[i];

            x = get_x(x, plane, c->x_axis);
            y = get_y(y, plane, c->y_axis);
            draw_point(buffer, p->marker_style, color, x, y);
        }
    }
    if (!p->color)
    {
        free(color);
    }
}

void
scatter_to_svg(char*        buffer,
               chart*       c,
               plot*        p,
               svg_plane    plane,
               unsigned int index)
{
    unsigned int n = p->n;
    if (!n || p->y_data == NULL){
        return;
    }
    unsigned int i;
    char * color = p->color;

    if (!color)
    {
        color = malloc(8*sizeof(char));
        memcpy(color, colormap[index%10], 7);
    }

    for (i=0; i< n;i++){
        double x = p->x_data == NULL ? i+1 : p->x_data[i];
        double y = p->y_data[i];

        x = get_x(x, plane, c->x_axis);
        y = get_y(y, plane, c->y_axis);
        draw_point(buffer, p->marker_style, color, x, y);
    }

    if (!p->color)
    {
        free(color);
    }
}


void
bar_to_svg(char*        buffer,
           chart*       c,
           plot*        p,
           svg_plane    plane,
           unsigned int index)
{
    unsigned int n = p->n;
    if (!n || p->y_data == NULL || p->extra_data == NULL){
        return;
    }
    unsigned int i;
    char * color = p->color;

    if (!color)
    {
        color = malloc(8*sizeof(char));
        memcpy(color, colormap[index%10], 7);
    }
    barPref * pref = p->extra_data;
    double y0 = get_y(0, plane, c->y_axis);
    double w = fabs(get_x(pref->bar_width, plane, c->x_axis));

    for (i=0; i< n;i++){
        double x = p->x_data == NULL ? i+1 : p->x_data[i];
        double y = p->y_data[i];

        x = get_x(x, plane, c->x_axis);
        y = get_y(y, plane, c->y_axis);

        draw_bar(buffer,p->line_width , color, pref->line_color, x, y, y0, w);
    }

    if (!p->color)
    {
        free(color);
    }
}


void
plots_to_svg(char*      buffer,
             chart*     c,
             svg_plane  plane)
{
    unsigned int i;
    for (i = 0; i<c->n_plots;i++)
    {
        plot * p = chart_get_plot(c, i);
        if (p->type == LINE)
        {
            line_plot_to_svg(buffer, c, p, plane, i);
        }
        if (p->type == SCATTER)
        {
            scatter_to_svg(buffer,c, p, plane, i);
        }
        if (p->type == BAR)
        {
            bar_to_svg(buffer, c, p, plane, i);
        }
    }
}


void
x_grid_to_svg(char*     buffer,
              svg_plane p,
              ticks     t)
{
    if (t.vals == NULL || p.x_max == p.x_min)
        return;
    unsigned int i;
    double sy = p.top;
    double se = p.y0;
    for (i=0;i<t.n;i++)
    {
        if (t.vals[i] < p.x_min || t.vals[i] > p.x_max)
        {
            continue;
        }
        line(buffer, t.pos[i], sy, t.pos[i], se, "#ccc", 0.5, "plot-area");
    }
}

void
y_grid_to_svg(char*     buffer,
              svg_plane p,
              ticks     t)
{
    if (t.vals == NULL || p.y_max == p.y_min)
        return;
    unsigned int i;
    double sy = p.x0;
    double se = p.x0+p.w;
    for (i=0;i<t.n;i++)
    {
        if (t.vals[i] < p.y_min || t.vals[i] > p.y_max)
        {
            continue;
        }
        line(buffer, sy, t.pos[i], se, t.pos[i], "#ccc", 0.5, "plot-area");
    }
}


void
title_to_svg(char* buffer,
             svg_plane p,
             char*  title)
{
    if (title == NULL)
        return;
    double tx = p.left + p.w/2;
    double ty = 15;
    bold_text(buffer, tx, ty, TXT_MIDDLE, title, NULL);
}

char *
chart_to_svg(chart* chart)
{
    char * buffer = malloc(1024*1024*sizeof(char));
    memset(buffer, 0, 1024*1024);
    svg_header(buffer, chart->width, chart->height);

    svg_plane p = compute_plane(chart);

    clip_region(buffer, p.left, p.top, p.w, p.h, "plot-area");

    ticks x_t = compute_x_ticks(p, chart->x_axis.mode == LOG);
    ticks y_t = compute_y_ticks(p, chart->y_axis.mode == LOG);

    title_to_svg(buffer, p, chart->title);

    x_grid_to_svg(buffer, p, x_t);
    y_grid_to_svg(buffer, p, y_t);

    plots_to_svg(buffer, chart, p);

    axis_to_svg(buffer, chart, p);

    x_ticks_to_svg(buffer, p, x_t);
    y_ticks_to_svg(buffer, p, y_t);

    legend_to_svg(buffer, chart->plots, p);

    svg_footer(buffer);

    ticks_free(x_t);
    ticks_free(y_t);
    return buffer;
}
