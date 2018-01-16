#include "parser.h"

#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "csv_parser/csvparser.h"

#define TOK_AXIS_X      "x-axis"    /*done*/
#define TOK_AXIS_Y      "y-axis"    /*done*/
#define TOK_PLOT        "plot"      /*done*/
/*later*/
#define TOK_SCATTER     "scatter"   /*done*/
#define TOK_BAR         "bar"       /*done*/

#define TOK_LABEL       "label"     /*done*/

#define TOK_MODE        "mode"      /*done*/
#define TOK_MODE_LOG    "log"       /*done*/
#define TOK_MODE_LIN    "linear"    /*done*/

#define TOK_X_DATA      "x"         /*done*/
#define TOK_Y_DATA      "y"         /*done*/
/*later*/
#define TOK_COLOR       "color"     /*done*/
#define TOK_LIN_STYLE   "line-style"/*done*/
#define TOK_LIN_WIDTH   "line-width"/*done*/
#define TOK_LIN_COLOR   "line-color"/*done*/
#define TOK_BAR_WIDTH   "bar-width" /*done*/
#define TOK_ALT_BAR_WIDTH "bw"      /*done*/
#define TOK_ALT_LIN_STYLE "ls"      /*done*/
#define TOK_ALT_LIN_WIDTH "lw"      /*done*/
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
    if (l == NULL)
        return;
    d_list_free(l->prev);
    free(l);
}


void strip(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

axisMode
parse_mode(char *line)
{
    unsigned int s = line[0] == ' ' ? 1 : 0;
    unsigned int n = strlen(line) - s;
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, &line[s], n);
    if (strcmp(copy, TOK_MODE_LIN) == 0)
    {
        free(copy);
        return LINEAR;
    } else if (strcmp(copy, TOK_MODE_LOG)==0)
    {
        free(copy);
        return LOG;
    }
    free(copy);
    return LINEAR;
}

unsigned int
parse_range(char *line, double * min, double * max, unsigned int * num)
{
    if (line == NULL)
        return 0;
    unsigned int n = strlen(line);
    char * copy = malloc((n+1)*sizeof(char));
    copy[n] = 0;
    memcpy(copy, line, n);
    char * tok = copy;
    char * point;
    unsigned int i = 0;
    while((tok = strtok_r(tok, " ,", &point)) != NULL)
    {
        if (i == 0)
        {
            *min = atof(tok);
        } else if (i == 1)
        {
            *max = atof(tok);
        } else if (i == 2 && num != NULL)
        {
            *num = atoi(tok);
        }
        i ++;
        tok = NULL;
    }
    free(copy);
    return i;
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

void* parse_math(char* text)
{
    unsigned int n = strlen(text)-5;
    if (n <= 0)
    {
        return NULL;
    }
    char * expr = malloc((n+1)*sizeof(char));
    expr[n] = 0;
    memcpy(expr, &text[5], n);
    strip(expr, ' ');
    return expr;
}

char* 
get_rest(char* text, unsigned int m)
{
    unsigned int n = strlen(text) - m;
    if (n <= 0)
    {
        return NULL;
    }
    char * rest = malloc((n+1) * sizeof(char));
    rest[n] = 0;
    memcpy(rest, &text[m], n);
    return rest;
}

void* parse_data(char* line, unsigned int *l, dtype  * type)
{   

    *l = 0;
    unsigned int n = strlen(line);
    if (n == 0)
        return NULL;

    char * local = malloc((1+n)*sizeof(char));
    local[n] = 0;
    memcpy(local, line, n);

    char * tok = local;
    char * point;
    *type = ND;
    _dList * list = NULL;
    if (startsWith("csv://", local))
    {   
            *type = CSV;
            list = parse_csv(local, l);
    } else if (startsWith("math:", local))
    {   
        *type = MATH;
        char * expr = parse_math(local);
        free(local);
        return expr;
    } else if (startsWith("range:", local))
    {
        *type = RANGE;
        double min = 0;
        double max = 0;
        unsigned int num = 10;
        char * rest = get_rest(local, 6);
        double * data = NULL; 
        
        if (parse_range(rest, &min, &max, &num)  && max != min && num > 0)
        {
            *l = num;
            data = malloc(num*sizeof(double));
            double step = (max-min)/(num-1);
            unsigned int i;
            for (i = 0; i < num; i++)
            {
                data[i] = min + i * step;
            }
        }
        free(local);
        free(rest);
        return data;
    } else if (startsWith("logrange:", local))
    {
        *type = RANGE;
        double min = 0;
        double max = 0;
        unsigned int num = 10;
        char * rest = get_rest(local, 9);
        double * data = NULL; 
        
        if (parse_range(rest, &min, &max, &num)  && max != min && num > 0)
        {
            *l = num;
            data = malloc(num*sizeof(double));
            double step = (max-min)/(num-1);
            unsigned int i;
            for (i = 0; i < num; i++)
            {
                data[i] = pow(10.0, min + i * step);
            }
        }
        free(local);
        free(rest);
        return data;
    } else
    {
        *type = DATA;
    
        /* count values */
        while((tok = strtok_r(tok, "\t ,\n", &point)) != NULL)
        {
            
            _dList *el = malloc(sizeof(_dList));
            el->value = atof(tok);
            el->prev = list;
            list = el;

            *l = (unsigned int)(*l + 1);
            tok = NULL;
        }
    }

    if (*l == 0 || list == NULL)
    {
        free(local);
        return NULL;
    }

    double * data = malloc(*l*sizeof(double)); 
    int i;
    _dList *head = list;
    for (i = *l-1 ; i >= 0 ; i--)
    {
        data[i] = list->value;
        list = list->prev;
    }
    d_list_free(head);
    free(local);
    return data;
}

void  
parse_x_data(chart *chart, char* line)
{
    unsigned int l = 0 ;
    dtype t;
    void * data = parse_data(line, &l, &t);
    if (t == ND || t == MATH)
        return;
    plot * p = plot_get_last_element(chart->plots)->plot;
    if (p->n == 0)
        p->n = l;
    p->x_data = data;
}

void  
parse_y_data(chart *chart, char* line)
{
    unsigned int l = 0;
    dtype t;
    void * data = parse_data(line, &l, &t);
    plot * p = plot_get_last_element(chart->plots)->plot;
    p->y_type = t; 
    if (t == ND)
        return;
    if (t == DATA || t == CSV)
    {        
        p->n = l;
    }
    p->y_data = data;
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


plot * init_scatter()
{
    plot * p = malloc(sizeof(plot));
    p->type = SCATTER;
    p->label = NULL;
    p->n = 0;
    p->x_data = NULL;
    p->y_data = NULL;
    p->color = NULL;
    p->line_style = NOLINE;
    p->marker_style = 'o';
    p->line_width = 0;
    p->extra_data = NULL;
    return p;   
}

plot * init_bar()
{
    plot * p = malloc(sizeof(plot));
    p->type = BAR;
    p->label = NULL;
    p->n = 0;
    p->x_data = NULL;
    p->y_data = NULL;
    p->color = NULL;
    p->line_style = NORMAL;
    p->marker_style = 0;
    p->line_width = 2;
    barPref * pref = malloc(sizeof(barPref));
    char * color = malloc(8*sizeof(char));
    color[7] = 0;
    memcpy(color, "#000000", 7);

    pref->bar_width = 0.8;
    pref->line_color = color; 
    p->extra_data = pref;
    return p; 
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
    p->line_width = 2;
    p->extra_data = NULL;
    return p;
}

lineStyle 
parse_line_style(char * c)
{
    strip(c, ' ');
    if (c == NULL || strlen(c) == 0)
        return NORMAL;
    
    if (strcmp(c, "-") == 0 || strcmp(c, "normal") ==0)
    {
        return NORMAL;
    }
    if (strcmp(c, ":") == 0 || strcmp(c, "dotted") == 0)
    {
        return DOTTED;
    }
    if (strcmp(c, "--") == 0 || strcmp(c, "dashed") == 0)
    {
        return DASHED;
    }
    if (strcmp(c, "/")  == 0 || strcmp(c, "none") == 0 || strcmp(c, "NONE") == 0 || strcmp(c, "None"))
    {
        return NOLINE;
    }
    return NORMAL;
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
                if (el->plot)
                    el->plot->label = label;
            }
            break;
        } else if (strcmp(tok, TOK_RANGE) == 0)
        {
            double min=0, max=0;
            parse_range(rest, &min, &max, NULL);
            if (min != max && max > min){
                if (prev == AXIS_X)
                {
                    chart->x_axis.autoscale = FALSE;
                    chart->x_axis.range_min = min;
                    chart->x_axis.range_max = max;
                } else if (prev == AXIS_Y)
                {
                    chart->y_axis.autoscale = FALSE;
                    chart->y_axis.range_min = min;
                    chart->y_axis.range_max = max;
                }
            }
            break;
        } else if (strcmp(tok, TOK_PLOT) == 0)
        {
            prev = PLOT;
            chart_add_plot(chart, init_plot());
        } else if (strcmp(tok, TOK_SCATTER) == 0)
        {
            prev = PLOT;
            chart_add_plot(chart, init_scatter());
        } else if (strcmp(tok, TOK_BAR) == 0 )
        {
            prev = PLOT;
            chart_add_plot(chart, init_bar());
        } else if (strcmp(tok, TOK_MODE) == 0)
        {
            if (prev == AXIS_X) 
            {
                chart->x_axis.mode = parse_mode(rest);
            } else if (prev == AXIS_Y)
            {
                chart->y_axis.mode = parse_mode(rest); 
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
            free(ms);
            break;
        }else if (((strcmp(tok, TOK_LIN_WIDTH) == 0) || 
                  (strcmp(tok, TOK_ALT_LIN_WIDTH) == 0)) && 
                  prev == PLOT)
        {
            strip(rest, ' ');
            double v = atof(rest);
            if (v >= 0)
            {
                plot * p = plot_get_last_element(chart->plots)->plot;
                p->line_width = v;
            }
            break;
        } else if (((strcmp(tok, TOK_LIN_STYLE) == 0) || 
                   (strcmp(tok, TOK_ALT_LIN_STYLE) == 0)) && 
                   prev == PLOT)
        {
            plot * p = plot_get_last_element(chart->plots)->plot;
            p->line_style = parse_line_style(rest);
            break;
        }  else if (((strcmp(tok, TOK_BAR_WIDTH) == 0) || 
                   (strcmp(tok, TOK_ALT_BAR_WIDTH) == 0)) && 
                   prev == PLOT)
        {
            strip(rest, ' ');
            double v = atof(rest);
            if (v > 0)
            {
                plot * p = plot_get_last_element(chart->plots)->plot;
                if (p->type == BAR && p->extra_data)
                {
                    barPref* pref = p->extra_data;
                    pref->bar_width = v;
                }    
            }
            break;
        } else if ((strcmp(tok, TOK_LIN_COLOR) == 0) && prev == PLOT)
        {
            plot * p = plot_get_last_element(chart->plots)->plot;
            if (p->type == BAR && p->extra_data)
            {
                barPref* pref = p->extra_data;
                free(pref->line_color);
                pref->line_color = parse_text(rest);
            }
            break;
        }
        tok = NULL;
        break;
    }
    free(copy);
    return prev;
}

chart * 
parse_chart(char *text)
{
    chart* nchart = initialize_empty_chart();
    
    char * copy = text;
    char* tok_pointer;
    char* line = strtok_r(copy, "\n", &tok_pointer);
    _pstate state = NONE;
    while (line != NULL)
    {   
        if (strlen(line) > 0)
            state = parse_line(line, nchart, state);
        line = strtok_r(NULL, "\n", &tok_pointer);
    }
    return nchart;
}
