#include "Com.h"

extern int errno;

static unsigned int numero_puerto_udp = 40000;
static char buffer_rec[MAX_TAMANO_BUFFER_PAQ];
static char buffer_trans[MAX_TAMANO_BUFFER_PAQ];

static unsigned int get_next_udp_port_number() {
	return numero_puerto_udp++;
}

static void _recibir_paquete(nodo_t *nodo_rec, char *paquete_con_datos_aux, unsigned int tamano_paq) {
	char *nombre_if = paquete_con_datos_aux;	
	interface_t *intf_destino = obtener_intf_por_nombre(nodo_rec, nombre_if);
	if(!intf_destino) {
		printf("Paquete recibido en interface desconocida %s del nodo %s\n.", nombre_if, nodo_rec->nombre_nodo);
		return;
	}
	//Aquí ya no se incluye la información sobre la interface de destino.
	recibir_paquete(nodo_rec, intf_destino, paquete_con_datos_aux + TAM_NOMBRE_IF, tamano_paq - TAM_NOMBRE_IF, nombre_if);
}

int recibir_paquete(nodo_t *nodo_rec, interface_t *interface, char *paquete, unsigned int tamano_paq, char *inicio) {
	printf("El tamaño del paquete recibido en %s es %u bytes.\n", nodo_rec->nombre_nodo, tamano_paq);
	//MAX_TAMANO_BUFFER_PAQ - TAM_NOMBRE_IF es el espacio que queda disponible considerando el espacio que ocupa la información de la interface.
	paquete = desp_der_buf_paq(paquete, tamano_paq, MAX_TAMANO_BUFFER_PAQ - TAM_NOMBRE_IF);	
	recibir_trama_capa2(nodo_rec, interface, paquete, tamano_paq);
	return 0;
}

static void revisar_set_fd(Lista_t *lista, fd_set *set_fd_sock) {
	struct sockaddr_in dir_transmisor;
	unsigned int long_dir = sizeof(struct sockaddr);
	ITERAR_LISTA_ENLAZADA(lista) {
		nodo_t *nodo_red = *(nodo_t **)(nodo_actual->elemento);
		if(FD_ISSET(nodo_red->fd_sock_udp, set_fd_sock)) {
			memset(buffer_rec, 0, MAX_TAMANO_BUFFER_PAQ);
			int bytes_rec = recvfrom(nodo_red->fd_sock_udp, buffer_rec, MAX_TAMANO_BUFFER_PAQ, 0, (struct sockaddr *) &dir_transmisor, &long_dir);
			_recibir_paquete(nodo_red, buffer_rec, bytes_rec);
		}
	} FIN_ITERACION;
}

static void agregar_set_fd(Lista_t *lista, fd_set *set_fd_sock, int *max_sock_fd) {
	ITERAR_LISTA_ENLAZADA(lista) {
		nodo_t *nodo_red = *(nodo_t **)(nodo_actual->elemento);
		if(!nodo_red->fd_sock_udp) continue;

		if(nodo_red->fd_sock_udp > *max_sock_fd) {
			*max_sock_fd = nodo_red->fd_sock_udp;
		}
		FD_SET(nodo_red->fd_sock_udp, set_fd_sock);
	} FIN_ITERACION;
}

static void * _iniciar_hilo_receptor_de_red(void *arg) {
	fd_set fd_set_sock_activo, fd_set_sock_respaldo;

	int max_sock_fd = 0;

	grafico_t *topo = (void *)arg;	

	FD_ZERO(&fd_set_sock_activo);
	FD_ZERO(&fd_set_sock_respaldo);

	agregar_set_fd(topo->lista_nodos, &fd_set_sock_respaldo, &max_sock_fd);

	while(1) {
		memcpy(&fd_set_sock_activo, &fd_set_sock_respaldo, sizeof(fd_set));
		select(max_sock_fd + 1, &fd_set_sock_activo, NULL, NULL, NULL);
		revisar_set_fd(topo->lista_nodos, &fd_set_sock_activo);
	}
}

static int _enviar_paquete(int sock, char *paquete_comp, unsigned int tamano_paq, unsigned int puerto_udp_destino) {
	int rc;
	struct sockaddr_in dir_destino;
	struct hostent *host = (struct hostent *) gethostbyname("127.0.0.1");
	dir_destino.sin_family = AF_INET;
	dir_destino.sin_port = puerto_udp_destino;
	dir_destino.sin_addr = *((struct in_addr *)host->h_addr);
	rc = sendto(sock, paquete_comp, tamano_paq, 0, (struct sockaddr *)&dir_destino, sizeof(struct sockaddr));
	return rc;
}

int enviar_paquete(char *paquete, unsigned int tamano_paq, interface_t *intf_origen) {
	int rc = 0;
	nodo_t *nodo_origen = intf_origen->nodo_padre;
	nodo_t *nodo_destino = obtener_nodo_vecino(intf_origen);
	//printf("Nodo de destino: %s.\n", nodo_destino->nombre_nodo);	
	if(!nodo_destino) return -1;

	unsigned int puerto_udp_destino = nodo_destino->numero_puerto_udp;
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0) {
		printf("Error en la creación del socket de envío.\n");
		return -1;
	}
	
	interface_t *intf_destino = &intf_origen->enlace->intf1 == intf_origen ? &intf_origen->enlace->intf2 : &intf_origen->enlace->intf1;
	memset(buffer_trans, 0, MAX_TAMANO_BUFFER_PAQ);
	char *paquete_con_datos_aux = buffer_trans;
	strncpy(paquete_con_datos_aux, intf_destino->nombre_if, TAM_NOMBRE_IF);
	paquete_con_datos_aux[TAM_NOMBRE_IF] = '\0';
	memcpy(paquete_con_datos_aux + TAM_NOMBRE_IF, paquete, tamano_paq);
	printf("Tamaño de paquete saliente desde %s: %u.\n", nodo_origen->nombre_nodo, tamano_paq);
	rc = _enviar_paquete(sock, paquete_con_datos_aux, tamano_paq + TAM_NOMBRE_IF, puerto_udp_destino);
	close(sock);
	return rc;
}

void inic_sock_udp(nodo_t *nodo) {
	int errnum;
	nodo->numero_puerto_udp = get_next_udp_port_number();
	printf("%s: %u\n", nodo->nombre_nodo, nodo->numero_puerto_udp);
	int fd_sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd_sock_udp == -1) {
		printf("Creación fallida del socket con número de puerto %u.\n", nodo->numero_puerto_udp);
		return;
	}

	struct sockaddr_in dir_nodo;
	dir_nodo.sin_family = AF_INET;
	dir_nodo.sin_port = nodo->numero_puerto_udp;
	dir_nodo.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd_sock_udp, (struct sockaddr *) &dir_nodo, sizeof(struct sockaddr)) == -1) {
		errnum = errno;
		fprintf(stderr, "Value of errno: %d\n", errno);
		perror("Error printed by perror");
		fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
		printf("Error: anexión de socket %u fallida para el nodo %s.\n", fd_sock_udp, nodo->nombre_nodo);
		return;
	}
	nodo->fd_sock_udp = fd_sock_udp;
}

void iniciar_hilo_receptor_de_red(grafico_t *topo) {
	pthread_attr_t atrib;
	pthread_t hilo_receptor;

	pthread_attr_init(&atrib);
	pthread_attr_setdetachstate(&atrib, PTHREAD_CREATE_DETACHED);

	pthread_create(&hilo_receptor, &atrib, _iniciar_hilo_receptor_de_red, (void *)topo);
}

int enviar_paquete_interfaces_capa2(nodo_t *nodo, interface_t *intf_entrada, char *paquete, unsigned int tamano_paq) {
	interface_t *intf_actual = NULL;
	for(int i = 0; i < MAX_INTF_POR_NODO; i++) {
		intf_actual = nodo->intf[i];
		if(!intf_actual) return 0;
		if(intf_actual != intf_entrada) {
			enviar_paquete(paquete, tamano_paq, intf_actual);
		}
	}
	return 1;
}
