#include "SwitchCapa2.h"

static void aprendizaje_mac_switch_capa2(nodo_t *nodo, unsigned char *mac_origen, char *nombre_if) {
	tabla_mac_t *tabla_mac = nodo->prop_nodo->tabla_mac;
	entrada_mac_t *entrada_mac = calloc(1, sizeof(entrada_mac_t));
	//POSIBLE VIOLACIÓN DE SEGMENTO*********************************
	memcpy(entrada_mac->dir_mac.dir_mac, mac_origen, TAM_DIR_MAC);
	memcpy(entrada_mac->nombre_if, nombre_if, TAM_NOMBRE_IF);
	agregar_entrada_tabla_mac(tabla_mac, entrada_mac);
}

bool enviar_paquete_switch_capa2(interface_t *intf_salida, char *paquete, unsigned int tamano_paq, bool *etiqueta_vlan) {
	assert(!IF_EN_MODO_L3(intf_salida));
	//printf("Interface %s.\n", intf_salida->nombre_if);
	//mostrar_prop_vlan(intf_salida->prop_intf->modo_l2_intf, intf_salida->prop_intf->vlans);
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	cab_vlan_8021q_t *cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(MODO_L2_IF(intf_salida) == ACCESO) {
		unsigned int vlan = intf_salida->prop_intf->vlans[0];
		if(cab_vlan_8021q) {
			printf("Sí hay cabecera VLAN.\n");
			if(vlan == -1) return false;
			else {
				if(vlan == (short) cab_vlan_8021q->VID) {
					printf("Las ID de las VLAN coinciden.\n");
					//quitar_etiqueta_paquete_con_id_vlan(cab_ethernet, tamano_paq);
					*etiqueta_vlan = false;
					return true;
				}
				else return false;
			}
		}
		else {
			if(vlan == -1) return true;
			else return false;
		}
	}
	if(MODO_L2_IF(intf_salida) == TRONCAL) {
		if(cab_vlan_8021q) {
			return vlan_esta_asignada_a_interface_troncal(intf_salida, (short) cab_vlan_8021q->VID);
		}
		else return false;	
	}
	return false;
}

void enviar_paquete_interfaces_switch_capa2(nodo_t *nodo, interface_t *intf_exenta, char *paquete, unsigned int tamano_paq) {
	interface_t *intf_actual = NULL;
	for(int i = 0; i < MAX_INTF_POR_NODO; i++) {
		intf_actual = nodo->intf[i];		
		if(!intf_actual) return;
		if(intf_actual != intf_exenta) {
			bool etiqueta_vlan = true;
			if(enviar_paquete_switch_capa2(intf_actual, paquete, tamano_paq, &etiqueta_vlan)) {
				char *temporal = calloc(1, MAX_TAMANO_BUFFER_PAQ);
				cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) (temporal + MAX_TAMANO_BUFFER_PAQ - tamano_paq);
				memcpy((char *)cab_ethernet, paquete, tamano_paq);
				unsigned int nuevo_tamano_paq = tamano_paq;
				if(!etiqueta_vlan) {
					cab_ethernet = quitar_etiqueta_paquete_con_id_vlan(cab_ethernet, &nuevo_tamano_paq);
				}
				enviar_paquete((char *)cab_ethernet, nuevo_tamano_paq, intf_actual);
				printf("El paquete se envió por la interface %s.\n", intf_actual->nombre_if);
			}
		}
	}
}

void enviar_trama_switch_capa2(nodo_t *nodo, interface_t *intf_entrada, char *paquete, unsigned int tamano_paq) {
	/***********PENDIENTE: revisar tamano_paq*************/
	cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	unsigned char *dir_mac = cab_ethernet->mac_destino.dir_mac;
	if(ES_DIR_MAC_BROADCAST(dir_mac)) {
		printf("El switch identificó una dirección MAC broadcast.\n");
		enviar_paquete_interfaces_switch_capa2(nodo, intf_entrada, paquete, tamano_paq);
		return;
	}
	//Buscar correspondencia entre dirección MAC e interface
	entrada_mac_t *entrada_mac = busqueda_tabla_mac(nodo->prop_nodo->tabla_mac, dir_mac);
	if(!entrada_mac) {
		/************************PENDIENTE: ejecutar búsqueda ARP en lugar de enviar el paquete en todas las interfaces.****/
		enviar_paquete_interfaces_switch_capa2(nodo, intf_entrada, paquete, tamano_paq);
		return;
	}
	
	interface_t *intf_salida = obtener_intf_por_nombre(nodo, entrada_mac->nombre_if);
	if(!intf_salida) {
		printf("Error: no se encontró la interface %s.\n", intf_salida->nombre_if);
		return;
	}
	bool etiqueta_vlan = true;
	if(enviar_paquete_switch_capa2(intf_salida, paquete, tamano_paq, &etiqueta_vlan)) {
		cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
		unsigned int nuevo_tamano_paq = tamano_paq;
		if(!etiqueta_vlan) {
			cab_ethernet = quitar_etiqueta_paquete_con_id_vlan(cab_ethernet, &nuevo_tamano_paq);
		}
		enviar_paquete((char *)cab_ethernet, tamano_paq, intf_salida);
	}
}

void recibir_trama_switch_capa2(interface_t *interface, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq, bool etiqueta_vlan) {
	//printf("%s\n", __FUNCTION__);
	nodo_t *nodo = interface->nodo_padre;
	//cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	printf("Nodo %s receptor.\n", nodo->nombre_nodo);
	unsigned char *mac_origen = cab_ethernet->mac_origen.dir_mac;
	aprendizaje_mac_switch_capa2(nodo, mac_origen, interface->nombre_if);
	unsigned int nuevo_tamano_paq = tamano_paq;
	if(etiqueta_vlan) {
		unsigned int vlan = interface->prop_intf->vlans[0];
		/*******AQUI ESTA EL ERROR*****/
		cab_ethernet = etiquetar_paquete_con_id_vlan(cab_ethernet, vlan, &nuevo_tamano_paq);
	}
	enviar_trama_switch_capa2(nodo, interface, (char *)cab_ethernet, nuevo_tamano_paq);
}

/*bool mensaje_es_solic_broadcast_arp(cab_ethernet_t *cab_ethernet) {
	cab_vlan_8021q_t *cab_vlan_8021q = paquete_tiene_etiqueta_vlan(cab_ethernet);
	if(!cab_vlan_8021q) return false;
	if(cab_ethernet->tipo != MENSAJE_ARP) return false;
	cab_arp_t *cab_arp = (cab_arp_t *) cab_ethernet->payload;
	if(cab_arp->cod_op != SOLIC_BROAD_ARP) return false;
	if(cab_vlan_8021q->VID < 10 || cab_vlan_8021q->VID > 20) return false;
	return true;
}*/

