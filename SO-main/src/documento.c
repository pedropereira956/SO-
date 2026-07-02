#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/documento.h"
#include "../include/utils.h"

struct doc {
    int id;
    char titulo[200];
    char autores[200];
    char path[64];
    char ano[5];
};

size_t size (){
    return sizeof(struct doc);
}

DOC create_doc (void){
    DOC doc = calloc(1, sizeof(struct doc));
    if(doc == NULL){
        perror("Erro a criar documento");
        return NULL;
    }


    return doc;
}

void set_titulo(DOC doc, const char *titulo) {
    strncpy(doc->titulo, titulo, 200);
    doc->titulo[199] = '\0';
}

void set_autores (DOC doc, const char *autor){
    strncpy(doc->autores, autor, 200);
    doc->autores[199] = '\0';
}

void set_path (DOC doc, const char *path){
    strncpy(doc->path, path, 64);
    doc->path[63] = '\0';
}

void set_ano (DOC doc, const char *ano){
    strncpy(doc->ano, ano, 5);
    doc->ano[4] = '\0';
}

void set_id (DOC doc, int id){
    int num = id;
    doc->id = num;
}

char* get_titulo(DOC doc){
    return strdup(doc->titulo);
}

char* get_autores(DOC doc){
    return strdup(doc->autores);
}

char* get_path(DOC doc){
    return strdup(doc->path);
}

char* get_ano(DOC doc){
    return strdup(doc->ano);
}

int get_id (DOC doc){
    int a = doc->id;
    return a;
}

void free_doc (DOC doc){
    free(doc);
}

DOC construir_doc (char** fields){
    if (!fields[0] || !fields[1] || !fields[2] || !fields[3]){
        perror("Campos insuficientes");
    }

    DOC doc = create_doc();
    set_titulo(doc, fields[0]);
    set_autores(doc, fields[1]);
    set_ano(doc, fields[2]);
    set_path(doc, fields[3]);
    

    return doc;
}

DOC clone (DOC doc){
    DOC clone = create_doc();
    set_titulo(clone, get_titulo(doc));
    set_autores(clone, get_autores(doc));
    set_path(clone, get_path(doc));
    set_ano(clone, get_ano(doc));
    set_id(clone, get_id(doc));

    return clone;

}
