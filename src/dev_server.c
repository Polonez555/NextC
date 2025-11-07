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
#include <sys/xattr.h>
#include <dirent.h> 

int serv_sock;
void* serv_handle = 0;
void (*process_client_ptr)(struct HttpContext*);
void (*server_setup_ptr)(void);

void equit(const char* err)
{
    if(serv_handle)dlclose(serv_handle);
    close(serv_sock);
    printf("Error %s\n",err);
    exit(1);
}

void hot_reload_reload(void)
{
    if(serv_handle)dlclose(serv_handle);
    serv_handle = dlopen("./server.dylib", RTLD_NOW | RTLD_GLOBAL);
    if(serv_handle == 0)equit("Could not open server dl");
    if((process_client_ptr = (void(*)(struct HttpContext*))dlsym(serv_handle, "process_client")) == 0)equit("Could not find sym process_client");
    if((server_setup_ptr = (void(*)(void))dlsym(serv_handle, "server_setup")) == 0)equit("Could not find sym server_setup");
    server_setup_ptr();
}

int last_hot_reload = 0;

int getFileCreationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtime;
}

void check_autobuild(void)
{
    DIR *d;
    struct dirent *dir;
    char fullpath[PATH_MAX];
    int last_changed_buf = 0;
    char is_renewed = 0;
    char do_rebuild = 0;
    int xattr_len = 0;

    d = opendir("app");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name,"..") != 0)
            {
                is_renewed = 0;
                snprintf(fullpath, sizeof(fullpath), "%s/%s", "app", dir->d_name);
                xattr_len = getxattr(fullpath,"nextc_autobuild_last_change",&last_changed_buf,sizeof(last_changed_buf),0,0);
                if(xattr_len == -1)
                {
                    is_renewed = 1;
                }else{
                    if(getFileCreationTime(fullpath) != last_changed_buf)
                    {
                        is_renewed = 1;
                    }
                }
                if(is_renewed)
                {
                    printf("Rebuilding %s changed\n",dir->d_name);
                    last_changed_buf = getFileCreationTime(fullpath);
                    setxattr(fullpath,"nextc_autobuild_last_change",&last_changed_buf,sizeof(last_changed_buf),0,0);
                    do_rebuild = 1;
                }
            }
        }
        closedir(d);
    }

    if(do_rebuild)
    {
        printf("Rebuilding\n");
        system("make dev_dylib");
    }
}

void check_hot_reload(void)
{
    int hr = getFileCreationTime("server.dylib");
    if(last_hot_reload != hr)
    {
        printf("HOT Reloading\n");
        hot_reload_reload();
        last_hot_reload = hr;
    }
}

void sighandl(int _)
{
    printf("\nBYE\n");
    close(serv_sock);
    exit(0);
}

void sigsegvhandler(int _)
{
    pthread_exit(NULL);
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
    process_client_ptr(&cta->ctx);

    printf("Gone\n");

    free(cta);
    return 0;
}

void* hot_reload_process(void* _)
{
    while(1)
    {
        check_autobuild();
        check_hot_reload();
        usleep(1000000);
    }
    return 0;
}

int main(void)
{
    struct sockaddr_in servaddr;
    int addr;
    int accepted;
    struct ClientThreadArgs* ctam;
    pthread_t thrd;
    pthread_t hthrd;

    signal(SIGINT, sighandl);
    signal(SIGSEGV, sigsegvhandler);
    signal(SIGPIPE, sigsegvhandler);
    printf("Hello\n");
    hot_reload_reload();

    pthread_create(&hthrd, 0, hot_reload_process, 0);

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)equit("Could not create socket");
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /*All allowed*/
    if(bind(serv_sock, (const struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)equit("Could not bind");
    if(listen(serv_sock, 50) != 0)equit("Listen failed\n");
    printf("Server running http://localhost:8080/\n");
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
