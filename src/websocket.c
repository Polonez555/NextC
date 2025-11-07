#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include "crypto.h"
#include "http.h"
#include "websocket.h"


void ws_create(struct WebSocketContext* wsc, void (*message_handler)(struct HttpContext*, struct WebSocketContext*, struct WebSocketReceivedData), void* misc)
{
    wsc->passed_misc = misc;
    wsc->message_handler = message_handler;
}

void flush(int _)
{
}

size_t base64_encode(const unsigned char *data, size_t len, char *out)
{
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i, j;
    unsigned int val;

    j = 0;
    for (i = 0; i + 2 < len; i += 3) {
        val = ((unsigned int)data[i] << 16) |
              ((unsigned int)data[i + 1] << 8) |
              ((unsigned int)data[i + 2]);
        out[j++] = table[(val >> 18) & 0x3F];
        out[j++] = table[(val >> 12) & 0x3F];
        out[j++] = table[(val >> 6) & 0x3F];
        out[j++] = table[val & 0x3F];
    }

    /* handle padding */
    if (i + 1 < len) {
        val = ((unsigned int)data[i] << 8) | data[i + 1];
        out[j++] = table[(val >> 10) & 0x3F];
        out[j++] = table[(val >> 4) & 0x3F];
        out[j++] = table[(val << 2) & 0x3F];
        out[j++] = '=';
    } else if (i < len) {
        val = data[i];
        out[j++] = table[(val >> 2) & 0x3F];
        out[j++] = table[(val << 4) & 0x3F];
        out[j++] = '=';
        out[j++] = '=';
    }

    out[j] = '\0';
    return j;
}

void ws_reply(struct WebSocketContext* _, struct HttpContext* hc)
{
    char* inkey;
    char catkey[70];
    char acceptor[100];
    unsigned char hash[SHA1_DIGEST_LENGTH];
    http_ctx_push_header(hc,"HTTP/1.1 101 Switching Protocols");
    http_ctx_push_header(hc,"Upgrade: websocket");
    http_ctx_push_header(hc,"Connection: Upgrade");
    inkey = http_find_iheader(hc,"Sec-WebSocket-Key");
    snprintf(catkey,70,"%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11",inkey);
    SHA1((const unsigned char*)catkey, strlen(catkey), hash);
    base64_encode(hash,SHA1_DIGEST_LENGTH,catkey);
    snprintf(acceptor,100,"Sec-WebSocket-Accept: %s",catkey);
    http_ctx_push_header(hc,acceptor);
    http_ctx_send(hc);
}

void ws_transmit(struct HttpContext* hc, char* data, int length, char opcode)
{
    int hdr_len = 2;
    char mask = 0;
    unsigned short base_hdr;
    unsigned short shr;
    unsigned long lng;
    unsigned char base_pl;
    char* frame;
    int i;
    FILE* rand_source;

    if(length == -1)length = strlen(data);
    if(length > 125)
    {
        if(length > 65535)
        {
            base_pl = 127;
            hdr_len+=8;
        }else{
            base_pl = 126;
            hdr_len+=2;
        }
    }else{
        base_pl = length;
    }
    frame = (char*)malloc(hdr_len+length+4);

    base_hdr = (
        (
            (
                1 << 3 /*FIN*/
            ) << 4 |
            (opcode & 0xF)
        ) |
        ((
            mask << 7 | /*MASK*/
            base_pl
        ) << 8)
    );

    memcpy(frame, &base_hdr, 2);
    if(length > 125)
    {
        if(length > 65535)
        {
            lng = length;
            memcpy(frame+2,&lng,8);
        }else{
            shr = length;
            memcpy(frame+2,&shr,2);
        }
    }

    if(mask)
    {
        rand_source = fopen("/dev/urandom","rb");
        fread(frame+hdr_len,1,4,rand_source);
        fclose(rand_source);
        hdr_len += 4;
    }

    memcpy(frame+hdr_len,data,length);

    if(mask)
    for(i = 0;i < length;i++)
    {
        frame[hdr_len+i] ^= frame[hdr_len-4+(i%4)];
    }

    http_ctx_emit_raw(hc,frame,hdr_len+length);
}

void ws_loop(struct HttpContext* hc, struct WebSocketContext* wc)
{
    
    struct pollfd poll_fd;
    struct WebSocketReceivedData wrc;
    poll_fd.fd = hc->socket;
    poll_fd.events = POLLIN;
    flush(hc->socket);
    while(1)
    {
        poll(&poll_fd,1,-1);
        if(poll_fd.revents & POLLIN)
        {
            if(ws_receive(hc, &wrc) == 0)return;
            wc->message_handler(hc,wc,wrc);
            ws_dispose_received(&wrc);
        }
    }
}

int ws_receive(struct HttpContext* hc, struct WebSocketReceivedData* wc)
{
    unsigned short raw_header;
    unsigned long length = 0;
    char mask_data[4];
    unsigned int i = 0;

    if(read(hc->socket,&raw_header,2) < 2)return 0;

    if(((raw_header>>8)&0x7F) == 126)
    {
        read(hc->socket,&length,2);
    }else if(((raw_header>>8)&0x7F) == 127)
    {
        read(hc->socket,&length,8);
    }else{
        length = (raw_header>>8)&0x7F;
    }

    if(length > 1000000)return 0;

    if(raw_header>>15)
    {
        read(hc->socket,mask_data,4);
    }

    wc->data = malloc(length+1);
    read(hc->socket,wc->data,length);
    if(raw_header>>15)
    {
        for(i = 0;i < length;i++)
        {
            wc->data[i] ^= mask_data[i%4];
        }
    }
    wc->data[length] = 0;
    flush(hc->socket);
    return 1;
}

void ws_dispose_received(struct WebSocketReceivedData* wc)
{
    free(wc->data);
}
