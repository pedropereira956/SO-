#include <stdio.h>
#include <string.h>
#include "../include/utils.h"


Comando comando(const char* arg) {
    if (strcmp(arg, "-a") == 0) return CMD_A;
    if (strcmp(arg, "-c") == 0) return CMD_C;
    if (strcmp(arg, "-d") == 0) return CMD_D;
    if (strcmp(arg, "-l") == 0) return CMD_L;
    if (strcmp(arg, "-s") == 0) return CMD_S;
    if (strcmp(arg, "-f") == 0) return CMD_F;
    return CMD_INVALID;
}