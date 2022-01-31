#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "codigoscmd.h"
#include "Grafico.h"
#include "Capa3.h"
#include "Capa5.h"
#include <stdio.h>

extern grafico_t *topo;

void enviar_solicitud_broadcast_arp(nodo_t *nodo, interface_t *intf, char *dir_ip);

static int mostrar_manejador_topo_red(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre_nodo", sizeof("nombre_nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
	} TLV_LOOP_END;
	switch(CODCMD) {
		case CODCMD_MOSTRAR_TOPO_RED:
			mostrar_grafico(topo);
			break;
		case CODCMD_MOSTRAR_NODO:
			{
				nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
				mostrar_nodo(nodo);
			}
			break;
		default:
			break;
	}
	return 0;
}

static int manejador_arp(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	
	char *dir_ip = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "dir-ip", sizeof("dir-ip")) == 0) {
			dir_ip = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	enviar_solicitud_broadcast_arp(nodo, NULL, dir_ip);

	printf("Nodo %s\n", nombre_nodo);
	printf("Dirección IP %s\n", dir_ip);
	return 0;
}

static int manejador_mostrar_arp(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
	} TLV_LOOP_END;	

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);	
	mostrar_tabla_arp(TABLA_ARP_NODO(nodo));
	return 0;
}

static int manejador_mostrar_mac(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
	} TLV_LOOP_END;	

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);	
	mostrar_tabla_mac(TABLA_MAC_NODO(nodo));
	return 0;
}

static int manejador_mostrar_tabla_ruteo(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	mostrar_tabla_ruteo(TABLA_RUTEO_NODO(nodo));
	return 0;
}

static int manejador_mostrar_nodo(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {
	return 0;
}

static int validar_nombre_nodo(char *nombre_nodo) {
	return 0;
}

static int manejador_agregar_ruta(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;
	char *destino = NULL;
	char *cadena_mascara = NULL;
	char *ip_gw = NULL;
	char *intf_salida = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "destino", sizeof("destino")) == 0) {
			destino = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "mascara", sizeof("mascara")) == 0) {
			cadena_mascara = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "ip-gw", sizeof("ip-gw")) == 0) {
			ip_gw = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "intf-salida", sizeof("intf-salida")) == 0) {
			intf_salida = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	char mascara = atoi(cadena_mascara);	
	tabla_ruteo_agregar_ruta_remota(TABLA_RUTEO_NODO(nodo), destino, mascara, ip_gw, intf_salida);
	mostrar_tabla_ruteo(TABLA_RUTEO_NODO(nodo));
	return 0;
}

static int manejador_ping(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;
	char *dir_ip = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "dir-ip", sizeof("dir-ip")) == 0) {
			dir_ip = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	hacer_ping(nodo, dir_ip);
	return 0;
}

static int manejador_ping_con_nodo_intermedio(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {	
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;
	char *dir_ip = NULL;
	char *ip_nodo_intermedio = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre-nodo", sizeof("nombre-nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "dir-ip", sizeof("dir-ip")) == 0) {
			dir_ip = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "ip-nodo-intermedio", sizeof("ip-nodo-intermedio")) == 0) {
			ip_nodo_intermedio = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	hacer_ping_con_nodo_intermedio(nodo, dir_ip, ip_nodo_intermedio);
	return 0;
}

void inic_cli_red() {
	init_libcli();

	param_t *show = libcli_get_show_hook();
	param_t *debug = libcli_get_debug_hook();
	param_t *config = libcli_get_config_hook();
	param_t *run = libcli_get_run_hook();
	param_t *debug_show = libcli_get_debug_show_hook();
	param_t *root = libcli_get_root();

	{
		//Mostrar topología
		static param_t topologia;
		init_param(&topologia, CMD, "topologia", mostrar_manejador_topo_red, 0, INVALID, 0, "Arrojar Topología Completa de Red");
		libcli_register_param(show, &topologia);
		set_param_cmd_code(&topologia, CODCMD_MOSTRAR_TOPO_RED);
	}

	{
		static param_t nodo;
		init_param(&nodo, CMD, "nodo", 0, 0, INVALID, 0, "Ayuda: nodo");
		libcli_register_param(show, &nodo);
		{
			static param_t nombre_nodo;
			init_param(&nombre_nodo, LEAF, 0, manejador_mostrar_nodo, validar_nombre_nodo, STRING, "nombre-nodo", "Ayuda: nombre del nodo");
			libcli_register_param(&nodo, &nombre_nodo);
			set_param_cmd_code(&nombre_nodo, CODCMD_MOSTRAR_NODO);
			{
				static param_t arp;
				init_param(&arp, CMD, "arp", manejador_mostrar_arp, 0, INVALID, 0, "Ayuda: mostrar tabla ARP");
				libcli_register_param(&nombre_nodo, &arp);
				set_param_cmd_code(&arp, CODCMD_MOSTRAR_ARP);
			}
			{
				static param_t mac;
				init_param(&mac, CMD, "mac", manejador_mostrar_mac, 0, INVALID, 0, "Ayuda: mostrar tabla MAC");
				libcli_register_param(&nombre_nodo, &mac);
				set_param_cmd_code(&mac, CODCMD_MOSTRAR_MAC);
			}
			{
				static param_t tabla_ruteo;
				init_param(&tabla_ruteo, CMD, "tabla-ruteo", manejador_mostrar_tabla_ruteo, 0, INVALID, 0, "Ayuda: mostrar tabla de ruteo");
				libcli_register_param(&nombre_nodo, &tabla_ruteo);
				set_param_cmd_code(&tabla_ruteo, CODCMD_MOSTRAR_TABLA_RUTEO);
			}
		}		
	}

	{	
		static param_t nodo;
		init_param(&nodo, CMD, "nodo", 0, 0, INVALID, 0, "Configurar propiedades del nodo");
		libcli_register_param(config, &nodo);
		{
			static param_t nombre_nodo;
			init_param(&nombre_nodo, LEAF, 0, 0, 0, STRING, "nombre-nodo", "Ayuda: nombre del nodo");
			libcli_register_param(&nodo, &nombre_nodo);
			{
				static param_t ruta;
				init_param(&ruta, CMD, "ruta", 0, 0, INVALID, 0, "Agregar ruta remota");
				libcli_register_param(&nombre_nodo, &ruta);
				{
					static param_t destino;
					init_param(&destino, LEAF, 0, 0, 0, STRING, "destino", "Ayuda: IP de destino");
					libcli_register_param(&ruta, &destino);
					{
						static param_t mascara;
						init_param(&mascara, LEAF, 0, 0, 0, INT, "mascara", "Ayuda: mascara de IP");
						libcli_register_param(&destino, &mascara);
						{
							static param_t ip_gw;
							init_param(&ip_gw, LEAF, 0, 0, 0, STRING, "ip-gw", "Ayuda: direcion IP de gateway");
							libcli_register_param(&mascara, &ip_gw);
							{
								static param_t intf_salida;
								init_param(&intf_salida, LEAF, 0, manejador_agregar_ruta, 0, STRING, "intf-salida", "Ayuda: interface de salida");
								libcli_register_param(&ip_gw, &intf_salida);
								set_param_cmd_code(&intf_salida, CODCMD_AGREGAR_RUTA);
							}
						}
					}

				}
			}
		}
		
	}

	{
		static param_t nodo;
		init_param(&nodo, CMD, "nodo", 0, 0, INVALID, 0, "Arrojar nodo");
		libcli_register_param(run, &nodo);
		{
			static param_t nombre_nodo;			
			init_param(&nombre_nodo, LEAF, 0, 0, 0, STRING, "nombre-nodo", "Ayuda: nombre del nodo");
			libcli_register_param(&nodo, &nombre_nodo);
			{
				static param_t resolver_arp;
				init_param(&resolver_arp, CMD, "resolver-arp", 0, 0, INVALID, 0, "Resolver arp");
				libcli_register_param(&nombre_nodo, &resolver_arp);
				{
					static param_t dir_ip;
					init_param(&dir_ip, LEAF, 0, manejador_arp, 0, STRING, "dir-ip", "Ayuda: dirección IP");
					libcli_register_param(&resolver_arp, &dir_ip);
					set_param_cmd_code(&dir_ip, CODCMD_RESOLVER_ARP);
				}

				static param_t ping;
				init_param(&ping, CMD, "ping", 0, 0, INVALID, 0, "Hacer PING");
				libcli_register_param(&nombre_nodo, &ping);
				{
					static param_t dir_ip;
					init_param(&dir_ip, LEAF, 0, manejador_ping, 0, STRING, "dir-ip", "Ayuda: dirección IP");
					libcli_register_param(&ping, &dir_ip);
					set_param_cmd_code(&dir_ip, CODCMD_HACER_PING);
					{
						static param_t nodo_intermedio;
						init_param(&nodo_intermedio, CMD, "nodo-intermedio", 0, 0, INVALID, 0, "Ayuda: nodo intermedio");
						libcli_register_param(&dir_ip, &nodo_intermedio);
						{
							static param_t ip_nodo_intermedio;
							init_param(&ip_nodo_intermedio, LEAF, 0, manejador_ping_con_nodo_intermedio, 0, STRING, "ip-nodo-intermedio", "Ayuda: dirección IP del nodo intermedio");
							libcli_register_param(&nodo_intermedio, &ip_nodo_intermedio);
							set_param_cmd_code(&ip_nodo_intermedio, CODCMD_HACER_PING_CON_NODO_INTERMEDIO);
						}

					}
				}
			}
		}	
	}
	support_cmd_negation(config);
}
