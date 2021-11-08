#ifndef LISTAENLAZADA_H
#define LISTAENLAZADA_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
//#include "Grafico.h"

typedef struct nodo_ nodo_t;
typedef struct ListaEnlazada Lista_t;
typedef struct NodoLista NodoLista_t;

struct NodoLista {
	nodo_t *nodo;
	struct NodoLista *sig_nodo;
};

struct ListaEnlazada {
	struct NodoLista *nodo_inicio;
	struct NodoLista *nodo_final;
	bool vacia;
};

void insertar(Lista_t *lista, nodo_t *nodo);
//nodo_t *obtener_elemento(Lista_t *lista, char *nombre_nodo);
bool lista_vacia(Lista_t *lista);
void recorrer_lista(Lista_t *lista, void (*operacion) (const nodo_t *nodo));

#endif