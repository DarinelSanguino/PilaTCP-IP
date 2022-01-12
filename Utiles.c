#include "Utiles.h"

void capa2_llenar_con_mac_broadcast(unsigned char *arreglo_mac) {
	unsigned char dir_mac[TAM_DIR_MAC] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	memcpy(arreglo_mac, dir_mac, TAM_DIR_MAC);
}

uint32_t dir_ip_p_a_n(char *dir_ip) {
	uint32_t ip_num;
	inet_pton(AF_INET, dir_ip, &ip_num);
	return htonl(ip_num);
}

char * dir_ip_n_a_p(uint32_t dir_ip, char *cadena_dir_ip) {
	dir_ip = htonl(dir_ip);
	memset(cadena_dir_ip, 0, TAM_DIR_IP);
	cadena_dir_ip = inet_ntop(AF_INET, &dir_ip, cadena_dir_ip, TAM_DIR_IP);
	cadena_dir_ip[TAM_DIR_IP - 1] = '\0';
	return cadena_dir_ip;
}
