#define PROCESS_CLIENT_DEF(h) \
void process_client(struct HttpContext* ctx) {          \
    char method[10];                                   \
    char path[256];                                    \
                                                       \
    if (http_parse_get(ctx, path, method, 0, h)) { \
        printf("Close\n");                             \
        http_ctx_close(ctx);                           \
        return;                                        \
    }                                                  \
                                                       \
    int ret = nc_FindRoute(path);                      \
    printf("FND:%d\n", ret);                           \
    nc_ExecRoute(ret, ctx);                            \
}
