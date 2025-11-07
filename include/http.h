#ifndef HTTP_H
#define HTTP_H

struct HttpHeader
{
    const char* name;
    const char* value;
};

struct HttpContext
{
    int socket;
    int content_length;
    char* content;
    int header_length;
    char* headers;

    struct HttpHeader* iheaders;
    int iheader_count;
};

void http_ctx_init(struct HttpContext* ctx, int sock);
void http_ctx_push_content(struct HttpContext* ctx, char* content, int content_length);
void http_ctx_push_header(struct HttpContext* ctx, char* content);
void http_ctx_push_string(struct HttpContext* ctx,char* text);
void http_ctx_send(struct HttpContext* ctx);
void http_ctx_close(struct HttpContext* ctx);
int http_parse_get(struct HttpContext* ctx, char* path, char* method, void* misc, void (*header_handler)(void*,char*,char*));
void http_ctx_erase(struct HttpContext* ctx);
void http_serve_file(struct HttpContext* ctx, const char* path);
char* http_find_iheader(struct HttpContext* ctx, char* key);
void http_ctx_emit_raw(struct HttpContext* ctx, char* text, int length);

#define HTTP_STANDARD_RESP(ctx, ctype, status_code, status) http_ctx_push_header(ctx, "HTTP/1.1 " #status_code " " #status); http_ctx_push_header(ctx, "Content-Type: " #ctype);
#define HTTP_STANDARD_OK(ctx, ctype) HTTP_STANDARD_RESP(ctx, ctype, 200, "OK")
#define HTTP_HTML "text/html"

#endif
