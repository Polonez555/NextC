
CFLAGS=-fsanitize=address -g -Wextra -Wall -pedantic -ansi -Werror
CINCLUDES=-I include

server:
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c server.c -o server.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c websocket.c -o websocket.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c http.c -o http.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c nextc.c -o nextc.o 
	gcc -rdynamic -fsanitize=address -g -shared -fPIC server.o http.o websocket.o nextc.o -o server.dylib

run:
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c http.c -o http.o
	gcc -rdynamic $(CFLAGS) $(CINCLUDES) main.c http.c -o main
	LD_LIBRARY_PATH=. ./main

build_server:
	mkdir -p server_libs
	gcc $(CFLAGS) $(CINCLUDES) -c src/http.c -o server_libs/http.o
	gcc $(CFLAGS) $(CINCLUDES) -c src/dev_server.c -o server_libs/dev_server.o
	gcc $(CFLAGS) $(CINCLUDES) -c src/prod_server.c -o server_libs/prod_server.o
	gcc $(CFLAGS) $(CINCLUDES) server_libs/http.o server_libs/dev_server.o -o dev_server
	rm -r server_libs
build_libs:
	mkdir -p libs
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c src/websocket.c -o libs/websocket.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c src/http.c -o libs/http.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c src/nextc.c -o libs/nextc.o 
	gcc $(CFLAGS) $(CINCLUDES) -fPIC -c src/router.c -o libs/router.o 
	ar rs nextcso.a libs/websocket.o libs/http.o libs/nextc.o libs/router.o
	rm -r libs