#ifndef INICRED_H_
#define INICRED_H_

#include "Tipos_red.h"

static inline void init_prop_nodo(prop_nodo_t *prop_nodo) {
	prop_nodo->tiene_dir_loopback = false;
	memset(prop_nodo->dir_loopback.dir_ip, 0, 16);
	inic_tabla_arp(prop_nodo->tabla_arp);
}

static inline void init_prop_intf(prop_intf_t *prop_intf) {
	prop_intf->tiene_dir_ip = false;
	prop_intf->mascara = 0;
	memset(prop_intf->dir_ip.dir_ip, 0, 16);
	memset(prop_intf->dir_mac.dir_mac, 0, 6);
}

void inicializar_interface(interface_t *interface);
void inicializar_nodo(nodo_t *nodo);

#endif