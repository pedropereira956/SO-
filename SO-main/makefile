CC = clang
CFLAGS = -Wall -g -Iinclude $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0)


SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
UTILS_DIR = src

OBJS_COMMON = obj/documento.o obj/utils.o obj/tabela.o


.PHONY: all folders clean dserver dclient

all: folders bin/dserver bin/dclient

folders:
	@mkdir -p src include obj pipes bin


bin/dserver: obj/dserver.o $(OBJS_COMMON)
	$(CC) $(LDFLAGS) $^ -o $@

bin/dclient: obj/dcliente.o $(OBJS_COMMON)
	$(CC) $(LDFLAGS) $^ -o $@

obj/dserver.o: $(SRC_DIR)/dserver.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/dcliente.o: $(SRC_DIR)/dcliente.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/documento.o: $(SRC_DIR)/documento.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/utils.o: $(UTILS_DIR)/utils.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/tabela.o: $(SRC_DIR)/tabela.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* pipes/* 
	rmdir obj/ pipes/ 
	rm -rf bin/*
	rmdir bin/
	rm memoria.bin
