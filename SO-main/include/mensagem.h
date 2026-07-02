#ifndef MENSAGEM_H
#define MENSAGEM_H

typedef enum {
    CMD_INVALID = -1,
    CMD_A = 0,
    CMD_C = 1,
    CMD_D = 2,
    CMD_L = 3,
    CMD_S = 4,
    CMD_F = 5,
} Comando;

typedef struct {
    int tipo;
    int data_size;
    char pid[20];
} HEADER;

#endif