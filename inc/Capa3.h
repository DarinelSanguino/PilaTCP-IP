#ifndef CAPA3_H_
#define CAPA3_H_

#include "Capa2_fwd.h"
#include "Capa3_fwd.h"
#include "Net_fwd.h"
#include "Grafico.h"
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

void inic_tabla_ruteo(tabla_ruteo_t **tabla_ruteo);
ruta_l3_t * busqueda_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip);
bool tabla_ruteo_agregar_ruta_local(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara);
bool tabla_ruteo_agregar_ruta_remota(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara, char *ip_gw, char *intf_salida);
void eliminar_entrada_tabla_ruteo(tabla_ruteo_t *tabla_ruteo, char *dir_ip, char mascara);
void mostrar_tabla_ruteo(tabla_ruteo_t *tabla_ruteo);

#endif