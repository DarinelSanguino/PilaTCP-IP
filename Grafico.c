#include "Grafico.h"

extern void inic_sock_udp(nodo_t *nodo);

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


/**********CAMBIO_TEMPORAL***************************/
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
	/*****PENDIENTE:violación de segmento*****/
	printf("%s\n", grafico->nombre_topologia);
	//nodo_t *primero = *(nodo_t **)(grafico->lista_nodos->nodo_inicio->elemento);	
	//printf("Nodo primero: %s", primero->nombre_nodo);
	//recorrer_lista(grafico->lista_nodos, mostrar_nodo);
	ITERAR_LISTA_ENLAZADA(grafico->lista_nodos) {
		nodo_t *nodo_red = *(nodo_t **)(nodo_actual->elemento);
		//printf("Nodo primero otra vez: %s", primero->nombre_nodo);
		//printf("Nodo %s", nodo_red->nombre_nodo);
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
	printf("Interface %s: ", interface->nombre_if);
	//printf("dirección IP %s/%i ", interface->prop_intf->dir_ip.dir_ip, interface->prop_intf->mascara);
	//printf("dirección MAC %s\n", interface->prop_intf->dir_mac.dir_mac);
	//mostrar_dir_mac(interface->prop_intf->dir_mac);
	//mostrar_enlace(interface->enlace);
}

void mostrar_enlace(const enlace_t *enlace) {
	printf("Enlace entre nodos %s y %s, costo %u\n", enlace->intf1.nombre_if, enlace->intf2.nombre_if, enlace->costo);
}

void mostrar_prop_intf(const prop_intf_t *prop_intf) {
	/**************CAMBIO_TEMPORAL******************/
	printf("Dirección IP %s/%i\n", prop_intf->dir_ip.dir_ip, prop_intf->mascara);
	//printf("Dirección MAC %s\n", prop_intf->dir_mac.dir_mac);
	mostrar_dir_mac(&prop_intf->dir_mac);
}

void mostrar_dir_mac(const dir_mac_t *dir_mac) {
	printf("Dirección MAC ");
	for(int i = 0; i < TAM_DIR_MAC; i++) {
		printf("%u-", dir_mac->dir_mac[i]);
	}
	printf("\n");
}

interface_t* obtener_intf_correspondiente_a_nodo(nodo_t *nodo, char *dir_ip) {
	interface_t *interface_actual = NULL;
	for (int i = 0; i < MAX_INTF_POR_NODO; ++i) {
		interface_actual = nodo->intf[i];
		if(!interface_actual) return NULL;
		if(strncmp(interface_actual->prop_intf->dir_ip.dir_ip, dir_ip, TAM_DIR_IP) == 0) return interface_actual;	
	}
	return NULL;
}