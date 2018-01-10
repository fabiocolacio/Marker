#include "parser.h"

#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "csv_parser/csvparser.h"

#define TOK_AXIS_X      "x-axis"    /*done*/
#define TOK_AXIS_Y      "y-axis"    /*done*/
#define TOK_PLOT        "plot"      /*done*/
/*later*/
#define TOK_SCATTER     "scatter" 
#define TOK_BAR         "bar"

#define TOK_LABEL       "label"     /*done*/

#define TOK_MODE        "mode"      /*done*/
#define TOK_MODE_LOG    "log"       /*done*/
#define TOK_MODE_LIN    "linear"    /*done*/

#define TOK_X_DATA      "x"         /*done*/
#define TOK_Y_DATA      "y"         /*done*/
/*later*/
#define TOK_COLOR       "color"     /*done*/
#define TOK_LIN_STYLE   "line-style"
#define TOK_LIN_WIDTH   "line-width"
#define TOK_MARKER      "marker"    /*done*/

#define TOK_RANGE       "range"     /*done*/

#define TOK_WIDTH       "width"     /*done*/
#define TOK_HEIGHT      "height"    /*done*/
#define TOK_TITLE       "title"     /*done*/


struct{
    double value;
    void* prev;
} typedef _dList;

void d_list_free(_dList * l)
{
    if (!l)
        return;
    d_list_free(l->prev);
    free(l);
}


void 
parse_mode(char *line, chart *chart, _pstate prev)
{
    unsigned int s = line[0] == ' ' ? 1 : 0;
    unsigned int n = strlen(line) - s;
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, &line[s], n);
    if (strcmp(copy, TOK_MODE_LIN) == 0)
    {
        if (prev == AXIS_X) 
        {
            chart->x_axis.mode = LINEAR;
        } else
        {
            chart->y_axis.mode = LINEAR;
        }
    } else if (strcmp(copy, TOK_MODE_LOG)==0)
    {
        if (prev == AXIS_X) 
        {
            chart->x_axis.mode = LOG;
        } else
        {
            chart->y_axis.mode = LOG;
        }
    }
    free(copy);
}

void 
parse_range(char *line, chart *chart, _pstate prev)
{
    unsigned int n = strlen(line);
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, line, n);
    char * tok = copy;
    char * point;
    unsigned int i = 0;
    while((tok = strtok_r(tok, " ,", &point)) != NULL)
    {
        double v = atof(tok);
        if (i == 0)
        {
            if (prev == AXIS_X)
            {
                chart->x_axis.autoscale = FALSE;
                chart->x_axis.range_min = v;
            } else
            {
                chart->y_axis.autoscale = FALSE;
                chart->y_axis.range_min = v;
            }
        } else if (i == 1)
        {
            if (prev == AXIS_X)
            {
                chart->x_axis.range_max = v;
            } else
            {
                chart->y_axis.range_max = v;
            }
        }
        i ++;
        tok = NULL;
    }
    free(copy);
}

cbool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? FALSE : strncmp(pre, str, lenpre) == 0;
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

_dList* parse_csv(char * link, unsigned int * size)
{
    unsigned int n = strlen(link)-6;
    if (n <= 0)
    {
        return NULL;
    }
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, &link[6], n);
    char *  tag;
    char * url = strtok_r(copy, "#", &tag);
    if (!tag || !is_regular_file(url)){
        return NULL;
    }
    CsvParser *csvparser = CsvParser_new(url, ",", 1);
    CsvRow *header;
    CsvRow *row;
    header = CsvParser_getHeader(csvparser);
    if (header == NULL)
    {
        return NULL;
    }

    char **headerFields = CsvParser_getFields(header);
    int i;
    int id = -1;
    for (i = 0 ; i < CsvParser_getNumFields(header) ; i++) {
        if (strcmp(headerFields[i], tag) == 0){
            id  = i;
        }
    }
    if (id < 0)
    {
        return NULL;
    }
    _dList * list = NULL;
    while ((row = CsvParser_getRow(csvparser)) ) {
        char **rowFields = CsvParser_getFields(row);
        double val = atof(rowFields[id]);
        _dList *el = malloc(sizeof(_dList));
        el->value = val;
        el->prev = list;
        list = el;
        *size = *size + 1;
        CsvParser_destroy_row(row);
    }
    CsvParser_destroy(csvparser);
    return list;
}


void  
parse_x_data(chart *chart, char* line)
{
    unsigned int n = strlen(line);
    char * local = malloc((1+n)*sizeof(char));
    local[n] = 0;
    memcpy(local, line, n);
    char * tok = local;
    char * point;
    
    unsigned int l = 0;
    _dList * list = NULL;
    /* count values */
    while((tok = strtok_r(tok, " ,)\n", &point)) != NULL)
    {
        if (startsWith("csv://", tok))
        {   
            list = parse_csv(tok, &l);
            break;
        }
        _dList *el = malloc(sizeof(_dList));
        el->value = atof(tok);
        el->prev = list;
        list = el;

        l ++;
        tok = NULL;
    }
    
    
    double * data = malloc(l*sizeof(double)); 
    int i;
    for (i = l-1 ; i >= 0 ; i--)
    {
        data[i] = list->value;
        list = list->prev;
    }
    plot * p = plot_get_last_element(chart->plots)->plot;
    if (p->n == 0)
        p->n = l;
    p->x_data = data;
    d_list_free(list);
    free(local);
}

void  
parse_y_data(chart *chart, char* line)
{
    unsigned int n = strlen(line);
    char * copy = malloc((1+n)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, line, n);
    char * tok = copy;
    char * point;
    
    unsigned int l = 0;
    _dList * list = NULL;
    /* count values */
    while((tok = strtok_r(tok, " ,", &point)) != NULL)
    {
        if (startsWith("csv://", tok))
        {   
            list = parse_csv(tok, &l);
            break;
        }
        _dList *el = malloc(sizeof(_dList));
        el->value = atof(tok);
        el->prev = list;

        list = el;
        l ++;
        tok = NULL;
    }
    
    double * data = malloc(l*sizeof(double)); 
    int i;
    for (i = l-1 ; i >= 0 ; i--)
    {
        data[i] = list->value;
        list = list->prev;
    }
    plot * p = plot_get_last_element(chart->plots)->plot;
    p->n = l;
    p->y_data = data;
    
    d_list_free(list);
    free(copy);
}



char * parse_text(char * rest)
{
    unsigned int i = rest[0] == ' '? 1 : 0;
    unsigned int n = strlen(rest) - i;
    if (n<=0)
    {
        char * copy = malloc(2*sizeof(char));
        copy[0] = ' ';
        copy[1] = 0;
        return copy;
    }
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, &rest[i], n);
    return copy;
}

void strip(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

plot * init_plot()
{
    plot * p = malloc(sizeof(plot));
    p->type = LINE;
    p->label = NULL;
    p->n = 0;
    p->x_data = NULL;
    p->y_data = NULL;
    p->color = NULL;
    p->line_style = NORMAL;
    p->marker_style = 0;
    p->line_width = 1;
    return p;
}

_pstate 
parse_line(char* line, chart * chart, _pstate prev)
{
    unsigned int n = strlen(line);
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, line, n);
    char * tok = copy;
    char * rest;
    while((tok = strtok_r(tok, "\t :", &rest)) != NULL)
    {
        if (strcmp(tok, TOK_AXIS_X) == 0)
        {
            prev = AXIS_X;
        } else if (strcmp(tok, TOK_AXIS_Y) ==0)
        {
            prev = AXIS_Y;
        } else if (strcmp(tok, TOK_LABEL) == 0)
        {
            char * label = parse_text(rest);
            if (prev == AXIS_X)
            {   
                chart->x_axis.label = label;
            }
            else if (prev == AXIS_Y)
            {
                chart->y_axis.label = label;
            }
            else if (prev == PLOT)
            {
                plotList * el = plot_get_last_element(chart->plots);
                if (el->plot == NULL){
                    el->plot = init_plot();
                    chart->n_plots ++;
                }
                el->plot->label = label;
            }
            break;
        } else if (strcmp(tok, TOK_RANGE) == 0)
        {
            if (prev == AXIS_X || prev == AXIS_Y)
            {
                parse_range(rest, chart, prev);
            }
            break;
        } else if (strcmp(tok, TOK_PLOT) == 0)
        {
            prev = PLOT;
            chart_add_plot(chart, init_plot());
        } else if (strcmp(tok, TOK_MODE) == 0)
        {
            if (prev == AXIS_X || prev == AXIS_Y)
            {
                parse_mode(rest, chart, prev);
            }
            break;
        } else if (strcmp(tok, TOK_X_DATA) == 0 && prev == PLOT)
        {
            parse_x_data(chart, rest);
            break;
        } else if (strcmp(tok, TOK_Y_DATA) == 0 && prev == PLOT)
        {
            parse_y_data(chart, rest);
            break;
        } else if (strcmp(tok, TOK_WIDTH) == 0)
        {
            strip(rest, ' ');
            chart->width = atof(rest);
            prev = NONE;
            break;
        } else if (strcmp(tok, TOK_HEIGHT) == 0)
        {
            strip(rest, ' ');
            chart->height = atof(rest);
            prev = NONE;
            break;
        } else if (strcmp(tok, TOK_TITLE) == 0)
        {
            chart->title = parse_text(rest);
            prev = NONE;
            break;
        } else if (strcmp(tok, TOK_COLOR) == 0 && prev == PLOT)
        {
            char * color = parse_text(rest);
            strip(color, ' ');
            plot_get_last_element(chart->plots)->plot->color = color;
            break;
        }else if (strcmp(tok, TOK_MARKER) == 0 && prev == PLOT)
        {
            char * ms = parse_text(rest);
            strip(ms, ' ');
            plot * p = plot_get_last_element(chart->plots)->plot;
            if (ms != NULL && strlen(ms) > 0)
            {
                p->marker_style = ms[0];
            }else {
                p->marker_style = 0;
            }
            break;
        }
        tok = NULL;
    }
    free(copy);
    return prev;
}

chart * 
parse_chart(char *text)
{
    chart* nchart = initialize_empty_chart();
    unsigned int n = strlen(text);
    char * copy = malloc(n*sizeof(char));
    memccpy(copy, text, 0, n);
    char* tok_pointer;
    char* line = strtok_r(copy, "\n", &tok_pointer);
    _pstate state = NONE;
    while (line != NULL)
    {
        state = parse_line(line, nchart, state);
        line = strtok_r(NULL, "\n", &tok_pointer);
    }
    free(copy);
    return nchart;
}
