#include "nextc.h"
#include "http.h"
#include <stdlib.h>
#include <string.h>

struct NextcContext* nc_Setup(void)
{
    struct NextcContext* nc = (struct NextcContext*)malloc(sizeof(struct NextcContext));
    nc->selected_chainlink = 0;
    nc->root_element = 0;
    return nc;
}

void nc_SetInnerText(struct NextcContext* nc, char* text)
{
    if(nc->selected_chainlink == 0)return;
    nc->selected_chainlink->inner_text = text; 
}

void nc_ElementOpen(struct NextcContext* nc,char* type)
{
    struct NextcElementChainlink* root_new;
    struct NextcElementChainlink* new_chainlink = (struct NextcElementChainlink*)malloc(sizeof(struct NextcElementChainlink));
    new_chainlink->type = type;
    new_chainlink->next_flat = 0;
    new_chainlink->next_inner = 0;
    new_chainlink->inner_text = 0;
    new_chainlink->attribs = 0;
    if(nc->selected_chainlink == 0)
    {
        new_chainlink->before = 0;
        new_chainlink->next_flat = 0;
        new_chainlink->next_inner = 0;
        nc->root_element = new_chainlink;
        nc->selected_chainlink = new_chainlink;
    }else{
        if(nc->selected_chainlink->next_inner == 0)
        {
            new_chainlink->before = nc->selected_chainlink;
            nc->selected_chainlink->next_inner = new_chainlink;
        }else{
            root_new = nc->selected_chainlink->next_inner;
            while(root_new->next_flat != 0)root_new = root_new->next_flat;
            new_chainlink->before = nc->selected_chainlink;
            root_new->next_flat = new_chainlink;
        }
        nc->selected_chainlink = new_chainlink;
    }
}

void nc_ElementClose(struct NextcContext* nc)
{
    if(nc->selected_chainlink != 0)nc->selected_chainlink = nc->selected_chainlink->before;
}

void _nc_RemoveLoop(struct NextcElementChainlink* nec)
{
    struct NextcElementChainlink* nexa;
    struct NextcElementChainlink* nexb;
    struct NextcAttribute* nabb;
    struct NextcAttribute* nebc;
    if(nec == 0)return;

    nabb = nec->attribs;
    while(nabb != 0)
    {
        nebc = nabb;
        nabb = nabb->next;
        free(nebc);
    }

    nexa = nec->next_inner;
    nexb = nec->next_flat;
    free(nec);
    _nc_RemoveLoop(nexa);
    _nc_RemoveLoop(nexb);
}

void nc_Dispose(struct NextcContext* nc)
{
    _nc_RemoveLoop(nc->root_element);
    free(nc);
}

void _nc_RenderHttpLoop(struct NextcElementChainlink* nc, struct HttpContext* hc)
{
    struct NextcAttribute* nabb;
    if(nc == 0)return;
    http_ctx_push_content(hc,"<",1);
    http_ctx_push_string(hc,nc->type);
    nabb = nc->attribs;
    while(nabb != 0)
    {
        http_ctx_push_string(hc," ");
        http_ctx_push_string(hc,nabb->name);
        http_ctx_push_string(hc,"=\"");
        http_ctx_push_content(hc,nabb->value,nabb->value_len);
        http_ctx_push_string(hc,"\"");
        nabb = nabb->next;
    }
    http_ctx_push_content(hc,">",1);
    if(nc->inner_text)http_ctx_push_string(hc,nc->inner_text);
    _nc_RenderHttpLoop(nc->next_inner,hc);
    http_ctx_push_content(hc,"</",2);
    http_ctx_push_string(hc,nc->type);
    http_ctx_push_content(hc,">",1);
   _nc_RenderHttpLoop(nc->next_flat,hc);
}

void nc_RenderHttp(struct NextcContext* nc, struct HttpContext* hc)
{
    _nc_RenderHttpLoop(nc->root_element, hc);
}

void nc_AppendAttribute(struct NextcContext* nc,char* name,char* data)
{
    struct NextcElementChainlink* nec = nc->selected_chainlink;
    struct NextcAttribute* found_attrib = 0;
    struct NextcAttribute* appendable_attrib = 0;
    struct NextcAttribute* looping_attrib = 0;
    int ndata_len;
    if(nec == 0)return;
    appendable_attrib = nec->attribs;
    looping_attrib = appendable_attrib;
    while(looping_attrib != 0)
    {
        if(strcmp(looping_attrib->name,name) == 0)
        {
            found_attrib = looping_attrib;
            break;
        }
        appendable_attrib = looping_attrib;
        looping_attrib = looping_attrib->next;
    }
    if(found_attrib == 0)
    {
        found_attrib = (struct NextcAttribute*)malloc(sizeof(struct NextcAttribute));
        found_attrib->name = name;
        found_attrib->value = 0;
        found_attrib->value_len = 0;
        found_attrib->next = 0;
        if(appendable_attrib == 0)
        {
            nec->attribs = found_attrib;
        }else{
            appendable_attrib->next = found_attrib;
        }
    }
    ndata_len = strlen(data);
    found_attrib->value = realloc(found_attrib->value,found_attrib->value_len+ndata_len);
    memcpy(found_attrib->value+found_attrib->value_len, data, ndata_len);
    found_attrib->value_len+=ndata_len;
}
