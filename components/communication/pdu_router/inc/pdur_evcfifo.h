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

#ifndef PDUR_EVCFIFO_H
#define PDUR_EVCFIFO_H

#include "pdur.h"
#include "pdur_buffermanager.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PDUR_SYNC_EVCFIFO 0x00u // 在中断处理函数中使用的evc_fifo
#define PDUR_ASYNC_EVCFIFO 0x01u // 在周期函数中使用的evc_fifo

/*!*********************************************************************************************************************
 * @brief   定义pdur_evc_fifo类型
 **********************************************************************************************************************/
typedef uint8 pdur_evc_fifo_sync_type;

/*!*********************************************************************************************************************
 * @brief   定义pdur事件数据类型
 **********************************************************************************************************************/
typedef struct pdur_evc_data pdur_evc_data_type;

/*!*********************************************************************************************************************
 * @brief   定义pdur事件队列描述类型
 **********************************************************************************************************************/
typedef struct pdur_evc_fifo_desc pdur_evc_fifo_desc_type;
/*!*********************************************************************************************************************
 * @brief   定义pdur事件队列类型
 **********************************************************************************************************************/
typedef struct pdur_evc_fifo pdur_evc_fifo_type;

/*!*********************************************************************************************************************
 * @brief   定义pdur事件数据类型
 **********************************************************************************************************************/
struct pdur_evc_data {
	PduIdType src_pdu_id; ///< 源的编号
	uint8 dest_pdu_idx; ///< 本次evc承载的目的信息，当前目的在路由路径中的位置
};

/*!*********************************************************************************************************************
 * @brief   定义pdur事件队列类型
 **********************************************************************************************************************/
struct pdur_evc_fifo {
	uint16 write_idx; ///< 写索引
	uint16 read_idx; ///< 读索引
};

/*!*********************************************************************************************************************
 * @brief   定义pdur事件队列描述类型
 **********************************************************************************************************************/
struct pdur_evc_fifo_desc {
	pdur_evc_fifo_type *fifo; ///< 事件队列指针
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	SpinlockIdType spinlock_id; ///< 自旋锁id
#endif
	pdur_evc_data_type *data; ///< 数据指针
	uint16 size; ///< 事件队列大小
};

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur缓存池初始化
 * @details        pdur缓存池初始化
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_evc_fifo_init(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          将evc data push到指定的fifo队列
 * @details        将evc data push到指定的fifo队列
 * @param[in]      core_id                               指定fifo所在的核
 * @param[in]      evc_data                              evc data 指针
 * @param[in]      sync_type                             evc_fifo类型 同步或异步
 * @retval         E_OK                                 事件队列压入成功
 * @retval         E_NOT_OK                             事件队列压入失败/尝试获取自旋锁失败/事件队列已满
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE)
pdur_evc_fifo_push(pdur_core_id_type core_id, const pdur_evc_data_type *evc_data, pdur_evc_fifo_sync_type sync_type);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          将evc data 从指定的fifo队列pop出来
 * @details        将evc data 从指定的fifo队列pop出来
 * @param[in]      core_id                               指定fifo所在的核
 * @param[in]      evc_data                              evc data 指针
 * @param[in]      sync_type                             evc_fifo类型 同步或异步
 * @retval         E_OK                                 事件队列压入成功
 * @retval         E_NOT_OK                             事件队列压入失败/尝试获取自旋锁失败/事件队列已空
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE)
pdur_evc_fifo_pop(pdur_core_id_type core_id, pdur_evc_data_type *evc_data, pdur_evc_fifo_sync_type sync_type);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          判断evc fifo是否为空
 * @details        判断evc fifo是否为空
 * @param[in]      write_idx        队列写索引
 * @param[in]      read_idx         队列读索引
 * @retval         TRUE            事件队列为空
 * @retval         FALSE           事件队列不为空
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_fifo_is_empty(uint16 write_idx, uint16 read_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          判断evc fifo是否已满
 * @details        判断evc fifo是否已满
 * @param[in]      write_idx        队列写索引
 * @param[in]      read_idx         队列读索引
 * @param[in]      size            队列深度
 * @retval         TRUE            事件队列为满
 * @retval         FALSE           事件队列不为满
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_evc_fifo_is_full(uint16 write_idx, uint16 read_idx, uint16 size);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          改变队列索引值
 * @details        改变队列索引值
 * @param[in]      index   队列索引（写与读）
 * @param[in]      size    队列深度
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_fifo_idx_increment(uint16 *index, uint16 size);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取evc_fifo的内存地址
 * @details        获取evc_fifo的内存地址
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(const pdur_evc_fifo_desc_type *const *, PDUR_CODE) pdur_get_evc_fifo_desc_ref_table(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          获取if类的evc_fifo的内存地址
 * @details        获取evc_fifo的内存地址
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(const pdur_evc_fifo_desc_type *const *, PDUR_CODE) pdur_get_if_evc_fifo_desc_ref_table(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif /* PDUR_MULTICORE_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* PDUR_EVCFIFO_H */
