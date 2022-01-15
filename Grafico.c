#include "Grafico.h"

extern void inic_sock_udp(nodo_t *nodo);
extern void mostrar_prop_vlan(modo_l2_intf_t modo_l2_intf, const unsigned int *vlans);

static inline int obtener_ranura_intf_disp(nodo_t *nodo) {
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i)
	{		
		if (nodo->intf[i] == NULL) return i;		
	}
	return -1;
}

nodo_t* obtener_nodo_vecino(interface_t *interface) {
	interface_t *intf_vecina = &interface->enlace->intf1 == interface ? &interface->enlace->intf2 : &interface->enlace->intf1;
	return intf_vecina->nodo_padre;
}

nodo_t * obtener_nodo_por_nombre(grafico_t *topologia, char *nombre_nodo) {
	return obtener_elemento(topologia->lista_nodos, nombre_nodo);
}

interface_t * obtener_intf_por_nombre(nodo_t *nodo, char *nombre_if) {
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i)
	{
		if(!nodo->intf[i]) return NULL;
		if(strncmp(nodo->intf[i]->nombre_if, nombre_if, TAM_NOMBRE_IF) == 0) {
			return nodo->intf[i];
		}
	}
	return NULL;
}

grafico_t * crear_nuevo_grafico(const char *nombre_topologia) {
	grafico_t *grafico = malloc(sizeof(grafico_t));
	inicializar_grafico(grafico);
	strncpy(grafico->nombre_topologia, nombre_topologia, 32);
	
	grafico->nombre_topologia[31] = '\0';
	return grafico;
}

nodo_t * crear_nodo_grafico(grafico_t *grafico, const char *nombre_nodo) {	
	nodo_t *nodo = malloc(sizeof(nodo_t));
	inicializar_nodo(nodo);
	strncpy(nodo->nombre_nodo, nombre_nodo, TAM_NOMBRE_NODO);
	nodo->nombre_nodo[TAM_NOMBRE_NODO - 1] = '\0';
	inic_sock_udp(nodo);
	insertar(grafico->lista_nodos, &nodo, sizeof(nodo_t *));
	return nodo;
}

void insertar_enlace_entre_nodos(nodo_t *nodo1, nodo_t *nodo2, char *de_nombre_if, char *a_nombre_if, unsigned int costo) {
	int ranura1 = obtener_ranura_intf_disp(nodo1);
	int ranura2 = obtener_ranura_intf_disp(nodo2);

	if(ranura1 != -1 && ranura2 != -1) {
		enlace_t *enlace = malloc(sizeof(enlace_t));
		inicializar_interface(&enlace->intf1);
		inicializar_interface(&enlace->intf2);
		strncpy(enlace->intf1.nombre_if, de_nombre_if, TAM_NOMBRE_IF);
		enlace->intf1.nombre_if[TAM_NOMBRE_IF - 1] = '\0';
		strncpy(enlace->intf2.nombre_if, a_nombre_if, TAM_NOMBRE_IF);
		enlace->intf2.nombre_if[TAM_NOMBRE_IF - 1] = '\0';

		enlace->intf1.enlace = enlace;
		enlace->intf2.enlace = enlace;
		
		enlace->intf1.nodo_padre = nodo1;
		enlace->intf2.nodo_padre = nodo2;
		nodo1->intf[ranura1] = &(enlace->intf1);
		nodo2->intf[ranura2] = &(enlace->intf2);

		init_prop_intf(enlace->intf1.prop_intf);
		init_prop_intf(enlace->intf2.prop_intf);
		asignar_dir_mac(&enlace->intf1);
		asignar_dir_mac(&enlace->intf2);

		enlace->costo = costo;
	}
}

void inicializar_interface(interface_t *interface) {
	memset(interface->nombre_if, 0, TAM_NOMBRE_IF);
	interface->nodo_padre = malloc(sizeof(nodo_t));
	interface->enlace = malloc(sizeof(enlace_t));
	interface->prop_intf = malloc(sizeof(prop_intf_t));
}

void inicializar_nodo(nodo_t *nodo) {
	nodo->nombre_nodo[0] = '\0';
	for(int i = 0; i < MAX_INTF_POR_NODO; i++) {
		nodo->intf[i] = NULL;	
	}
	nodo->prop_nodo = malloc(sizeof(prop_nodo_t));
	init_prop_nodo(nodo->prop_nodo);
}

nodo_t* obtener_elemento(Lista_t *lista, char *nombre_nodo) {
	ITERAR_LISTA_ENLAZADA(lista) {
		nodo_t *nodo_red = *(nodo_t **)(nodo_actual->elemento);
		if(!strncmp(nodo_red->nombre_nodo, nombre_nodo, TAM_NOMBRE_NODO)) {
			return nodo_red;
		}
	} FIN_ITERACION;
	return NULL;
}

void inicializar_grafico(grafico_t *grafico) {
	grafico->nombre_topologia[0] = '\0';
	grafico->lista_nodos = malloc(sizeof(Lista_t));
	grafico->lista_nodos->vacia = true;
}

void mostrar_grafico(const grafico_t *grafico) {
	printf("%s\n", grafico->nombre_topologia);
	
	ITERAR_LISTA_ENLAZADA(grafico->lista_nodos) {
		nodo_t *nodo_red = *(nodo_t **)(nodo_actual->elemento);		
		mostrar_nodo(nodo_red);
	} FIN_ITERACION;
}

void mostrar_nodo(const nodo_t *nodo) {
	printf("Nodo %s: \n", nodo->nombre_nodo);
	printf("dirección loopback %s\n", nodo->prop_nodo->dir_loopback.dir_ip);
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i)	{		
		if(nodo->intf[i] == NULL) {
			break;
		}
		else {
			mostrar_interface(nodo->intf[i]);
		}
	}
}

void mostrar_interface(const interface_t *interface) {
	printf("Interface %s: \n", interface->nombre_if);	
	mostrar_enlace(interface->enlace);
	mostrar_prop_intf(interface->prop_intf);
	
}

void mostrar_enlace(const enlace_t *enlace) {
	printf("Enlace entre nodos %s y %s, costo %u.\n", enlace->intf1.nombre_if, enlace->intf2.nombre_if, enlace->costo);
}

void mostrar_prop_intf(const prop_intf_t *prop_intf) {	
	printf("Dirección IP %s/%i\n", prop_intf->dir_ip.dir_ip, prop_intf->mascara);
	mostrar_dir_mac(&prop_intf->dir_mac);
	mostrar_prop_vlan(prop_intf->modo_l2_intf, prop_intf->vlans);
}

void mostrar_dir_mac(const dir_mac_t *dir_mac) {
	printf("Dirección MAC ");
	for(int i = 0; i < TAM_DIR_MAC; i++) {
		printf("%hu-", dir_mac->dir_mac[i] & 0xFF);
	}
	printf("\n");
}

interface_t* obtener_intf_correspondiente_a_nodo(nodo_t *nodo, char *dir_ip) {
	interface_t *interface_actual = NULL;
	char *dir_ip_local;
	char mascara;
	char subred_dir_ip_local[TAM_DIR_IP];
	char subred_dir_ip[TAM_DIR_IP];
	for(int i = 0; i < MAX_INTF_POR_NODO; ++i) {
		interface_actual = nodo->intf[i];
		if(!interface_actual) return NULL;		
		dir_ip_local = IP_IF(interface_actual);
		mascara = MASCARA_IF(interface_actual);
		aplicar_mascara(dir_ip_local, mascara, subred_dir_ip_local);
		aplicar_mascara(dir_ip, mascara, subred_dir_ip);
		if(strncmp(subred_dir_ip, subred_dir_ip_local, TAM_DIR_IP) == 0) {
			return interface_actual;
		}
	}
	return NULL;
}
