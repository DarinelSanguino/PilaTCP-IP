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

struct cab_vlan_8021q_ {
	unsigned short TPID;
	short PRI : 3;
	short CFI : 1;
	short VID : 12;
};

struct cab_ethernet_vlan_ {
	dir_mac_t mac_destino;
	dir_mac_t mac_origen;
	cab_vlan_8021q_t cab_vlan_8021q;
	short tipo;
	char payload[TAM_MAX_PAYLOAD];
	unsigned int FCS;
};

#pragma pack(pop)

struct entrada_arp_ {
	dir_ip_t dir_ip;
	dir_mac_t dir_mac;	
	char nombre_if[16];	
};

struct tabla_arp_ {
	Lista_t *entradas_arp;
};

struct entrada_mac_ {
	dir_mac_t dir_mac;
	char nombre_if[TAM_NOMBRE_IF];
};

struct tabla_mac_ {
	Lista_t *entradas_mac;
};

#define TAM_CAB_ETH_EXC_PAYLOAD (sizeof(cab_ethernet_t) - sizeof(((cab_ethernet_t *)0)->payload))
#define TAM_CAB_ETH_VLAN_EXC_PAYLOAD (sizeof(cab_ethernet_vlan_t) - sizeof(((cab_ethernet_vlan_t *)0)->payload))

#define FCS_ETH(ptr_cab_eth, tam_payload) (*(unsigned int *) (((char *) (((cab_ethernet_t *) ptr_cab_eth)->payload)) + tam_payload))
#define FCS_ETH_VLAN(ptr_cab_eth_vlan, tam_payload) (*(unsigned int *) (((char *) (((cab_ethernet_vlan_t *) ptr_cab_eth_vlan)->payload)) + tam_payload))
#define INTF_EN_MODO_L3(ptr_intf) ((ptr_intf)->prop_intf->tiene_dir_ip == true)

void conf_intf_modo_l2(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf);

void bajar_paquete_a_capa2(nodo_t *nodo, char *intf_salida, uint32_t ip_gw, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo);
bool rellenar_cab_ethernet(nodo_t *nodo, interface_t *intf_salida, char *ip_destino, cab_ethernet_t *cab_ethernet);
void recibir_paquete_ip_en_capa2(nodo_t *nodo, char *intf_salida, uint32_t ip_gw, char *paquete, unsigned int tamano_payload);

bool recibir_trama_capa2_en_interface(interface_t *interface, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq, bool *etiqueta_vlan);
void recibir_trama_capa2(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq);
cab_vlan_8021q_t * paquete_tiene_etiqueta_vlan(cab_ethernet_t *cab_ethernet);

void inic_tabla_arp(tabla_arp_t **tabla_arp);
entrada_arp_t * busqueda_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip);
bool agregar_entrada_tabla_arp(tabla_arp_t *tabla_arp, entrada_arp_t *entrada_arp);
void eliminar_entrada_tabla_arp(tabla_arp_t *tabla_arp, char *dir_ip);
void actualizar_tabla_arp(tabla_arp_t *tabla_arp, cab_arp_t *cab_arp, interface_t *interface);
void mostrar_tabla_arp(tabla_arp_t *tabla_arp);
void enviar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf_salida, char *dir_ip);

void inic_tabla_mac(tabla_mac_t **tabla_mac);
entrada_mac_t * busqueda_tabla_mac(tabla_mac_t *tabla_mac, unsigned char *dir_mac);
bool agregar_entrada_tabla_mac(tabla_mac_t *tabla_mac, entrada_mac_t *entrada_mac);
void eliminar_entrada_tabla_mac(tabla_mac_t *tabla_mac, unsigned char *dir_mac);
void mostrar_tabla_mac(tabla_mac_t *tabla_mac);
cab_ethernet_t * etiquetar_paquete_con_id_vlan(cab_ethernet_t *cab_ethernet, int id_vlan, unsigned int *tamano_paq);
cab_ethernet_t * quitar_etiqueta_paquete_con_id_vlan(cab_ethernet_t *cab_ethernet, unsigned int *tamano_paq);
void asignar_modo_l2_nodo(nodo_t *nodo, char *nombre_if, modo_l2_intf_t modo_l2_intf);
void asignar_id_vlan_nodo(nodo_t *nodo, char *nombre_if, unsigned int id_vlan);
void mostrar_prop_vlan(modo_l2_intf_t modo_l2_intf, const unsigned int *vlans);
unsigned int obtener_id_vlan_intf_acceso(interface_t *interface);
bool vlan_esta_asignada_a_interface_troncal(interface_t *interface, unsigned int vlan);

#endif
