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

#ifndef PDUR_BUFFERMANAGER_H
#define PDUR_BUFFERMANAGER_H

#include "pdur_cfg.h"
#include "Os_Types.h"
#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   定义pdur缓存管理类型
 **********************************************************************************************************************/
typedef struct {
	const uint8 self_node_idx; ///< buffer_manager节点自身索引*/
	const uint8 pool_idx; ///< buffer_manager所属的缓存池编号
	uint8 next_node_idx; ///< buffer_manager节点指向下一个节点的索引
	uint8 queue_next_node_idx; ///< buffer_manager节点指向fifo中下一个节点的索引
	uint8 routing_state[PDUR_MAX_GW_DESTINATIONS]; ///< 缓存池路由状态
	PduLengthType buffer_read_offset[PDUR_MAX_GW_DESTINATIONS]; ///< 缓存池读索引
	PduLengthType buffer_write_offset; ///< 缓存池写索引
	PduLengthType pdu_length; ///< src_pdu长度
	PduIdType src_pdu_id; ///< 源id
	PduLengthType buf_length; ///< buffer_manager中包含缓存的长度
	uint8 *buf_addr; ///< buffer_manager中包含缓存的地址
} pdur_buffer_manager_type;

/*!*********************************************************************************************************************
 * @brief   定义空闲链表和忙碌链表
 **********************************************************************************************************************/
typedef struct {
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	const SpinlockIdType spinlock_id; ///< 链表对应的自旋锁
#endif
	const PduLengthType block_length; ///< 缓存块长度
	uint8 head_node_idx; ///< 头节点索引
} pdur_buffer_manager_header_type;

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取buffer_manager空间
 * @details        根据src_pdu_id来申请buffer_manager空间
 * @param[in]      src_pdu_id      源pdu_id
 * @param[in]      tp_sdu_length   申请的缓存长度
 * @retval         buffer_manager 地址
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buffer_manager_by_src_pdu_id(PduIdType src_pdu_id, PduLengthType tp_sdu_length);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          释放buffer_manager空间
 * @details        根据src_pdu_id来释放buffer_manager空间
 * @param[in]      src_pdu_id     源pdu_id
 * @param[in]      is_queue_first TRUE-操作队列首个元素 / FALSE-操作队列末尾元素
 * @retval         E_OK         成功
 * @retval         E_NOT_OK     失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_free_buf_mager_by_src_pdu_id(PduIdType src_pdu_id, boolean is_queue_first);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          释放整个queue中所有的buffer_manager空间
 * @details        根据src_pdu_id来释放buffer_manager空间
 * @param[in]      src_pdu_id     源pdu_id
 * @retval         E_OK         成功
 * @retval         E_NOT_OK     失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_free_all_buf_mager_by_src_pdu_id(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          寻找buffer_manager空间
 * @details        根据src_pdu_id来寻找buffer_manager空间
 * @param[in]      src_pdu_id      源pdu_id
 * @retval         buffer_manager 地址
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE) pdur_find_buf_mager_by_src_pdu_id(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          判断忙碌链表中是否存在src_pdu_id
 * @details        检查忙碌链表中是否存在pdu_id未src_pdu_id的节点
 * @param[in]      src_pdu_id      源pdu_id
 * @retval         TRUE          存在
 * @retval         FALSE         不存在
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_check_buf_mag_with_pdu_id_existed_in_busy_list(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pbuffer_manager初始化
 * @details        pbuffer_manager初始化
 * @param[in]      applied_buffer_manager_addr      buffer_block内存块
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_pdu_id_buffer_manager_init(pdur_buffer_manager_type *applied_buffer_manager_addr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取当前忙碌节点的个数
 * @details        获取当前忙碌节点的个数
 * @retval         0         没有忙碌节点
 * @retval         1-255     当前忙碌节点的个数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, PDUR_CODE) pdur_get_num_of_buffer_manager_busy_node(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur缓存池管理初始化
 * @details        pdur缓存池管理初始化
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_buffer_manager_init(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取当前src_pdu_id对应的queue的队尾元素
 * @details        queue不存在或者当前queue中没有更多元素的时候直接返回在忙碌链表上面的节点地址，忙碌链表上不存在时返回null
 * @param[in]      src_pdu_id      用来申请缓存的pdu_id
 * @retval         queue的队尾元素
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_find_last_buf_mager_in_queue_by_src_pdu_id(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取当前src_pdu_id对应的queue的队首元素
 * @details        queue不存在或者当前queue中没有更多元素的时候直接返回在忙碌链表上面的节点地址，忙碌链表上不存在时返回null
 * @param[in]      src_pdu_id      用来申请缓存的pdu_id
 * @retval         queue的队首元素
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_find_first_buf_mager_in_queue_by_src_pdu_id(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          检查当前队列中有没有正在接收的元素
 * @details        一个pdu_id在同一时刻只能有一个元素处于接收状态，当队列中有接收元素时，不能申请新缓存
 *                 检查队列末尾元素rx流程开始触发
 * @param[in]      src_pdu_id      用来申请缓存的pdu_id
 * @retval         TRUE - 可以申请
 *                 FALSE - 不可申请
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_check_if_new_buf_alloc_allowed_in_queue(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          检查当前队列元素释放后是否要直接触发队列下一个元素直接开始发送
 * @details        队列缓存的元素需要接连不断地发送出去
 *                 队列首个元素tx流程结束触发
 * @param[in]      src_pdu_id      源pdu_id
 * @param[in]      dest_pdu_idx    当前dest在routing_path中的位置编号
 * @retval         TRUE - 可以直接触发下一个发送
 *                 FALSE - 不可直接触发
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_check_if_buf_mager_transmit_allowed_in_queue(PduIdType src_pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取当前src_pdu_id对应的queue的元素个数
 * @details        遍历整个queue，返回计数
 * @param[in]      用来申请缓存的pdu_id
 * @retval         TRUE - 可以直接触发下一个发送
 *                 FALSE - 不可直接触发
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, PDUR_CODE) pdur_get_buf_mager_queue_length(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          从空闲链表中获取一个缓存节点
 * @details        根据入参需要的长度获取一个缓存节点，返回缓存节点的地址
 * @param[in]      buffer_length - 缓存的需求长度
 * @retval         缓存地址
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE) pdur_buffer_alloc(PduLengthType buffer_length);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          从忙碌链表中释放一个缓存节点
 * @details        释放节点
 * @param[in]      buf_mager - 缓存地址
 * @retval         void
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_buffer_free(pdur_buffer_manager_type *buf_mager);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* PDUR_BUFFERMANAGER_H */
