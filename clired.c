#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "codigoscmd.h"
#include "Grafico.h"
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
	printf("Aquí\n");
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	
	char *dir_ip = NULL;

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre_nodo", sizeof("nombre_nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
		if(strncmp(tlv->leaf_id, "dir_ip", sizeof("dir_ip")) == 0) {
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
	printf("Aquí\n");
	int CODCMD = -1;
	CODCMD = EXTRACT_CMD_CODE(buf_tlv);

	tlv_struct_t *tlv = NULL;
	char *nombre_nodo = NULL;	

	TLV_LOOP_BEGIN(buf_tlv, tlv) {
		if(strncmp(tlv->leaf_id, "nombre_nodo", sizeof("nombre_nodo")) == 0) {
			nombre_nodo = tlv->value;
		}
	} TLV_LOOP_END;

	nodo_t *nodo = obtener_nodo_por_nombre(topo, nombre_nodo);
	mostrar_tabla_arp(nodo->prop_nodo->tabla_arp);
	return 0;
}

static int manejador_mostrar_nodo(param_t *param, ser_buff_t *buf_tlv, op_mode hab_o_deshab) {
	return 0;
}

static int validar_nombre_nodo(char *nombre_nodo) {
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
		}		
	}

	/*{
		static param_t nodo;
		init_param(&nodo, CMD, "nodo", 0, 0, INVALID, 0, "Arrojar nodo");
		libcli_register_param(show, &nodo);
		{
			static param_t nombre_nodo;
			//Validación de nombre pendiente
			init_param(&nombre_nodo, LEAF, 0, mostrar_manejador_topo_red, 0, STRING, "nombre_nodo", "Ayuda: nombre del nodo");
			libcli_register_param(&nodo, &nombre_nodo);
			set_param_cmd_code(&nombre_nodo, CODCMD_MOSTRAR_NODO);
		}		
	}*/

	{
		static param_t nodo;
		init_param(&nodo, CMD, "nodo", 0, 0, INVALID, 0, "Arrojar nodo");
		libcli_register_param(run, &nodo);
		{
			static param_t nombre_nodo;
			//Validación de nombre pendiente
			init_param(&nombre_nodo, LEAF, 0, 0, 0, STRING, "nombre_nodo", "Ayuda: nombre del nodo");
			libcli_register_param(&nodo, &nombre_nodo);
			{
				static param_t resolver_arp;
				init_param(&resolver_arp, CMD, "resolver-arp", 0, 0, INVALID, 0, "Resolver arp");
				libcli_register_param(&nombre_nodo, &resolver_arp);
				{
					static param_t dir_ip;
					init_param(&dir_ip, LEAF, 0, manejador_arp, 0, STRING, "dir_ip", "Ayuda: dirección IP");
					libcli_register_param(&resolver_arp, &dir_ip);
					set_param_cmd_code(&dir_ip, CODCMD_RESOLVER_ARP);
				}
			}
		}	
	}
	support_cmd_negation(config);
}
