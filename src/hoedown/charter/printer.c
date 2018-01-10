#include "printer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char* 
print_plot(plot* p)
{

    char * buffer = malloc(64*sizeof(char));
    memset(buffer, 0, 64);
    if (p)
    {
        sprintf(buffer, "label: %s, type: %d, size: %u", 
                p->label, p->type, p->n);
    }
    return buffer;
}

char* 
print_plots(plotList *plots, unsigned int n)
{
    char * buffer = malloc(128*sizeof(char));
    memset(buffer, 0, 128);
    if (n)
    {
        unsigned int i = 0;
        for (;i<n;i++){
            plot * p = plot_at(plots, i);
            char * pp = print_plot(p);
            sprintf(buffer, "%s\t%u: %s<br>\n",
                    buffer, i, pp);
            free(pp);
        }
    }
    return buffer;
}

char* 
print_chart(chart * c)
{
    char * buffer = malloc(256*sizeof(char));
    memset(buffer, 0, 256);
    char * plots = print_plots(c->plots, c->n_plots);
    sprintf(buffer, "size: (%u, %u)<br>\n"
                    "x-axis(mode: %d, autoscale: %d, range(%f, %f), label: %s)<br>\n"
                    "y-axis(mode: %d, autoscale: %d, range(%f, %f), label: %s)<br>\n"
                    "n_plots: %u<br>\n"
                    "plots: <br>\n%s", 
                    c->width, c->height,
                    c->x_axis.mode, c->x_axis.autoscale, c->x_axis.range_min, c->x_axis.range_max, c->x_axis.label,
                    c->y_axis.mode, c->y_axis.autoscale, c->y_axis.range_min, c->y_axis.range_max, c->y_axis.label,
                    c->n_plots,
                    plots);
    return buffer;
}