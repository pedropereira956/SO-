#include "../include/tabela.h"
#include "../include/documento.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib.h>
#include <stdio.h>

static GHashTable* bd = NULL;
static GHashTable* posicoes = NULL;
static GList* ordem = NULL;
static int tamanho;
static int tamanho_atual;
static int next_id = 1; 

void doc_serialize(DOC doc, int fd) {
    write(fd, doc, size());
}

DOC doc_deserialize(int fd) {
    DOC doc = malloc(size());
    if (!doc) return NULL;

    ssize_t r = read(fd, doc, size());
    if (r != size()) {
        free(doc);
        return NULL;
    }

    return doc;
}


static int doc_existe_em_memoria(int id) {
    int fd = open("memoria.bin", O_RDONLY);
    if (fd == -1) return 0;

    int existe = 0;
    while (1) {
        DOC temp = doc_deserialize(fd);
        if (!temp) break;
        if (get_id(temp) == id) {
            existe = 1;
            free_doc(temp);
            break;
        }
        free_doc(temp);
    }
    close(fd);
    return existe;
}

static void guardar_em_memoria(DOC doc) {
    int id = get_id(doc);
    if (doc_existe_em_memoria(id)) return;

    int fd = open("memoria.bin", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) return;
    doc_serialize(doc, fd);
    close(fd);
}

static int remover_de_memoria(int id) {
    int fd_old = open("memoria.bin", O_RDONLY);
    if (fd_old == -1) return 0;

    int fd_temp = open("memoria_temp.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_temp == -1) {
        close(fd_old);
        return 0;
    }

    int encontrado = 0;
    while (1) {
        DOC temp = doc_deserialize(fd_old);
        if (!temp) break;

        if (get_id(temp) == id) {
            encontrado = 1;
            free_doc(temp);
            continue;
        }

        doc_serialize(temp, fd_temp);
        free_doc(temp);
    }

    close(fd_old);
    close(fd_temp);

    remove("memoria.bin");
    rename("memoria_temp.bin", "memoria.bin");

    return encontrado;
}

void tabela_init(int max) {
    if (!bd) {
        bd = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)free_doc);
    }
    if (!posicoes) {
        posicoes = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    }
    ordem = NULL;
    tamanho = max;
    tamanho_atual = 0;
    open("memoria.bin", O_CREAT , 0644);
}

void tabela_destroy() {
    if (bd) {
        GList* values = g_hash_table_get_values(bd);
        for (GList* l = values; l != NULL; l = l->next) {
            DOC doc = (DOC)l->data;
            guardar_em_memoria(doc);
        }
        g_list_free(values);
        g_hash_table_destroy(bd);
        bd = NULL;
    }
    if (posicoes) {
        g_hash_table_destroy(posicoes);
        posicoes = NULL;
    }
    g_list_free(ordem);
    ordem = NULL;
}

int tabela_add(DOC doc) {  //MUDEI TAMBÈM AQUI
    int id = next_id++;          // 🔧 gerar ID sequencial
    set_id(doc, id);             // 🔧 atribuir ID ao documento
    gpointer key = GINT_TO_POINTER(id);

    // Se já existir o documento com este id, remove primeiro
    if (g_hash_table_contains(bd, key)) {
        GList* pos = g_hash_table_lookup(posicoes, key);
        ordem = g_list_delete_link(ordem, pos);
        g_hash_table_remove(posicoes, key);
        g_hash_table_remove(bd, key);
        tamanho_atual--;
    }

    // Se a capacidade for excedida, remover o mais antigo
    if (g_hash_table_size(bd) >= tamanho) {
        GList* mais_antigo = ordem;
        int id_antigo = GPOINTER_TO_INT(mais_antigo->data);
        gpointer k_antigo = GINT_TO_POINTER(id_antigo);

        DOC antigo = g_hash_table_lookup(bd, k_antigo);
        guardar_em_memoria(antigo);

        g_hash_table_remove(bd, k_antigo);
        g_hash_table_remove(posicoes, k_antigo);
        ordem = g_list_delete_link(ordem, mais_antigo);
    }

    // Adicionar o novo documento
    ordem = g_list_append(ordem, key);
    GList* pos = g_list_last(ordem);

    g_hash_table_insert(bd, key, doc);
    g_hash_table_insert(posicoes, key, pos);
    tamanho_atual++;

    return id;
}


DOC tabela_get(int id) {
    gpointer key = GINT_TO_POINTER(id); //troquei isto
    DOC original = (DOC)g_hash_table_lookup(bd, key); //troquei isto
    if (original) return clone(original);

    int fd = open("memoria.bin", O_RDONLY);
    if (fd == -1) return NULL;

    DOC doc = NULL;
    while (1) {
        DOC temp = doc_deserialize(fd);
        if (!temp) break;

        if (get_id(temp) == id) {
            doc = temp;
            break;
        } else {
            free_doc(temp);
        }
    }

    close(fd);
    if (!doc) return NULL;

    DOC novo = clone(doc);
    tabela_add(novo);
    remover_de_memoria(get_id(novo));

    if (doc != novo)
        free_doc(doc);

    return novo;


}

int tabela_remove(int id) {
    int removido_mem = remover_de_memoria(id);
    gpointer key = GINT_TO_POINTER(id);
    int removido_cache = g_hash_table_remove(bd, key);
    if (removido_cache) tamanho_atual--;
    return removido_mem || removido_cache;
}


DOC* tabela_iter(size_t* count) {
    if (!bd) {
        *count = 0;
        return NULL;
    }

    GList* values = g_hash_table_get_values(bd);
    size_t n_cache = g_list_length(values);
    size_t capacidade = n_cache + 16;
    DOC* docs = malloc(capacidade * size());
    size_t i = 0;

    for (GList* l = values; l != NULL; l = l->next) {
        docs[i++] = clone((DOC)l->data);
    }
    g_list_free(values);
    
    if (tamanho_atual == tamanho){
    int fd = open("memoria.bin", O_RDONLY);
    if (fd != -1) {
        while (1) {
            DOC temp = doc_deserialize(fd);
            if (!temp) break;

            if (i >= capacidade) {
                capacidade *= 2;
                docs = realloc(docs, capacidade * size());
            }

            docs[i++] = temp;
        }
        close(fd);
    }
    }
    *count = i;
    return docs;
}
