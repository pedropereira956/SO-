#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "../include/documento.h"
#include "../include/mensagem.h"
#include "../include/tabela.h"

int main (int argc, char** argv){
    int running = 1;

    tabela_init(atoi(argv[2]));

    if (mkfifo("pipes/pipe", 0666) == -1 && errno != EEXIST) {
        perror("Erro ao criar FIFO");
        return 1;
    }
    

    while (running == 1) {
        int fd_read = open("pipes/pipe", O_RDONLY);
        if (fd_read == -1) {
            perror("Erro ao abrir o FIFO para leitura");
            _exit(1);
        }

        HEADER h;
        if (read(fd_read, &h, sizeof(HEADER)) <= 0) {
            close(fd_read);
            continue;
        }

        char* buffer = NULL;
        if (h.data_size > 0) {
            buffer = malloc(h.data_size+1);
            int read_size = read(fd_read, buffer, h.data_size);
            buffer[read_size] = '\0';
        }
        close(fd_read);

        switch (h.tipo) {
            case CMD_A: {
                char* fields[4];
                int i = 0;
                char* token = strtok(buffer, "|");
                char fifoname[100];
                snprintf(fifoname, sizeof(fifoname), "pipes/%s",h.pid);
                
                while (token && i < 4)
                    fields[i++] = strdup(token), token = strtok(NULL, "|");

                char full_path[64];
                snprintf(full_path, sizeof(full_path), "%s/%s", argv[1], fields[3]);
                free(fields[3]);
                fields[3] = strdup(full_path);
                DOC doc = construir_doc(fields);
                int id = tabela_add(doc);
                set_id(doc, id);  

                int fd_write = open(fifoname, O_WRONLY);
                write(fd_write, doc, size());
                close(fd_write);
                for (int j = 0; j < i; j++) {
                    free(fields[j]);
                }

                break;
            }

            case CMD_C: {
                int id = atoi(buffer);

                DOC doc = tabela_get(id);

                char fifoname[100];
                snprintf(fifoname, sizeof(fifoname), "pipes/%s",h.pid);
                
                int fd_write = open(fifoname, O_WRONLY); 

                if (doc == NULL){
                    DOC vazio = create_doc();
                    set_titulo(vazio, "<nao encontrado>");
                    set_autores(vazio, "-");
                    set_ano(vazio, "-");
                    set_path(vazio, "-");
                    set_id(vazio, -1);

                    write(fd_write, vazio, size());
                    free_doc(vazio);
                    
                }
                else{
                    write(fd_write, doc, size());
                }

                close(fd_write);
                break;
            }

            case CMD_D:{
                int id = atoi(buffer);
                tabela_remove(id);
                
                break;
            }

            case CMD_L:{
                int f;
                if ((f = fork() )== 0 ){
                char *fields[2];
                int i = 0;
                char* token = strtok(buffer, "|");
                char fifoname[100];
                snprintf(fifoname, sizeof(fifoname), "pipes/%s",h.pid);

                while (token && i < 2)
                    fields[i++] = strdup(token), token = strtok(NULL, "|");
                
                DOC doc = tabela_get(atoi(fields[0]));

                char* buffer2[] = {"grep", "-i", "-c", fields[1], get_path(doc), NULL};

                pid_t pid;

                int pipefd[2];
                pipe(pipefd);

                if ((pid = fork()) == 0){
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);

                    execvp(buffer2[0], buffer2);
                } 

                else{
                    close(pipefd[1]);
                    int status;

                    waitpid(pid,&status,0);
                    int fd_write = open (fifoname, O_WRONLY);
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                        char buf[2] = {'0', '\0'};
                        write(fd_write, buf, 2);
                        close(fd_write);
                        close(pipefd[0]);
                        break;
                    }

                    char buffer3[32];

                    int lidos = read(pipefd[0], buffer3, sizeof(buffer3) - 1);

                    buffer3[lidos] = '\0';
                    
                    close(pipefd[0]);

                    write(fd_write, buffer3, lidos+1);

                    close(fd_write);
                }
                for (int j = 0; j < i; j++) free(fields[j]);
                exit(0);
                }
                

                break;
            
            }
            case CMD_S: {
                int s_res;
                if ((s_res = fork())== 0){
                    char fifoname[100];
                    snprintf(fifoname, sizeof(fifoname), "pipes/%s",h.pid);
                    
                    int fd_write = open(fifoname, O_WRONLY);
                    char* fields[2];
                    int i = 0;
                    char* token = strtok(buffer, "|");
                    
                    while (token && i < 2)
                        fields[i++] = strdup(token), token = strtok(NULL, "|");
                    
                    int num_procs = atoi(fields[1]);
                    if (num_procs < 1) num_procs = 1;

                    size_t count;
                    DOC* docs = tabela_iter(&count);

                    int docs_per_proc = count / num_procs;
                    int resto = count % num_procs;

                    int pipes[num_procs][2];
                    pid_t pids[num_procs];

                    for(int i = 0; i < num_procs; i++){
                        pipe(pipes[i]);
                        if ((pids[i] = fork()) == 0){

                            close(pipes[i][0]);
                            int inicio = i * docs_per_proc + (i < resto ? i : resto);
                            int fim = inicio + docs_per_proc + (i < resto ? 1 : 0);
                            
                            for (int j = inicio; j < fim; j++){
                                int grep[2];
                                pipe(grep);
                                pid_t pid_grep;
                                if ((pid_grep = fork()) == 0){
                                    close(grep[0]);
                                    dup2(grep[1], STDOUT_FILENO);
                                    close(grep[1]);

                                    char* buffer2[] = {"grep", "-q", fields[0], get_path(docs[j]), NULL};
                                    execvp(buffer2[0], buffer2);
                                    perror("Exec grep falhou");
                                    exit(1);
                                    
                                }
                                else{
                                    close(grep[1]);
                                    int status;
                                    waitpid(pid_grep, &status, 0);
                                    
                                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0){
                                        char buffer3[64];
                                        snprintf(buffer3, sizeof(buffer3), "%d ", get_id(docs[j]));
                                        write(pipes[i][1], buffer3, strlen(buffer3));
                                }
                                close(grep[0]);
                            }
                                
                        }
                            close(pipes[i][1]);
                            exit(0);
                        } else{
                            close(pipes[i][1]);
                        }
                        
                    }
                    char* resposta = malloc(16384);
                    resposta[0] = '\0';
                    for (int i = 0; i < num_procs; i++){
                        char buffer4[2048];
                        int lidos;
                        while ((lidos = read(pipes[i][0], buffer4, 2047)) > 0){
                            buffer4[lidos] = '\0';
                            strcat(resposta, buffer4);
                        }
                        close(pipes[i][0]);
                        waitpid(pids[i], NULL, 0);
                    }

                
                    if (resposta[0] != '\0'){
                        resposta[strlen(resposta) - 1] = '\0';
                    }
                    write(fd_write, resposta, strlen(resposta));

                    for (int i = 0; i < count; i++){
                        free_doc(docs[i]);
                    }

                    free(docs);
                    close(fd_write);
                    free(resposta);
                    exit(0);
                }
            
                break;
            }
            
            case CMD_F: {
                running = 0;
                printf("Servidor a terminar...\n");
                fflush(stdout);
                unlink("pipes/pipe");
                break;
            }

            default:
                break;
        }
        if (buffer) free(buffer);
    }

    tabela_destroy();

    return 0;
}

