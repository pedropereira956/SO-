#ifndef DOCUMENTO_H
#define DOCUMENTO_H

typedef struct doc *DOC;
size_t size ();
DOC create_doc (void);
void set_titulo(DOC doc, const char *titulo);
void set_autores (DOC doc, const char *autor);
void set_path (DOC doc, const char *path);
void set_ano (DOC doc, const char *ano);
void set_id (DOC doc, int id);
char* get_titulo(DOC doc);
char* get_autores(DOC doc);
char* get_path(DOC doc);
char* get_ano(DOC doc);
int get_id (DOC doc);
void free_doc (DOC doc);
DOC construir_doc (char** fields);
DOC clone (DOC doc);

#endif