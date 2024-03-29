#ifndef GRAFICO_H_
#define GRAFICO_H_

#include <stdlib.h>
#include <string.h>
#include "ListaEnlazadaGenerica.h"
#include "Tipos_red.h"
#include "Net.h"

nodo_t* obtener_nodo_vecino(interface_t *interface);
nodo_t * obtener_nodo_por_nombre(grafico_t *topologia, char *nombre_nodo);
interface_t * obtener_intf_por_nombre(nodo_t *nodo, char *nombre_if);

grafico_t * crear_nuevo_grafico(const char *nombre_topologia);
nodo_t * crear_nodo_grafico(grafico_t *grafico, const char *nombre_nodo);
void insertar_enlace_entre_nodos(nodo_t *nodo1, nodo_t *nodo2, char *de_nombre_if, char *a_nombre_if, unsigned int costo);

nodo_t *obtener_elemento(Lista_t *lista, char *nombre_nodo);
void inicializar_grafico(grafico_t *grafico);
void inicializar_nodo(nodo_t *nodo);
void inicializar_interface(interface_t *interface);
void mostrar_grafico(const grafico_t *grafico);
void mostrar_nodo(const nodo_t *nodo);
void mostrar_interface(const interface_t *interface);
void mostrar_enlace(const enlace_t *enlace);
void mostrar_prop_intf(const prop_intf_t *prop_intf);
void mostrar_dir_mac(const dir_mac_t *dir_mac);
interface_t* obtener_intf_correspondiente_a_nodo(nodo_t *nodo, char *dir_ip);

#endif
