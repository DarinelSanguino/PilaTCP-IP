#include "Capa3.h"

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
		/*****PENDIENTE:0****/	
		if(ruta_l3->mascara > mascara_mayor) {
			memset(id_red, 0, TAM_DIR_IP);
			aplicar_mascara(ruta_l3->destino, ruta_l3->mascara, id_red);
			if(strncmp(id_red, dir_ip, TAM_DIR_IP) == 0) {
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
	ruta_l3_t *ruta_l3 = calloc(1, sizeof(ruta_l3_t));
	strncpy(ruta_l3->destino, destino, TAM_DIR_IP);
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
		printf("Destino: %s, mascara:%u, local: %s, ip gateway: %s, interface de salida: %s.\n", 
			ruta_l3->destino,
			ruta_l3->mascara,
			ruta_l3->es_local ? "true" : "false",
			ruta_l3->ip_gw,
			ruta_l3->intf_salida);
	} FIN_ITERACION;
}
