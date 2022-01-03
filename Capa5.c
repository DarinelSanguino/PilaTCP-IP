#include "Capa5.h"

extern void bajar_datos_a_capa3(nodo_t *nodo, char *datos, unsigned int tamano_datos, unsigned int num_protocolo, unsigned int ip_destino);

void hacer_ping(nodo_t *nodo, char *ip_destino) {
	printf("Nodo origen: %s, IP de destino PING: %s.\n", nodo->nombre_nodo, ip_destino);
	unsigned int ip_dest;
	inet_pton(AF_INET, ip_destino, &ip_dest);
	bajar_datos_a_capa3(nodo, NULL, 0, ip_dest, ICMP);
}

void subir_datos_a_capa5(nodo_t *nodo, interface_t *interface_rec, char *datos, unsigned int tamano_datos) {
	return;
}
