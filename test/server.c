#include "http.h"
#include "websocket.h"
#include "nextc.h"
#include <stdio.h>
#include <stdlib.h>

void header_handler(void* _, char* name, char* value)
{
    printf("'%s' = '%s'\n",name,value);
}

void process_client(struct HttpContext* ctx)
{
    char method[10];
    char path[256];
    struct WebSocketContext wsc;
    struct NextcContext* nct;
    int x = 0;
    int y = 0;
    int gs = 0;

    ws_create(&wsc, header_handler, ctx);
    if(http_parse_get(ctx, path, method, &wsc, ws_header_handler))
    {
        printf("Close\n");
        http_ctx_close(ctx);
        return;
    }
    printf("Fetched %s\n",path);
    printf("Is websocket: %d\n",wsc.detected_websocket);
    http_ctx_push_header(ctx, "HTTP/1.1 200 OK");
    http_ctx_push_header(ctx, "Content-Type: text/html");

    gs = atoi(path+1);

    nct = nc_Setup();

    /*HTML*/
    nc_ElementOpen(nct,"body");
        nc_AppendAttribute(nct, "style", "display: flex;");
        nc_ElementOpen(nct, "div");
            nc_AppendAttribute(nct, "style", "display: flex;");
            nc_AppendAttribute(nct, "style", "flex-direction: column;");
            nc_AppendAttribute(nct, "style", "justify-content: space-around; flex: 1;");
            nc_ElementOpen(nct, "div");
                nc_AppendAttribute(nct, "style", "display: flex;");
                nc_AppendAttribute(nct, "style", "flex-direction: row;");
                nc_AppendAttribute(nct, "style", "justify-content: center;");
                for(y = 0; y < gs; y++)
                {
                    nc_ElementOpen(nct, "div");
                        nc_AppendAttribute(nct, "style", "display: flex;");
                        nc_AppendAttribute(nct, "style", "flex-direction: column;");
                        nc_AppendAttribute(nct, "style", "justify-content: center;");
                        for(x = 0;x < gs;x++)
                        {
                            nc_ElementOpen(nct, "div");
                                nc_AppendAttribute(nct, "style", "display: flex; width:50px; height:50px;");
                                if((x+y)%2 == 0)
                                {
                                    nc_AppendAttribute(nct,"style", "background-color:black;");
                                }else{
                                    nc_AppendAttribute(nct,"style", "background-color:transparent;");
                                }
                            nc_ElementClose(nct);
                        }
                    nc_ElementClose(nct);
                }
            nc_ElementClose(nct);
        nc_ElementClose(nct);
    nc_ElementClose(nct);
    /*END HTML*/

    nc_RenderHttp(nct,ctx);
    nc_Dispose(nct);

    http_ctx_send(ctx);
    http_ctx_close(ctx);
}
