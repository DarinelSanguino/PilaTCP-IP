#ifndef CAPA3_H_
#define CAPA3_H_

#include "Capa2_fwd.h"
#include "Capa3_fwd.h"
#include "Net_fwd.h"
#include "consttcp.h"
#include "Grafico.h"
#include "Capa2.h"
#include "Utiles.h"

struct ruta_l3_ {
	char destino[TAM_DIR_IP];
	char mascara;
	bool es_local;
	char ip_gw[TAM_DIR_IP];
	char intf_salida[TAM_NOMBRE_IF];
};

struct tabla_ruteo_ {
	Lista_t *lista_rutas;
};

#pragma pack(push, 1)

struct cabecera_ip_ {
	unsigned int version : 4;
	unsigned int IHL : 4;
	char tos;
	unsigned short longitud_total;
	unsigned short identificacion;
	unsigned int bandera_no_usada : 1;
	unsigned int bandera_NF : 1;
	unsigned int bandera_MAS : 1;
	unsigned int offset_frag : 13;
	char ttl;
	char protocolo;
	unsigned short checksum;
	unsigned int ip_origen;
	unsigned int ip_destino;
};

#pragma pack(pop)

void inic_tabla_ruteo(tabla_ruteo_t **tabla_ruteo);
ruta_l3_t * busqueda_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip);
bool tabla_ruteo_agregar_ruta_local(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara);
bool tabla_ruteo_agregar_ruta_remota(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara, char *ip_gw, char *intf_salida);
void eliminar_entrada_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip, char mascara);
void mostrar_tabla_ruteo(tabla_ruteo_t *tabla_ruteo);
void _recibir_datos_de_capa_sup_a_capa3(nodo_t *nodo, char *datos, unsigned int tamano_datos, unsigned int ip_destino, unsigned int num_protocolo);
void recibir_paquete_ip_en_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq);
void _recibir_paquete_de_capa2_a_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo);
void subir_paquete_a_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo);
void bajar_datos_a_capa3(nodo_t *nodo, char *datos, unsigned int tamano_datos, unsigned int num_protocolo, unsigned int ip_destino);
bool nodo_es_destino(nodo_t *nodo, char *ip_destino);

#define LONGITUD_EN_BYTES_CAB_IP(ptr_cab_ip) ptr_cab_ip->IHL * 4
#define LONGITUD_TOTAL_EN_BYTES_CAB_IP(ptr_cab_ip) ptr_cab_ip->longitud_total * 4
#define INICIO_PAYLOAD_CAB_IP(ptr_cab_ip) (char *) ptr_cab_ip + LONGITUD_EN_BYTES_CAB_IP(ptr_cab_ip)
#define TAMANO_PAYLOAD_CAB_IP(ptr_cab_ip) LONGITUD_TOTAL_EN_BYTES_CAB_IP(ptr_cab_ip) - LONGITUD_EN_BYTES_CAB_IP(ptr_cab_ip)

#endif