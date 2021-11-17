#include "Net.h"
//#include "Grafico.h"

static unsigned int obtener_cod_hash(void *ptr, unsigned int tamano) {
	unsigned int valor = 0, i = 0;
	char *str = (char *) ptr;
	while(i < tamano) {
		valor += *str;
		valor *= 97;
		str++;
		i++;
	}
	return valor;
}

static inline char * cadena_modo_l2_intf(modo_l2_intf_t modo_l2_intf) {
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
	strncpy(nodo->prop_nodo->dir_loopback.dir_ip, dir_loopback, 16);
	nodo->prop_nodo->tiene_dir_loopback = true;
	return true;
}

bool asignar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if, char *dir_ip, char mascara) {
	assert(dir_ip);
	interface_t *interface = obtener_intf_por_nombre(nodo, nombre_if);
	assert(interface);
	interface->prop_intf->mascara = mascara;
	strncpy(interface->prop_intf->dir_ip.dir_ip, dir_ip, 16);
	interface->prop_intf->tiene_dir_ip = true;
	return true;
}

bool quitar_dir_ip_intf_nodo(nodo_t *nodo, char *nombre_if) {
	return true;
}

void asignar_dir_mac(interface_t *interface) {
	nodo_t *nodo = interface->nodo_padre;
	unsigned int codigo_hash = obtener_cod_hash(nodo->nombre_nodo, TAM_NOMBRE_NODO);
	codigo_hash *= obtener_cod_hash(interface->nombre_if, TAM_NOMBRE_IF);
	//char *cod_hash;
	//itoa(codigo_hash, cod_hash, 10);
	memset(interface->prop_intf->dir_mac.dir_mac, 0, sizeof(interface->prop_intf->dir_mac.dir_mac));
	memcpy(interface->prop_intf->dir_mac.dir_mac, (char *)&codigo_hash, sizeof(unsigned int));

}

char * desp_der_buf_paq(char *paquete, unsigned int tam_paq, unsigned int tam_total_buf) {
	//cab_ethernet_t *cab_ethernet = (cab_ethernet_t *) paquete;
	char *dir_fin_paq = paquete + tam_total_buf;
	char *nueva_dir_paq = dir_fin_paq - tam_paq;
	char temp[tam_paq];
	memset(temp, 0, tam_paq);
	memcpy(temp, paquete, tam_paq);
	printf("temp es %s\n", temp);
	memset(paquete, 0, tam_total_buf);
	memcpy(nueva_dir_paq, temp, tam_paq);
	printf("nueva_dir_paq es %s\n", nueva_dir_paq);
	return nueva_dir_paq;
}

void aplicar_mascara(char *dir_ip, char mascara, char *dir_ip_subred) {
	uint32_t ip_temp = 0;
	uint32_t mascara_temp = 0xFFFFFFFF;
	mascara_temp = mascara_temp << (32 - mascara);
	
	inet_pton(AF_INET, dir_ip, &ip_temp);
	ip_temp = htonl(ip_temp);
	printf("ip_temp %u.\n", ip_temp);
	
	uint32_t ip = ip_temp & mascara_temp;
	printf("ip %u.\n", ip);
	ip = htonl(ip);
	inet_ntop(AF_INET, &ip, dir_ip_subred, TAM_DIR_IP);
	dir_ip_subred[TAM_DIR_IP - 1] = '\0';
	
	printf("Dir IP: %s. Mascara: %hu.\n", dir_ip_subred, mascara);
}

/*void mostrar_prop_intf(const prop_intf_t *prop_intf) {
	
	printf("Dirección IP %s/%i\n", prop_intf->dir_ip.dir_ip, prop_intf->mascara);
	printf("Dirección MAC %s\n", prop_intf->dir_mac.dir_mac);
}*/
