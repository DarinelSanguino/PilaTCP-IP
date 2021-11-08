#include "Topologias.h"

grafico_t * const_primera_topo() {
	grafico_t *primer_grafico = crear_nuevo_grafico("Grafico Hola Mundo");
	nodo_t *nodo_R0 = crear_nodo_grafico(primer_grafico, "R0");
	printf("Grafico despues de la primera insercion.\n");
	mostrar_grafico(primer_grafico);
	nodo_t *nodo_R1 = crear_nodo_grafico(primer_grafico, "R1");
	nodo_t *nodo_R2 = crear_nodo_grafico(primer_grafico, "R2");

	insertar_enlace_entre_nodos(nodo_R0, nodo_R1, "ethR0/0", "ethR1/0", 1);
	insertar_enlace_entre_nodos(nodo_R0, nodo_R2, "ethR0/1", "ethR2/0", 1);
	insertar_enlace_entre_nodos(nodo_R1, nodo_R2, "ethR1/1", "ethR2/1", 1);

	asignar_dir_loopback_nodo(nodo_R0, "122.1.1.0");
	asignar_dir_ip_intf_nodo(nodo_R0, "ethR0/0", "40.1.1.1", 24);
	asignar_dir_ip_intf_nodo(nodo_R0, "ethR0/1", "20.1.1.1", 24);

	asignar_dir_loopback_nodo(nodo_R1, "122.1.1.1");
	asignar_dir_ip_intf_nodo(nodo_R1, "ethR1/0", "40.1.1.2", 24);
	asignar_dir_ip_intf_nodo(nodo_R1, "ethR1/1", "30.1.1.1", 24);

	asignar_dir_loopback_nodo(nodo_R2, "122.1.1.2");
	asignar_dir_ip_intf_nodo(nodo_R2, "ethR2/0", "20.1.1.2", 24);
	asignar_dir_ip_intf_nodo(nodo_R2, "ethR2/1", "30.1.1.2", 24);

	iniciar_hilo_receptor_de_red(primer_grafico);

	return primer_grafico;
}
