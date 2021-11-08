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

static inline void init_prop_nodo(prop_nodo_t *prop_nodo) {
	prop_nodo->tiene_dir_loopback = false;
	memset(prop_nodo->dir_loopback.dir_ip, 0, 16);
	inic_tabla_arp(&prop_nodo->tabla_arp);
}

static inline void init_prop_intf(prop_intf_t *prop_intf) {
	prop_intf->tiene_dir_ip = false;
	prop_intf->mascara = 0;
	memset(prop_intf->dir_ip.dir_ip, 0, 16);
	memset(prop_intf->dir_mac.dir_mac, 0, 6);
}

bool asignar_dir_loopback_nodo(nodo_t *nodo, char *dir_loopback);
bool asignar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if, char *dir_ip, char mascara);
bool quitar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if);
void asignar_dir_mac(interface_t *interface);
char * desp_der_buf_paq(char *paquete, unsigned int tam_paq, unsigned int tam_total_buf);
//void mostrar_prop_intf(const prop_intf_t *prop_intf);
//void mostrar_dir_ip(const dir_ip_t *dir_ip);
//void mostrar_dir_mac(const dir_mac_t *dir_mac);

#define MAC_IF(ptr_if) ptr_if->prop_intf->dir_mac.dir_mac
#define IP_IF(ptr_if) ptr_if->prop_intf->dir_ip.dir_ip
#define DIR_LO_NODO(ptr_nodo) ptr_nodo->prop_nodo->dir_loopback.dir_ip
#define IF_EN_MODO_L3(ptr_if) ptr_if->prop_intf->tiene_dir_ip
#define TABLA_ARP_NODO(ptr_nodo) ptr_nodo->prop_nodo->tabla_arp

#endif
