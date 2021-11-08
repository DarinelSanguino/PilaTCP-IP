#include "Capa2.h"

void recibir_trama_capa2(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq) {
	return;
}

void inic_tabla_arp(tabla_arp_t **tabla_arp) {
	*tabla_arp = malloc(sizeof(tabla_arp_t));
	(*tabla_arp)->entradas_arp = malloc(sizeof(Lista_t));
}

entrada_arp_t * busqueda_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		if(!strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP)) {
			return entrada_arp;
		}
	} FIN_ITERACION;
	return NULL;
}

bool agregar_entrada_tabla_arp(tabla_arp_t *tabla_arp, entrada_arp_t *entrada_arp) {
	entrada_arp_t *entrada_arp_antigua = busqueda_tabla_arp(tabla_arp, entrada_arp->dir_ip.dir_ip);
	if(entrada_arp_antigua) {
		eliminar_entrada_tabla_arp(tabla_arp, entrada_arp->dir_ip.dir_ip);
	}
	return insertar(tabla_arp->entradas_arp, &entrada_arp, sizeof(entrada_arp_t *));
}

void eliminar_entrada_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		if(!strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP)) {
			eliminar(tabla_arp->entradas_arp, nodo_actual);
		}
	} FIN_ITERACION;
}

void actualizar_tabla_arp(tabla_arp_t *tabla_arp, cab_arp_t *cab_arp, interface_t *interface_entrada) {
	entrada_arp_t *entrada_arp = calloc(1, sizeof(entrada_arp_t));
	uint32_t dir_ip = cab_arp->ip_origen;
	dir_ip = htonl(dir_ip);
	char dir_ip_arp[TAM_DIR_IP];
	inet_ntop(AF_INET, &dir_ip, dir_ip_arp, TAM_DIR_IP);
	dir_ip_arp[TAM_DIR_IP - 1] = '\0';
	memcpy(entrada_arp->dir_ip.dir_ip, dir_ip_arp, TAM_DIR_IP);
	memcpy(entrada_arp->dir_mac.dir_mac, cab_arp->mac_origen.dir_mac, TAM_DIR_MAC);
	strncpy(entrada_arp->nombre_if, interface_entrada->nombre_if, TAM_NOMBRE_IF);
	agregar_entrada_tabla_arp(tabla_arp, entrada_arp);
}

void mostrar_tabla_arp(tabla_arp_t *tabla_arp) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		printf("IP: %s, MAC:%u%u%u%u%u%u, INTF: %s\n",
			entrada_arp->dir_ip.dir_ip,
			entrada_arp->dir_mac.dir_mac[0],
			entrada_arp->dir_mac.dir_mac[1],
			entrada_arp->dir_mac.dir_mac[2],
			entrada_arp->dir_mac.dir_mac[3],
			entrada_arp->dir_mac.dir_mac[4],
			entrada_arp->dir_mac.dir_mac[5],
			entrada_arp->nombre_if);
	} FIN_ITERACION;
}

void enviar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_salida, char *dir_ip) {
	unsigned int tamano_payload = sizeof(cab_arp_t);
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) calloc(1, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload);
	if(!intf_salida) {
		intf_salida = obtener_intf_correspondiente_a_nodo(nodo, dir_ip);
		if(!intf_salida) {
			printf("Error: %s: ninguna subred apropiada para la resolución ARP de la dirección ip %s\n", nodo->nombre_nodo, dir_ip);
			return;
		}
	}
	/*PASO 1: preparar cabecera ethernet*/
	capa2_llenar_con_mac_broadcast(cab_ethernet->mac_destino.dir_mac);
	memcpy(cab_ethernet->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));
	cab_ethernet->tipo = MENSAJE_ARP;
	/*PASO 2: preparar mensaje broadcast ARP*/
	cab_arp_t *cab_arp = (cab_arp_t *) cab_ethernet->payload;
	cab_arp->tipo_hw = 1;
	cab_arp->tipo_proto = 0x0800;
	cab_arp->lon_dir_hw = sizeof(dir_mac_t);
	cab_arp->lon_dir_proto = 4;
	cab_arp->cod_op = SOLIC_BROAD_ARP;
	memcpy(cab_ethernet->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));

	inet_pton(AF_INET, IP_IF(intf_salida), &cab_arp->ip_origen);
	cab_arp->ip_origen = htonl(cab_arp->ip_origen);
	memset(cab_arp->mac_destino.dir_mac, 0, sizeof(dir_mac_t));

	/*********PENDIENTE: ¿uint32_t?**********/
	inet_pton(AF_INET, dir_ip, &cab_arp->ip_destino);
	cab_arp->ip_destino = htonl(cab_arp->ip_destino);

	FCS_ETH(cab_ethernet, sizeof(cab_ethernet_t)) = 0;
	/*****PENDIENTE: cab_ethernet a char**********/
	enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);
	/******PENDIENTE: revisar las demás solicitudes de memoria******/
	free(cab_ethernet);
}
