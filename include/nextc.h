#ifndef NEXTC_H
#define NEXTC_H
#include <regex.h>
#include "http.h"

struct NextcContext
{
    struct NextcElementChainlink* selected_chainlink;
    struct NextcElementChainlink* root_element;
};

struct NextcElementChainlink
{
    struct NextcElementChainlink* next_flat;
    struct NextcElementChainlink* next_inner;
    struct NextcElementChainlink* before;
    struct NextcAttribute* attribs;
    char* inner_text;
    char* type;
};

struct NextcAttribute
{
    char* name;
    char* value;
    int value_len;
    struct NextcAttribute* next;
};

struct NextcRoute
{
    char* path;
    regex_t rgx;
    void(*handler)(struct HttpContext*);
};

struct NextcContext* nc_Setup(void);
void nc_ElementOpen(struct NextcContext* nc,char* type);
void nc_ElementClose(struct NextcContext* nc);
void nc_Dispose(struct NextcContext* nc);
void nc_SetInnerText(struct NextcContext* nc, char* text);
void nc_RenderHttp(struct NextcContext* nc, struct HttpContext* hc);
void nc_AppendAttribute(struct NextcContext* nc,char*name,char* data);
void nc_RegisterRoute(const char* path, void(*handler)(struct HttpContext*));
int nc_FindRoute(const char* requested_path);
void nc_ExecRoute(int route, struct HttpContext* ctx);
void nc_Set404Handler(void(*handler)(struct HttpContext*));

#define MAKE_NEXTC struct NextcContext* __int__nct = nc_Setup()
#define FINALIZE_NEXTC(ctx) nc_RenderHttp(__int__nct,ctx); nc_Dispose(__int__nct); http_ctx_send(ctx); http_ctx_close(ctx);
#define NEXTC_OPEN(tag) nc_ElementOpen(__int__nct,tag)
#define NEXTC_CLOSE nc_ElementClose(__int__nct)
#define NEXTC_ATTRIB(name, value) nc_AppendAttribute(__int__nct,name,value)
#define NEXTC_SETINNERTEXT(text) nc_SetInnerText(__int__nct,text)
#define NEXTC_INSTANCE __int__nct

#endif
