#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../include/documento.h"
#include "../include/mensagem.h"
#include "../include/utils.h"

int main (int argc, char* argv[]){
    int fd_write;
    pid_t pid = getpid();
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    if (mkfifo("pipes/pipe", 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar FIFO");
        return 1;
    }

    char fifoname[100];
    snprintf(fifoname, sizeof(fifoname), "pipes/%s", pid_str);
    if (mkfifo(fifoname, 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar FIFO cliente");
        return 1;
    }

    fd_write = open("pipes/pipe", O_WRONLY);
    if (fd_write < 0) {
        perror("Erro a abrir pipe para escrita");
        return 1;
    }

    Comando cmd = comando(argv[1]);

    switch (cmd) {
        case CMD_A: {
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "%s|%s|%s|%s",
                     argv[2], argv[3], argv[4], argv[5]);

            HEADER h;
            h.tipo = CMD_A;
            h.data_size = strlen(buffer) + 1;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            write(fd_write, &h, sizeof(HEADER));
            write(fd_write, buffer, h.data_size);
            close(fd_write);

            DOC doc = create_doc();
            int fd_read = open(fifoname, O_RDONLY);
            if (fd_read < 0) {
                perror("Erro a abrir pipe leitura");
                return 1;
            }
            if (read(fd_read, doc, size()) < 0) {
                perror("Erro ao ler resposta do servidor");
            }

            printf("Document %d indexed\n", get_id(doc));

            close(fd_read);
            free_doc(doc);
            break;
        }

        case CMD_C: {
            if (argc < 3) {
                perror("Uso: -c <id>\n");
                return 1;
            }

            HEADER h;
            h.tipo = CMD_C;
            h.data_size = strlen(argv[2]) + 1;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            write(fd_write, &h, sizeof(HEADER));
            write(fd_write, argv[2], h.data_size);
            close(fd_write);

            int fd_read = open(fifoname, O_RDONLY);
            if (fd_read < 0) {
                perror("Erro ao abrir pipe para leitura");
                return 1;
            }

            DOC doc = create_doc();
            if (read(fd_read, doc, size()) < 0) {
                perror("Erro ao ler resposta do servidor");
            }

            if (get_id(doc) == -1) {
                printf("Document not found\n");
            } else {
                printf("Title: %s\n", get_titulo(doc));
                printf("Authors: %s\n", get_autores(doc));
                printf("Year: %s\n", get_ano(doc));
                printf("Path: %s\n", get_path(doc));
            }

            close(fd_read);
            free_doc(doc);
            break;
        }

        case CMD_D: {
            if (argc < 3) {
                perror("Uso: -d <id>\n");
                return 1;
            }

            HEADER h;
            h.tipo = CMD_D;
            h.data_size = strlen(argv[2]) + 1;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            write(fd_write, &h, sizeof(HEADER));
            write(fd_write, argv[2], h.data_size);
            close(fd_write);

            printf("Index entry %s deleted\n", argv[2]);
            break;
        }

        case CMD_L: {
            HEADER h;
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "%s|%s", argv[2], argv[3]);

            h.tipo = CMD_L;
            h.data_size = strlen(buffer) + 1;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            write(fd_write, &h, sizeof(HEADER));
            write(fd_write, buffer, h.data_size);
            close(fd_write);

            int fd_read = open(fifoname, O_RDONLY);
            char buffer2[32];
            read(fd_read, buffer2, sizeof(buffer2));
            printf("Número de linhas com a palavra %s: %s\n", argv[3], buffer2);

            close(fd_read);
            break;
        }

        case CMD_S: {
            HEADER h;
            h.tipo = CMD_S;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            char dados[128];
            if (argc == 4)
                snprintf(dados, sizeof(dados), "%s|%s", argv[2], argv[3]);
            else
                snprintf(dados, sizeof(dados), "%s", argv[2]);

            h.data_size = strlen(dados) + 1;
            write(fd_write, &h, sizeof(HEADER));
            write(fd_write, dados, h.data_size);
            close(fd_write);

            int fd_read = open(fifoname, O_RDONLY);
            if (fd_read < 0) {
                perror("Erro ao abrir FIFO do cliente para leitura");
                unlink(fifoname);
                return 1;
            }

            printf("[");
            char buffer[512];
            int lidos;
            while ((lidos = read(fd_read, buffer, 511)) > 0) {
                buffer[lidos] = '\0';
                printf("%s", buffer);
            }
            printf("]\n");

            close(fd_read);
            break;
        }

        case CMD_F: {
            HEADER h;
            h.tipo = CMD_F;
            h.data_size = 0;
            snprintf(h.pid, sizeof(h.pid), "%s", pid_str);

            write(fd_write, &h, sizeof(HEADER));
            close(fd_write);

            printf("Server is shutting down\n");
            break;
        }

        default:
            fprintf(stderr, "Comando não reconhecido.\n");
            break;
    }

    unlink(fifoname);
    return 0;
}