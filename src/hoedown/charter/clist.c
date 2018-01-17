#include "clist.h"

#include <stdlib.h>

cbool
clist_is_empty(clist* list)
{
    return (list != NULL && list->data == NULL);
}


clist* 
clist_new(void* data)
{
    clist * head = malloc(sizeof(clist));
    head->prev = NULL;
    head->next = NULL;
    head->data = data;

    return head;
}

clist*
clist_get_last(clist* list)
{
    if (list == NULL)
    {
        return NULL;
    }

    clist * el = list;
    while (el->next != NULL)
    {
        el = el->next; 
    }
    return el;
}

clist* 
clist_get_first(clist* list)
{
    if (list == NULL)
    {
        return NULL;
    }
    clist * el = list;
    while (el->prev != NULL)
    {
        el = el->prev; 
    }
    return el;
}

clist* 
clist_append(clist* list, 
             void*  data)
{
    clist* el = clist_get_last(list);
    if (clist_is_empty(el))
    {
        el->data = data;
        return el;
    }

    clist *new_el = malloc(sizeof(clist));
    new_el->data = data;
    new_el->next = NULL;

    new_el->prev = el;
    if (el)
    {
        el->next = new_el;
    }
    return new_el;
}

clist* 
clist_prepend(clist*    list, 
              void*     data)
{
    clist* el = clist_get_first(list);
    if (clist_is_empty(el))
    {
        el->data = data;
        return el;
    }

    clist *new_el = malloc(sizeof(clist));
    new_el->data = data;
    new_el->prev = NULL;

    new_el->next = el;
    if (el)
    {
        el->prev = new_el;
    }
    return new_el;
}


unsigned int 
clist_size(clist* list)
{
    clist * el = clist_get_first(list);
    if (el == NULL)
        return 0;
    
    unsigned int l = 1;
    while (el->next != NULL)
    {
        el = el->next;
        l ++;
    }

    return l;
}

clist* 
clist_at(clist*         list, 
         unsigned int   i)
{
    clist * el = clist_get_first(list);
    if (el == NULL)
        return NULL;
    
    unsigned int l = 0;
    while (el->next != NULL && l < i)
    {
        el = el->next;
        l ++;
    }
    return el;
}

void*  
clist_data_at(clist*        list, 
              unsigned int  i)
{
    clist * el = clist_at(list, i);
    if (el)
        return el->data;
    return NULL;
}

void* 
clist_set_data_at(clist*        list, 
                  unsigned int  i, 
                  void *        data)
{
    clist *el = clist_at(list, i);
    void *old = el->data;
    el->data = data;
    return old;
}


void 
clist_free(clist* list)
{
    if (list == NULL)
        return;
    if (list->next != NULL)
        clist_free(list->next);
    free(list->data);
    free(list);
}