#include "ListaEnlazadaGenerica.h"

bool insertar(Lista_t *lista, void *elemento, size_t tam_elemento) {
	//Pendiente**********************************************************************
	printf("Iniciando insercion.\n");
	NodoLista_t *nodo_lista = malloc(sizeof(NodoLista_t));

	//ptrNodo nuevoPtr = malloc(sizeof(NodoLista));
	if(nodo_lista != NULL) {
		nodo_lista->elemento = malloc(tam_elemento);
		//nodo_lista->elemento = elemento;
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
	printf("Finalizando insercion.\n");
}

void eliminar(Lista_t *lista, NodoLista_t *nodo_lista) {
	NodoLista_t *nodo_previo = NULL;
	NodoLista_t *nodo_actual = lista->nodo_inicio;
	while(nodo_actual != NULL) {
		if(nodo_actual == nodo_lista) {
			if(nodo_previo == NULL) {
				lista->nodo_inicio = nodo_actual->sig_nodo;			
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
	return;
}

bool lista_vacia(Lista_t *lista) {
	return lista->vacia;
}

/*void recorrer_lista(Lista_t *lista, void (*operacion) (const void *elemento)) {
	if(lista_vacia(lista)) {
		return;
	}
	NodoLista_t *nodo_actual = lista->nodo_inicio;
	while(nodo_actual != NULL) {
		operacion(nodo_actual->elemento);
		nodo_actual = nodo_actual->sig_nodo;
	}
}*/
