#include "InicRed.h"

void inicializar_interface(interface_t *interface) {
	memset(interface->nombre_if, 0, TAM_NOMBRE_IF);
	interface->nodo_padre = malloc(sizeof(nodo_t));
	interface->enlace = malloc(sizeof(enlace_t));
	interface->prop_intf = malloc(sizeof(prop_intf_t));
}

void inicializar_nodo(nodo_t *nodo) {
	nodo->nombre_nodo[0] = '\0';
	for(int i = 0; i < MAX_INTF_POR_NODO; i++) {
		nodo->intf[i] = NULL;		
	}
	nodo->prop_nodo = malloc(sizeof(prop_nodo_t));
	init_prop_nodo(nodo->prop_nodo);
}