#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#define ws_CONT 0
#define ws_TEXT 1
#define ws_BIN 2

#include "http.h"

struct WebSocketReceivedData
{
    char opcode;
    int length;
    char* data;
};

struct WebSocketContext
{
    void* passed_misc;
    void (*message_handler)(struct HttpContext*, struct WebSocketContext*, struct WebSocketReceivedData);
};

void ws_create(struct WebSocketContext* wsc, void (*message_handler)(struct HttpContext*, struct WebSocketContext*, struct WebSocketReceivedData), void* misc);
void ws_reply(struct WebSocketContext* wsc, struct HttpContext* hc);
void ws_transmit(struct HttpContext* hc, char* data, int length, char opcode);
int ws_receive(struct HttpContext* hc, struct WebSocketReceivedData* wc);
void ws_dispose_received(struct WebSocketReceivedData* wc);
void ws_loop(struct HttpContext* hc, struct WebSocketContext* wc);

#endif
