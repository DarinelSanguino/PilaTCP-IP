#ifndef NET_H_
#define NET_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Grafico.h"
#include "Utiles.h"

extern void inic_tabla_arp(tabla_arp_t **tabla_arp);
extern void inic_tabla_mac(tabla_mac_t **tabla_mac);
extern void inic_tabla_ruteo(tabla_ruteo_t **tabla_ruteo);

void init_prop_nodo(prop_nodo_t *prop_nodo);
void init_prop_intf(prop_intf_t *prop_intf);
char * cadena_modo_l2_intf(modo_l2_intf_t modo_l2_intf);
bool asignar_dir_loopback_nodo(nodo_t *nodo, char *dir_loopback);
bool asignar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if, char *dir_ip, char mascara);
bool quitar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if);
void asignar_dir_mac(interface_t *interface);
char * desp_der_buf_paq(char *paquete, unsigned int tam_paq, unsigned int tam_total_buf);
void aplicar_mascara(char *dir_ip, char mascara, char *dir_ip_subred);

#define MAC_IF(ptr_if) ptr_if->prop_intf->dir_mac.dir_mac
#define IP_IF(ptr_if) ptr_if->prop_intf->dir_ip.dir_ip
#define MASCARA_IF(ptr_if) ptr_if->prop_intf->mascara
#define VLANS_IF(ptr_if) ptr_if->prop_intf->vlans
#define VLAN_IF(ptr_if, i) ptr_if->prop_intf->vlans[i]
#define DIR_LO_NODO(ptr_nodo) ptr_nodo->prop_nodo->dir_loopback.dir_ip
#define IF_EN_MODO_L3(ptr_if) ptr_if->prop_intf->tiene_dir_ip
#define MODO_L2_IF(ptr_if) ptr_if->prop_intf->modo_l2_intf
#define TABLA_ARP_NODO(ptr_nodo) ptr_nodo->prop_nodo->tabla_arp
#define TABLA_MAC_NODO(ptr_nodo) ptr_nodo->prop_nodo->tabla_mac
#define TABLA_RUTEO_NODO(ptr_nodo) ptr_nodo->prop_nodo->tabla_ruteo

#endif
