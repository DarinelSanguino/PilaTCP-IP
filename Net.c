#include "Net.h"

extern bool tabla_ruteo_agregar_ruta_local(tabla_ruteo_t *tabla_ruteo, char *destino, char mascara);

static unsigned long obtener_cod_hash(void *ptr, unsigned int tamano) {
	unsigned long valor = 0, i = 0;
	char *str = (char *) ptr;
	while(i < tamano) {
		valor += *str;
		valor *= 97;
		str++;
		i++;
	}
	return valor;
}

void init_prop_nodo(prop_nodo_t *prop_nodo) {
	prop_nodo->tiene_dir_loopback = false;
	memset(prop_nodo->dir_loopback.dir_ip, 0, TAM_DIR_IP);
	inic_tabla_arp(&prop_nodo->tabla_arp);
	inic_tabla_mac(&prop_nodo->tabla_mac);
	inic_tabla_ruteo(&prop_nodo->tabla_ruteo);
}

void init_prop_intf(prop_intf_t *prop_intf) {
	memset(prop_intf->dir_mac.dir_mac, 0, TAM_DIR_MAC);
	prop_intf->modo_l2_intf = DESCONOCIDO;
	for(int i = 0; i < MAX_VLANS_POR_INTF; i++) {
		prop_intf->vlans[i] = -1;
	}
	prop_intf->tiene_dir_ip = false;
	prop_intf->mascara = 0;
	memset(prop_intf->dir_ip.dir_ip, 0, TAM_DIR_IP);
}

char * cadena_modo_l2_intf(modo_l2_intf_t modo_l2_intf) {
	switch(modo_l2_intf) {
		case ACCESO:
			return "acceso";
		case TRONCAL:
			return "troncal";
		default:
			return "desconocido";
	}
}

bool asignar_dir_loopback_nodo(nodo_t *nodo, char *dir_loopback) {
	assert(dir_loopback);
	strncpy(DIR_LO_NODO(nodo), dir_loopback, TAM_DIR_IP);
	nodo->prop_nodo->tiene_dir_loopback = true;
	tabla_ruteo_agregar_ruta_local(TABLA_RUTEO_NODO(nodo), dir_loopback, 32);
	return true;
}

bool asignar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if, char *dir_ip, char mascara) {
	assert(dir_ip);
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	assert(interface);
	interface->prop_intf->mascara = mascara;
	strncpy(IP_IF(interface), dir_ip, TAM_DIR_IP);
	IF_EN_MODO_L3(interface) = true;
	tabla_ruteo_agregar_ruta_local(TABLA_RUTEO_NODO(nodo), dir_ip, mascara);
	return true;
}

bool quitar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if) {
	return true;
}

void asignar_dir_mac(interface_t *interface) {
	nodo_t *nodo = interface->nodo_padre;
	unsigned long codigo_hash = obtener_cod_hash(nodo->nombre_nodo, TAM_NOMBRE_NODO);
	codigo_hash *= obtener_cod_hash(interface->nombre_if, TAM_NOMBRE_IF);	
	memset(MAC_IF(interface), 0, sizeof(MAC_IF(interface)));
	memcpy(MAC_IF(interface), (char *)&codigo_hash, TAM_DIR_MAC);
}

char * desp_der_buf_paq(char *paquete, unsigned int tam_paq, unsigned int tam_total_buf) {
	//cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	char *dir_fin_paq = paquete + tam_total_buf;
	char *nueva_dir_paq = dir_fin_paq - tam_paq;
	char temp[tam_paq];
	memset(temp, 0, tam_paq);
	memcpy(temp, paquete, tam_paq);	
	memset(paquete, 0, tam_total_buf);
	memcpy(nueva_dir_paq, temp, tam_paq);	
	return nueva_dir_paq;
}

void aplicar_mascara(char *dir_ip, char mascara, char *dir_ip_subred) {
	uint32_t ip_temp = 0;
	uint32_t mascara_temp = 0xFFFFFFFF;
	mascara_temp = mascara_temp << (32 - mascara);
	
	inet_pton(AF_INET, dir_ip, &ip_temp);
	ip_temp = htonl(ip_temp);	
	
	uint32_t ip = ip_temp & mascara_temp;	
	ip = htonl(ip);
	inet_ntop(AF_INET, &ip, dir_ip_subred, TAM_DIR_IP);
	dir_ip_subred[TAM_DIR_IP - 1] = '\0';
	
	//printf("Dir IP: %s. Mascara: %hu.\n", dir_ip_subred, mascara);
}