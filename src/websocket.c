#include <string.h>
#include <stdio.h>
#include "http.h"
#include "websocket.h"

void ws_header_handler(void* c, char* name, char* value)
{
    struct WebSocketContext* wsc = (struct WebSocketContext*)c;
    if(strcmp(name, "Upgrade") == 0 && strcmp(value, "websocket") == 0)
    {
        wsc->detected_websocket = 1;
    }
    if(strcmp(name,"Sec-WebSocket-Key") == 0)
    {
        snprintf(wsc->hash_accept,2048,"%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11",value);
    }
    wsc->packaged_header_handler(wsc->passed_misc,name,value);
}

void ws_create(struct WebSocketContext* wsc, void (*header_handler)(void*,char*,char*), void* misc)
{
    wsc->passed_misc = misc;
    wsc->packaged_header_handler = header_handler;
    wsc->detected_websocket = 0;
}

void ws_reply(struct WebSocketContext* wsc, struct HttpContext* hc)
{
    if(wsc->detected_websocket)
    {
        http_ctx_push_header(hc,"HTTP/1.1 101 Switching Protocols");
        http_ctx_push_header(hc,"Upgrade: websocket");
        http_ctx_push_header(hc,"Connection: Upgrade");
        http_ctx_send(hc);
    }
}
