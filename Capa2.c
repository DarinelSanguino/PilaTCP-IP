#include "Capa2.h"

void conf_intf_modo_l2(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf) {
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	if(!interface) {
		printf("Error: no se encontró la interface %s.\n", nombre_if);
		return;
	}
	/*****PENDIENTE: considerar los estados en los que se puede encontrar la interface.******/
	interface->prop_intf->modo_l2_intf = modo_l2_intf;
}

void mover_paq_a_capa3(nodo_t *nodo_rec, interface_t *interface, char *paquete, size_t tamano_paq) {
	return;
}

void recibir_trama_capa2(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq) {
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	if(recibir_trama_l2_en_interface(interface, cab_ethernet) == false) {
		printf("Trama L2 rechazada.\n");
		return;
	}
	printf("Trama L2 aceptada.\n");

	if(IF_EN_MODO_L3(interface)) {
		switch(cab_ethernet->tipo) {
			case MENSAJE_ARP:
				{
					cab_arp_t *cab_arp = (cab_arp_t *) cab_ethernet->payload;
					switch(cab_arp->cod_op) {
						case SOLIC_BROAD_ARP:
							procesar_solicitud_broadcast_arp(nodo_rec, interface, cab_ethernet);
							break;
						case RESPUESTA_ARP:
							procesar_mensaje_respuesta_arp(nodo_rec, interface, cab_ethernet);
							break;
						default:
							printf("No se identificó a qué tipo de mensaje ARP pertenece.\n");
							break;
					}
					break;
				}
			default:
				printf("No se identificó como mensaje ARP.\n");
				mover_paq_a_capa3(nodo_rec, interface, paquete, tamano_paq);
				break;
		}
	}
	else if(MODO_L2_INTF(interface) == ACCESO || MODO_L2_INTF(interface) == TRONCAL) {
		recibir_trama_switch_capa2(interface, paquete, tamano_paq);
	}	
}

void inic_tabla_arp(tabla_arp_t **tabla_arp) {
	*tabla_arp = malloc(sizeof(tabla_arp_t));
	(*tabla_arp)->entradas_arp = malloc(sizeof(Lista_t));
	(*tabla_arp)->entradas_arp->vacia = true;
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
	printf("%s\n", __FUNCTION__);
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);		
		printf("IP: %s, MAC:%u-%u-%u-%u-%u-%u, INTF: %s\n",
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
		if(strncmp(dir_ip, "20.1.1.2", 8) == 0) {
			printf("Detener aquí.\n");
		}
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
	memcpy(cab_arp->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));

	inet_pton(AF_INET, IP_IF(intf_salida), &cab_arp->ip_origen);
	cab_arp->ip_origen = htonl(cab_arp->ip_origen);
	capa2_llenar_con_mac_broadcast(cab_arp->mac_destino.dir_mac);

	/*********PENDIENTE: ¿uint32_t?**********/
	inet_pton(AF_INET, dir_ip, &cab_arp->ip_destino);
	cab_arp->ip_destino = htonl(cab_arp->ip_destino);

	FCS_ETH(cab_ethernet, sizeof(cab_ethernet_t)) = 0;
	/*****PENDIENTE: cab_ethernet a char**********/
	enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);
	/******PENDIENTE: revisar las demás solicitudes de memoria******/
	free(cab_ethernet);
}

void inic_tabla_mac(tabla_mac_t **tabla_mac) {
	*tabla_mac = malloc(sizeof(tabla_mac_t));
	(*tabla_mac)->entradas_mac = malloc(sizeof(Lista_t));
	(*tabla_mac)->entradas_mac->vacia = true;
}

entrada_mac_t * busqueda_tabla_mac(tabla_mac_t *tabla_mac, char *dir_mac) {
	ITERAR_LISTA_ENLAZADA(tabla_mac->entradas_mac) {
		entrada_mac_t *entrada_mac = *(entrada_mac_t **)(nodo_actual->elemento);
		if(!strncmp(entrada_mac->dir_mac.dir_mac, dir_mac, TAM_DIR_MAC)) {
			return entrada_mac;
		}
	} FIN_ITERACION;
	return NULL;
}

bool agregar_entrada_tabla_mac(tabla_mac_t *tabla_mac, entrada_mac_t *entrada_mac) {
	entrada_mac_t *entrada_mac_antigua = busqueda_tabla_mac(tabla_mac, entrada_mac->dir_mac.dir_mac);
	if(entrada_mac_antigua) {
		eliminar_entrada_tabla_mac(tabla_mac, entrada_mac->dir_mac.dir_mac);
	}
	return insertar(tabla_mac->entradas_mac, &entrada_mac, sizeof(entrada_mac_t *));
}

void eliminar_entrada_tabla_mac(tabla_mac_t *tabla_mac, char *dir_mac) {
	ITERAR_LISTA_ENLAZADA(tabla_mac->entradas_mac) {
		entrada_mac_t *entrada_mac = *(entrada_mac_t **)(nodo_actual->elemento);
		if(!strncmp(entrada_mac->dir_mac.dir_mac, dir_mac, TAM_DIR_MAC)) {
			eliminar(tabla_mac->entradas_mac, nodo_actual);
		}
	} FIN_ITERACION;
}

void mostrar_tabla_mac(tabla_mac_t *tabla_mac) {	
	ITERAR_LISTA_ENLAZADA(tabla_mac->entradas_mac) {
		entrada_mac_t *entrada_mac = *(entrada_mac_t **)(nodo_actual->elemento);		
		printf("MAC:%u-%u-%u-%u-%u-%u, INTF: %s\n",
			entrada_mac->dir_mac.dir_mac[0],
			entrada_mac->dir_mac.dir_mac[1],
			entrada_mac->dir_mac.dir_mac[2],
			entrada_mac->dir_mac.dir_mac[3],
			entrada_mac->dir_mac.dir_mac[4],
			entrada_mac->dir_mac.dir_mac[5],
			entrada_mac->nombre_if);
	} FIN_ITERACION;
}
