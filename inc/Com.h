#ifndef COM_H_
#define COM_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "Grafico.h"
#include "Net.h"
#include "Capa2.h"

#define MAX_TAMANO_BUFFER_PAQ 2048

int enviar_paquete(char *paquete, unsigned int tamano_paq, interface_t *intf_origen);
int recibir_paquete(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq, char *inicio);
void inic_sock_udp(nodo_t *nodo);
void iniciar_hilo_receptor_de_red(grafico_t *topo);

#endif