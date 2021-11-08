#ifndef UTILES_H_
#define UTILES_H_

#include <arpa/inet.h>
#include "Net_fwd.h"
#include "Grafico.h"
#include "Capa2_fwd.h"

#define ES_DIR_MAC_BROADCAST(mac) (mac[0] == 0xFF && mac[1] == 0xFF && mac[2] == 0xFF && mac[3] == 0xFF && mac[4] == 0xFF && mac[5] == 0xFF && mac[6] == 0xFF)

void capa2_llenar_con_mac_broadcast(char *arreglo_mac);
uint32_t dir_ip_p_a_n(char *dir_ip);
char * dir_ip_n_a_p(uint32_t dir_ip, char *cadena_dir_ip);
#endif