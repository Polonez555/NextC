NEXTC_ORG=..

build_deps:
	cd $(NEXTC_ORG); make build_libs; make build_server

SRC=$(wildcard app/*.c)

dev_dylib: build_deps
	gcc -rdynamic -shared -fsanitize=address -fPIC -c $(SRC) -o server.po -I$(NEXTC_ORG)/include
	gcc -rdynamic -shared -fsanitize=address -lcrypto -fPIC server.po $(NEXTC_ORG)/nextcso.a -o server.dylib

dev: dev_dylib build_deps
	echo Running dev server
	LD_LIBRARY_PATH=. $(NEXTC_ORG)/dev_server