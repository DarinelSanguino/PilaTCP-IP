#ifndef CAPA2_H_
#define CAPA2_H_

#include <stdint.h>
#include <arpa/inet.h>
#include "consttcp.h"
#include "Capa2_fwd.h"
#include "Net_fwd.h"
#include "Grafico.h"
#include "Utiles.h"

#define TAM_MAX_PAYLOAD 248

#pragma pack(push, 1)

struct cab_arp_ {
	short tipo_hw;
	short tipo_proto;
	char lon_dir_hw;
	char lon_dir_proto;
	short cod_op;
	dir_mac_t mac_origen;
	unsigned int ip_origen;
	dir_mac_t mac_destino;
	unsigned int ip_destino;
};

struct cab_ethernet_ {
	dir_mac_t mac_destino;
	dir_mac_t mac_origen;
	short tipo;
	char payload[TAM_MAX_PAYLOAD];
	unsigned int FCS;
};

#pragma pack(pop)

struct entrada_arp_ {
	dir_ip_t dir_ip;
	dir_mac_t dir_mac;
	/******************PENDIENTE******************/
	char nombre_if[16];	
};

struct tabla_arp_ {
	Lista_t *entradas_arp;
};

#define TAM_CAB_ETH_EXC_PAYLOAD (sizeof(cab_ethernet_t) - sizeof(((cab_ethernet_t *)0)->payload))

#define FCS_ETH(ptr_cab_eth, tam_payload) (*(unsigned int *) (((char *) (((cab_ethernet_t *) ptr_cab_eth)->payload)) + tam_payload))
#define INTF_EN_MODO_L3(ptr_intf) ((ptr_intf)->prop_intf->tiene_dir_ip == true)

void conf_intf_modo_l2(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf);

extern int enviar_paquete(char *paquete, unsigned int tamano_paq, interface_t *intf_origen);
void mover_paq_a_capa3(nodo_t *nodo_rec, interface_t *interface, char *paquete, size_t tamano_paq);

void recibir_trama_capa2(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq);
void inic_tabla_arp(tabla_arp_t **tabla_arp);
entrada_arp_t * busqueda_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip);

bool agregar_entrada_tabla_arp(tabla_arp_t *tabla_arp, entrada_arp_t *entrada_arp);
void eliminar_entrada_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip);
void actualizar_tabla_arp(tabla_arp_t *tabla_arp, cab_arp_t *cab_arp, interface_t *interface);
void mostrar_tabla_arp(tabla_arp_t *tabla_arp);
void enviar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_salida, char *dir_ip);

static inline char* OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_t *cab_ethernet) {
	return cab_ethernet->payload;
}

static inline bool recibir_trama_l2_en_interface(interface_t *interface, cab_ethernet_t *cab_ethernet) {
	unsigned char *mac = MAC_IF(interface);
	if(IF_EN_MODO_L3(interface)) {
		unsigned char *mac_destino = cab_ethernet->mac_destino.dir_mac;
		mostrar_dir_mac(&interface->prop_intf->dir_mac);
		mostrar_dir_mac(&cab_ethernet->mac_destino);
		//bool bandera_prueba = true;
		//bandera_prueba = mac_destino[0] == 0xFF;

		/*if(mac_destino[0] == 0xFF && mac_destino[1] == 0xFF && mac_destino[2] == 0xFF && mac_destino[3] == 0xFF && mac_destino[4] == 0xFF && mac_destino[5] == 0xFF) {
			printf("Esto está pasando.\n");
		}
		printf("AABBNINKD.\n");
		if(mac_destino[0] == 0xFF) {
			printf("Esto sí pasa.\n");
		}*/
		if(memcmp(mac, mac_destino, TAM_DIR_MAC) == 0 || ES_DIR_MAC_BROADCAST(mac_destino)) return true;

	}
	return false;
}

static inline void enviar_mensaje_arp_respuesta(cab_ethernet_t *cab_ethernet_entrada, interface_t *intf_salida) {
	unsigned int tamano_payload = sizeof(cab_arp_t);
	cab_arp_t *cab_arp_entrada = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_entrada);
	cab_ethernet_t *cab_ethernet_resp = calloc(1, sizeof(cab_ethernet_t));
	memcpy(cab_ethernet_resp->mac_destino.dir_mac, cab_arp_entrada->mac_origen.dir_mac, TAM_DIR_MAC);
	memcpy(cab_ethernet_resp->mac_origen.dir_mac, MAC_IF(intf_salida), TAM_DIR_MAC);
	cab_ethernet_resp->tipo = MENSAJE_ARP;
	cab_arp_t *cab_arp_resp = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet_resp);
	cab_arp_resp->tipo_hw = 1;
	cab_arp_resp->tipo_proto = 0x0800;
	cab_arp_resp->lon_dir_hw = sizeof(dir_mac_t);
	cab_arp_resp->lon_dir_proto = 4;
	cab_arp_resp->cod_op = RESPUESTA_ARP;
	memcpy(cab_arp_resp->mac_origen.dir_mac, MAC_IF(intf_salida), TAM_DIR_MAC);
	/******PENDIENTE: IP*****/
	cab_arp_resp->ip_origen = cab_arp_entrada->ip_destino;
	memcpy(cab_arp_resp->mac_destino.dir_mac, cab_arp_entrada->mac_origen.dir_mac, TAM_DIR_MAC);
	cab_arp_resp->ip_destino = cab_arp_entrada->ip_origen;
	FCS_ETH(cab_ethernet_resp, sizeof(cab_arp_t)) = 0;
	/******PENDIENTE: HACE FALTA ENVIAR LA RESPUESTA.******************/
	enviar_paquete((char *) cab_ethernet_resp, TAM_CAB_ETH_EXC_PAYLOAD + tamano_payload, intf_salida);
}

static inline void procesar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_entrada, cab_ethernet_t *cab_ethernet) {
	printf("%s: mensaje broadcast ARP recibido en la interface %s del nodo %s", __FUNCTION__, intf_entrada->nombre_if, nodo->nombre_nodo);
	/********PENDIENTE: corregir****************/
	cab_arp_t *cab_arp = (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet);
	uint32_t ip_destino_arp = htonl(cab_arp->ip_destino);
	char ip_destino[TAM_DIR_IP];
	inet_ntop(AF_INET, &ip_destino_arp, ip_destino, TAM_DIR_IP);	
	ip_destino[TAM_DIR_IP - 1] = '\0';
	if(strncmp(intf_entrada->prop_intf->dir_ip.dir_ip, ip_destino, TAM_DIR_IP) != 0) {
		printf("%s: solicitud broadcast ARP descartada. La IP de destino %s no coincide con la IP %s de la interface.\n",
			nodo->nombre_nodo, ip_destino, IP_IF(intf_entrada));
		return;
	}
	actualizar_tabla_arp(TABLA_ARP_NODO(nodo), cab_arp, intf_entrada);
	enviar_mensaje_arp_respuesta(cab_ethernet, intf_entrada);
}

static inline void procesar_mensaje_respuesta_arp(nodo_t *nodo, interface_t *intf_entrada, cab_ethernet_t *cab_ethernet) {
	printf("%s: mensaje ARP recibido en la interface %s del nodo %s", __FUNCTION__, intf_entrada->nombre_if, nodo->nombre_nodo);
	actualizar_tabla_arp(TABLA_ARP_NODO(nodo), (cab_arp_t *) OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet), intf_entrada);
}

static inline cab_ethernet_t * ASIG_CAB_ETH_CON_PAYLOAD(char *paquete, unsigned int tam_paq) {
	cab_ethernet_t *cab_ethernet = calloc(1, sizeof(cab_ethernet_t));
	if(sizeof(paquete) > TAM_MAX_PAYLOAD) return NULL;
	memcpy(OBTENER_PAYLOAD_DE_CAB_ETHERNET(cab_ethernet), paquete, tam_paq);
	//cab_ethernet->payload = paquete;
	return cab_ethernet;
}

#endif
