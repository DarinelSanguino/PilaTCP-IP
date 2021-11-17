#include "Topologias.h"

grafico_t * const_primera_topologia() {
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

grafico_t * const_topologia_switch_capa2() {
	grafico_t *grafico_capa2 = crear_nuevo_grafico("Grafico con switch de capa 2");
	nodo_t *nodo_H0 = crear_nodo_grafico(grafico_capa2, "H0");	
	nodo_t *nodo_H1 = crear_nodo_grafico(grafico_capa2, "H1");
	nodo_t *nodo_H2 = crear_nodo_grafico(grafico_capa2, "H2");
	nodo_t *nodo_H3 = crear_nodo_grafico(grafico_capa2, "H3");
	nodo_t *SWC2 = crear_nodo_grafico(grafico_capa2, "HSWC2");

	insertar_enlace_entre_nodos(nodo_H0, SWC2, "ethR0/0", "ethC0/0", 1);
	insertar_enlace_entre_nodos(nodo_H1, SWC2, "ethR1/0", "ethC0/1", 1);
	insertar_enlace_entre_nodos(nodo_H2, SWC2, "ethR2/0", "ethC0/2", 1);
	insertar_enlace_entre_nodos(nodo_H3, SWC2, "ethR3/0", "ethC0/3", 1);

	asignar_dir_loopback_nodo(nodo_H0, "122.1.1.1");
	asignar_dir_ip_intf_nodo(nodo_H0, "ethR0/0", "10.1.1.1", 24);
	
	asignar_dir_loopback_nodo(nodo_H1, "122.1.1.2");
	asignar_dir_ip_intf_nodo(nodo_H1, "ethR1/0", "10.1.1.2", 24);

	asignar_dir_loopback_nodo(nodo_H2, "122.1.1.3");
	asignar_dir_ip_intf_nodo(nodo_H2, "ethR2/0", "10.1.1.3", 24);

	asignar_dir_loopback_nodo(nodo_H3, "122.1.1.4");
	asignar_dir_ip_intf_nodo(nodo_H3, "ethR3/0", "10.1.1.4", 24);

	conf_intf_modo_l2(nodo_H0, "ethR0/0", ACCESO);
	conf_intf_modo_l2(nodo_H1, "ethR1/0", ACCESO);
	conf_intf_modo_l2(nodo_H2, "ethR2/0", ACCESO);
	conf_intf_modo_l2(nodo_H3, "ethR3/0", ACCESO);

	iniciar_hilo_receptor_de_red(grafico_capa2);

	return grafico_capa2;
}
