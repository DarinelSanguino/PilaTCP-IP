#ifndef LISTAENLAZADAGENERICA_H_
#define LISTAENLAZADAGENERICA_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ListaEnlazada Lista_t;
typedef struct NodoLista NodoLista_t;

struct NodoLista {
	void *elemento;
	struct NodoLista *sig_nodo;
};

struct ListaEnlazada {
	struct NodoLista *nodo_inicio;
	struct NodoLista *nodo_final;
	bool vacia;
};

bool insertar(Lista_t *lista, void *elemento, size_t tam_elemento);
bool lista_vacia(Lista_t *lista);
void eliminar(Lista_t *lista, NodoLista_t *nodo_lista);
void limpiar(Lista_t *lista);

#define ITERAR_LISTA_ENLAZADA(lista) {	\
	NodoLista_t *nodo_actual = lista->nodo_inicio;	\
	while(nodo_actual != NULL) {
		
#define FIN_ITERACION nodo_actual = nodo_actual->sig_nodo;	\
	}	\
}

#endif