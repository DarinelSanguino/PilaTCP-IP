#include "Capa3.h"

static inline void inic_cabecera_ip(cabecera_ip_t *cabecera_ip) {
	cabecera_ip->version = 4;
	cabecera_ip->IHL = 5;
	cabecera_ip->tos = 0;
	cabecera_ip->longitud_total = 0;
	cabecera_ip->identificacion = 0;
	cabecera_ip->bandera_no_usada = 0;
	cabecera_ip->bandera_NF = 1;
	cabecera_ip->bandera_MAS = 0;
	cabecera_ip->offset_frag = 0;
	cabecera_ip->ttl = 64;
	cabecera_ip->protocolo = 0;
	cabecera_ip->checksum = 0;
	cabecera_ip->ip_origen = 0;
	cabecera_ip->ip_destino = 0;
}

void inic_tabla_ruteo(tabla_ruteo_t **tabla_ruteo) {
	*tabla_ruteo = malloc(sizeof(tabla_ruteo_t));
	(*tabla_ruteo)->lista_rutas = malloc(sizeof(Lista_t));
	(*tabla_ruteo)->lista_rutas->vacia = true;
}

ruta_l3_t * busqueda_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip) {
	char id_red[TAM_DIR_IP];
	char mascara_mayor = 0;
	ruta_l3_t *ruta_l3_coincidente = NULL;
	ITERAR_LISTA_ENLAZADA(tabla_ruteo->lista_rutas) {
		ruta_l3_t *ruta_l3 = *(ruta_l3_t **)(nodo_actual->elemento);		
		if(ruta_l3->mascara > mascara_mayor) {
			memset(id_red, 0, TAM_DIR_IP);
			aplicar_mascara(dir_ip, ruta_l3->mascara, id_red);
			if(strncmp(id_red, ruta_l3->destino, TAM_DIR_IP) == 0) {
				mascara_mayor = ruta_l3->mascara;
				ruta_l3_coincidente = ruta_l3;
			}
		}
	} FIN_ITERACION;
	return ruta_l3_coincidente;
}

bool tabla_ruteo_agregar_ruta_local(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara) {
	uint32_t ip;
	inet_pton(AF_INET, destino, &ip);
	ruta_l3_t *ruta_l3_antigua = busqueda_tabla_ruteo(tabla_ruteo, destino);
	if(ruta_l3_antigua) {
		eliminar_entrada_tabla_ruteo(tabla_ruteo, ruta_l3_antigua->destino, ruta_l3_antigua->mascara);
	}
	char id_red[TAM_DIR_IP];
	memset(id_red, 0, TAM_DIR_IP);
	aplicar_mascara(destino, mascara, id_red);

	ruta_l3_t *ruta_l3 = calloc(1, sizeof(ruta_l3_t));
	strncpy(ruta_l3->destino, id_red, TAM_DIR_IP);
	ruta_l3->mascara = mascara;
	ruta_l3->es_local = true;
	return insertar(tabla_ruteo->lista_rutas, &ruta_l3, sizeof(ruta_l3_t *));
}

bool tabla_ruteo_agregar_ruta_remota(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara, char *ip_gw, char *intf_salida) {
	uint32_t ip;
	inet_pton(AF_INET, destino, &ip);
	ruta_l3_t *ruta_l3_antigua = busqueda_tabla_ruteo(tabla_ruteo, destino);
	if(ruta_l3_antigua) {
		eliminar_entrada_tabla_ruteo(tabla_ruteo, ruta_l3_antigua->destino, ruta_l3_antigua->mascara);
	}
	char id_red[TAM_DIR_IP];
	memset(id_red, 0, TAM_DIR_IP);
	aplicar_mascara(destino, mascara, id_red);

	ruta_l3_t *ruta_l3 = calloc(1, sizeof(ruta_l3_t));
	strncpy(ruta_l3->destino, id_red, TAM_DIR_IP);
	ruta_l3->mascara = mascara;
	ruta_l3->es_local = false;
	strncpy(ruta_l3->ip_gw, ip_gw, TAM_DIR_IP);
	strncpy(ruta_l3->intf_salida, intf_salida, TAM_NOMBRE_IF);
	return insertar(tabla_ruteo->lista_rutas, &ruta_l3, sizeof(ruta_l3_t *));
}

void eliminar_entrada_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip, char mascara) {
	ITERAR_LISTA_ENLAZADA(tabla_ruteo->lista_rutas) {
		ruta_l3_t *ruta_l3 = *(ruta_l3_t **)(nodo_actual->elemento);
		if(strncmp(ruta_l3->destino, dir_ip, TAM_DIR_IP) == 0 && ruta_l3->mascara == mascara) {
			eliminar(tabla_ruteo->lista_rutas, nodo_actual);
			break;
		}
	} FIN_ITERACION;
}

void mostrar_tabla_ruteo(tabla_ruteo_t *tabla_ruteo) {
	ITERAR_LISTA_ENLAZADA(tabla_ruteo->lista_rutas) {
		ruta_l3_t *ruta_l3 = *(ruta_l3_t **)(nodo_actual->elemento);		
		printf("Destino: %s, mascara:%u, local: %s, ip gateway: %s, interface de salida: %s\n", 
			ruta_l3->destino,
			ruta_l3->mascara,
			ruta_l3->es_local ? "true" : "false",
			ruta_l3->ip_gw,
			ruta_l3->intf_salida);
	} FIN_ITERACION;
}

void _recibir_datos_de_capa_sup_a_capa3(nodo_t *nodo, char *datos, unsigned int tamano_datos, unsigned int ip_destino, unsigned int num_protocolo) {
	char dir_ip[TAM_DIR_IP];
	memset(dir_ip, 0, TAM_DIR_IP);
	inet_ntop(AF_INET, &ip_destino, dir_ip, TAM_DIR_IP);
	ruta_l3_t *ruta_l3 = busqueda_tabla_ruteo(nodo->prop_nodo->tabla_ruteo, dir_ip);
	if(!ruta_l3) {
		printf("No se encontró alguna entrada coincidente con la dirección IP %s en la tabla del nodo %s.\n", dir_ip, nodo->nombre_nodo);
		return;
	}
	char *nuevo_ptr_paq = calloc(1, sizeof(cabecera_ip_t) + tamano_datos);
	cabecera_ip_t *cabecera_ip = (cabecera_ip_t *) nuevo_ptr_paq;
	inic_cabecera_ip(cabecera_ip);	
	cabecera_ip->longitud_total = cabecera_ip->IHL + (tamano_datos / 4);
	if((tamano_datos % 4) > 0) cabecera_ip->longitud_total += 1;
	cabecera_ip->protocolo = num_protocolo;
	
	unsigned int ip_origen;
	inet_pton(AF_INET, DIR_LO_NODO(nodo), &ip_origen);
	cabecera_ip->ip_origen = ip_origen;
	cabecera_ip->ip_destino = ip_destino;
	
	memcpy(nuevo_ptr_paq + sizeof(cabecera_ip_t), datos, tamano_datos);
	
	unsigned int ip_gw = ip_destino;	
	if(!ruta_l3->es_local) {
		inet_pton(AF_INET, ruta_l3->ip_gw, &ip_gw);
	}
	
	bajar_paquete_a_capa2(nodo, ruta_l3->intf_salida, ip_gw, nuevo_ptr_paq, cabecera_ip->longitud_total * 4, IPv4);
	free(nuevo_ptr_paq);
}

void recibir_paquete_ip_en_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq) {
	cabecera_ip_t *cabecera_ip = (cabecera_ip_t *)paquete;
	char ip_destino[TAM_DIR_IP];
	memset(ip_destino, 0, TAM_DIR_IP);
	inet_ntop(AF_INET, &cabecera_ip->ip_destino, ip_destino, TAM_DIR_IP);
	ruta_l3_t *ruta_l3 = busqueda_tabla_ruteo(nodo_rec->prop_nodo->tabla_ruteo, ip_destino);
	if(!ruta_l3) {
		printf("No se encontró alguna entrada coincidente con la dirección IP %s en la tabla del nodo %s.\n", ip_destino, nodo_rec->nombre_nodo);
		return;
	}
	if(!ruta_l3->es_local) {
		unsigned int ip_gw;
		inet_pton(AF_INET, ruta_l3->ip_gw, &ip_gw);
		bajar_paquete_a_capa2(nodo_rec, ruta_l3->intf_salida, ip_gw, paquete, tamano_paq, IPv4);
		return;
	}
	if(nodo_es_destino(nodo_rec, ip_destino)) {
		switch(cabecera_ip->protocolo) {
			case ICMP:
			{
				char ip_origen[TAM_DIR_IP];
				memset(ip_origen, 0, TAM_DIR_IP);
				inet_ntop(AF_INET, &cabecera_ip->ip_origen, ip_origen, TAM_DIR_IP);
				printf("PING recibido en la dirección %s desde %s.\n", ip_destino, ip_origen);
			}
				break;
			default:
				break;
		}
		return;
	}	
	bajar_paquete_a_capa2(nodo_rec, "\0", cabecera_ip->ip_destino, paquete, tamano_paq, IPv4);
}

void _recibir_paquete_de_capa2_a_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo) {
	switch(num_protocolo) {
		case IPv4:
			recibir_paquete_ip_en_capa3(nodo_rec, interface_rec, paquete, tamano_paq);
			break;
		default:
			break;
	}
}

void subir_paquete_a_capa3(nodo_t *nodo_rec, interface_t *interface_rec, char *paquete, unsigned int tamano_paq, unsigned int num_protocolo) {
	_recibir_paquete_de_capa2_a_capa3(nodo_rec, interface_rec, paquete, tamano_paq, num_protocolo);
}

void bajar_datos_a_capa3(nodo_t *nodo, char *datos, unsigned int tamano_datos, unsigned int ip_destino, unsigned int num_protocolo) {
	_recibir_datos_de_capa_sup_a_capa3(nodo, datos, tamano_datos, ip_destino, num_protocolo);
}

bool nodo_es_destino(nodo_t *nodo, char *ip_destino) {
	if(strncmp(DIR_LO_NODO(nodo), ip_destino, TAM_DIR_IP) == 0) return true;
	interface_t *intf_actual = NULL;
	for(int i = 0; i < MAX_INTF_POR_NODO; i++) {
		intf_actual = nodo->intf[i];
		if(!intf_actual) return false;
		if(strncmp(IP_IF(intf_actual), ip_destino, TAM_DIR_IP) == 0) return true;
	}
	return false;
}
