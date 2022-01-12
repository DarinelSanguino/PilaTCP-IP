#include "ListaEnlazadaGenerica.h"

bool insertar(Lista_t *lista, void *elemento, size_t tam_elemento) {	
	//printf("Iniciando insercion.\n");
	NodoLista_t *nodo_lista = malloc(sizeof(NodoLista_t));

	if(nodo_lista != NULL) {
		nodo_lista->elemento = malloc(tam_elemento);		
		for(int i = 0; i < tam_elemento; i++) {
			*((char *) (nodo_lista->elemento + i)) = *((char *) (elemento + i));
		}
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
		return true;
	}
	else {
		printf("Memoria insuficiente.\n");
		return false;
	}	
}

void eliminar(Lista_t *lista, NodoLista_t *nodo_lista) {
	NodoLista_t *nodo_previo = NULL;
	NodoLista_t *nodo_actual = lista->nodo_inicio;
	while(nodo_actual != NULL) {
		if(nodo_actual == nodo_lista) {
			if(nodo_actual == lista->nodo_final) {
				lista->nodo_final = nodo_previo;
			}
			if(nodo_previo == NULL) {
				lista->nodo_inicio = nodo_actual->sig_nodo;
				lista->vacia = true;
			}
			else {
				nodo_previo->sig_nodo = nodo_actual->sig_nodo;
			}
			free(nodo_actual);
			return;
		}
		nodo_previo = nodo_actual;
		nodo_actual = nodo_actual->sig_nodo;
	}	
}

void limpiar(Lista_t *lista) {
	NodoLista_t *nodo_previo = NULL;
	NodoLista_t *nodo_actual = lista->nodo_inicio;
	while(nodo_actual != NULL) {
		nodo_previo = nodo_actual;
		nodo_actual = nodo_actual->sig_nodo;
		free(nodo_previo);
	}
	lista->vacia = true;
}

bool lista_vacia(Lista_t *lista) {
	return lista->vacia;
}
