#include "renderer.h"
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


void ticks_free(ticks t)
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

svg_plane compute_plane(chart * c)
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


ticks compute_y_ticks(svg_plane plane)
{
    ticks r;
    r.labels = NULL;
    r.vals = NULL;
    r.pos = NULL;
    
    double range = plane.y_max- plane.y_min;
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
                t0 = floor(plane.y_min/tick);
 
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
    double dx = plane.h/range;
    int i;
    for (i = 0; i< r.n; i++)
    {
        double v = r.tick*pow(10, r.t_exp)*(t0 + i-1);
        int e = (int)floor(log10(fabs(v)));
        double d = v/pow(10,e);
        
        vals[i] = v;
        char * l = malloc(8*sizeof(char));
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
        
        labels[i] = l;
        pos[i] = plane.y0 - dx*(vals[i] - plane.y_min);
    }
    r.vals = vals;
    r.pos = pos;
    r.labels = labels;

    return r; 
}



ticks compute_x_ticks(svg_plane plane)
{
    ticks r;
    r.labels = NULL;
    r.vals = NULL;
    r.pos = NULL;
    
    double range = plane.x_max- plane.x_min;
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
                t0 = (int)floor(plane.x_min/tick);

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
    double dx = plane.w/range;
    int i;
    for (i = 0; i< r.n; i++)
    {
        double v = r.tick*pow(10, r.t_exp)*(t0 + i-1);
        int e = (int)floor(log10(fabs(v)));
        double d = v/pow(10,e);
        
        vals[i] = v;
        char * l = malloc(6*sizeof(char));
        memset(l, 0, 6);
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
        labels[i] = l;
        pos[i] = plane.left + dx*(vals[i] - plane.x_min);
    }
    r.vals = vals;
    r.pos = pos;
    r.labels = labels;

    return r;
}

void axis_to_svg(char* buffer, chart* c, svg_plane plane)
{
    int p_h = plane.top;
    int p_w = plane.left;
    int w = plane.w;
    int h = plane.h;
    sprintf(buffer,
            "%s<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
            "style=\"fill:none; stroke:black; stroke-width:0.5;\" />\n",
            buffer, p_w, p_h, w, h);
    if (c->x_axis.label != NULL)
    {
        double tx = p_w + plane.w/2;
        double ty = y_margin + plane.y0 +y_margin/2;
        sprintf(buffer,
                "%s<text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\">%s</text>\n",
                buffer, tx, ty, c->x_axis.label);
    }
    if (c->y_axis.label != NULL)
    {
        double tx = 1;
        double ty = p_h+plane.h/2;
        sprintf(buffer,
                "%s<text x=\"%.2f\" y=\"%.2f\" transform=\"rotate(270,%.2f,%.2f) translate(0, 10)\" text-anchor=\"middle\">%s</text>\n",
                buffer, tx, ty, tx, ty, c->y_axis.label);
    }
}

void x_ticks_to_svg(char* buffer, svg_plane plane, ticks t)
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
        sprintf(buffer,
                "%s<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" style=\"stroke: black;stroke-width:1\"/>\n",
                buffer, t.pos[i], plane.y0+1, t.pos[i], plane.y0+6);
        sprintf(buffer,
                "%s<text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\">%s</text>\n",
                buffer, t.pos[i], ty, t.labels[i]);

    }
}

void y_ticks_to_svg(char* buffer, svg_plane plane, ticks t)
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
        sprintf(buffer,
                "%s<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" style=\"stroke: black;stroke-width:1\"/>\n",
                buffer, plane.left-6,  t.pos[i], plane.left-1, t.pos[i]);
        sprintf(buffer,
                "%s<text x=\"%.2f\" y=\"%.2f\"  text-anchor=\"end\" dy=\".4em\">%s</text>\n",
                buffer, tx, t.pos[i],  t.labels[i]);

    }
}

double get_x(double x, svg_plane p, axis a)
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

double get_y(double y, svg_plane p, axis a)
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

cbool is_marker(char c)
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

void draw_point(char* buffer, char style, char* color, double x, double y)
{
    switch (style){
        case 'o':
            sprintf(buffer, "%s<circle clip-path=\"url(#plot-area)\" cx=\"%.2f\" cy=\"%.2f\" r=\"3\" fill=\"%s\" />",
                    buffer, x, y, color);
            break;
        case 's':
            sprintf(buffer, "%s<rect clip-path=\"url(#plot-area)\" x=\"%.2f\" y=\"%.2f\" width=\"6\" height=\"6\" "
                            "style=\"fill:%s; stroke:none; stroke-width:0;\" />\n",
                    buffer, x-3, y-3, color);
            break;
        case 'x':
            sprintf(buffer, "%s<line clip-path=\"url(#plot-area)\" x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
                            "style=\"fill:none; stroke:%s; stroke-width:1.5;\" />\n" ,
                    buffer, x-3, y-3, x+3, y+3, color);

            sprintf(buffer, "%s<line clip-path=\"url(#plot-area)\" x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
                            "style=\"fill:none; stroke:%s; stroke-width:1.5;\" />\n" ,
                    buffer, x+3, y-3, x-3, y+3, color);
            break;
        case '+':
            sprintf(buffer, "%s<line clip-path=\"url(#plot-area)\" x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
                            "style=\"fill:none; stroke:%s; stroke-width:1.5;\" />\n" ,
                    buffer, x-4.5, y, x+4.5, y, color);

            sprintf(buffer, "%s<line clip-path=\"url(#plot-area)\" x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
                            "style=\"fill:none; stroke:%s; stroke-width:1.5;\" />\n" ,
                    buffer, x, y-4.5, x, y+4.5, color);
            break;
    }
}

char * get_style(lineStyle s)
{
    if (s == DOTTED)
        return "stroke-dasharray=\"1, 5\"";
    if (s == DASHED)
        return "stroke-dasharray=\"5, 5\"";
    return "";
}

void legend_to_svg(char* buffer, plotList * plots, svg_plane plane)
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
    sprintf(buffer, "%s<rect x=\"%.2f\" y=\"%.2f\" width=\"%d\" height=\"%d\" "
            "style=\"fill:white; fill-opacity:0.5; stroke:#333; stroke-width:0.5;\" />\n",
            buffer, x, y, w, h);
    sprintf(buffer, "%s<defs>"
                "<clipPath id=\"leg-area\">"
                "<rect x=\"%.2f\" y=\"%.2f\" width=\"%d\" height=\"%d\"/>"
                "</clipPath>"
                "</defs>\n",
                buffer, x+dw, y, w-dw-2, h);
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
                sprintf(buffer, "%s<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\"  y2=\"%.2f\" %s style=\"fill:none; stroke:%s; stroke-width:%.2f;\" />\n",
                        buffer, x+10, y-4, x+dw, y-4, get_style(p->line_style), color, p->line_width);
            }
            sprintf(buffer, "%s<text x=\"%.2f\" y=\"%.2f\" fill=\"#111\" font-size=\"12\" text-anchor=\"middle\"  clip-path=\"url(#leg-area)\">%s</text>\n",
                    buffer, x+dw+30, y, p->label);

            if ((is_marker(p->marker_style) && p->type == LINE ) || p->type == SCATTER)
            {
                draw_point(buffer, p->marker_style, color, x+10+(dw-10)/2, y-4);
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


void line_plot_to_svg(char* buffer, chart* c, plot* p, svg_plane plane, unsigned int index)
{
    unsigned int n = p->n;
    if (!n || p->y_data == NULL){
        return;
    }
    unsigned int i;
    double dy = plane.h/(plane.y_max-plane.y_min);
    char * color = p->color;
    
    if (!color)
    {
        color = malloc(8*sizeof(char));
        memcpy(color, colormap[index%10], 7);
    }

    if (p->line_style != NOLINE)
    {
        sprintf(buffer, "%s<polyline points=\"",
                buffer);
        for (i=0; i< n;i++){
            double x = p->x_data == NULL ? i+1 : p->x_data[i];
            double y = p->y_data[i];

            x = get_x(x, plane, c->x_axis);
            y = get_y(y, plane, c->y_axis);

            sprintf(buffer, "%s%.2f,%.2f ",
                    buffer, x, y);
        }
       
        char * dash_style = get_style(p->line_style);

        sprintf(buffer, "%s\" style=\"fill:none; stroke:%s; stroke-width:%.2f;\" %s clip-path=\"url(#plot-area)\"/>\n",
                buffer, color, p->line_width, dash_style);
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

void scatter_to_svg(char* buffer, chart* c, plot* p, svg_plane plane, unsigned int index)
{
 unsigned int n = p->n;
    if (!n || p->y_data == NULL){
        return;
    }
    unsigned int i;
    double dy = plane.h/(plane.y_max-plane.y_min);
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


void plots_to_svg(char* buffer, chart* c, svg_plane plane)
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
    }
}


void x_grid_to_svg(char* buffer, svg_plane p, ticks t)
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
        sprintf(buffer,
                "%s<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" style=\"stroke: #ccc;stroke-width:0.5\"/>\n",
                buffer, t.pos[i], sy, t.pos[i], se);
    }
}

void y_grid_to_svg(char* buffer, svg_plane p, ticks t)
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
        sprintf(buffer,
                "%s<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" style=\"stroke: #ccc;stroke-width:0.5\"/>\n",
                buffer, sy, t.pos[i], se, t.pos[i]);
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
    sprintf(buffer,
            "%s<text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"  font-weight=\"bold\">%s</text>\n",
            buffer, tx, ty, title);
}

char * 
chart_to_svg(chart* chart)
{
    char * buffer = malloc(1024*1024*sizeof(char));
    memset(buffer, 0, 1024*1024);
    
    sprintf(buffer, "%s<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%u\" height=\"%u\">\n", 
            buffer, chart->width, chart->height);
    svg_plane p = compute_plane(chart);
    sprintf(buffer, "%s<defs>"
                    "<clipPath id=\"plot-area\">"
                    "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>"
                    "</clipPath>"
                    "</defs>",
                    buffer, p.left, p.top, p.w, p.h);
    ticks x_t = compute_x_ticks(p);
    ticks y_t = compute_y_ticks(p);
    title_to_svg(buffer, p, chart->title);
    x_grid_to_svg(buffer, p, x_t);
    y_grid_to_svg(buffer, p, y_t);
    plots_to_svg(buffer, chart, p);
    axis_to_svg(buffer, chart, p);
    x_ticks_to_svg(buffer, p, x_t);
    y_ticks_to_svg(buffer, p, y_t);
    legend_to_svg(buffer, chart->plots, p);
    sprintf(buffer, "%s</svg>\n", buffer);
    ticks_free(x_t);
    ticks_free(y_t);
    return buffer;
}