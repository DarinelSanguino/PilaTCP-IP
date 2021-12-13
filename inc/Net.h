#ifndef NET_H_
#define NET_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Grafico.h"
#include "Utiles.h"
//#include "Net_fwd.h"
//#include "Capa2_fwd.h"
//#include "Capa2.h"
/*#define TAM_DIR_MAC 6
#define TAM_DIR_IP 16*/

/*#pragma pack(push, 1)

typedef struct dir_ip_ {
	char dir_ip[TAM_DIR_IP];
} dir_ip_t;

typedef struct dir_mac_ {
	char dir_mac[TAM_DIR_MAC];
} dir_mac_t;

#pragma pack (pop)

struct prop_nodo_ {
	bool tiene_dir_loopback;
	dir_ip_t dir_loopback;
	tabla_arp_t *tabla_arp;
};

struct prop_intf_ {
	bool tiene_dir_ip;
	dir_ip_t dir_ip;
	dir_mac_t dir_mac;
	char mascara;
};*/
extern void inic_tabla_arp(tabla_arp_t **tabla_arp);
extern void inic_tabla_mac(tabla_mac_t **tabla_mac);

static inline void init_prop_nodo(prop_nodo_t *prop_nodo) {
	prop_nodo->tiene_dir_loopback = false;
	memset(prop_nodo->dir_loopback.dir_ip, 0, TAM_DIR_IP);
	inic_tabla_arp(&prop_nodo->tabla_arp);
	inic_tabla_mac(&prop_nodo->tabla_mac);
}

static inline void init_prop_intf(prop_intf_t *prop_intf) {
	memset(prop_intf->dir_mac.dir_mac, 0, TAM_DIR_MAC);
	prop_intf->modo_l2_intf = DESCONOCIDO;
	for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
		prop_intf->vlans[i] = -1;
	}
	prop_intf->tiene_dir_ip = false;
	prop_intf->mascara = 0;
	memset(prop_intf->dir_ip.dir_ip, 0, TAM_DIR_IP);
	
}

char * cadena_modo_l2_intf(modo_l2_intf_t modo_l2_intf);
bool asignar_dir_loopback_nodo(nodo_t *nodo, char *dir_loopback);
bool asignar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if, char *dir_ip, char mascara);
bool quitar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if);
void asignar_dir_mac(interface_t *interface);
char * desp_der_buf_paq(char *paquete, unsigned int tam_paq, unsigned int tam_total_buf);
void aplicar_mascara(char *dir_ip, char mascara, char *dir_ip_subred);
//void mostrar_prop_intf(const prop_intf_t *prop_intf);
//void mostrar_dir_ip(const dir_ip_t *dir_ip);
//void mostrar_dir_mac(const dir_mac_t *dir_mac);

#define MAC_IF(ptr_if) ptr_if->prop_intf->dir_mac.dir_mac
#define IP_IF(ptr_if) ptr_if->prop_intf->dir_ip.dir_ip
#define MASCARA_IF(ptr_if) ptr_if->prop_intf->mascara
#define VLANS_IF(ptr_if) ptr_if->prop_intf->vlans
#define VLAN_IF(ptr_if, i) ptr_if->prop_intf->vlans[i]
#define DIR_LO_NODO(ptr_nodo) ptr_nodo->prop_nodo->dir_loopback.dir_ip
#define IF_EN_MODO_L3(ptr_if) ptr_if->prop_intf->tiene_dir_ip
#define MODO_L2_IF(ptr_if) ptr_if->prop_intf->modo_l2_intf
#define TABLA_ARP_NODO(ptr_nodo) ptr_nodo->prop_nodo->tabla_arp

#endif
