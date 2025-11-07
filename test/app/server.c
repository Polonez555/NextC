#include "http.h"
#include "websocket.h"
#include "nextc.h"
#include "autoprocess_client.h"
#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>

void header_handler(void* _, char* name, char* value)
{
    printf("'%s' = '%s'\n",name,value);
}

void on_ws_message(struct HttpContext* hc, struct WebSocketContext* ctx, struct WebSocketReceivedData wrc)
{
    ws_transmit(hc,wrc.data,-1,ws_TEXT);
}

void ws(struct HttpContext* ctx)
{
    printf("Wsocket\n");
    struct WebSocketContext ws_ctx;
    ws_create(&ws_ctx,on_ws_message,0);
    ws_reply(&ws_ctx, ctx);
    ws_transmit(ctx,"hello1",-1, ws_TEXT);
    ws_transmit(ctx,"hello2",-1, ws_TEXT);
    ws_transmit(ctx,"hello3",-1, ws_TEXT);
    ws_loop(ctx,&ws_ctx);
    http_ctx_close(ctx);
}

void test_page(struct HttpContext* ctx)
{
    MAKE_NEXTC;
    http_ctx_push_header(ctx, "HTTP/1.1 200 OK");
    http_ctx_push_header(ctx, "Content-Type: text/html");
    NEXTC_OPEN("body");
        NEXTC_ATTRIB("style","background-color:blue;");
        NEXTC_OPEN("h1");
            NEXTC_SETINNERTEXT("Testing");
        NEXTC_CLOSE;
    NEXTC_CLOSE;
    
    FINALIZE_NEXTC(ctx);
}

void server_setup(void)
{
    nc_RegisterRoute("/ws",ws);
    nc_RegisterRoute("/",test_page);
    printf("Server setup ok\n");
}

PROCESS_CLIENT_DEF(header_handler);