#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <dlfcn.h>
#include "http.h"
#include <sys/stat.h>
#include <sys/types.h>

int serv_sock;
extern void process_client(struct HttpContext* ctx);

void equit(const char* err)
{
    close(serv_sock);
    printf("Error %s\n",err);
    exit(1);
}

void sighandl(int _)
{
    printf("\nBYE\n");
    close(serv_sock);
    exit(0);
}

struct ClientThreadArgs
{
    int client_fd;
    struct sockaddr_in addr;
    struct HttpContext ctx;
};


void* client_thread(void* args)
{
    struct ClientThreadArgs* cta = (struct ClientThreadArgs*)args;
    
    printf("Client connected from %s\n",inet_ntoa(cta->addr.sin_addr));
    
    /*Client processing*/
    http_ctx_init(&cta->ctx, cta->client_fd);
    process_client(&cta->ctx);

    printf("Gone\n");

    free(cta);
    return 0;
}

int main(void)
{
    struct sockaddr_in servaddr;
    int addr;
    int accepted;
    struct ClientThreadArgs* ctam;
    pthread_t thrd;

    signal(SIGINT, sighandl);
    printf("Hello\n");

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)equit("Could not create socket");
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /*All allowed*/
    if(bind(serv_sock, (const struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)equit("Could not bind");
    if(listen(serv_sock, 50) != 0)equit("Listen failed\n");
    printf("Server running\n");
    while(1)
    {
        ctam = (struct ClientThreadArgs*)malloc(sizeof(struct ClientThreadArgs));
        addr = sizeof(ctam->addr);
        accepted = accept(serv_sock, (struct sockaddr*)&(ctam->addr), (unsigned int*)&(addr));
        ctam->client_fd = accepted;
        if(accepted > -1)
        {
            printf("Accepted\n");
            pthread_create(&thrd, 0, client_thread, ctam);
        }else{
            free(ctam);
            break;
        }
    }
    close(serv_sock);
    return 0;
}
