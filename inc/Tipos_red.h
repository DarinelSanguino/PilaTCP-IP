#ifndef TIPOS_RED_H_
#define TIPOS_RED_H_

#include "Capa2_fwd.h"

#define TAM_NOMBRE_NODO 16
#define TAM_NOMBRE_IF 16
#define MAX_INTF_POR_NODO 10
#define MAX_NODOS_POR_GRAFICO 256
#define TAM_DIR_MAC 6

#define TAM_DIR_MAC 6
#define TAM_DIR_IP 16

typedef struct nodo_ nodo_t;
typedef struct enlace_ enlace_t;
typedef struct interface_ interface_t;
typedef struct grafico_ grafico_t;

typedef struct dir_ip_ dir_ip_t;
typedef struct dir_mac_ dir_mac_t;
typedef struct prop_nodo_ prop_nodo_t;
typedef struct prop_intf_ prop_intf_t;

struct interface_ {
	char nombre_if[TAM_NOMBRE_IF];
	struct nodo_ *nodo_padre;
	struct enlace_ *enlace;
	prop_intf_t *prop_intf;
};

struct enlace_ {
	interface_t intf1;
	interface_t intf2;
	unsigned int costo;
};

struct nodo_ {
	char nombre_nodo[TAM_NOMBRE_NODO];
	interface_t *intf[MAX_INTF_POR_NODO];
	prop_nodo_t *prop_nodo;
	unsigned int numero_puerto_udp;
	int fd_sock_udp;
};

struct grafico_ {
	char nombre_topologia[32];
	Lista_t *lista_nodos;
};

#pragma pack(push, 1)

typedef struct dir_ip_ {
	char dir_ip[TAM_DIR_IP];
} dir_ip_t;

typedef struct dir_mac_ {
	unsigned char dir_mac[TAM_DIR_MAC];
} dir_mac_t;

#pragma pack (pop)

typedef enum {
	ACCESO,
	TRONCAL,
	MODO_L2_DESCONOCIDO
} modo_l2_intf_t;

struct prop_nodo_ {
	bool tiene_dir_loopback;
	dir_ip_t dir_loopback;
	tabla_arp_t *tabla_arp;
	tabla_mac_t *tabla_mac;
};

struct prop_intf_ {
	//Propiedades Capa 2
	dir_mac_t dir_mac;
	modo_l2_intf_t modo_l2_intf;
	//Propiedades Capa 3
	bool tiene_dir_ip;
	dir_ip_t dir_ip;	
	char mascara;	
};

#endif
