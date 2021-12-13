#ifndef SWITCHCAPA2_H_
#define SWITCHCAPA2_H_

#include "Grafico.h"
#include "Com.h"
#include "Capa2.h"

void recibir_trama_switch_capa2(interface_t *interface, cab_ethernet_t *cab_ethernet, unsigned int tamano_paq, bool etiqueta_vlan);
bool enviar_paquete_switch_capa2(interface_t *intf_salida, char *paquete, unsigned int tamano_paq, bool *etiqueta_vlan);
void enviar_paquete_interfaces_switch_capa2(nodo_t *nodo, interface_t *intf_exenta, char *paquete, unsigned int tamano_paq);

#endif
