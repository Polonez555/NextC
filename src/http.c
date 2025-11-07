#include "http.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void http_ctx_init(struct HttpContext* ctx, int sock)
{
    ctx->socket = sock;
    ctx->content_length = 0;
    ctx->header_length = 0;
    ctx->content = 0;
    ctx->headers = 0;
}

void http_ctx_push_content(struct HttpContext* ctx, char* content, int content_length)
{
    ctx->content = realloc(ctx->content, ctx->content_length + content_length);
    memcpy(ctx->content + ctx->content_length, content, content_length);
    ctx->content_length += content_length;
}

void http_ctx_push_header(struct HttpContext* ctx, char* content)
{
    int content_length = strlen(content);
    ctx->headers = realloc(ctx->headers, ctx->header_length + content_length + 2);
    memcpy(ctx->headers + ctx->header_length, content, content_length);
    memcpy(ctx->headers + ctx->header_length + content_length, "\r\n", 2);
    ctx->header_length += content_length+2;
}

void http_serve_file(struct HttpContext* ctx, const char* path)
{
    long content_length;
    FILE* fle = fopen(path,"r");
    fseek(fle,0,SEEK_END);
    content_length = ftell(fle);
    fseek(fle,0,SEEK_SET);
    ctx->content = realloc(ctx->content, ctx->content_length + content_length);
    fread(ctx->content + ctx->content_length, 1, content_length, fle);
    ctx->content_length += content_length;
    fclose(fle);
}

void http_ctx_push_string(struct HttpContext* ctx,char* text)
{
    http_ctx_push_content(ctx,text,strlen(text));
}

void http_ctx_send(struct HttpContext* ctx)
{
    char cl[256];
    char* complete_packet;

    snprintf(cl, 256, "Content-Length: %d",ctx->content_length);
    http_ctx_push_header(ctx,cl);
    complete_packet = malloc(ctx->header_length + 2 + ctx->content_length);
    memcpy(complete_packet, ctx->headers, ctx->header_length);
    memcpy(complete_packet + ctx->header_length,"\r\n",2);
    memcpy(complete_packet + ctx->header_length + 2, ctx->content, ctx->content_length);
    write(ctx->socket, complete_packet, ctx->header_length + 2 + ctx->content_length);
    free(complete_packet);
    free(ctx->headers);
    free(ctx->content);
    ctx->content_length = 0;
    ctx->header_length = 0;
}

void http_ctx_erase(struct HttpContext* ctx)
{
    free(ctx->headers);
    free(ctx->content);
    ctx->content_length = 0;
    ctx->header_length = 0;
}

void http_ctx_close(struct HttpContext* ctx)
{
    close(ctx->socket);
}

int http_parse_get(struct HttpContext* ctx, char* path, char* method, void* misc, void (*header_handler)(void*,char*,char*))
{
    char parsing_space[2048];
    char escape_nl = 0;
    char cret = 0;
    char stage = 0;
    int prr = 0;
    char a = 0;
    int dotdot = 0;
    int i = 0;
    int q = 0;
    int len = 0;

    while(1)
    {
        read(ctx->socket, &a, 1);
        if(a == '\r')
        {
            cret = 1;
        }else if(a == '\n')
        {
            if(cret)
            {
                dotdot = 0;
                if(escape_nl)
                {
                    prr = 0;
                    escape_nl = 0;
                    stage = 0;
                    return 0;
                }
                cret = 0;
                escape_nl = 1;
                if(stage == 0)
                {
                    dotdot = 0;
                    memcpy(method,parsing_space,10);
                    for(i = 0;i < 10;i++)
                    {
                        if(method[i] == ' ')q = 1;
                        if(q)method[i] = 0;
                    }
                    while(parsing_space[dotdot] != ' ')dotdot++;
                    i = dotdot+1;
                    dotdot++;
                    while(parsing_space[dotdot] != ' ')dotdot++;
                    parsing_space[dotdot] = 0;
                    len = dotdot-i;
                    if(len > 255)len = 255;
                    bzero(path,256);
                    memcpy(path,parsing_space+i,len);
                    stage++;
                }
                if(stage == 1)
                {
                    dotdot = 0;
                    while(parsing_space[dotdot] != 0 && parsing_space[dotdot] != ':')dotdot++;
                    parsing_space[dotdot] = 0;
                    dotdot++;
                    while(parsing_space[dotdot] == ' ')dotdot++;
                    header_handler(misc,parsing_space,parsing_space+dotdot);
                }
                bzero(parsing_space,sizeof(parsing_space));
                prr = 0;
                cret = 0;
            }
        }else{
            escape_nl = 0;
            parsing_space[prr] = a;
            prr++; 
            if(((unsigned int)prr) >= sizeof(parsing_space))return 1;
        }
    }
    return 0;
}
