#include "ListaEnlazada.h"

void insertar(Lista_t *lista, nodo_t *nodo) {
	//Pendiente**********************************************************************
	NodoLista_t *nodo_lista = malloc(sizeof(NodoLista_t));

	//ptrNodo nuevoPtr = malloc(sizeof(NodoLista));
	if(nodo_lista != NULL) {
		nodo_lista->nodo = nodo;
		nodo_lista->sig_nodo = NULL;
		if(lista->vacia) {
			lista->nodo_inicio = nodo_lista;
			lista->nodo_final = nodo_lista;
			lista->vacia = false;		
		}
		else {
			lista->nodo_final->sig_nodo = nodo_lista;
			lista->nodo_final = nodo_lista;
		}
	}
	else {
		printf("Memoria insuficiente.\n");
	}
}

bool lista_vacia(Lista_t *lista) {
	return lista->vacia;
}

void recorrer_lista(Lista_t *lista, void (*operacion) (const nodo_t *nodo)) {
	if(lista_vacia(lista)) {
		return;
	}
	NodoLista_t *nodo_actual = lista->nodo_inicio;
	while(nodo_actual != NULL) {
		operacion(nodo_actual->nodo);
		nodo_actual = nodo_actual->sig_nodo;
	}
}
