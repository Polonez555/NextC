#include "nextc.h"
#include <stdlib.h>
#include <regex.h>


static struct NextcRoute* route_registry = 0;
static int registered_routes = 0;
static void(*fourofour_handler)(struct HttpContext*) = 0;

void nc_RegisterRoute(const char* path, void(*handler)(struct HttpContext*))
{
    route_registry = realloc(route_registry,sizeof(struct NextcRoute)*(registered_routes+1));
    route_registry[registered_routes].path = (char*)path;
    route_registry[registered_routes].handler = handler;
    regcomp(&(route_registry[registered_routes].rgx),path,0);
    registered_routes++;
}

void nc_Set404Handler(void(*handler)(struct HttpContext*))
{
    fourofour_handler = handler;
}



int nc_FindRoute(const char* requested_path)
{
    int i = 0;
    for(i = 0; i < registered_routes; i++)
    {
        if(regexec( &(route_registry[i].rgx), requested_path, 0, NULL, 0) == 0)
        {
            return i;
        }
    }
    return -1;
}

void nc_ExecRoute(int route, struct HttpContext* ctx)
{
    if(route == -1)
    {
        if(fourofour_handler == 0)return;
        fourofour_handler(ctx);
        return;
    }
    route_registry[route].handler(ctx);
}
