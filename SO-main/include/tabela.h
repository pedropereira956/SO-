#ifndef TABELA_H
#define TABELA_H

#include <glib.h>
#include "documento.h"

void tabela_init(int max);                      
void tabela_destroy();                   

int tabela_add(DOC doc);                 
DOC tabela_get(int id);                 
int tabela_remove(int id);        // Remove por ID (1 = sucesso, 0 = não existe)
DOC tabela_get_by_filename(const char* filename);
DOC* tabela_iter(size_t* count);
#endif