/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nm.h" /** @req NM123 */
#include "SchM_Nm.h"

#if (CONFIG_MODULE_CANNM == 1)
#include "cannm.h"
#endif
#if (CONFIG_MODULE_COMM == 1)
#include "comm_nm.h"
#endif
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
#include "bswm_bsw.h"
#endif

#include "Compiler.h"

#if (NM_DEV_ERROR_DETECT == STD_ON) && defined(VCOS_MODULE_CONFIG_DET) && (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

#define NM_E_NO_ERROR (0u)

extern const nm_cate_config nm_config;
extern boolean nm_init_status;
static boolean             nm_check_init_status(void);
static boolean             nm_check_network_handle(const network_handle_type id_net);

#define NM_NODE_ID_ENABLED                  (STD_ON) ///< Define whether to enable node ID recognition function
#define NM_NODE_DETECTION_ENABLED           (STD_ON) ///< Define whether to enable node detection function
#define NM_COM_USER_DATA_ENABLED            (STD_ON) ///< Define whether to enable the communication user data function
#define NM_BUSNM_CANNM_NOT_PASSIVE          (STD_ON) ///< Define whether CAN NM is not in passive mode

#define  NM_START_SEC_CODE
#include "nm_memmap.h"

static FUNC(boolean, NM_CODE) nm_check_init_status(void)
{
	return (nm_init_status == (boolean)TRUE);
}

static FUNC(boolean, NM_CODE) nm_check_network_handle(const network_handle_type id_net)
{
	return ((id_net < nm_config.tunnel_trans_len) && (NM_CHANNEL_COUNT >
	nm_config.tunnel_trans[id_net]));
}

FUNC(void, NM_CODE) nm_init(void)
{
	nm_init_status = (boolean)TRUE;
}

FUNC(void, NM_CODE) nm_de_init(void)
{
	nm_init_status = (boolean)FALSE;
}

FUNC(Std_ReturnType, NM_CODE) nm_pas_stup(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PASSIVESTARTUP_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PASSIVESTARTUP_NM_E_HANDLE_UNDEF);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
	#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				status = cannm_pas_stup(id_net);
				break;
	#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}
	return status;
}

#if (CANNM_PASSIVE_MODE_ENABLED == STD_OFF)
FUNC(Std_ReturnType, NM_CODE) nm_req_net(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKREQUEST_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKREQUEST_NM_E_HANDLE_UNDEF);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1) && (NM_BUSNM_CANNM_NOT_PASSIVE == STD_ON)
			case NM_BUSNM_CANNM:
				status = cannm_req_net(id_net);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (CANNM_PASSIVE_MODE_ENABLED == STD_OFF)
FUNC(Std_ReturnType, NM_CODE) nm_rel_net(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKRELEASE_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKRELEASE_NM_E_HANDLE_UNDEF);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1) && (NM_BUSNM_CANNM_NOT_PASSIVE == STD_ON)
			case NM_BUSNM_CANNM:
				status = cannm_rel_net(id_net);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_COM_CONTROL_ENABLED == STD_ON)
FUNC(Std_ReturnType, NM_CODE) nm_disable_comm(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_DISABLECOMMUNICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_DISABLECOMMUNICATION_NM_E_HANDLE_UNDEF);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1) && (CANNM_COM_CONTROL_ENABLED == STD_ON)
			case NM_BUSNM_CANNM:
				status = cannm_disable_comm(id_net);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_COM_CONTROL_ENABLED == STD_ON)
FUNC(Std_ReturnType, NM_CODE) nm_enable_comm(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_ENABLECOMMUNICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_ENABLECOMMUNICATION_NM_E_HANDLE_UNDEF);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
	#if (CONFIG_MODULE_CANNM == 1)
	#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
			case NM_BUSNM_CANNM:
				status = cannm_enable_comm(id_net);
				break;
	#endif
	#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_COM_USER_DATA_ENABLED == STD_OFF) && (NM_USER_DATA_ENABLED == STD_ON) && (CANNM_PASSIVE_MODE_ENABLED == STD_OFF)
FUNC(Std_ReturnType, NM_CODE) nm_set_ur_data(const network_handle_type id_net, const uint8 *const data_ur_ptr)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_SETUSERDATA_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_SETUSERDATA_NM_E_HANDLE_UNDEF);
	} else if (data_ur_ptr == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_SETUSERDATA_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1) && (NM_BUSNM_CANNM_NOT_PASSIVE == STD_ON)
			case NM_BUSNM_CANNM:
				status = cannm_set_ur_data(id_net, data_ur_ptr);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_USER_DATA_ENABLED == STD_ON)
FUNC(Std_ReturnType, NM_CODE) nm_get_ur_data(const network_handle_type id_net, uint8 *const data_ur_ptr)
{
#if (CONFIG_MODULE_CANNM == 1)
	Std_ReturnType user_data_ret = E_NOT_OK;
	Std_ReturnType node_id_ret = E_NOT_OK;
	uint8 nm_node_id_ptr;
#endif

	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETUSERDATA_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETUSERDATA_NM_E_HANDLE_UNDEF);
	} else if (data_ur_ptr == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETUSERDATA_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				user_data_ret = cannm_get_ur_data(id_net, data_ur_ptr);
				node_id_ret = cannm_get_nd_id(id_net, &nm_node_id_ptr);
				break;
#endif
			default:
				(void)(id_net);
				break;
		}

#if (CONFIG_MODULE_CANNM == 1)
		if ((E_OK != user_data_ret) || (E_OK != node_id_ret)) {
			status = E_NOT_OK;
		} else {
			status = E_OK;
		}
#endif
	}

	return status;
}
#endif

#if ((NM_NODE_ID_ENABLED == STD_ON) || (NM_NODE_DETECTION_ENABLED == STD_ON) || \
	 (NM_USER_DATA_ENABLED == STD_ON))
FUNC(Std_ReturnType, NM_CODE) nm_get_msg_data(const network_handle_type id_net, uint8 *const nm_pdu_data)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETPDUDATA_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETPDUDATA_NM_E_HANDLE_UNDEF);
	} else if (nm_pdu_data == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETPDUDATA_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				status = cannm_get_msg_data(id_net, nm_pdu_data);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_NODE_DETECTION_ENABLED == STD_ON)
FUNC(Std_ReturnType, NM_CODE) nm_req_rpt_msg(const network_handle_type id_net)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_REPEATMESSAGEREQUEST_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_REPEATMESSAGEREQUEST_NM_E_HANDLE_UNDEF);
	} else {
	channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

	switch (channel_conf->line_cate) {
#if ((CONFIG_MODULE_CANNM == 1) && (CANNM_NODE_DETECTION_ENABLED == 1))
			case NM_BUSNM_CANNM:
				status = cannm_req_rpt_msg(id_net);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

#if (NM_NODE_ID_ENABLED == STD_ON)
FUNC(Std_ReturnType, NM_CODE) nm_get_nd_id(const network_handle_type id_net,
	uint8 *const nm_node_id_ptr)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_HANDLE_UNDEF);
	} else if (nm_node_id_ptr == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				status = cannm_get_nd_id(id_net, nm_node_id_ptr);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}

FUNC(Std_ReturnType, NM_CODE) nm_get_local_nd_id(const network_handle_type id_net,
	uint8 *const nm_node_id_ptr)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_HANDLE_UNDEF);
	} else if (nm_node_id_ptr == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				status = cannm_get_local_nd_id(id_net, nm_node_id_ptr);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}
#endif

FUNC(Std_ReturnType, NM_CODE) nm_get_state(const network_handle_type id_net,
	nm_state_type *const nm_st, nm_mode_type *const nm_md)
{
	Std_ReturnType status = E_NOT_OK;
	const nm_line_channel_cate *channel_conf;

	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETSTATE_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETSTATE_NM_E_HANDLE_UNDEF);
	} else if (nm_st == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETSTATE_NM_E_PARAM_POINTER);
	} else if (nm_md == NULL_PTR) {
		NM_DET_REPORT(NM_NM_SERVICEID_GETSTATE_NM_E_PARAM_POINTER);
	} else {
		channel_conf = &nm_config.nm_tunnel[nm_config.tunnel_trans[id_net]];

		switch (channel_conf->line_cate) {
#if (CONFIG_MODULE_CANNM == 1)
			case NM_BUSNM_CANNM:
				status = cannm_get_state(id_net, nm_st, nm_md);
				break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
	}

	return status;
}

FUNC(void, NM_CODE) nm_start_net_ind(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKSTARTINDICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKSTARTINDICATION_NM_E_HANDLE_UNDEF);
	} else {
#if (CONFIG_MODULE_COMM == 1)
		comm_nm_net_start_indication(id_net);
#endif
	}
}

FUNC(void, NM_CODE) nm_mode_net_proc(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKMODE_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_NETWORKMODE_NM_E_HANDLE_UNDEF);
	} else {
#if (CONFIG_MODULE_COMM == 1)
		comm_nm_net_mode(id_net);
#endif
	}
}

FUNC(void, NM_CODE) nm_mode_pbs_proc(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PREPAREBUSSLEEPMODE_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PREPAREBUSSLEEPMODE_NM_E_HANDLE_UNDEF);
	} else {
#if (CONFIG_MODULE_COMM == 1)
		comm_nm_pre_bus_sleep_mode(id_net);
#endif
	}
}

FUNC(void, NM_CODE) nm_mode_bs_proc(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_BUSSLEEPMODE_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_BUSSLEEPMODE_NM_E_HANDLE_UNDEF);
	} else {
#if (CONFIG_MODULE_COMM == 1)
		comm_nm_bus_sleep_mode(id_net);
#endif
	}
}

FUNC(void, NM_CODE) nm_rx_pdu_ind(network_handle_type id_net, const PduInfoType *pdu_info_ptr)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PDURXINDICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_PDURXINDICATION_NM_E_HANDLE_UNDEF);
	} else {
		nm_pdu_rx_cbk(id_net, pdu_info_ptr);
		nm_pdu_rx_total_ind(id_net);
	}
}

#if (NM_STATE_CHANGE_IND_ENABLED == STD_ON)
void nm_ntfy_state_chg_bswm(const network_handle_type id_net,
	const nm_state_type nm_pre_state, const nm_state_type nm_cur_state)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_HANDLE_UNDEF);
	} else {
		(void)(nm_pre_state);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
		bswm_nm_state_change_notification(id_net, nm_cur_state);
#endif
	}
}

FUNC(void, NM_CODE) nm_ntfy_state_chg(const network_handle_type id_net,
	const nm_state_type nm_pre_state,	const nm_state_type nm_cur_state)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_HANDLE_UNDEF);
	} else {
		nm_state_chg_cbk(id_net, nm_pre_state, nm_cur_state);
	}
}
#endif

FUNC(void, NM_CODE) nm_rpt_msg_ind(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_REPEATMESSAGEINDICATION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_REPEATMESSAGEINDICATION_NM_E_HANDLE_UNDEF);
	} else {
		/* Do nothing */
	}

	(void)id_net;
}

FUNC(void, NM_CODE) nm_tx_tmot_excpt_pro(const network_handle_type id_net)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_TXTIMEOUTEXCEPTION_NM_E_UNINIT);
	} else if ((nm_check_network_handle(id_net)) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_TXTIMEOUTEXCEPTION_NM_E_HANDLE_UNDEF);
	} else {
		/* Do nothing */
	}

	(void)id_net;
}

FUNC(void, NM_CODE) nm_main_function(void)
{
	if ((nm_check_init_status()) == (boolean)FALSE) {
		NM_DET_REPORT(NM_NM_SERVICEID_MAINFUNCTION_NM_E_UNINIT);
	} else {
		/* Do nothing */
	}
}

#define NM_STOP_SEC_CODE
#include "nm_memmap.h"

