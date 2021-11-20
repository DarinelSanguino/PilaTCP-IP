#include "SwitchCapa2.h"

static void aprendizaje_mac_switch_capa2(nodo_t *nodo, char *mac_origen, char *nombre_if) {
	tabla_mac_t *tabla_mac = nodo->prop_nodo->tabla_mac;
	entrada_mac_t *entrada_mac = calloc(1, sizeof(entrada_mac_t));
	//POSIBLE VIOLACIÓN DE SEGMENTO*********************************
	strncpy(entrada_mac->dir_mac.dir_mac, mac_origen, TAM_DIR_MAC);
	agregar_entrada_tabla_mac(tabla_mac, entrada_mac);
}

static void enviar_trama_switch_capa2(nodo_t *nodo, interface_t *intf_entrada, char *paquete, unsigned int tamano_paq) {
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	char *dir_mac = cab_ethernet->mac_destino.dir_mac;
	if(ES_DIR_MAC_BROADCAST(dir_mac)) {
		enviar_paquete_interfaces_l2(nodo, intf_entrada, paquete, tamano_paq);
	}
	//Buscar correspondencia entre dirección MAC e interface
	entrada_mac_t *entrada_mac = busqueda_tabla_mac(nodo->prop_nodo->tabla_mac, dir_mac);
	if(!entrada_mac) {
		/************************PENDIENTE: ejecutar búsqueda ARP en lugar de enviar el paquete en todas las interfaces.****/
		enviar_paquete_interfaces_l2(nodo, intf_entrada, paquete, tamano_paq);
	}
	
	interface_t *intf_salida = obtener_intf_por_nombre(nodo, entrada_mac->nombre_if);
	if(!intf_salida) {
		printf("Error: no se encontró la interface %s.\n", nombre_if);
		return;
	}
	enviar_paquete(paquete, tamano_paq, intf_salida);
}

void recibir_trama_switch_capa2(interface_t *interface, char *paquete, unsigned int tamano_paq) {
	nodo_t *nodo = interface->nodo_padre;
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	char *mac_origen = cab_ethernet->mac_destino.dir_mac;
	aprendizaje_mac_switch_capa2(nodo, mac_origen, interface->nombre_if);
	enviar_trama_switch_capa2(nodo, interface, paquete, tamano_paq);
}