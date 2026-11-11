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

/*!*********************************************************************************************************************
 * @file        Os_Types.h
 * @brief       Os类型定义
 * @details     主要包含Os功能相关的宏定义，结构体定义等
 * @date        2023-3-17
 * @version     V1.0
 * @copyright   Copyright (C) 2015-2024 Lixiang Group Holding Limited
 **********************************************************************************************************************/

#ifndef __OS_TYPES_H
#define __OS_TYPES_H

#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <nuttx/event.h>
#include <nuttx/arch.h>
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* OSEK Error Codes */
#define E_OS_ACCESS                   ((StatusType)1) ///< 访问错误，通常指无权限访问资源。
#define E_OS_CALLEVEL                 ((StatusType)2) ///< 调用级别错误，表示服务调用不在允许的上下文中。
#define E_OS_ID                       ((StatusType)3) ///< 标识符错误，指示传递给服务的ID不合法。
#define E_OS_LIMIT                    ((StatusType)4) ///< 限制错误，表示超出了资源或值的限制。
#define E_OS_NOFUNC                   ((StatusType)5) ///< 无功能错误，指服务请求无法执行。
#define E_OS_RESOURCE                 ((StatusType)6) ///< 资源错误，指示资源管理相关的错误。
#define E_OS_STATE                    ((StatusType)7) ///< 状态错误，表示对象的状态不允许该操作。
#define E_OS_VALUE                    ((StatusType)8) ///< 值错误，指示传递给服务的参数值不合法。

/* AUTOSAR OS Error Classification */
#define E_OS_SERVICEID                ((StatusType)9) ///< 服务ID错误
#define E_OS_ILLEGAL_ADDRESS          ((StatusType)10) ///< 非法地址错误
#define E_OS_MISSINGEND               ((StatusType)11) ///< 缺少结束标记错误
#define E_OS_DISABLEDINT              ((StatusType)12) ///< 禁用中断错误
#define E_OS_STACKFAULT               ((StatusType)13) ///< 栈错误
#define E_OS_PROTECTION_MEMORY        ((StatusType)14) ///< 内存保护错误
#define E_OS_PROTECTION_TIME          ((StatusType)15) ///< 时间保护错误
#define E_OS_PROTECTION_ARRIVAL       ((StatusType)16) ///< 到达时间保护错误
#define E_OS_PROTECTION_LOCKED        ((StatusType)17) ///< 锁定保护错误
#define E_OS_PROTECTION_EXCEPTION     ((StatusType)18) ///< 异常保护错误
#define E_OS_CORE                     ((StatusType)19) ///< 核心错误
#define E_OS_SPINLOCK                 ((StatusType)20) ///< 自旋锁错误
#define E_OS_INTERFERENCE_DEADLOCK    ((StatusType)21) ///< 干扰死锁错误
#define E_OS_NESTING_DEADLOCK         ((StatusType)22) ///< 嵌套死锁错误
#define E_OS_PARAM_POINTER            ((StatusType)23) ///< 参数指针错误

/* RTFW Error codes */
#define E_OS_RATE                     ((StatusType)24) ///< 调用频率错误
#define E_OS_PROTECTION_RATE          ((StatusType)25) ///< 保护机制调用频率错误

#define E_OS_TIMEOUT                  ((StatusType)50) ///< 操作超时
#define E_OS_CRC                      ((StatusType)51) ///< CRC校验错误
#define E_OS_QUEUE_FULL               ((StatusType)52) ///< 队列已满
#define E_OS_QUEUE_EMPTY              ((StatusType)53) ///< 队列为空
#define E_OS_QUEUE_OVERWRITTEN        ((StatusType)54) ///< 队列数据被覆写
#define E_OS_QUEUE_NOCMPFUNC          ((StatusType)55) ///< 队列比较函数未设置
#define E_OS_NOTFOUND                 ((StatusType)56) ///< 未找到指定元素

#define E_OS_IPICALL_UNAVAILABLE      ((StatusType)57) ///< IPI调用不可用
#define E_OS_IPICALL_OVERFLOW         ((StatusType)58) ///< IPI调用溢出

#define E_OS_IOC_CHECK_FAILED         ((StatusType)60) ///< IOC检查失败
#define E_OS_IOC_LOST_DATA            ((StatusType)61) ///< IOC数据丢失
#define E_OS_IOC_NO_DATA              ((StatusType)62) ///< IOC无可用数据
#define E_OS_IOC_INVALID_CLASS        ((StatusType)63) ///< IOC无效类别
#define E_OS_IOC_UNAVAILABLE          ((StatusType)64) ///< IOC不可用

#define E_OS_FIFO_FULL                ((StatusType)65) ///< FIFO队列已满
#define E_OS_FIFO_EMPTY               ((StatusType)66) ///< FIFO队列为空

#define E_OS_INT_API_OVERFLOW         ((StatusType)67) ///< 中断api调用次数溢出
#define E_OS_INT_API_ORDER            ((StatusType)68) ///< 中断api调用顺序错误

#define E_OS_ILLEGAL                  ((StatusType)255) ///< 非法操作

#define INVALID_TASK (0xdeadU) ///< OS无效任务定义
#define INVALID_ISR ((sint16)(-1)) ///< OS无效ISR定义

#define OS_MODULE_ID (0x01U) ///< OS版本ID号

#define MAX_DELAY (0xffffffff) ///< OS 最大延迟

typedef enum
{
    /* Task management */
    OSServiceId_ActivateTask                  = 0x01,
    OSServiceId_TerminateTask                 = 0x02,
    OSServiceId_ChainTask                     = 0x03,
    OSServiceId_Schedule                      = 0x04,
    OSServiceId_GetTaskID                     = 0x05,
    OSServiceId_GetTaskState                  = 0x06,

    /* Interrupt control */
    OSServiceId_EnableInterruptSource         = 0x07,
    OSServiceId_DisableInterruptSource        = 0x08,
    OSServiceId_ClearPendingInterrupt         = 0x09,
    OSServiceId_DisableAllInterrupts          = 0x0A,
    OSServiceId_EnableAllInterrupts           = 0x0B,
    OSServiceId_SuspendAllInterrupts          = 0x0C,
    OSServiceId_ResumeAllInterrupts           = 0x0D,
    OSServiceId_SuspendOSInterrupts           = 0x0E,
    OSServiceId_ResumeOSInterrupts            = 0x0F,

    /* Resource management */
    OSServiceId_GetResource                   = 0x10,
    OSServiceId_ReleaseResource               = 0x11,

    /* Event control (extended tasks) */
    OSServiceId_SetEvent                      = 0x12,
    OSServiceId_ClearEvent                    = 0x13,
    OSServiceId_GetEvent                      = 0x14,
    OSServiceId_WaitEvent                     = 0x15,

    /* Alarm control */
    OSServiceId_GetAlarmBase                  = 0x16,
    OSServiceId_GetAlarm                      = 0x17,
    OSServiceId_SetRelAlarm                   = 0x18,
    OSServiceId_SetAbsAlarm                   = 0x19,
    OSServiceId_CancelAlarm                   = 0x1A,

    /* Schedule table services */
    OSServiceId_StartScheduleTableRel         = 0x1B,
    OSServiceId_StartScheduleTableAbs         = 0x1C,
    OSServiceId_StopScheduleTable             = 0x1D,
    OSServiceId_NextScheduleTable             = 0x1E,
    OSServiceId_GetScheduleTableStatus        = 0x1F,
    OSServiceId_StartScheduleTableSynchron    = 0x20,
    OSServiceId_SyncScheduleTable             = 0x21,
    OSServiceId_SetScheduleTableAsync         = 0x22,

    /* Counter services */
    OSServiceId_IncrementCounter              = 0x23,
    OSServiceId_GetCounterValue               = 0x24,
    OSServiceId_GetElapsedValue               = 0x25,

    /* Spinlock services (Multi-core) */
    OSServiceId_GetSpinlock                   = 0x26,
    OSServiceId_ReleaseSpinlock               = 0x27,
    OSServiceId_TryToGetSpinlock              = 0x28,

    /* OS application & core services */
    OSServiceId_AllowAccess                   = 0x29,
    OSServiceId_GetApplicationState           = 0x2A,
    OSServiceId_GetActiveApplicationMode      = 0x2B,

    OSServiceId_GetApplicationID              = 0x2C,
    OSServiceId_GetCurrentApplicationID       = 0x2D,
    OSServiceId_GetISRID                      = 0x2E,
    OSServiceId_CheckObjectAccess             = 0x2F,
    OSServiceId_CheckObjectOwnership          = 0x30,
    OSServiceId_TerminateApplication          = 0x31,

    OSServiceId_StartCore                     = 0x32,
    OSServiceId_GetCoreID                     = 0x33,
    OSServiceId_GetNumberOfActivatedCores     = 0x34,

    /* Trusted/NonTrusted Function calls */
    OSServiceId_CallTrustedFunction           = 0x35,
    OSServiceId_CallNonTrustedFunction        = 0x36,

    /* IOC (Inter-OS-Application Communication) services */
    OSServiceId_IocSend                       = 0x37,
    OSServiceId_IocWrite                      = 0x38,
    OSServiceId_IocReceive                    = 0x39,
    OSServiceId_IocRead                       = 0x3A,

    OSServiceId_IocSendGroup                  = 0x3B,
    OSServiceId_IocWriteGroup                 = 0x3C,
    OSServiceId_IocReceiveGroup               = 0x3D,
    OSServiceId_IocReadGroup                  = 0x3E,

    OSServiceId_IocEmptyQueue                 = 0x3F,

    /* Start and Shutdown service */
    OSServiceId_StartOS                       = 0x40,
    OSServiceId_ShutdownOS                    = 0x41,
    OSServiceId_ShutdownAllCores              = 0x42,

    /* RTFW Extensions */
    OSServiceId_GetHookInfo                   = 0x80,
    OSServiceId_GetTickCount                  = 0x81,
    OSServiceId_task_tickdelay                = 0x82,
    OSServiceId_DeviateScheduleTable          = 0x83,
    OSServiceId_NextScheduleTableFromStop     = 0x84,
    OSServiceId_SetScheduleTableAppId         = 0x85,
    OSServiceId_ClearScheduleTableAppId       = 0x86,
    OSServiceId_event_tickwait                = 0x87,

    OSSERVICEID_COUNT                         = 0xFF
} OSServiceIdType;

/* Global Type & Structure Definition */

typedef uint32 CoreIdType;

/*!*********************************************************************************************************************
 * @brief       数据类型表示一个Tick
 **********************************************************************************************************************/
typedef uint32 *TickRefType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个中断服务程序（ISR）。
 **********************************************************************************************************************/

typedef void (*TaskFunction_t)(void *);

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个中断服务程序（ISR）的入口函数。
 **********************************************************************************************************************/
typedef void (*ISRFunction_t)(void);

/*!*********************************************************************************************************************
 * @brief       这个引用类型表示一个中断服务函数。
 **********************************************************************************************************************/
typedef ISRFunction_t OSISRFunctionType;

/*!*********************************************************************************************************************
 * @brief       这种数据类型表示一个任务状态。.
 **********************************************************************************************************************/
typedef enum {
	RUNNING, ///< CPU 已经分配给该任务。
	READY, ///< 任务已经准备好切换到 RUNNING 状态，但正在等待处理器的分配。
	WAITING, ///< 任务无法继续执行，因为它正在等待至少一个事件。
	SUSPENDED, ///< 任务处于终止状态，可以被激活。
	INVALID_STATE ///< 任务处于无效状态。
} TaskStateType;

/*!*********************************************************************************************************************
 * @brief       应用程序访问状态
 **********************************************************************************************************************/
typedef enum {
	APPLICATION_TERMINATED = 0, ///< 应用程序已终止且不可访问。
	APPLICATION_RESTARTING, ///< 应用程序处于重新启动阶段且不可访问。
	APPLICATION_ACCESSIBLE ///< 应用程序处于活动状态且可访问。
} ApplicationStateType;

/*!*********************************************************************************************************************
 * @brief       任务类型
 **********************************************************************************************************************/
typedef enum {
	OBJECT_TASK, ///< 提供的对象是一个任务。
	OBJECT_ISR, ///< 提供的对象是一个中断服务程序（ISR）。
	OBJECT_COUNTER, ///< 提供的对象是一个计数器。
	OBJECT_SCHEDULETABLE, ///< 提供的对象是一个调度表。
} ObjectTypeType;

typedef uint32 StackType_t;

/*!*********************************************************************************************************************
 * @brief       操作系统OS对象的标识符。
 **********************************************************************************************************************/
typedef uint32 Os_ObjectIdType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个应用程序模式。
 **********************************************************************************************************************/
typedef uint8 AppModeType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个任务标识符。
 **********************************************************************************************************************/
typedef uint16 TaskType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个任务引用类型。
 **********************************************************************************************************************/
typedef TaskType *TaskRefType;

/*!*********************************************************************************************************************
 * @brief       这个引用类型表示一个任务状态。
 **********************************************************************************************************************/
typedef TaskStateType *TaskStateRefType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个hook标识符。
 **********************************************************************************************************************/
typedef uint32 HookType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示系统时钟。
 **********************************************************************************************************************/
typedef uint32 TickType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个 Ioc 标识符。
 **********************************************************************************************************************/
typedef uint32 IocIdType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示调度表计数器与同步计数器的偏差，既有正值又有负值。
 **********************************************************************************************************************/
typedef sint32 DeviateType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个任务优先级。
 **********************************************************************************************************************/
typedef uint32 OsTaskPrioType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个堆栈大小。
 **********************************************************************************************************************/
typedef uint16 OsStackSizeType;

/*!*********************************************************************************************************************
 * @brief       这个引用类型表示一个堆栈地址。
 **********************************************************************************************************************/
typedef uint32 *OsStackAddrType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个事件掩码。
 **********************************************************************************************************************/
typedef nxevent_mask_t EventMaskType;

/*!*********************************************************************************************************************
 * @brief       这个引用类型表示一个事件掩码。
 **********************************************************************************************************************/
typedef nxevent_mask_t *EventMaskRefType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个调度表类型。
 **********************************************************************************************************************/
typedef uint16 ScheduleTableType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个计数器类型。
 **********************************************************************************************************************/
typedef uint32 CounterType;

/*!*********************************************************************************************************************
 * @brief       此数据类型表示资源标识符。
 **********************************************************************************************************************/
typedef uint32 ResourceType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个自旋锁标识符。
 **********************************************************************************************************************/
typedef uint16 SpinlockIdType;

/*!*********************************************************************************************************************
 * @brief       这个类型是指向操作系统应用程序的引用。
 **********************************************************************************************************************/
typedef uint8 ApplicationIDType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个应用程序掩码状态。
 **********************************************************************************************************************/
typedef uint32 AccessApplMaskType;

/*!*********************************************************************************************************************
 * @brief       这个数据类型表示一个应用程序状态。
 **********************************************************************************************************************/
typedef ApplicationIDType ApplicationType;

/*!*********************************************************************************************************************
 * @brief       这个引用类型表示一个应用程序状态。
 **********************************************************************************************************************/
typedef ApplicationStateType *ApplicationStateRefType;

/*!*********************************************************************************************************************
 * @brief       在初始化期间迭代操作系统对象的索引类型。
 **********************************************************************************************************************/
typedef uint16 OsObjIndexType;

/*!*********************************************************************************************************************
 * @brief       指示自旋锁是否已被占用。
 **********************************************************************************************************************/
typedef enum {
	TRYTOGETSPINLOCK_SUCCESS, ///< 自旋锁成功占用
	TRYTOGETSPINLOCK_NOSUCCESS ///< 无法占用自旋锁
} TryToGetSpinlockType;

typedef enum ScheduleTableStatus {
	SCHEDULETABLE_STOPPED, ///< 调度表为STOPPED状态
	SCHEDULETABLE_NEXT, ///< 调度表为NEXT状态
	SCHEDULETABLE_WAITING, ///< 调度表为WAITING状态
	SCHEDULETABLE_RUNNING, ///< 调度表为RUNNING状态
	SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS ///< 调度表为RUNNING_AND_SYNCHRONOUS状态
} ScheduleTableStatusType;

typedef ScheduleTableStatusType *ScheduleTableStatusRefType;

#ifdef __cplusplus
}
#endif

#endif /* OS_AUTOSAROS_INC_OS_TYPES_H */
