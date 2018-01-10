#include "charter.h"
#include <stdlib.h>

chart * 
initialize_empty_chart()
{
    chart * new_chart = malloc(sizeof(chart));
    new_chart->width = 600;
    new_chart->height = 400;
    new_chart->title = NULL;

    new_chart->n_plots = 0;

    new_chart->x_axis.autoscale = TRUE;
    new_chart->x_axis.mode = LINEAR;
    new_chart->x_axis.label = NULL;

    new_chart->y_axis.autoscale = TRUE;
    new_chart->y_axis.mode = LINEAR;
    new_chart->y_axis.label = NULL;

    new_chart->plots = plot_new_element(NULL);        

    return new_chart; 
}


double plot_get_max_x(plot* p)
{
    if (!p->n)
        return 0;
    if (p->x_data == NULL)
        return p->n-1;
    double max_x = p->x_data[0];
    unsigned int i;
    for (i = 1; i<p->n;i++)
    {
        if (p->x_data[i] > max_x)
            max_x = p->x_data[i];
    }
    return max_x;
}

double plot_get_min_x(plot* p)
{
    if (!p->n)
        return 0;
    if (p->x_data == NULL)
        return 0;
    double min_x = p->x_data[0];
    unsigned int i;
    for (i = 1; i<p->n;i++)
    {
        if (p->x_data[i] < min_x)
            min_x = p->x_data[i];
    }
    return min_x;
}

double plot_get_max_y(plot* p)
{
    if (!p->n || !p->y_data)
        return 0;
    double max_x = p->y_data[0];
    unsigned int i;
    for (i = 1; i<p->n;i++)
    {
        if (p->y_data[i] > max_x)
            max_x = p->y_data[i];
    }
    return max_x;
}

double plot_get_min_y(plot* p)
{
    if (!p->n || !p->y_data)
        return 0;
    double min_x = p->y_data[0];
    unsigned int i;
    for (i = 1; i<p->n;i++)
    {
        if (p->y_data[i] < min_x)
            min_x = p->y_data[i];
    }
    return min_x;
}


double chart_get_max_x(chart *c)
{
    if (c->x_axis.autoscale == TRUE){
        if (!c->n_plots)
            return 0;
        unsigned int i;
        double M = plot_get_max_x(c->plots->plot);
        double m = plot_get_min_x(c->plots->plot);
        for (i = 1; i < c->n_plots; i++)
        {
            plot * p = chart_get_plot(c, i);
            double v = plot_get_max_x(p);
            if (v > M)
                M = v;
            v = plot_get_min_x(p);
            if (v < m)
                m = v;
        }
        M += (M-m)*0.05;
        return M;
    }
    return c->x_axis.range_max;
}

double chart_get_min_x(chart *c)
{
    if (c->x_axis.autoscale  == TRUE){
        if (!c->n_plots) 
            return 0;
        unsigned int i;
        double M = plot_get_max_x(c->plots->plot);
        double m = plot_get_min_x(c->plots->plot);
        for (i = 1; i < c->n_plots; i++)
        {
            plot * p = chart_get_plot(c, i);
            double v = plot_get_max_x(p);
            if (v > M)
                M = v;
            v = plot_get_min_x(p);
            if (v < m)
                m = v;
        }
        m -= (M-m)*0.05;
        return m;
    }
    return c->x_axis.range_min;
}

double chart_get_max_y(chart *c)
{
    if (c->y_axis.autoscale == TRUE){
        if (!c->n_plots)
            return 0;
        unsigned int i;
        double M = plot_get_max_y(c->plots->plot);
        double m = plot_get_min_y(c->plots->plot);
        for (i = 1; i < c->n_plots; i++)
        {
            plot * p = chart_get_plot(c, i);
            double v = plot_get_max_y(p);
            if (v > M)
                M = v;
            v = plot_get_min_y(p);
            if (v < m)
                m = v;
        }
        M += (M-m)*0.05;
        return M;
    }
    return c->y_axis.range_max;
}

double chart_get_min_y(chart *c)
{
    if (c->y_axis.autoscale == TRUE){
        if (!c->n_plots)
            return 0;
        unsigned int i;
        double M = plot_get_max_y(c->plots->plot);
        double m = plot_get_min_y(c->plots->plot);
        for (i = 1; i < c->n_plots; i++)
        {
            plot * p = chart_get_plot(c, i);
            double v = plot_get_max_y(p);
            if (v > M)
                M = v;
            v = plot_get_min_y(p);
            if (v < m)
                m = v;
        }
        m -= (M-m)*0.05;
        return m;
    }
    return c->y_axis.range_min;
}


cbool 
is_empty(plotList *e)
{
    if (e->plot == NULL)
    {
        return TRUE;
    }
    return FALSE;
}

plotList * 
plot_new_element(plot *p)
{
    plotList * l = malloc(sizeof(plotList));
    l->next = NULL;
    l->plot = p;
    return l;
}

void 
plot_append(plotList * list, plot* plot)
{
    plotList * last = plot_get_last_element(list);
    if (is_empty(last))
    {
        last->plot = plot;
    } else
    {
        last->next = plot_new_element(plot);
    }
}


plotList * 
plot_get_last_element(plotList* list)
{
    plotList * next = list;        
    while(next->next != NULL)
    {
        next = next->next;        
    }
    return next;
}


plot* 
plot_at(plotList* l, unsigned int i)
{
    unsigned int c = 0;
    plotList * next = l;
    while(i>c && next->next != NULL)
    {
        next = next->next;
        c ++;
    }
    if (c == i)
    {
        return next->plot;
    }
    return NULL;
}

unsigned int 
chart_add_plot(chart* c, plot* p)
{
    plot_append(c->plots, p);
    return ++c->n_plots;
}

plot* 
chart_get_plot(chart* c, unsigned int i)
{
    return plot_at(c->plots, i);
}


void chart_free(chart *c)
{
    if (c->title)
        free(c->title);
    if (c->x_axis.label)
        free(c->x_axis.label);
    if (c->y_axis.label)
        free(c->y_axis.label);
    plot_list_free(c->plots);
    free(c);
}


void plot_list_free(plotList *pl)
{
    if (!pl)
        return;
    plot_free(pl->plot);
    plot_list_free(pl->next);
    free(pl);
}

void plot_free(plot *p)
{
    if (!p)
        return;
    if (p->color)
        free(p->color);
    if (p->label)
        free(p->label);
    if (p->x_data)
        free(p->x_data);
    if (p->y_data)
        free(p->y_data);
    free(p);
}