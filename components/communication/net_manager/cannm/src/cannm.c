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

/* 包含AUTOSAR通信栈类型定义 */
#include "comstack_types.h"
/* 包含CAN网络管理模块头文件 */
#include "cannm.h"
/* 包含CAN网络管理内部头文件 */
#include "cannm_internal.h"
/* 如果配置了网络管理模块 */
#if (CONFIG_MODULE_NM == 1)
#include "nm.h"
#endif
/* 包含CAN网络管理的调度管理头文件 */
#include "SchM_CanNm.h"
/* 如果配置了CAN接口模块 */
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif

/* 包含VCOS日志相关头文件 */
#include "vcos_log.h"
#include "cannm_vcos_log.h"
/* 如果启用了用户数据支持并且配置了PDUR模块 */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON) && (CONFIG_MODULE_PDUR == 1)
#include "pdur_cannm.h"
#endif
/* 如果启用了部分网络功能并且配置了CAN状态管理模块 */
#if (CANNM_PN_FEATURE_ENABLED == STD_ON) && (CONFIG_MODULE_CANSM == 1)
# include "cansm.h"
#endif
/* 包含标准字符串处理头文件 */
#include <string.h>
/* 如果启用了开发错误检测并且配置了VCOS检测模块 */
#if (CANNM_DEV_ERROR_DETECT == STD_ON) && defined(VCOS_MODULE_CONFIG_DET) && (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

/* 包含编译器特定定义 */
#include "Compiler.h"

/* 开始代码段定义 */
#define CANNM_START_SEC_CODE
#include "cannm_memmap.h"

/*
 * @brief 获取通道运行时间数据
 * @param channel_id 网络通道ID
 * @param left_msg_cyc_tm 剩余消息周期时间指针
 * @param left_tmot_tm 剩余超时时间指针
 * @return 无
 */
FUNC(void, CANNM_CODE) cannm_get_chnl_run_time_data(const network_handle_type channel_id,
	uint16 *left_msg_cyc_tm, uint16 *left_tmot_tm)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NO_INIT);
		return;
	}
	/* 检查通道ID是否有效 */
	if (channel_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}
	/* 检查指针是否有效 */
	if ((NULL == left_msg_cyc_tm) || (NULL == left_tmot_tm)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NULL_POINTER);
		return;
	}
	/* 获取指定通道的内部数据结构 */
	const cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_id];

	/* 返回剩余消息周期时间和超时时间 */
	*left_msg_cyc_tm =  channel_internal->left_msg_cyc_tm;
	*left_tmot_tm = channel_internal->left_tmot_tm;
}

/*
 * @brief 初始化CAN网络管理模块
 * @param cannm_config CAN网络管理配置指针
 * @return 无
 */
FUNC(void, CANNM_CODE) cannm_init(const cannm_cate_cfg *const cannm_config)
{
	/* 检查配置指针是否有效 */
	if (NULL == cannm_config) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_INIT_CANNM_E_NULL_POINTER);
		return;
	}
	/* 保存配置指针 */
	cannm_config_ptr = cannm_config;

	/* 初始化所有通道 */
	for (uint8 channel = 0u; channel < CANNM_CHANNEL_COUNT; channel++) {
		const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel];
		cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel];
		/* 清零内部数据结构 */
		(void)memset(channel_internal, 0x00u, sizeof(cannm_op_chnl_cate));
		/* 默认启用通信支持 */
		channel_internal->support_comm = (boolean)TRUE;

		/* 如果配置了节点ID位置，设置发送PDU中的节点ID */
		if (channel_conf->pos_nid != CANNM_PDU_OFF) {
			channel_internal->pdu_tx[channel_conf->pos_nid] = channel_conf->nd_id;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			channel_internal->spts_tx_msg[channel_conf->pos_nid] = channel_conf->nd_id;
#endif
		}

		/* 如果配置了控制位向量位置，初始化PDU的其他字节 */
		if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
			for (uint8_t i = 2; i < 8; i++) {
				channel_internal->pdu_tx[i] = CANNM_INVALID_VALUE;
			}
		}
	}

	/* 设置模块初始化状态为已初始化 */
	cannm_internal.init_status = CANNM_STATUS_INIT;
}

/*
 * @brief 反初始化CAN网络管理模块
 * @return 无
 */
FUNC(void, CANNM_CODE) cannm_de_init(void)
{
	boolean cannm_wake_flag = (boolean)FALSE;

	/* 检查是否有通道不在总线睡眠模式 */
	for (uint8 channel = 0u; channel < CANNM_CHANNEL_COUNT; channel++) {
		const cannm_op_chnl_cate *channel_internal =
		(const cannm_op_chnl_cate *)&cannm_internal.nm_tunnel[channel];
		if (channel_internal->mode != NM_MODE_BUS_SLEEP) {
			cannm_wake_flag = (boolean)TRUE;
			break;
		}
	}
	/* 如果有通道不在总线睡眠模式，报告错误 */
	if ((boolean)TRUE == cannm_wake_flag) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_DEINIT_CANNM_E_NOT_IN_BUS_SLEEP);
	}
}

/*
 * @brief 被动启动网络
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_pas_stup(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_PASSIVESTARTUP_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_PASSIVESTARTUP_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}

	Std_ReturnType status = E_OK;
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 根据当前模式处理被动启动 */
	switch (channel_internal->mode) {
		case NM_MODE_BUS_SLEEP:
			cannm_bs_to_rpt_msg_proc(channel_conf, channel_internal, (boolean)FALSE);
			status = E_OK;
			break;
		case NM_MODE_PREPARE_BUS_SLEEP:
			cannm_pbs_to_rpt_msg_proc(channel_conf, channel_internal, (boolean)FALSE);
			status = E_OK;
			break;
		default:
			status = E_NOT_OK;
			break;
	}

#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
	/* 如果支持部分网络，设置PNI控制位 */
	if (channel_conf->pn_support) {
		cannm_set_pni_cbv(channel_conf,channel_internal);
	}
#endif
	return status;
}

/*
 * @brief 请求网络
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_req_net(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKREQUEST_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKREQUEST_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 设置网络请求标志 */
	channel_internal->requested = (boolean)TRUE;
	/* 根据当前模式处理网络请求 */
	switch (channel_internal->mode) {
		case NM_MODE_BUS_SLEEP:
			cannm_bs_to_rpt_msg_proc(channel_conf, channel_internal, (boolean)TRUE);
#if (CANNM_ACTIVE_WUP_BIT_FEATURE_ENABLED == STD_ON)
			cannm_set_active_wk_cbv(channel_conf, channel_internal);
#endif
			break;
		case NM_MODE_PREPARE_BUS_SLEEP:
			cannm_pbs_to_rpt_msg_proc(channel_conf, channel_internal, (boolean)TRUE);
#if (CANNM_ACTIVE_WUP_BIT_FEATURE_ENABLED == STD_ON)
			cannm_set_active_wk_cbv(channel_conf, channel_internal);
#endif
			break;
		case NM_MODE_NETWORK:
			/* 根据当前状态处理网络请求 */
			switch (channel_internal->state) {
#if (STD_OFF == CANNM_PN_HANDLE_MULTIPLE_NETWORK_REQUESTS)
				case NM_STATE_READY_SLEEP:
					cannm_rdy_slp_to_nor_opr_proc(channel_conf, channel_internal);
					break;
#else
				case NM_STATE_READY_SLEEP:
					cannm_rdy_slp_to_rpt_msg_proc(channel_conf, channel_internal);
					break;
				case NM_STATE_NORMAL_OPERATION:
					cannm_nor_opr_to_rpt_msg_proc(channel_conf, channel_internal);
					break;
				case NM_STATE_REPEAT_MESSAGE:
					cannm_rpt_msg_to_rpt_msg_proc(channel_conf, channel_internal);
					break;
#endif
				default:
					break;
			}
			break;
		default:
			break;
	}

#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
	/* 如果支持部分网络，设置PNI控制位 */
	if (channel_conf->pn_support) {
		cannm_set_pni_cbv(channel_conf, channel_internal);
	}
#endif
	return E_OK;
}

/*
 * @brief 释放网络
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_rel_net(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKRELEASE_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKRELEASE_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}

	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 检查是否支持通信 */
	if (FALSE == channel_internal->support_comm) {
		return E_NOT_OK;
	}
	/* 清除网络请求标志 */
	channel_internal->requested = (boolean)FALSE;
	/* 如果当前在正常操作状态，切换到准备睡眠状态 */
	if (NM_MODE_NETWORK == channel_internal->mode && NM_STATE_NORMAL_OPERATION == channel_internal->state) {
		cannm_nor_opr_to_rdy_slp_proc(channel_conf, channel_internal);
	}
	return E_OK;
}

/* 如果启用了通信控制功能 */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
/*
 * @brief 禁用通信
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_disable_comm(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_DISABLECOMMUNICATION_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}

#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON)
	return E_NOT_OK;
#endif

	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_DISABLECOMMUNICATION_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 检查当前模式是否为网络模式 */
	if (channel_internal->mode != NM_MODE_NETWORK) {
		return E_NOT_OK;
	}

	/* 禁用通信支持 */
	channel_internal->support_comm = (boolean)FALSE;

	return E_OK;
}
#endif

/* 如果启用了通信控制功能 */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
/*
 * @brief 启用通信
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_enable_comm(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_ENABLECOMMUNICATION_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}

#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON)
	return E_NOT_OK;
#endif

	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_ENABLECOMMUNICATION_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];

	/* 检查当前模式是否为网络模式 */
	if (channel_internal->mode != NM_MODE_NETWORK) {
		return E_NOT_OK;
	}

	/* 检查通信是否已经启用 */
	if (channel_internal->support_comm) {
		return E_NOT_OK;
	}

	/* 设置剩余消息周期时间和超时时间 */
	channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm +	channel_conf->m_func_pid;
	channel_internal->left_tmot_tm = channel_conf->tm_out + channel_conf->m_func_pid;
	/* 启用通信支持 */
	channel_internal->support_comm = (boolean)TRUE;

	return E_OK;
}
#endif

/* 如果未启用被动模式但启用了用户数据支持 */
#if (CANNM_PASSIVE_MODE_ENABLED == STD_OFF) && (CANNM_USER_DATA_ENABLED == STD_ON) && \
(CANNM_COM_USER_DATA_SUPPORT == STD_OFF)
/*
 * @brief 设置用户数据
 * @param nm_id 网络管理ID
 * @param nm_user_data_ptr 用户数据指针
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_set_ur_data(const network_handle_type nm_id,
const uint8 *const nm_user_data_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_SETUSERDATA_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_SETUSERDATA_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查用户数据指针是否有效 */
	if (NULL == nm_user_data_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_SETUSERDATA_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	Std_ReturnType ret_val = E_NOT_OK;
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 检查PDU是否已经设置 */
	if (!channel_internal->is_pdu_set) {
		channel_internal->is_pdu_set = (boolean)TRUE;
		/* 获取用户数据指针和长度 */
		uint8 *dest_ur_data = cannm_ur_data_ptr_get(channel_conf, channel_internal->pdu_tx);
		PduLengthType user_data_length = cannm_ur_data_len_get(channel_conf);
		/* 复制用户数据 */
		(void)memcpy(dest_ur_data, nm_user_data_ptr, (size_t)user_data_length);
		channel_internal->is_pdu_set = (boolean)FALSE;
		ret_val = E_OK;
	}

	return ret_val;
}
#endif

/* 如果启用了用户数据支持 */
#if (CANNM_USER_DATA_ENABLED == STD_ON)
/*
 * @brief 获取用户数据
 * @param nm_id 网络管理ID
 * @param nm_user_data_ptr 用户数据指针
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_ur_data(const network_handle_type nm_id, uint8 *const nm_user_data_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETUSERDATA_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETUSERDATA_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查用户数据指针是否有效 */
	if (NULL == nm_user_data_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETUSERDATA_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	/* 获取用户数据指针和长度 */
	const uint8 *source_user_data = cannm_ur_data_ptr_get(channel_conf, channel_internal->pdu_rx);
	PduLengthType user_data_length = cannm_ur_data_len_get(channel_conf);

	/* 复制用户数据 */
	if (0u < user_data_length) {
		(void)memcpy(nm_user_data_ptr, source_user_data, (size_t)user_data_length);
	}

	return E_OK;
}
#endif

/*
 * @brief 获取节点ID
 * @param nm_id 网络管理ID
 * @param nm_node_id_ptr 节点ID指针
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_nd_id(const network_handle_type nm_id,
uint8 *const nm_node_id_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETNODEIDENTIFIER_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETNODEIDENTIFIER_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查节点ID指针是否有效 */
	if (NULL == nm_node_id_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETNODEIDENTIFIER_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	const cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	Std_ReturnType status = E_NOT_OK;

	/* 如果配置了节点ID位置，从接收PDU中获取节点ID */
	if (CANNM_PDU_OFF != channel_conf->pos_nid) {
		*nm_node_id_ptr = channel_internal->pdu_rx[channel_conf->pos_nid];
		status = E_OK;
	}
	return status;
}

/*
 * @brief 获取本地节点ID
 * @param nm_id 网络管理ID
 * @param nm_node_id_ptr 节点ID指针
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_local_nd_id(const network_handle_type nm_id,
	uint8 *const nm_node_id_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETLOCALNODEIDENTIFIER_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETLOCALNODEIDENTIFIER_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查节点ID指针是否有效 */
	if (NULL == nm_node_id_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETLOCALNODEIDENTIFIER_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	/* 从配置中获取本地节点ID */
	*nm_node_id_ptr = channel_conf->nd_id;

	return E_OK;
}

/* 如果启用了节点检测功能 */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/*
 * @brief 请求重复消息
 * @param nm_id 网络管理ID
 * @return 标准返回类型
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_req_rpt_msg(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_REPEATMESSAGEREQUEST_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查网络ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_REPEATMESSAGEREQUEST_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	Std_ReturnType status = E_NOT_OK;

	/* 检查当前状态是否为重复消息、准备总线睡眠或总线睡眠模式 */
	if ((NM_STATE_REPEAT_MESSAGE == channel_internal->state) ||
	(NM_MODE_PREPARE_BUS_SLEEP == channel_internal->mode) ||
	(NM_MODE_BUS_SLEEP == channel_internal->mode)) {
		return status;
	}
	/* 如果配置了控制位向量位置，设置重复消息请求位 */
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		switch (channel_internal->state) {
			case NM_STATE_READY_SLEEP:
				channel_internal->pdu_tx[channel_conf->pos_cbv] |= CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
				channel_internal->spts_tx_msg[channel_conf->pos_cbv] |=
				CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
				cannm_rdy_slp_to_rpt_msg_proc(channel_conf, channel_internal);
				status = E_OK;
				break;
			case NM_STATE_NORMAL_OPERATION:
				channel_internal->pdu_tx[channel_conf->pos_cbv] |=  CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
				channel_internal->spts_tx_msg[channel_conf->pos_cbv] |=
				CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
				cannm_nor_opr_to_rpt_msg_proc(channel_conf, channel_internal);
				status = E_OK;
				break;
			default:
				break;
		}
	}
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON) && (CANNM_REPEAT_MSG_IND_ENABLED == STD_ON)
	/* 通知重复消息指示 */
	nm_rpt_msg_ind(CANNM_CHANNEL_COUNT);
#endif
	return status;
}
#endif
/*
 * CAN网络管理模块
 * 实现AUTOSAR CAN网络管理协议的核心功能
 * 包括状态管理、消息传输、超时处理等
 */

/* 获取指定网络通道的NM PDU数据 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_msg_data(const network_handle_type nm_id, uint8 *const nm_pdu_data_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETPDUDATA_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查通道ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETPDUDATA_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查指针是否为空 */
	if (NULL == nm_pdu_data_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETPDUDATA_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	/* 获取通道配置和内部数据 */
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	const cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 从内部数据结构复制NM PDU数据到提供的缓冲区 */
	(void)memcpy(nm_pdu_data_ptr, channel_internal->pdu_rx, (size_t)channel_conf->msg_size);

	return E_OK;
}

/* 获取指定网络通道的NM状态和模式 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_state(const network_handle_type nm_id,
	nm_state_type *const nm_st, nm_mode_type *const nm_md)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NO_INIT);
		return E_NOT_OK;
	}
	/* 检查通道ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}
	/* 检查状态指针是否为空 */
	if (NULL == nm_st) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 检查模式指针是否为空 */
	if (NULL == nm_md)
	{
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	/* 从内部数据结构读取状态和模式 */
	const cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	*nm_st = channel_internal->state;
	*nm_md = channel_internal->mode;

	return E_OK;
}

/* NM消息传输确认回调函数 */
FUNC(void, CANNM_CODE) cannm_tx_confirmation(PduIdType cannm_tx_pdu_id, Std_ReturnType result)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TXCONFIRMATION_CANNM_E_NO_INIT);
		return;
	}
	/* 检查PDU ID是否有效 */
	if (cannm_tx_pdu_id >= NM_CHANNEL_COUNT) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TXCONFIRMATION_CANNM_E_INVALID_PDUID);
		return;
	}
	/* 增加传输确认计数 */
	(void)Os_AtomicInc(&cannm_msg_cnt.txconf_ok_msg_cnt);
	(void)result;
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[cannm_tx_pdu_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	/* 获取通道配置和内部数据 */
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 根据传输状态和当前模式处理传输确认 */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
#if (CONFIG_MODULE_PDUR == 1)
	pdur_cannm_tx_confirmation(channel_conf->ur_data_tx_pid, result);
#endif

	if (CANNM_SPONTANEOUS_TRANSMISSION == channel_internal->tx_sts) {
		channel_internal->tx_sts = CANNM_NO_TRANSMISSION;
	} else
#endif
	{
		switch (channel_internal->mode) {
			case NM_MODE_NETWORK:
				cannm_net_mode_to_net_mode_proc(channel_conf, channel_internal);
				break;
			default:
				break;
		}
	}

	/* 重置消息超时计时器 */
	channel_internal->left_msg_tmot_tm = 0u;
}

/* 网络模式下接收指示处理函数（需要节点检测功能） */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
FUNC(void, CANNM_CODE) cannm_net_mode_rx_ind_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean repeat_message_bit_indication)
{
	/* 检查通道配置指针是否有效 */
	if (NULL == channel_conf) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKMODE_RXINDICATION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 检查内部数据指针是否有效 */
	if (NULL == channel_internal) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_NETWORKMODE_RXINDICATION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 如果重复消息位指示为真，根据当前状态处理重复消息 */
	if (TRUE == repeat_message_bit_indication) {
		switch (channel_internal->state) {
			case NM_STATE_READY_SLEEP:
				cannm_rdy_slp_to_rpt_msg_proc(channel_conf, channel_internal);
				break;
			case NM_STATE_NORMAL_OPERATION:
				cannm_nor_opr_to_rpt_msg_proc(channel_conf, channel_internal);
				break;
			default:
				break;
		}
	}
	return;
}
#endif

/* NM消息接收指示回调函数 */
FUNC(void, CANNM_CODE) cannm_rx_indication(PduIdType cannm_rx_pdu_id, const PduInfoType *pdu_info_ptr)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RXINDICATION_CANNM_E_NO_INIT);
		return;
	}
	/* 检查PDU信息指针是否为空 */
	if (NULL == pdu_info_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RXINDICATION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 检查数据指针是否为空 */
	if (NULL == pdu_info_ptr->SduDataPtr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RXINDICATION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 检查PDU ID是否有效 */
	if (cannm_rx_pdu_id >= NM_CHANNEL_COUNT) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RXINDICATION_CANNM_E_INVALID_PDUID);
		return;
	}
	/* 增加接收计数 */
	(void)Os_AtomicInc(&cannm_msg_cnt.rx_ok_msg_cnt);

	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[cannm_rx_pdu_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	/* 获取通道配置和内部数据 */
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 复制接收到的数据到内部缓冲区 */
	(void)memcpy(channel_internal->pdu_rx, pdu_info_ptr->SduDataPtr, (size_t)channel_conf->msg_size);

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
	boolean repeat_message_bit_indication = (boolean)FALSE;
#endif

	/* 处理控制位向量（CBV）中的各种标志位 */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON) || (CANNM_PN_FEATURE_ENABLED == STD_ON)
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		uint8 cbv = channel_internal->pdu_rx[channel_conf->pos_cbv];
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
		/* 检查重复消息请求位 */
		if ((cbv & CANNM_CBV_REPEAT_MESSAGE_REQUEST) != 0u) {
			repeat_message_bit_indication = (boolean)TRUE;
		}
#endif
#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
		boolean pni = FALSE;

		/* 检查部分网络指示位 */
		if ((cbv & CANNM_CBV_PNI) != 0U) {
			pni = (boolean)TRUE;
		}
		/* 处理接收到的消息 */
		if (E_NOT_OK == cannm_rx_proc(channel_conf,channel_internal, pni)) {
			return;
		}
#endif
	}
#endif

	/* 根据当前模式处理接收到的消息 */
	switch (channel_internal->mode) {
		case NM_MODE_BUS_SLEEP:
			cannm_bs_to_bs_proc(channel_conf, channel_internal);
			break;
		case NM_MODE_PREPARE_BUS_SLEEP:
			cannm_pbs_to_rpt_msg_proc(channel_conf, channel_internal,(boolean)FALSE);
			break;
		case NM_MODE_NETWORK:
			cannm_net_mode_to_net_mode_proc(channel_conf, channel_internal);
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
			cannm_net_mode_rx_ind_proc(channel_conf, channel_internal, repeat_message_bit_indication);
#endif
			break;
		default:
			break;
	}

	/* 调用用户定义的接收指示回调（如果启用） */
#if (CANnm_pdu_rx_ind_ENABLED == STD_ON)
	nm_rx_pdu_ind((network_handle_type)cannm_rx_pdu_id, pdu_info_ptr);
#endif

	return;
}

/* 传输NM用户数据 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_transmit(PduIdType ur_data_tx_pid, const PduInfoType *pdu_info_ptr)
{
	/* 检查PDU ID是否有效 */
	if (ur_data_tx_pid >= NM_CHANNEL_COUNT) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRANSMIT_CANNM_E_INVALID_PDUID);
		return E_NOT_OK;
	}
	/* 检查PDU信息指针是否为空 */
	if (NULL == pdu_info_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRANSMIT_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 检查数据指针是否为空 */
	if (NULL == pdu_info_ptr->SduDataPtr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRANSMIT_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	Std_ReturnType ret_val = E_NOT_OK;

	/* 用户数据传输功能（如果启用） */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
	boolean found = (boolean)FALSE;
	uint8 chnl_idx = 0u;

	/* 查找对应的通道配置 */
	for (chnl_idx = 0u; chnl_idx < CANNM_CHANNEL_COUNT; chnl_idx++) {
		if (ur_data_tx_pid == cannm_config_ptr->nm_tunnel[chnl_idx].ur_data_tx_pid) {
			found = (boolean)TRUE;
			break;
		}
	}
	if (FALSE == found) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRANSMIT_CANNM_E_INVALID_PDUID);
		return E_NOT_OK;
	}
	if ((chnl_idx >= cannm_config_ptr->chnl_num_len) ||
	(cannm_config_ptr->chnl_num[chnl_idx] == CANNM_INVALID_VALUE)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRANSMIT_CANNM_E_INVALID_CHANNEL);
		return E_NOT_OK;
	}

	if (chnl_idx < CANNM_CHANNEL_COUNT) {
		const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[chnl_idx];
		cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[chnl_idx];

		/* 在适当的状态下传输用户数据 */
		if (((NM_STATE_REPEAT_MESSAGE == channel_internal->state) || (NM_STATE_NORMAL_OPERATION ==
		channel_internal->state)) && (channel_internal->support_comm)) {
			if (CANNM_NO_TRANSMISSION == channel_internal->tx_sts) {
				uint8 *dest_ur_data = cannm_ur_data_ptr_get(channel_conf,
					channel_internal->spts_tx_msg);
				PduLengthType user_data_length = cannm_ur_data_len_get(channel_conf);

				channel_internal->tx_sts = CANNM_SPONTANEOUS_TRANSMISSION;
				if (0u < user_data_length) {
					(void)memcpy(dest_ur_data, pdu_info_ptr->SduDataPtr, (size_t)user_data_length);
				}

				PduInfoType pdu =
				{
					.SduDataPtr = dest_ur_data,
					.SduLength = channel_conf->msg_size,
				};
				ret_val = canif_transmit(channel_conf->canif_pid, &pdu);

				if (ret_val != E_OK) {
					channel_internal->tx_sts = CANNM_NO_TRANSMISSION;
				}
			} else {
				/* 无操作 */
			}
		}
	}

#endif
	return ret_val;
}

/* 重复消息状态处理函数 */
FUNC(void, CANNM_CODE) cannm_state_rpt_msg_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 处理重复消息计时器 */
	cannm_rpt_msg_time_proc(channel_conf, channel_internal);

	/* 处理传输超时计时器（如果被动模式未启用） */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
	cannm_tx_tmot_troc(channel_conf, channel_internal);
#endif

	/* 如果支持通信，处理超时计时器 */
	if (channel_internal->support_comm) {
		cannm_tmot_time_proc(channel_conf, channel_internal);

		/* 处理消息周期计时器（如果被动模式未启用） */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
		if ((NM_STATE_REPEAT_MESSAGE == channel_internal->state) || (NM_STATE_NORMAL_OPERATION ==
			channel_internal->state)) {
			cannm_msg_cyc_time_proc(channel_conf, channel_internal);
		}
#endif
	}
}

/* 正常操作状态处理函数 */
FUNC(void, CANNM_CODE) cannm_state_nor_opr_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 处理传输超时计时器（如果被动模式未启用） */
	#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
	cannm_tx_tmot_troc(channel_conf, channel_internal);
	#endif

	/* 如果支持通信，处理超时计时器 */
	if (channel_internal->support_comm) {
		cannm_tmot_time_proc(channel_conf, channel_internal);

		/* 处理消息周期计时器（如果被动模式未启用） */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
		if ((NM_STATE_REPEAT_MESSAGE == channel_internal->state) ||
			(NM_STATE_NORMAL_OPERATION == channel_internal->state)) {
			cannm_msg_cyc_time_proc(channel_conf, channel_internal);
		}
#endif
	}
}

/* 准备睡眠状态处理函数 */
FUNC(void, CANNM_CODE) cannm_state_rdy_slp_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 处理传输超时计时器（如果被动模式未启用） */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
	cannm_tx_tmot_troc(channel_conf, channel_internal);
#endif

	/* 如果支持通信，处理超时计时器 */
	if (channel_internal->support_comm) {
		cannm_tmot_time_proc(channel_conf, channel_internal);
	}
}

/* 网络模式处理函数 */
FUNC(void, CANNM_CODE) cannm_mode_net_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 根据当前状态调用相应的状态处理函数 */
	switch (channel_internal->state) {
		case NM_STATE_REPEAT_MESSAGE:
			cannm_state_rpt_msg_handling(channel_conf, channel_internal);
			break;
		case NM_STATE_NORMAL_OPERATION:
			cannm_state_nor_opr_handling(channel_conf, channel_internal);
			break;
		case NM_STATE_READY_SLEEP:
			cannm_state_rdy_slp_handling(channel_conf, channel_internal);
			break;
		default:
			break;
	}
}

/* 单个通道的主处理函数 */
FUNC(void, CANNM_CODE) cannm_m_func_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 根据当前模式调用相应的处理函数 */
	switch (channel_internal->mode) {
		case NM_MODE_NETWORK:
			cannm_mode_net_handling(channel_conf, channel_internal);
			break;

		case NM_MODE_PREPARE_BUS_SLEEP:
			cannm_mode_pbs_handling(channel_conf, channel_internal);
			break;
		default:
			break;
	}
}

/* 主处理函数（在多核环境下处理所有通道） */
FUNC(void, CANNM_CODE) cannm_mtpl_m_func_proc(void)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NO_INIT);
		return;
	}

	/* 获取当前核心ID */
#if (CANNM_MULTICORE_ENABLED == STD_ON)
	cannm_core_id_type core_id = (cannm_core_id_type)GetCoreID();
#else
	cannm_core_id_type core_id = CANNM_SINGLECORE_COREID;
#endif

	/* 获取当前核心对应的通道映射 */
	const cannm_core_channel_id_mapping_type *tunnel_trans = cannm_channel_id_mapping_by_core_id(core_id);

	if (NULL == tunnel_trans) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	if (NULL == tunnel_trans->net_tbl_addr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 获取通道数量 */
	cannm_channel_num_type channel_num = tunnel_trans->network_table_len;

	/* 遍历所有通道并调用主处理函数 */
	for (uint8 channel = 0u; channel < channel_num; channel++) {
		const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[tunnel_trans->net_tbl_addr[channel]];
		cannm_op_chnl_cate *channel_internal =
			&cannm_internal.nm_tunnel[tunnel_trans->net_tbl_addr[channel]];
		cannm_m_func_proc(channel_conf, channel_internal);
	}
}

/* 确认PN（Partial Networking）可用性 */
FUNC(void, CANNM_CODE) cannm_confirm_pn_availability(const network_handle_type nm_id)
{
	/* 检查模块是否已初始化 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_CONFIRMPNAVAILABILITY_CANNM_E_NO_INIT);
		return;
	}
	/* 检查通道ID是否有效 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_CONFIRMPNAVAILABILITY_CANNM_E_INVALID_CHANNEL);
		return;
	}

	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];

	/* 设置通道的PN支持标志 */
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];

	channel_internal->filter_pdu = channel_conf->pn_support;
}

/* 超时计时器处理函数 */
FUNC(void, CANNM_CODE) cannm_tmot_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 处理超时计时器 */
	if (channel_conf->m_func_pid >= channel_internal->left_tmot_tm) {
		channel_internal->left_tmot_tm = 0u;

		/* 根据当前状态处理超时 */
		switch (channel_internal->state) {
			case NM_STATE_REPEAT_MESSAGE:
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
				channel_internal->tx_sts = CANNM_NO_TRANSMISSION;
#endif
				cannm_rpt_msg_to_rpt_msg_proc(channel_conf, channel_internal);
				break;
			case NM_STATE_NORMAL_OPERATION:
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
				channel_internal->tx_sts = CANNM_NO_TRANSMISSION;
#endif
				cannm_nor_opr_to_nor_opr_proc(channel_conf, channel_internal);
				break;
			case NM_STATE_READY_SLEEP:
				cannm_rdy_slp_to_pbs_proc(channel_conf, channel_internal);
				break;
			default:
				break;
		}
	} else {
		channel_internal->left_tmot_tm -= channel_conf->m_func_pid;
	}
}

/* 重复消息计时器处理函数 */
FUNC(void, CANNM_CODE) cannm_rpt_msg_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 更新重复消息计时器 */
	channel_internal->left_rpt_msg_tm = (channel_conf->m_func_pid >= channel_internal->left_rpt_msg_tm)
	? 0u : (channel_internal->left_rpt_msg_tm - channel_conf->m_func_pid);

	/* 如果计时器到期，根据请求状态处理状态转换 */
	if (0u == channel_internal->left_rpt_msg_tm) {
		(channel_internal->requested)
			? cannm_rpt_msg_to_nor_opr_proc(channel_conf, channel_internal)
			: cannm_rpt_msg_to_rdy_slp_proc(channel_conf, channel_internal);
	}
}

/* 准备总线睡眠模式处理函数 */
FUNC(void, CANNM_CODE) cannm_mode_pbs_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 更新等待总线睡眠计时器 */
	channel_internal->left_wait_bs_tm = (channel_conf->m_func_pid >= channel_internal->left_wait_bs_tm)
	? 0u : (channel_internal->left_wait_bs_tm - channel_conf->m_func_pid);

	/* 如果计时器到期且仍在准备总线睡眠模式，转换到总线睡眠模式 */
	if ((0u == channel_internal->left_wait_bs_tm) && (NM_MODE_PREPARE_BUS_SLEEP == channel_internal->mode)) {
		cannm_pbs_to_bs_proc(channel_conf, channel_internal);
	}
}

/* 传输超时处理函数 */
FUNC(void, CANNM_CODE) cannm_tx_tmot_troc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 处理消息传输超时 */
	if (0U != channel_internal->left_msg_tmot_tm) {
		if (channel_conf->m_func_pid >= channel_internal->left_msg_tmot_tm) {
			/* 触发传输超时异常处理 */
			nm_tx_tmot_excpt_pro(channel_conf->id_net);

			/* 如果支持部分网络，通知CAN状态管理器 */
#if (CANNM_PN_FEATURE_ENABLED == STD_ON) && (CONFIG_MODULE_CANSM == 1)
			if ((boolean)TRUE == channel_conf->pn_support) {
				(void)cansm_tx_timeout_exception(channel_conf->id_net);
			}
#endif
			channel_internal->left_msg_tmot_tm = 0u;
		} else {
			channel_internal->left_msg_tmot_tm -= channel_conf->m_func_pid;
		}
	}
}

/* 消息周期计时器处理函数 */
FUNC(void, CANNM_CODE) cannm_msg_cyc_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 处理消息周期计时器 */
	if (channel_conf->m_func_pid >= channel_internal->left_msg_cyc_tm) {
		/* 重置计时器 */
		if ((boolean)TRUE == channel_internal->imdt_en_mode) {
			channel_internal->left_msg_cyc_tm = channel_conf->imdt_nm_cyc_tm;
		} else {
			channel_internal->left_msg_cyc_tm = channel_conf->msg_cyc_tm;
		}

		/* 如果启用了总线负载减少功能，调整周期时间 */
#if (CANNM_BUS_LOAD_REDUCTION_ENABLED == STD_ON) && (CANNM_BUS_LOAD_REDUCTION_ACTIVE == STD_ON)
		if (NM_STATE_NORMAL_OPERATION == channel_internal->state) {
			channel_internal->left_msg_cyc_tm = channel_conf->msg_rdc_tm;
		}
#endif
		/* 传输NM消息 */
		cannm_tx_msg(channel_conf, channel_internal);
	} else {
		channel_internal->left_msg_cyc_tm -= channel_conf->m_func_pid;
	}
}

/* 传输NM消息函数 */
FUNC(void, CANNM_CODE) cannm_tx_msg(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	Std_ReturnType transmit_status = E_NOT_OK;
	PduInfoType pdu;
	(void)memset(&pdu, 0, sizeof(pdu));
	pdu.SduDataPtr = channel_internal->pdu_tx;
	pdu.SduLength = channel_conf->msg_size;

	/* 如果不支持通信，直接返回 */
	if (FALSE == channel_internal->support_comm) {
		return;
	}

	/* 用户数据传输处理（如果启用） */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
	if (CANNM_NO_TRANSMISSION == channel_internal->tx_sts) {
		SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
		channel_internal->tx_sts = CANNM_ONGOING_TRANSMISSION;
		SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CONFIG_MODULE_PDUR == 1)
		PduInfoType user_data;

		user_data.SduDataPtr = cannm_ur_data_ptr_get(channel_conf, channel_internal->pdu_tx);
		user_data.SduLength = cannm_ur_data_len_get(channel_conf);
		(void)pdur_cannm_trigger_transmit(channel_conf->ur_data_tx_pid, &user_data);
#endif
#elif (CANNM_USER_DATA_ENABLED == STD_ON)
	if (FALSE == channel_internal->is_pdu_set) {
		SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
		channel_internal->is_pdu_set = (boolean)TRUE;
		SchM_Exit_CANNM_EXCLUSIVE_AREA_0();
#endif
		/* 设置消息传输超时计时器 */
		channel_internal->left_msg_tmot_tm = channel_conf->msg_tmot_tm;

		/* 调用传输回调函数（如果启用） */
#if (CANNM_CALLOUT_CONFIG == STD_ON)
		if (E_OK == CANNM_CALLBACK_FUNCTION()) {
			transmit_status = canif_transmit(channel_conf->canif_pid, &pdu);
		}
#else
		transmit_status = canif_transmit(channel_conf->canif_pid, &pdu);
#endif

		/* 更新传输统计 */
		if (transmit_status != E_OK) {
			(void)Os_AtomicInc(&cannm_msg_cnt.tx_not_ok_msg_cnt);
		} else {
			(void)Os_AtomicInc(&cannm_msg_cnt.tx_ok_msg_cnt);
		}

		/* 首次PDU失败重试功能（如果启用） */
#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
		network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[channel_conf->id_net];

		cannm_first_pdu_failed_retry_flg[cannm_net_id] = cannm_get_first_pdu_failed_retry_flg(cannm_net_id);
		if ((boolean)TRUE == cannm_first_pdu_failed_retry_flg[cannm_net_id]) {
			if (E_OK == transmit_status) {
				cannm_first_pdu_failed_retry_flg[cannm_net_id] = (boolean)FALSE;
				if (canif_transmit_flag == 0u) {
					canif_transmit_flag = 1u;
					canif_first_nm_msg_id = channel_conf->canif_pid;
				}
			} else {
				channel_internal->left_msg_cyc_tm = 0u;
			}
		}
#endif

		/* 立即传输模式处理 */
		if ((boolean)TRUE == channel_internal->imdt_en_mode) {
			channel_internal->imdt_tx_sd++;
#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
			if ((boolean)TRUE == cannm_first_pdu_failed_retry_flg[cannm_net_id])
				channel_internal->imdt_tx_sd--;
#endif
			if (channel_internal->imdt_tx_sd == channel_conf->imdt_tx) {
				channel_internal->imdt_en_mode = (boolean)FALSE;
				channel_internal->left_msg_cyc_tm = (0U != channel_conf->msg_cys_ofst_tm)
				? channel_conf->msg_cys_ofst_tm : channel_conf->msg_cyc_tm;
			}
		}

		/* 总线负载减少功能处理 */
#if (CANNM_BUS_LOAD_REDUCTION_ENABLED == STD_ON) && (CANNM_BUS_LOAD_REDUCTION_ACTIVE == STD_ON)
		if (NM_STATE_NORMAL_OPERATION == channel_internal->state) {
			channel_internal->left_msg_cyc_tm = channel_conf->msg_rdc_tm;
		}
#endif

		/* 重置传输状态 */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		channel_internal->tx_sts = CANNM_NO_TRANSMISSION;
	}
#elif (CANNM_USER_DATA_ENABLED == STD_ON)
		channel_internal->is_pdu_set = (boolean)FALSE;
	}
#endif
}

/* 触发NM消息传输 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_trigger_transmit(PduIdType tx_pdu_id, PduInfoType *pdu_info_ptr)
{
	/* 检查指针是否有效 */
	if (NULL == pdu_info_ptr) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRIGGERTRANSMIT_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 检查PDU ID是否有效 */
	if (tx_pdu_id >= NM_CHANNEL_COUNT) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_TRIGGERTRANSMIT_CANNM_E_INVALID_PDUID);
		return E_NOT_OK;
	}
	Std_ReturnType ret_val = E_NOT_OK;

	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[tx_pdu_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 准备用户数据 */
#if ( CANNM_COM_USER_DATA_SUPPORT == STD_ON )
	pdu_info_ptr->SduDataPtr = cannm_ur_data_ptr_get(channel_conf, channel_internal->pdu_tx);
	pdu_info_ptr->SduLength = cannm_ur_data_len_get(channel_conf);
#if (CONFIG_MODULE_PDUR == 1)
	(void)pdur_cannm_trigger_transmit(channel_conf->ur_data_tx_pid, pdu_info_ptr);
#endif
#endif

	/* 准备NM消息数据 */
	pdu_info_ptr->SduDataPtr = channel_internal->pdu_tx;
	pdu_info_ptr->SduLength = channel_conf->msg_size;

	/* 调用底层接口传输消息 */
	ret_val = canif_transmit(tx_pdu_id, pdu_info_ptr);

	/* 更新传输统计 */
	if (ret_val != E_OK) {
		(void)Os_AtomicInc(&cannm_msg_cnt.tt_tx_not_ok_msg_cnt);
	} else {
		(void)Os_AtomicInc(&cannm_msg_cnt.tt_tx_ok_msg_cnt);
	}

	return ret_val;
}

/* 获取用户数据在NM PDU中的偏移量 */
FUNC(PduLengthType, CANNM_CODE) cannm_ur_data_ofst_get(const cannm_chnl_cate *channel_conf)
{
	/* 检查指针是否有效 */
	if (NULL == channel_conf) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return 0u;
	}
	PduLengthType user_data_pos = 0u;

	/* 计算NID和CBV字段的偏移量 */
	user_data_pos += (CANNM_PDU_OFF == channel_conf->pos_nid) ? 0U : 1U;
	user_data_pos += (CANNM_PDU_OFF == channel_conf->pos_cbv) ? 0U : 1U;
	return user_data_pos;
}

/* 获取用户数据在NM PDU中的指针 */
FUNC_P2VAR(uint8, AUTOMATIC, CANNM_CODE) cannm_ur_data_ptr_get(const cannm_chnl_cate *channel_conf,
	uint8 *MessageSduPtr)
{
	PduLengthType user_data_offset = cannm_ur_data_ofst_get(channel_conf);

	return &MessageSduPtr[user_data_offset];
}

/* 获取用户数据长度 */
FUNC(PduLengthType, CANNM_CODE) cannm_ur_data_len_get(const cannm_chnl_cate *channel_conf)
{
	/* 检查指针是否有效 */
	if (NULL == channel_conf) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return 0u;
	}
	PduLengthType user_data_offset = cannm_ur_data_ofst_get(channel_conf);

	/* 检查偏移量是否有效 */
	if (user_data_offset > channel_conf->msg_size) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return 0u;
	}
	return channel_conf->msg_size - user_data_offset;
}

/* 清除重复消息请求控制位（需要节点检测功能） */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
FUNC(void, CANNM_CODE) cannm_clr_rpt_msg_req_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 清除CBV中的重复消息请求位 */
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		channel_internal->pdu_tx[channel_conf->pos_cbv] &= (uint8)(~(CANNM_CBV_REPEAT_MESSAGE_REQUEST));
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		channel_internal->spts_tx_msg[channel_conf->pos_cbv] &= ~(CANNM_CBV_REPEAT_MESSAGE_REQUEST);
#endif
	}
}
#endif

/* 设置部分网络指示控制位（需要部分网络功能） */
#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
FUNC(void, CANNM_CODE) cannm_set_pni_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 设置CBV中的部分网络指示位 */
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		channel_internal->pdu_tx[channel_conf->pos_cbv] |= CANNM_CBV_PNI;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		channel_internal->spts_tx_msg[channel_conf->pos_cbv] |= CANNM_CBV_PNI;
#endif
	}
}
#endif

/* 设置主动唤醒控制位（需要主动唤醒功能） */
#if (CANNM_ACTIVE_WUP_BIT_FEATURE_ENABLED == STD_ON)
FUNC(void, CANNM_CODE) cannm_set_active_wk_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 设置CBV中的主动唤醒位 */
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		channel_internal->pdu_tx[channel_conf->pos_cbv] |= CANNM_CBV_ACTIVE_WAKEUP;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		channel_internal->spts_tx_msg[channel_conf->pos_cbv] |= CANNM_CBV_ACTIVE_WAKEUP;
#endif
	}
}

/* 清除主动唤醒控制位 */
FUNC(void, CANNM_CODE) cannm_clr_active_wk_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 清除CBV中的主动唤醒位 */
	if (channel_conf->pos_cbv != CANNM_PDU_OFF) {
		channel_internal->pdu_tx[channel_conf->pos_cbv] =
			(uint8)(channel_internal->pdu_tx[channel_conf->pos_cbv] & (~(CANNM_CBV_ACTIVE_WAKEUP)));
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		channel_internal->spts_tx_msg[channel_conf->pos_cbv] &= (uint8)(~(CANNM_CBV_ACTIVE_WAKEUP));
#endif
	}
}
#endif

/* 切换模式和状态的处理函数 */
FUNC(void, CANNM_CODE) cannm_switch_mode_state_proc(cannm_op_chnl_cate *channel_internal, nm_mode_type mode,
	nm_state_type state)
{
	/* 检查指针是否有效 */
	if (NULL == channel_internal) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 设置新的模式和状态 */
	channel_internal->mode = mode;
	channel_internal->state = state;
}

/* 立即传输处理函数 */
FUNC(void, CANNM_CODE) cannm_imdt_tx_proc(cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if (NULL == channel_internal) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 设置立即传输模式 */
	channel_internal->left_msg_cyc_tm = 0u;
	channel_internal->imdt_tx_sd = 0u;
	channel_internal->imdt_en_mode = (boolean)TRUE;
}

/* 从准备总线睡眠状态到重复消息状态的处理函数 */
FUNC(void, CANNM_CODE) cannm_pbs_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean is_nw_req)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 首次PDU失败重试功能（如果启用） */
#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[channel_conf->id_net];

	cannm_first_pdu_failed_retry_flg[cannm_net_id] = (boolean)TRUE;
#endif

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();

	/* 切换到网络模式和重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);

	/* 如果需要立即传输，设置立即传输模式 */
	if ((channel_conf->imdt_tx > 0U) && is_nw_req) {
		cannm_imdt_tx_proc(channel_internal);
	} else {
		channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm + channel_conf->m_func_pid;
	}

	/* 设置重复消息计时器和超时计时器 */
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm + channel_conf->m_func_pid;
	channel_internal->left_tmot_tm = channel_conf->tm_out + channel_conf->m_func_pid;

	/* 通知网络模式处理 */
	nm_mode_net_proc(channel_conf->id_net);

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
}

/* 从准备总线睡眠状态到总线睡眠状态的处理函数 */
FUNC(void, CANNM_CODE) cannm_pbs_to_bs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();

	/* 切换到总线睡眠模式 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_BUS_SLEEP, NM_MODE_BUS_SLEEP);

	/* 通知总线睡眠模式处理 */
	nm_mode_bs_proc(channel_conf->id_net);

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_BUS_SLEEP);
#endif

	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_BUS_SLEEP);
#endif
}

/* 从总线睡眠状态到重复消息状态的处理函数 */
FUNC(void, CANNM_CODE) cannm_bs_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean is_nw_req)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 首次PDU失败重试功能（如果启用） */
#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[channel_conf->id_net];

	cannm_first_pdu_failed_retry_flg[cannm_net_id] = (boolean)TRUE;
#endif

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();

	/* 切换到网络模式和重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);

	/* 如果需要立即传输，设置立即传输模式 */
	if ((channel_conf->imdt_tx > 0U) && is_nw_req) {
		cannm_imdt_tx_proc(channel_internal);
	} else {
		channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm + channel_conf->m_func_pid;
	}

	/* 设置超时计时器和重复消息计时器 */
	channel_internal->left_tmot_tm = channel_conf->tm_out + channel_conf->m_func_pid;
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm + channel_conf->m_func_pid;

	/* 通知网络模式处理 */
	nm_mode_net_proc(channel_conf->id_net);

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

	/* 通知状态变化（如果启用） */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
}

/* 总线睡眠状态处理函数 */
FUNC(void, CANNM_CODE) cannm_bs_to_bs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 检查指针是否有效 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 通知网络启动指示 */
	nm_start_net_ind(channel_conf->id_net);
	(void) channel_internal;
}
/**
 * @file cannm_state_transitions.c
 * @brief CAN网络管理状态转换处理函数集合
 *
 * 该文件包含CAN网络管理模块的各种状态转换处理函数，用于处理不同状态之间的转换逻辑，
 * 包括重复消息状态、准备睡眠状态、正常操作状态等之间的转换。
 */

/**
 * @brief 从重复消息状态处理到重复消息状态
 *
 * 该函数用于在重复消息状态下更新超时时间。
 *
 * @param[in] channel_conf 通道配置信息，包含超时时间等参数
 * @param[in,out] channel_internal 通道内部状态信息，用于更新剩余超时时间
 */
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		/* 报告空指针错误 */
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 更新剩余超时时间为配置的超时时间 */
	channel_internal->left_tmot_tm = channel_conf->tm_out;
}

/**
 * @brief 从重复消息状态处理到准备睡眠状态
 *
 * 该函数处理从重复消息状态转换到准备睡眠状态的逻辑，包括状态切换、清除重复消息请求等。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rdy_slp_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
	/* 获取网络ID */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[channel_conf->id_net];
	/* 重置首次PDU失败重试标志 */
	cannm_first_pdu_failed_retry_flg[cannm_net_id] = (boolean)FALSE;
#endif

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到准备睡眠状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_READY_SLEEP);

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
	/* 清除重复消息请求回调 */
	cannm_clr_rpt_msg_req_cbv(channel_conf, channel_internal);
#endif

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_REPEAT_MESSAGE, NM_STATE_READY_SLEEP);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_REPEAT_MESSAGE, NM_STATE_READY_SLEEP);
#endif
}

/**
 * @brief 从重复消息状态处理到正常操作状态
 *
 * 该函数处理从重复消息状态转换到正常操作状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到正常操作状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_NORMAL_OPERATION);

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
	/* 清除重复消息请求回调 */
	cannm_clr_rpt_msg_req_cbv(channel_conf, channel_internal);
#endif

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_REPEAT_MESSAGE, NM_STATE_NORMAL_OPERATION);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_REPEAT_MESSAGE, NM_STATE_NORMAL_OPERATION);
#endif
}

#if ((CANNM_NODE_DETECTION_ENABLED == STD_ON) || (STD_OFF != CANNM_PN_HANDLE_MULTIPLE_NETWORK_REQUESTS))
/**
 * @brief 从正常操作状态处理到重复消息状态
 *
 * 该函数处理从正常操作状态转换到重复消息状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);
	/* 设置重复消息时间和消息周期时间 */
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm + channel_conf->m_func_pid;
	channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm + channel_conf->m_func_pid;

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
}
#endif

/**
 * @brief 从正常操作状态处理到准备睡眠状态
 *
 * 该函数处理从正常操作状态转换到准备睡眠状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rdy_slp_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

#if (CANNM_FIRST_PDU_FAILED_RETRY == STD_ON)
	/* 获取网络ID */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[channel_conf->id_net];
	/* 重置首次PDU失败重试标志 */
	cannm_first_pdu_failed_retry_flg[cannm_net_id] = (boolean)FALSE;
#endif

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到准备睡眠状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_READY_SLEEP);

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_READY_SLEEP);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

	/* 避免编译器警告未使用参数 */
	(void) channel_conf;

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_READY_SLEEP);
#endif
}

/**
 * @brief 从正常操作状态处理到正常操作状态
 *
 * 该函数用于在正常操作状态下更新超时时间。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_nor_opr_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 更新剩余超时时间为配置的超时时间 */
	channel_internal->left_tmot_tm = channel_conf->tm_out;
}

/**
 * @brief 从准备睡眠状态处理到正常操作状态
 *
 * 该函数处理从准备睡眠状态转换到正常操作状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到正常操作状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_NORMAL_OPERATION);
	/* 设置消息周期时间 */
	channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm + channel_conf->m_func_pid;

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_NORMAL_OPERATION);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_NORMAL_OPERATION);
#endif
}

/**
 * @brief 从准备睡眠状态处理到准备总线睡眠状态
 *
 * 该函数处理从准备睡眠状态转换到准备总线睡眠状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_pbs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到准备总线睡眠状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_PREPARE_BUS_SLEEP, NM_STATE_PREPARE_BUS_SLEEP);
	/* 设置等待总线睡眠时间和消息超时时间 */
	channel_internal->left_wait_bs_tm = channel_conf->wait_bs_tm;
	channel_internal->left_msg_tmot_tm = 0u;
	/* 处理准备总线睡眠模式 */
	nm_mode_pbs_proc(channel_conf->id_net);

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_PREPARE_BUS_SLEEP);
#endif

#if (CANNM_ACTIVE_WUP_BIT_FEATURE_ENABLED == STD_ON)
	/* 清除主动唤醒回调 */
	cannm_clr_active_wk_cbv(channel_conf, channel_internal);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_PREPARE_BUS_SLEEP);
#endif
}

#if ((CANNM_NODE_DETECTION_ENABLED == STD_ON) || (STD_OFF != CANNM_PN_HANDLE_MULTIPLE_NETWORK_REQUESTS))
/**
 * @brief 从准备睡眠状态处理到重复消息状态
 *
 * 该函数处理从准备睡眠状态转换到重复消息状态的逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);
	/* 设置重复消息时间和消息周期时间 */
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm + channel_conf->m_func_pid;
	channel_internal->left_msg_cyc_tm = channel_conf->msg_cys_ofst_tm + channel_conf->m_func_pid;

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
}
#endif

/**
 * @brief 网络模式到网络模式处理
 *
 * 该函数用于在网络模式下更新超时时间。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in,out] channel_internal 通道内部状态信息
 */
FUNC(void, CANNM_CODE) cannm_net_mode_to_net_mode_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return;
	}
	/* 更新剩余超时时间 */
	channel_internal->left_tmot_tm = channel_conf->tm_out + channel_conf->m_func_pid;
}

#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
/**
 * @brief 正常接收处理
 *
 * 该函数处理正常接收逻辑，检查PNI（Partial Network Information）等条件。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in] pni PNI标志
 * @return Std_ReturnType 处理结果
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_nor_rx_proc(const cannm_chnl_cate *channel_conf,
	boolean pni)
{
	/* 参数检查 */
	if (NULL == channel_conf) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RX_NORMAL_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 如果不支持PN，直接返回OK */
	if (!channel_conf->pn_support) {
		return E_OK;
	}
	/* 如果配置为所有消息唤醒且PNI为假，返回OK */
	if ((channel_conf->all_msg_wk) && (!pni)) {
		return E_OK;
	}
	return E_NOT_OK;
}

/**
 * @brief 丢弃接收处理
 *
 * 该函数处理需要丢弃的接收逻辑。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in] channel_internal 通道内部状态信息
 * @param[in] pni PNI标志
 * @return Std_ReturnType 处理结果
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_dscd_rx_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean pni)
{
	/* 参数检查 */
	if (NULL == channel_conf) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RX_DISCARD_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	if (NULL == channel_internal) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_RX_DISCARD_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 如果配置不是所有消息唤醒且PNI为假，返回NOT_OK */
	if ((!channel_conf->all_msg_wk) && (!pni)) {
		return E_NOT_OK;
	}
	/* 如果过滤PDU为假，返回NOT_OK */
	if (!channel_internal->filter_pdu) {
		return E_NOT_OK;
	}
	return E_OK;
}

/**
 * @brief 接收处理总函数
 *
 * 该函数综合处理接收逻辑，先尝试正常接收，再检查是否需要丢弃。
 *
 * @param[in] channel_conf 通道配置信息
 * @param[in] channel_internal 通道内部状态信息
 * @param[in] pni PNI标志
 * @return Std_ReturnType 处理结果
 */
FUNC(Std_ReturnType, CANNM_CODE) cannm_rx_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean pni)
{
	/* 参数检查 */
	if ((NULL == channel_conf) || (NULL == channel_internal)) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_MAINFUNCTION_CANNM_E_NULL_POINTER);
		return E_NOT_OK;
	}
	/* 先尝试正常接收处理 */
	if (E_OK == cannm_nor_rx_proc(channel_conf, pni)) {
		return E_OK;
	}
	/* 再检查丢弃接收处理 */
	if (E_NOT_OK == cannm_dscd_rx_proc(channel_conf, channel_internal, pni)) {
		return E_NOT_OK;
	}
}
#endif

/**
 * @brief 获取当前请求状态
 *
 * 该函数用于获取指定通道的当前请求状态。
 *
 * @param[in] channel 通道号
 * @return boolean 请求状态
 */
FUNC(boolean, CANNM_CODE) cannm_monitor_get_current_requested(uint8 channel)
{
	return cannm_internal.nm_tunnel[channel].requested;
}

/**
 * @brief 获取CAN接口首个NM消息ID
 *
 * 该函数用于获取CAN接口的第一个网络管理消息ID。
 *
 * @return uint16 首个NM消息ID
 */
FUNC(uint16, CANNM_CODE) cannm_get_canif_first_nm_msg_id(void)
{
	return canif_first_nm_msg_id;
}

/**
 * @brief 从正常操作状态立即发送重复消息
 *
 * 该函数用于从正常操作状态立即切换到重复消息状态并发送消息。
 *
 * @param[in] nm_id 网络管理ID
 */
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rpt_msg_imdt_tx(const network_handle_type nm_id)
{
	/* 初始化状态检查 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NO_INIT);
		return;
	}
	/* 通道有效性检查 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 状态检查：必须在正常操作状态 */
	if (channel_internal->state != NM_STATE_NORMAL_OPERATION) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);
	/* 设置重复消息时间 */
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm;
	/* 如果配置了立即发送，则处理立即发送 */
	if (channel_conf->imdt_tx > 0U) {
		cannm_imdt_tx_proc(channel_internal);
	}

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
}

/**
 * @brief 从重复消息状态立即发送重复消息
 *
 * 该函数用于在重复消息状态下立即发送消息并重置相关计时器。
 *
 * @param[in] nm_id 网络管理ID
 */
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rpt_msg_imdt_tx(const network_handle_type nm_id)
{
	/* 初始化状态检查 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NO_INIT);
		return;
	}
	/* 通道有效性检查 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 状态检查：必须在重复消息状态 */
	if (channel_internal->state != NM_STATE_REPEAT_MESSAGE) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 重置超时时间和重复消息时间 */
	channel_internal->left_tmot_tm = channel_conf->tm_out;
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm;
	/* 如果配置了立即发送，则处理立即发送 */
	if (channel_conf->imdt_tx > 0U) {
		cannm_imdt_tx_proc(channel_internal);
	}
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();
}

/**
 * @brief 从准备睡眠状态立即发送重复消息
 *
 * 该函数用于从准备睡眠状态立即切换到重复消息状态并发送消息。
 *
 * @param[in] nm_id 网络管理ID
 */
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_rpt_msg_imdt_tx(const network_handle_type nm_id)
{
	/* 初始化状态检查 */
	if (CANNM_STATUS_UNINIT == cannm_internal.init_status) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_NO_INIT);
		return;
	}
	/* 通道有效性检查 */
	if (nm_id >= cannm_config_ptr->chnl_num_len) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}
	/* 获取网络ID和通道索引 */
	network_handle_type cannm_net_id = cannm_config_ptr->tunnel_trans[nm_id];
	uint8 channel_index = cannm_config_ptr->chnl_num[cannm_net_id];
	const cannm_chnl_cate *channel_conf = &cannm_config_ptr->nm_tunnel[channel_index];
	cannm_op_chnl_cate *channel_internal = &cannm_internal.nm_tunnel[channel_index];

	/* 状态检查：必须在准备睡眠状态 */
	if (channel_internal->state != NM_STATE_READY_SLEEP) {
		CANNM_DET_REPORT(CANNM_CANNM_SERVICEID_GETSTATE_CANNM_E_INVALID_CHANNEL);
		return;
	}

	/* 进入临界区 */
	SchM_Enter_CANNM_EXCLUSIVE_AREA_0();
	/* 切换到重复消息状态 */
	cannm_switch_mode_state_proc(channel_internal, NM_MODE_NETWORK, NM_STATE_REPEAT_MESSAGE);
	/* 设置重复消息时间 */
	channel_internal->left_rpt_msg_tm = channel_conf->rpt_msg_tm;
	/* 如果配置了立即发送，则处理立即发送 */
	if (channel_conf->imdt_tx > 0U) {
		cannm_imdt_tx_proc(channel_internal);
	}

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知BSWM状态变化 */
	nm_ntfy_state_chg_bswm(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
	/* 退出临界区 */
	SchM_Exit_CANNM_EXCLUSIVE_AREA_0();

#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	/* 通知状态变化 */
	nm_ntfy_state_chg(channel_conf->id_net, NM_STATE_READY_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif
}

#define CANNM_STOP_SEC_CODE
#include "cannm_memmap.h"
