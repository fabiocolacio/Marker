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


svg_plane compute_plane(chart * c)
{
    int p_h = y_margin;
    int p_w = x_margin;
    int w = c->width -  (c->y_axis.label != NULL ? 3*p_w : 2.5*p_w);
    int h = c->height - (c->x_axis.label != NULL ? 3*p_h : 2*p_h);
    svg_plane p;
    p.h = h;
    p.w = w;
    p.top = p_h;
    p.left = (c->y_axis.label != NULL ? 2*p_w : 1.5*p_w); 
    p.x0 =  (c->y_axis.label != NULL ? 2*p_w : 1.5*p_w);
    p.y0 = h+p_h;
    p.x_max = chart_get_max_x(c);
    p.y_max = chart_get_max_y(c);
    p.x_min = chart_get_min_x(c);
    p.y_min = chart_get_min_y(c);
    return p;
}


ticks compute_y_ticks(svg_plane plane)
{
   ticks r;
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

void line_plot_to_svg(char* buffer, chart* c, plot* p, svg_plane plane, unsigned int index)
{
    unsigned int n = p->n;
    if (!n || p->y_data == NULL){
        return;
    }
    unsigned int i;
    double dx = plane.w/(plane.x_max-plane.x_min);
    double dy = plane.h/(plane.y_max-plane.y_min);
    
    sprintf(buffer, "%s<polyline points=\"",
            buffer);
    for (i=0; i< n;i++){
        double x = p->x_data == NULL ? i : p->x_data[i];
        double y = p->y_data[i];

        x -= plane.x_min;
        x *= dx;
        x += plane.x0;
        y -= plane.y_min;
        y *= dy;
        y = plane.y0-y;

        sprintf(buffer, "%s%.2f,%.2f ",
                buffer, x, y);
    }
    sprintf(buffer, "%s\" style=\"fill:none; stroke:%s; stroke-width:2;\" clip-path=\"url(#plot-area)\"/>\n",
            buffer, colormap[index%10]);
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


char * 
chart_to_svg(chart* chart)
{
    char * buffer = malloc(10000*sizeof(char));
    memset(buffer, 0, 10000);
    
    sprintf(buffer, "%s<svg width=\"%u\" height=\"%u\">\n", 
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
    x_grid_to_svg(buffer, p, x_t);
    y_grid_to_svg(buffer, p, y_t);
    plots_to_svg(buffer, chart, p);
    axis_to_svg(buffer, chart, p);
    x_ticks_to_svg(buffer, p, x_t);
    y_ticks_to_svg(buffer, p, y_t);
    sprintf(buffer, "%s</svg>\n", buffer);

    
    return buffer;
}