#include "Capa2.h"

extern int enviar_paquete(char *paquete, unsigned int tamano_paq, interface_t *intf_origen);
extern void recibir_trama_switch_capa2(interface_t *interface, char *paquete, unsigned int tamano_paq, bool etiqueta_vlan);
extern bool nodo_es_destino(nodo_t *nodo, char *ip_destino);
extern void subir_paquete_a_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo);

static inline unsigned int OBTENER_ID_VLAN_8021Q(cab_vlan_8021q_t *cab_vlan_8021q) {
	return (unsigned int) (cab_vlan_8021q->VID);
}

static char * OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_t *cab_ethernet) {
	if(paquete_tiene_etiqueta_vlan(cab_ethernet)) {
		return ((cab_ethernet_vlan_t *) cab_ethernet)->payload;
	}
	else {
		return cab_ethernet->payload;
	}
}

static inline void ASIGNAR_FCS_ETH(cab_ethernet_t *cab_ethernet, unsigned int tamano_payload, unsigned int FCS) {
	if(paquete_tiene_etiqueta_vlan(cab_ethernet)) {
		FCS_ETH_VLAN(cab_ethernet, tamano_payload) = FCS;
	}
	else {
		FCS_ETH(cab_ethernet, tamano_payload) = FCS;
	}
}

static cab_ethernet_t * ASIGNAR_PAYLOAD_CAB_ETHERNET(char *paquete, unsigned int tamano_paq) {
	char temp[tamano_paq];
	memcpy(temp, paquete, tamano_paq);
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *)(paquete - TAM_CAB_ETH_EXC_PAYLOAD);
	memset(cab_ethernet, 0, sizeof(cab_ethernet_t));
	memcpy(cab_ethernet->payload, temp, tamano_paq);
	return cab_ethernet;
}

static inline unsigned int OBTENER_TAM_DE_CAB_ETHERNET_EXC_PAYLOAD(cab_ethernet_t *cab_ethernet) {
	if(paquete_tiene_etiqueta_vlan(cab_ethernet)) {
		return TAM_CAB_ETH_VLAN_EXC_PAYLOAD;
	}
	return TAM_CAB_ETH_EXC_PAYLOAD;
}

static void enviar_mensaje_arp_respuesta(cab_ethernet_t *cab_ethernet_entrada, interface_t *intf_salida) {
	unsigned int tamano_payload = sizeof(cab_arp_t);
	cab_arp_t *cab_arp_entrada = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_entrada);
	cab_ethernet_t *cab_ethernet_resp = calloc(1, sizeof(cab_ethernet_t));
	memcpy(cab_ethernet_resp->mac_destino.dir_mac, cab_arp_entrada->mac_origen.dir_mac, TAM_DIR_MAC);
	memcpy(cab_ethernet_resp->mac_origen.dir_mac, MAC_IF(intf_salida), TAM_DIR_MAC);
	cab_ethernet_resp->tipo = MENSAJE_ARP;
	cab_arp_t *cab_arp_resp = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_resp);
	cab_arp_resp->tipo_hw = 1;
	cab_arp_resp->tipo_proto = IPv4;
	cab_arp_resp->lon_dir_hw = sizeof(dir_mac_t);
	cab_arp_resp->lon_dir_proto = 4;
	cab_arp_resp->cod_op = RESPUESTA_ARP;
	memcpy(cab_arp_resp->mac_origen.dir_mac, MAC_IF(intf_salida), TAM_DIR_MAC);
	
	cab_arp_resp->ip_origen = cab_arp_entrada->ip_destino;
	memcpy(cab_arp_resp->mac_destino.dir_mac, cab_arp_entrada->mac_origen.dir_mac, TAM_DIR_MAC);
	cab_arp_resp->ip_destino = cab_arp_entrada->ip_origen;
	FCS_ETH(cab_ethernet_resp, sizeof(cab_arp_t)) = 0;
	//printf("Tamaño del paquete enviado desde %s: %lu bytes.\n", intf_salida->nodo_padre->nombre_nodo, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload);
	enviar_paquete((char *) cab_ethernet_resp, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);
	free(cab_ethernet_resp);
}

static void procesar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_entrada, cab_ethernet_t *cab_ethernet) {
	printf("Mensaje broadcast ARP recibido en la interface %s del nodo %s.\n", intf_entrada->nombre_if, nodo->nombre_nodo);
	cab_arp_t *cab_arp = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet);
	uint32_t ip_destino_arp = cab_arp->ip_destino;
	char ip_destino[TAM_DIR_IP];
	memset(ip_destino, 0, TAM_DIR_IP);
	inet_ntop(AF_INET, &ip_destino_arp, ip_destino, TAM_DIR_IP);
	ip_destino[TAM_DIR_IP - 1] = '\0';
	actualizar_tabla_arp(TABLA_ARP_NODO(nodo), cab_arp, intf_entrada);
	if(strncmp(intf_entrada->prop_intf->dir_ip.dir_ip, ip_destino, TAM_DIR_IP) != 0) {
		printf("%s: solicitud broadcast ARP descartada. La IP de destino %s no coincide con la IP %s de la interface.\n",
			nodo->nombre_nodo, ip_destino, IP_IF(intf_entrada));
		return;
	}
	enviar_mensaje_arp_respuesta(cab_ethernet, intf_entrada);
}

static void procesar_mensaje_respuesta_arp(nodo_t *nodo, interface_t *intf_entrada, cab_ethernet_t *cab_ethernet) {
	printf("Mensaje ARP de respuesta recibido en la interface %s del nodo %s.\n", intf_entrada->nombre_if, nodo->nombre_nodo);
	completar_entrada_tabla_arp(TABLA_ARP_NODO(nodo), (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet), intf_entrada);
}

static void _procesamiento_arp_pendiente(nodo_t *nodo, interface_t *intf_salida, entrada_arp_t *entrada_arp, entrada_arp_pendiente_t *entrada_arp_pendiente) {
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *)(entrada_arp_pendiente->paquete);	
	memcpy(cab_ethernet->mac_destino.dir_mac, entrada_arp->dir_mac.dir_mac, sizeof(dir_mac_t));
	memcpy(cab_ethernet->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));
	enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + entrada_arp_pendiente->tamano_paq, intf_salida);
	free(cab_ethernet);
}

static void procesamiento_arp_pendiente(nodo_t *nodo, interface_t *intf_salida, entrada_arp_t *entrada_arp) {
	ITERAR_LISTA_ENLAZADA(entrada_arp->lista_paq_arp_pendientes) {
		entrada_arp_pendiente_t *entrada_arp_pendiente = *(entrada_arp_pendiente_t **)(nodo_actual->elemento);
		_procesamiento_arp_pendiente(nodo, intf_salida, entrada_arp, entrada_arp_pendiente);		
	} FIN_ITERACION;
	limpiar(entrada_arp->lista_paq_arp_pendientes);
}

void conf_intf_modo_l2(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf) {
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	if(!interface) {
		printf("Error: no se encontró la interface %s en el nodo %s.\n", nombre_if, nodo->nombre_nodo);
		return;
	}	
	interface->prop_intf->modo_l2_intf = modo_l2_intf;
}

void bajar_paquete_a_capa2(nodo_t *nodo, char *intf_salida, uint32_t ip_gw, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo) {
	switch(num_protocolo) {
		case IPv4:
			recibir_paquete_ip_en_capa2(nodo, intf_salida, ip_gw, paquete, tamano_paq);
			break;
		default:
			break;
	}	
}

bool rellenar_cab_ethernet(nodo_t *nodo, interface_t *intf_salida, char *ip_destino, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq) {
	entrada_arp_t *entrada_arp = busqueda_tabla_arp(TABLA_ARP_NODO(nodo), ip_destino);
	if(!entrada_arp) {
		entrada_arp = crear_entrada_arp(TABLA_ARP_NODO(nodo), ip_destino, NULL, intf_salida);
		if(entrada_arp) {
			agregar_paquete_lista_arp_pendiente(entrada_arp, (char *)cab_ethernet, tamano_paq);
		}
		enviar_solicitud_broadcast_arp(nodo, intf_salida, ip_destino);
		return false;
	}
	if(!entrada_arp->esta_completa) {
		agregar_paquete_lista_arp_pendiente(entrada_arp, (char *)cab_ethernet, tamano_paq);
		return false;
	}
	memcpy(cab_ethernet->mac_destino.dir_mac, entrada_arp->dir_mac.dir_mac, sizeof(dir_mac_t));
	memcpy(cab_ethernet->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));	
	return true;
}

void recibir_paquete_ip_en_capa2(nodo_t *nodo, char *intf_salida, uint32_t ip_destino, char *paquete, unsigned int tamano_paq) {
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) calloc(1, TAM_CAB_ETH_EXC_PAYLOAD + tamano_paq);
	char dir_ip[TAM_DIR_IP];
	memset(dir_ip, 0, TAM_DIR_IP);
	inet_ntop(AF_INET, &ip_destino, dir_ip, TAM_DIR_IP);

	memcpy(cab_ethernet->payload, paquete, tamano_paq);
	cab_ethernet->tipo = IPv4;

	if(intf_salida[0] != '\0') {
		interface_t *interface_salida = obtener_intf_por_nombre(nodo, intf_salida);
		if(interface_salida) {
			if(rellenar_cab_ethernet(nodo, interface_salida, dir_ip, cab_ethernet, tamano_paq)) {
				enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_paq, interface_salida);
				free(cab_ethernet);
			}
		}
		else {
			printf("Error: ninguna subred apropiada para el envío del paquete a %s desde el nodo %s.\n", dir_ip, nodo->nombre_nodo);
		}
	}
	else {
		if(nodo_es_destino(nodo, dir_ip)) {
			subir_paquete_a_capa3(nodo, NULL, OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet), tamano_paq, IPv4);
		}
		else {
			interface_t *interface_salida = obtener_intf_correspondiente_a_nodo(nodo, dir_ip);
			if(interface_salida) {
				if(rellenar_cab_ethernet(nodo, interface_salida, dir_ip, cab_ethernet, tamano_paq)) {
					enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_paq, interface_salida);
					free(cab_ethernet);
				}
			}
			else {
				printf("Error: ninguna subred apropiada para el envío del paquete a %s desde el nodo %s.\n", dir_ip, nodo->nombre_nodo);
			}
		}
	}
}

bool recibir_trama_capa2_en_interface(interface_t *interface, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq, bool *etiqueta_vlan) {
	unsigned char *mac = MAC_IF(interface);
	cab_vlan_8021q_t *cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(IF_EN_MODO_L3(interface)) {
		unsigned char *mac_destino = cab_ethernet->mac_destino.dir_mac;
		//mostrar_dir_mac(&interface->prop_intf->dir_mac);
		//mostrar_dir_mac(&cab_ethernet->mac_destino);
		if(cab_vlan_8021q) return false;
		if(memcmp(mac, mac_destino, TAM_DIR_MAC) == 0 || ES_DIR_MAC_BROADCAST(mac_destino)) return true;
	}
	if(MODO_L2_IF(interface) == ACCESO) {
		unsigned int vlan = interface->prop_intf->vlans[0];
		if(vlan == -1) {
			printf("Error: la interface %s está configurada en modo ACCESO pero no tiene una VLAN asignada.\n", interface->nombre_if);
			return false;
		}
		if(cab_vlan_8021q) {
			printf("Error: paquete con etiqueta VLAN recibido en interface %s configurada en modo ACCESO.\n", interface->nombre_if);
			assert(0);			
		}
		else {
			*etiqueta_vlan = true;
			return true;			
		}
	}
	if(MODO_L2_IF(interface) == TRONCAL) {		
		if(cab_vlan_8021q) {
			return vlan_esta_asignada_a_interface_troncal(interface, (short) cab_vlan_8021q->VID);
		}
		else {
			printf("Error: paquete sin etiqueta VLAN recibido en interface %s configurada en modo TRONCAL.\n", interface->nombre_if);
			return false;
		}		
	}	
	return false;
}

void recibir_trama_capa2(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq) {
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	bool etiqueta_vlan = false;
	if(recibir_trama_capa2_en_interface(interface, cab_ethernet, tamano_paq, &etiqueta_vlan) == false) {
		printf("Trama L2 rechazada.\n");
		return;
	}
	//printf("Trama L2 aceptada.\n");

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
							printf("No se identificó el tipo de mensaje ARP recibido en %s.\n", nodo_rec->nombre_nodo);
							break;
					}
					break;
				}
			case IPv4:
				subir_paquete_a_capa3(nodo_rec, interface, OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet), tamano_paq - OBTENER_TAM_DE_CAB_ETHERNET_EXC_PAYLOAD(cab_ethernet), cab_ethernet->tipo);
				break;
			default:
				printf("No se identificó el tipo de mensaje recibido en %s.\n", nodo_rec->nombre_nodo);
				break;
		}
	}
	else if(MODO_L2_IF(interface) == ACCESO || MODO_L2_IF(interface) == TRONCAL) {		
		recibir_trama_switch_capa2(interface, (char *) cab_ethernet, tamano_paq, etiqueta_vlan);
	}
}

cab_vlan_8021q_t * paquete_tiene_etiqueta_vlan(cab_ethernet_t *cab_ethernet) {
	if((unsigned short) cab_ethernet->tipo == VLAN_8021Q) {
		cab_ethernet_vlan_t *cab_ethernet_vlan = (cab_ethernet_vlan_t *) cab_ethernet;
		cab_vlan_8021q_t *cab_vlan_8021q = &cab_ethernet_vlan->cab_vlan_8021q;
		return cab_vlan_8021q;
	}
	return NULL;
}

void inic_tabla_arp(tabla_arp_t **tabla_arp) {
	*tabla_arp = malloc(sizeof(tabla_arp_t));
	(*tabla_arp)->entradas_arp = malloc(sizeof(Lista_t));
	(*tabla_arp)->entradas_arp->vacia = true;
}

static void inic_entrada_arp(entrada_arp_t *entrada_arp) {
	entrada_arp->lista_paq_arp_pendientes = calloc(1, sizeof(Lista_t));
	entrada_arp->lista_paq_arp_pendientes->vacia = true;
	memset(entrada_arp->dir_ip.dir_ip, 0, TAM_DIR_IP);
	memset(entrada_arp->dir_mac.dir_mac, 0, TAM_DIR_MAC);
	memset(entrada_arp->nombre_if, 0, TAM_NOMBRE_IF);
	entrada_arp->esta_completa = false;
}

entrada_arp_t * busqueda_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		if(strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP) == 0) {
			return entrada_arp;
		}
	} FIN_ITERACION;
	return NULL;
}

bool agregar_entrada_tabla_arp(tabla_arp_t *tabla_arp, entrada_arp_t *entrada_arp, interface_t *interface) {
	entrada_arp_t *entrada_arp_antigua = busqueda_tabla_arp(tabla_arp, entrada_arp->dir_ip.dir_ip);
	if(entrada_arp_antigua) {
		if(!entrada_arp_antigua->esta_completa) {
			procesamiento_arp_pendiente(interface->nodo_padre, interface, entrada_arp_antigua);
		}
		eliminar_entrada_tabla_arp(tabla_arp, entrada_arp->dir_ip.dir_ip);
	}
	return insertar(tabla_arp->entradas_arp, &entrada_arp, sizeof(entrada_arp_t *));
}

void completar_entrada_tabla_arp(tabla_arp_t *tabla_arp, cab_arp_t *cab_arp, interface_t *interface) {
	uint32_t dir_ip = cab_arp->ip_origen;
	char dir_ip_arp[TAM_DIR_IP];
	inet_ntop(AF_INET, &dir_ip, dir_ip_arp, TAM_DIR_IP);
	dir_ip_arp[TAM_DIR_IP - 1] = '\0';
	entrada_arp_t *entrada_arp_antigua = busqueda_tabla_arp(tabla_arp, dir_ip_arp);
	if(entrada_arp_antigua) {
		if(!entrada_arp_antigua->esta_completa) {
			memcpy(entrada_arp_antigua->dir_mac.dir_mac, cab_arp->mac_origen.dir_mac, TAM_DIR_MAC);	
			strncpy(entrada_arp_antigua->nombre_if, interface->nombre_if, TAM_NOMBRE_IF);
			entrada_arp_antigua->esta_completa = true;
			procesamiento_arp_pendiente(interface->nodo_padre, interface, entrada_arp_antigua);
		}
	}
	else {
		actualizar_tabla_arp(tabla_arp, cab_arp, interface);
	}
}

void eliminar_entrada_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		if(strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP) == 0) {
			eliminar(tabla_arp->entradas_arp, nodo_actual);
			break;
		}
	} FIN_ITERACION;
}

entrada_arp_t * crear_entrada_arp(tabla_arp_t *tabla_arp, char *dir_ip, unsigned char *dir_mac, interface_t *interface) {
	entrada_arp_t *entrada_arp = calloc(1, sizeof(entrada_arp_t));
	inic_entrada_arp(entrada_arp);
	if(!entrada_arp) {
		printf("Error: la entrada ARP no pudo ser creada.\n");
		return NULL;
	}
	memcpy(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP);
	if(dir_mac) {
		memcpy(entrada_arp->dir_mac.dir_mac, dir_mac, TAM_DIR_MAC);
		entrada_arp->esta_completa = true;
	}
	else entrada_arp->esta_completa = false;
	strncpy(entrada_arp->nombre_if, interface->nombre_if, TAM_NOMBRE_IF);
	if(!agregar_entrada_tabla_arp(tabla_arp, entrada_arp, interface)) {
		printf("Error: la entrada no pudo agregarse a la tabla ARP del nodo %s.\n", interface->nodo_padre->nombre_nodo);
		return NULL;
	}
	return entrada_arp;
}

void actualizar_tabla_arp(tabla_arp_t *tabla_arp, cab_arp_t *cab_arp, interface_t *interface_entrada) {	
	uint32_t dir_ip = cab_arp->ip_origen;
	char dir_ip_arp[TAM_DIR_IP];
	inet_ntop(AF_INET, &dir_ip, dir_ip_arp, TAM_DIR_IP);
	dir_ip_arp[TAM_DIR_IP - 1] = '\0';	
	crear_entrada_arp(tabla_arp, dir_ip_arp, cab_arp->mac_origen.dir_mac, interface_entrada);
}

void mostrar_tabla_arp(tabla_arp_t *tabla_arp) {	
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

void agregar_paquete_lista_arp_pendiente(entrada_arp_t *entrada_arp, char *paquete, unsigned int tamano_paq) {
	entrada_arp_pendiente_t *entrada_arp_pendiente = calloc(1, sizeof(entrada_arp_pendiente_t));
	entrada_arp_pendiente->tamano_paq = tamano_paq;
	entrada_arp_pendiente->paquete = paquete;
	insertar(entrada_arp->lista_paq_arp_pendientes, &entrada_arp_pendiente, sizeof(entrada_arp_pendiente_t *));
}

void enviar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_salida, char *dir_ip) {
	unsigned int tamano_payload = sizeof(cab_arp_t);
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) calloc(1, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload);
	if(!intf_salida) {
		intf_salida = obtener_intf_correspondiente_a_nodo(nodo, dir_ip);
		if(!intf_salida) {
			printf("Error: ninguna subred apropiada en %s para la resolución ARP de la dirección IP %s.\n", nodo->nombre_nodo, dir_ip);
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
	cab_arp->tipo_proto = IPv4;
	cab_arp->lon_dir_hw = sizeof(dir_mac_t);
	cab_arp->lon_dir_proto = 4;
	cab_arp->cod_op = SOLIC_BROAD_ARP;
	memcpy(cab_arp->mac_origen.dir_mac, MAC_IF(intf_salida), sizeof(dir_mac_t));

	inet_pton(AF_INET, IP_IF(intf_salida), &cab_arp->ip_origen);
	capa2_llenar_con_mac_broadcast(cab_arp->mac_destino.dir_mac);
	inet_pton(AF_INET, dir_ip, &cab_arp->ip_destino);
	
	FCS_ETH(cab_ethernet, sizeof(cab_ethernet_t)) = 0;
	//printf("Tamaño del paquete enviado desde %s: %lu bytes.\n", intf_salida->nodo_padre->nombre_nodo, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload);

	enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);	
	free(cab_ethernet);
}

void inic_tabla_mac(tabla_mac_t **tabla_mac) {
	*tabla_mac = malloc(sizeof(tabla_mac_t));
	(*tabla_mac)->entradas_mac = malloc(sizeof(Lista_t));
	(*tabla_mac)->entradas_mac->vacia = true;
}

entrada_mac_t * busqueda_tabla_mac(tabla_mac_t *tabla_mac, unsigned char *dir_mac) {
	ITERAR_LISTA_ENLAZADA(tabla_mac->entradas_mac) {
		entrada_mac_t *entrada_mac = *(entrada_mac_t **)(nodo_actual->elemento);
		if(memcmp(entrada_mac->dir_mac.dir_mac, dir_mac, TAM_DIR_MAC) == 0) {
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

void eliminar_entrada_tabla_mac(tabla_mac_t *tabla_mac, unsigned char *dir_mac) {
	ITERAR_LISTA_ENLAZADA(tabla_mac->entradas_mac) {
		entrada_mac_t *entrada_mac = *(entrada_mac_t **)(nodo_actual->elemento);
		if(memcmp(entrada_mac->dir_mac.dir_mac, dir_mac, TAM_DIR_MAC) == 0) {			
			eliminar(tabla_mac->entradas_mac, nodo_actual);
			break;
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

cab_ethernet_t * etiquetar_paquete_con_id_vlan(cab_ethernet_t *cab_ethernet, int id_vlan, unsigned int *tamano_paq) {
	cab_vlan_8021q_t *cab_vlan_8021q = NULL;
	cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(cab_vlan_8021q) {
		cab_vlan_8021q->VID = (short) id_vlan;
		return cab_ethernet;
	}

	cab_ethernet_t *cab_ethernet_temporal = calloc(1, sizeof(cab_ethernet_t));
	memcpy(cab_ethernet_temporal, cab_ethernet, *tamano_paq);
	cab_ethernet_vlan_t *cab_ethernet_vlan = (cab_ethernet_vlan_t *)((char *) cab_ethernet - sizeof(cab_vlan_8021q_t));
	memset(cab_ethernet_vlan, 0, TAM_CAB_ETH_VLAN_EXC_PAYLOAD - sizeof(unsigned int));
	memcpy(cab_ethernet_vlan->mac_destino.dir_mac, cab_ethernet_temporal->mac_destino.dir_mac, TAM_DIR_MAC);
	memcpy(cab_ethernet_vlan->mac_origen.dir_mac, cab_ethernet_temporal->mac_origen.dir_mac, TAM_DIR_MAC);
	cab_ethernet_vlan->tipo = cab_ethernet_temporal->tipo;
	cab_ethernet_vlan->cab_vlan_8021q.TPID = VLAN_8021Q;
	cab_ethernet_vlan->cab_vlan_8021q.PRI = 0;
	cab_ethernet_vlan->cab_vlan_8021q.CFI = 0;
	cab_ethernet_vlan->cab_vlan_8021q.VID = (short) id_vlan;
	
	*tamano_paq = *tamano_paq + sizeof(cab_vlan_8021q_t);
	
	free(cab_ethernet_temporal);
	return (cab_ethernet_t *)cab_ethernet_vlan;
}

cab_ethernet_t * quitar_etiqueta_paquete_con_id_vlan(cab_ethernet_t *cab_ethernet, unsigned int *tamano_paq) {
	cab_vlan_8021q_t *cab_vlan_8021q = NULL;
	cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(!cab_vlan_8021q) {		
		return cab_ethernet;
	}

	cab_ethernet_vlan_t *cab_ethernet_vlan_temporal = calloc(1, sizeof(cab_ethernet_vlan_t));
	memcpy(cab_ethernet_vlan_temporal, cab_ethernet, *tamano_paq);
	
	memset(cab_ethernet, 0, TAM_CAB_ETH_VLAN_EXC_PAYLOAD - sizeof(cab_ethernet->tipo) - sizeof(cab_ethernet->FCS));
	cab_ethernet = (cab_ethernet_t *)((char *)cab_ethernet + sizeof(cab_vlan_8021q_t));
	memcpy(cab_ethernet->mac_destino.dir_mac, cab_ethernet_vlan_temporal->mac_destino.dir_mac, TAM_DIR_MAC);
	memcpy(cab_ethernet->mac_origen.dir_mac, cab_ethernet_vlan_temporal->mac_origen.dir_mac, TAM_DIR_MAC);
	
	*tamano_paq = *tamano_paq - sizeof(cab_vlan_8021q_t);
	free(cab_ethernet_vlan_temporal);
	return cab_ethernet;
}

void asignar_modo_l2_nodo(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf) {	
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	if(MODO_L2_IF(interface) == modo_l2_intf) {
		printf("La interface ya se encuentra configurada en modo %s.\n", cadena_modo_l2_intf(modo_l2_intf));
		return;
	}

	for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
		interface->prop_intf->vlans[i] = -1;
	}

	if(IF_EN_MODO_L3(interface)) {
		interface->prop_intf->tiene_dir_ip = false;
		interface->prop_intf->mascara = 0;
		memset(IP_IF(interface), 0, TAM_DIR_IP);
		MODO_L2_IF(interface) = modo_l2_intf;
		return;
	}
	if(modo_l2_intf == TRONCAL) {		
		MODO_L2_IF(interface) = TRONCAL;	
		return;
	}
	if(modo_l2_intf == ACCESO) {		
		MODO_L2_IF(interface) = ACCESO;
		return;
	}
	if(modo_l2_intf == DESCONOCIDO) {	
		MODO_L2_IF(interface) = DESCONOCIDO;
	}
}

void asignar_id_vlan_nodo(nodo_t *nodo, char *nombre_if, unsigned int id_vlan) {
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	if(IF_EN_MODO_L3(interface)) {
		printf("Error: la interface no está configurada en modo L2.\n");
		return;
	}
	if(MODO_L2_IF(interface) == TRONCAL) {
		unsigned int vlan;
		for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
			vlan = interface->prop_intf->vlans[i];
			if(vlan == -1) {
				VLAN_IF(interface, i) = id_vlan;
				return;
			}
		}
		printf("Se ha alcanzado el máximo número de VLANs al que la interface puede pertenecer.\n");
		return;
	}
	if(MODO_L2_IF(interface) == ACCESO) {
		VLAN_IF(interface, 0) = id_vlan;
		return;
	}
	if(MODO_L2_IF(interface) == DESCONOCIDO) {
		printf("La interface no se ha configurado en modo TRONCAL o ACCESO.\n");
	}
}

void mostrar_prop_vlan(modo_l2_intf_t modo_l2_intf, const unsigned int *vlans) {
	switch(modo_l2_intf) {
		case TRONCAL:
			printf("Modo troncal. VLANs asignadas: ");
			unsigned int vlan;
			for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
				vlan = vlans[i];
				if(vlan == -1) {
					printf("\n");		
					return;
				}
				else {
					printf("%u ", vlan);
				}
			}
			break;
		case ACCESO:
			printf("Modo acceso.");			
			printf("VLAN asignada: %u.\n", vlans[0]);
			break;
		case DESCONOCIDO:
			printf("La interface no se ha configurado en modo TRONCAL o ACCESO.\n");
			break;
	}
}

unsigned int obtener_id_vlan_intf_acceso(interface_t *interface) {
	if(MODO_L2_IF(interface) == ACCESO) {
		return VLAN_IF(interface, 0);
	}
	return -1;
}

bool vlan_esta_asignada_a_interface_troncal(interface_t *interface, unsigned int vlan) {
	if(MODO_L2_IF(interface) == TRONCAL) {
		unsigned int *vlans = VLANS_IF(interface);
		for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
			if(vlans[i] == -1) break;
			else if(vlans[i] == vlan) return true;
		}
		printf("Error: la interface %s configurada como TRONCAL no tiene la VLAN %u asignada.\n", interface->nombre_if, vlan);
		return false;
	}
	else {
		printf("Error: la interface %s no está configurada en modo TRONCAL.\n", interface->nombre_if);
		return false;
	}	
}
