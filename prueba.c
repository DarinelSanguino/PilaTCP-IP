#include <stdio.h>
#include "Net.h"
#include "Grafico.h"
#include "Topologias.h"
#include "CommandParser/libcli.h"
#include "clired.c"

//extern void inic_cli_red();
extern grafico_t *const_primera_topo();
grafico_t *topo = NULL;


int main(void) {
	inic_cli_red();
	topo = const_primera_topo();

	sleep(2);
	nodo_t *nodo_trans = obtener_nodo_por_nombre(topo, "R0");
	interface_t *interface_trans = obtener_intf_por_nombre(nodo_trans, "ethR0/0");
	char *mensaje = "Este es un mensaje de prueba\0";
	printf("Llegamos hasta aquí señores. Fue un honor.\n");
	enviar_paquete(mensaje, strlen(mensaje), interface_trans);

	//mostrar_grafico(primera_topo);
	start_shell();
	return 0;
}