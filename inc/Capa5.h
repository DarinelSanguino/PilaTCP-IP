#ifndef CAPA5_H_
#define CAPA5_H_

#include <stdio.h>
#include <arpa/inet.h>
#include "consttcp.h"
#include "Capa2_fwd.h"
#include "Capa3_fwd.h"
#include "Grafico_fwd.h"
#include "Net_fwd.h"
#include "Grafico.h"

void hacer_ping(nodo_t *nodo, char *ip_destino);
void subir_datos_a_capa5(nodo_t *nodo, interface_t *interface_rec, char *datos, unsigned int tamano_datos);

#endif