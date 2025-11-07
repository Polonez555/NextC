#ifndef WEBSOCKET_H
#define WEBSOCKET_H

struct WebSocketContext
{
    void* passed_misc;
    void (*packaged_header_handler)(void*,char*,char*);
    char detected_websocket;
    char hash_accept[2048];
};

void ws_create(struct WebSocketContext* wsc, void (*header_handler)(void*,char*,char*), void* misc);
void ws_header_handler(void* c, char* name, char* value);

#endif
