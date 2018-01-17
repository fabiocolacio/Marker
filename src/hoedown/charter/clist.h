#ifndef __CLIST_H__
#define __CLIST_H__

enum{
    FALSE       = 0,
    TRUE        = 1
}typedef cbool;


struct{
    void * data;
    void * next;
    void * prev;
}typedef clist;


cbool
clist_is_empty(clist* list);

clist* 
clist_new(void *data);

clist* 
clist_append(clist* list, void* data);

clist* 
clist_prepend(clist* list, void* data);

clist* 
clist_get_last(clist* list);

clist* clist_get_first(clist* list);

unsigned int clist_size(clist* list);

clist* clist_at(clist* list, unsigned int i);
void*  clist_data_at(clist* list, unsigned int i);

void*  clist_set_data_at(clist* list, unsigned int i, void * data);


void clist_free(clist* list);

#endif