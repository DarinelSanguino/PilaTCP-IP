#include "Capa2.h"

extern void recibir_trama_switch_capa2(interface_t *interface, char *paquete, unsigned int tamano_paq, bool etiqueta_vlan);

static inline unsigned int OBTENER_ID_VLAN_8021Q(cab_vlan_8021q_t *cab_vlan_8021q) {
	return (unsigned int) (cab_vlan_8021q->VID);
}

static inline char * OBTENER_PAYLOAD_CAB_ETHERNET(cab_ethernet_t *cab_ethernet) {
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

static inline cab_ethernet_t * ASIGNAR_PAYLOAD_CAB_ETHERNET(char *paquete, unsigned int tamano_paq) {
	char temp[tamano_paq];
	memcpy(temp, paquete, tamano_paq);
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t*)(paquete - TAM_CAB_ETH_EXC_PAYLOAD);
	memset(cab_ethernet, 0, sizeof(cab_ethernet_t));
	memcpy(cab_ethernet->payload, temp, tamano_paq);
	return cab_ethernet;
}

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

bool recibir_trama_capa2_en_interface(interface_t *interface, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq, bool *etiqueta_vlan) {
	/***********PENDIENTE: verificar si el nombre es adecuado.*************/
	unsigned char *mac = MAC_IF(interface);
	cab_vlan_8021q_t *cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(IF_EN_MODO_L3(interface)) {
		unsigned char *mac_destino = cab_ethernet->mac_destino.dir_mac;
		mostrar_dir_mac(&interface->prop_intf->dir_mac);
		mostrar_dir_mac(&cab_ethernet->mac_destino);
		if(cab_vlan_8021q) return false;
		if(memcmp(mac, mac_destino, TAM_DIR_MAC) == 0 || ES_DIR_MAC_BROADCAST(mac_destino)) return true;
	}
	if(MODO_L2_IF(interface) == ACCESO) {
		unsigned int vlan = interface->prop_intf->vlans[0];
		if(cab_vlan_8021q) {
			if(vlan == -1) return false;
			else {
				if(vlan == (short) cab_vlan_8021q->VID) return true;
				else return false;
			}
		}
		else {
			if(vlan == -1) return true;
			else {
				//Modificar cab_ethernet para agregar para agregar el segemento VLAN.
				//printf("Etiquetando paquete con la ID de la VLAN.\n");
				//cab_ethernet = etiquetar_paquete_con_id_vlan(cab_ethernet, vlan, tamano_paq);
				*etiqueta_vlan = true;
				return true;
			}
		}
	}
	if(MODO_L2_IF(interface) == TRONCAL) {		
		if(cab_vlan_8021q) {
			return vlan_esta_asignada_a_interface_troncal(interface, (short) cab_vlan_8021q->VID);
		}
		else return false;		
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
	else if(MODO_L2_IF(interface) == ACCESO || MODO_L2_IF(interface) == TRONCAL) {
		/******PENDIENTE: revisar el tipo de dato del segundo parámetro que debe recibir la función****/
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

entrada_arp_t * busqueda_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip) {
	ITERAR_LISTA_ENLAZADA(tabla_arp->entradas_arp) {
		entrada_arp_t *entrada_arp = *(entrada_arp_t **)(nodo_actual->elemento);
		if(strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP) == 0) {
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
		if(strncmp(entrada_arp->dir_ip.dir_ip, dir_ip, TAM_DIR_IP) == 0) {
			eliminar(tabla_arp->entradas_arp, nodo_actual);
			break;
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
	enviar_paquete((char *) cab_ethernet, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);
	/******PENDIENTE: revisar las demás solicitudes de memoria******/
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
			printf("Antes de eliminar la entrada.\n");
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

/**********PENDIENTE: actualizar la variable que indica el tamaño del paquete**********/
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
	//cab_ethernet_vlan_t *cab_prueba = cab_ethernet;
	//unsigned int tt = TAM_CAB_ETH_VLAN_EXC_PAYLOAD - sizeof(cab_ethernet->tipo) - sizeof(cab_ethernet->FCS);
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
		printf("La interface ya se encuentra configurada en modo %s", cadena_modo_l2_intf(modo_l2_intf));
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
			if(vlans[i] == -1) return false;
			else if(vlans[i] == vlan) return true;
		}
	}
	return false;
}
