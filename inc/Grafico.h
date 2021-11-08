#ifndef GRAFICO_H_
#define GRAFICO_H_

#include <stdlib.h>
#include <string.h>
#include "ListaEnlazadaGenerica.h"
//#include "Grafico_fwd.h"
//#include "Net_fwd.h"
//#include "Com.h"
#include "Tipos_red.h"
#include "Net.h"

// #define TAM_NOMBRE_NODO 16
// #define TAM_NOMBRE_IF 16
// #define MAX_INTF_POR_NODO 10
// #define MAX_NODOS_POR_GRAFICO 256
// #define TAM_DIR_MAC 6

/*struct interface_ {
	char nombre_if[TAM_NOMBRE_IF];
	struct nodo_ *nodo_padre;
	struct enlace_ *enlace;
	prop_intf_t *prop_intf;
};

struct enlace_ {
	interface_t intf1;
	interface_t intf2;
	unsigned int costo;
};

struct nodo_ {
	char nombre_nodo[TAM_NOMBRE_NODO];
	interface_t *intf[MAX_INTF_POR_NODO];
	prop_nodo_t *prop_nodo;
	unsigned int numero_puerto_udp;
	int fd_sock_udp;
};

struct grafico_ {
	char nombre_topologia[32];
	Lista_t *lista_nodos;
};*/

grafico_t * crear_nuevo_grafico(const char *nombre_topologia);
nodo_t * crear_nodo_grafico(grafico_t *grafico, const char *nombre_nodo);
void insertar_enlace_entre_nodos(nodo_t *nodo1, nodo_t *nodo2, char *de_nombre_if, char *a_nombre_if, unsigned int costo);

nodo_t *obtener_elemento(Lista_t *lista, char *nombre_nodo);
void inicializar_grafico(grafico_t *grafico);
void inicializar_nodo(nodo_t *nodo);
void inicializar_interface(interface_t *interface);
void mostrar_grafico(const grafico_t *grafico);
void mostrar_nodo(const nodo_t *nodo);
void mostrar_interface(const interface_t *interface);
void mostrar_enlace(const enlace_t *enlace);
void mostrar_prop_intf(const prop_intf_t *prop_intf);
void mostrar_dir_mac(const dir_mac_t *dir_mac);
interface_t* obtener_intf_correspondiente_a_nodo(nodo_t *nodo, char *dir_ip);

static inline nodo_t* obtener_nodo_vecino(interface_t *interface) {
	interface_t *intf_vecina = &interface->enlace->intf1 == interface ? &interface->enlace->intf2 : &interface->enlace->intf1;
	return intf_vecina->nodo_padre;
}

static inline int obtener_ranura_intf_disp(nodo_t *nodo) {
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i)
	{
		/* code */
		if (nodo->intf[i] == NULL) return i;		
	}
	return -1;
}

static inline interface_t * obtener_intf_por_nombre(nodo_t *nodo, char *nombre_if) {
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i)
	{
		if(!nodo->intf[i]) return NULL;
		if(strncmp(nodo->intf[i]->nombre_if, nombre_if, TAM_NOMBRE_IF) == 0) {
			return nodo->intf[i];
		}
	}
	return NULL;
}

static inline nodo_t * obtener_nodo_por_nombre(grafico_t *topologia, char *nombre_nodo) {
	return obtener_elemento(topologia->lista_nodos, nombre_nodo);
}

#endif
