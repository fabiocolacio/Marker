#ifndef __CHARTER_H__
#define __CHARTER_H__

enum{
    NONE        = 0,
    AXIS_X      = 1,
    AXIS_Y      = 2,
    PLOT        = 3
}typedef _pstate;

enum{
    FALSE       = 0,
    TRUE        = 1
}typedef cbool;

enum{
    LINEAR      = 0,
    LOG         = 1
}typedef axisMode;

enum{
    LINE        = 0,
    SCATTER     = 1,
    BAR         = 2
}typedef plotType;

enum{
    NORMAL      = 0,
    DOTTED      = 1,
    DASHED      = 2,
    DOT_DASH    = 3,
    NOLINE      = 4
}typedef lineStyle;

struct{
    axisMode        mode;
    char*           label;
    cbool           autoscale;
    double          range_min;
    double          range_max;
}typedef axis;

enum{
    ND = 0,
    DATA = 1,
    CSV = 2,
    MATH = 3,
    RANGE = 4
}typedef dtype;

struct{
    plotType        type;
    char*           label;
    char*           color;
    lineStyle       line_style;
    char            marker_style;
    double          line_width;
    double*         x_data;
    void*           y_data;
    unsigned int    n;
    dtype           y_type;
    void*           extra_data;
}typedef plot;

struct{
    char*           line_color;
    double          bar_width;
}typedef barPref;

struct
{
    plot*           plot;
    void*           next;
}typedef plotList;

struct{
    axis            x_axis;
    axis            y_axis;   
    unsigned int    width;
    unsigned int    height;
    plotList*       plots;
    unsigned int    n_plots;
    char *          title;
}typedef chart;



cbool 
is_empty(plotList *e);

plotList* 
plot_new_element(plot*);

void 
plot_append(plotList *, plot*);

plot* 
plot_at(plotList*, unsigned int);

double * 
plot_eval_y(plot *p);

plotList* 
plot_get_last_element(plotList*);

unsigned int 
chart_add_plot(chart*, plot*);

plot* 
chart_get_plot(chart*, unsigned int);

chart*
initialize_empty_chart();


double 
plot_get_max_x(plot *);

double 
plot_get_max_y(plot *);

double 
plot_get_min_x(plot *);

double 
plot_get_min_y(plot *);

double 
chart_get_max_x(chart *);

double 
chart_get_max_y(chart *);

double 
chart_get_min_x(chart *);

double 
chart_get_min_y(chart *);

void chart_free(chart *);

void plot_list_free(plotList *);

void plot_free(plot *);

void bar_pref_free(barPref *);

#endif