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
 * @file        Os.h
 * @brief       操作系统抽象层头文件
 * @details     包含任务管理、调度表、中断控制等核心功能接口
 * @date        2023-3-17
 * @version     V1.0
 * @copyright   Copyright (C) 2015-2024 Lixiang Group Holding Limited
 **********************************************************************************************************************/

#ifndef AUTOSAROS_INC_OS_H
#define AUTOSAROS_INC_OS_H

#include <nuttx/config.h>
#include "Os_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief        激活指定任务
 * @details      将任务从挂起态转为就绪态，确保从任务入口函数开始执行
 * @param[in]    TaskID      任务标识符
 * @retval       E_OK        操作成功
 * @retval       E_OS_LIMIT  激活次数超过阈值
 * @retval       E_OS_ID     无效的任务ID
 * @reentrant    可重入
 * @synchronous  同步调用
 * @swsid        SWS_Os_00509
 **********************************************************************************************************************/
StatusType ActivateTask(TaskType TaskID);

/*!*********************************************************************************************************************
 * @brief        异步任务激活
 * @details      非阻塞式激活任务，立即返回不等待任务执行
 * @param[in]    TaskID  任务标识符
 * @reentrant    可重入
 * @synchronous  异步调用
 * @swsid        SWS_Os_91022
 **********************************************************************************************************************/
void ActivateTaskAsyn(TaskType TaskID);

/*!*********************************************************************************************************************
 * @brief        终止当前任务
 * @details      将调用任务从运行态转为挂起态，释放所有占用的资源
 *               注意：在终止前需释放持有的自旋锁
 * @retval       E_OK     操作成功
 * @retval       E_NOT_OK 终止失败（如处于中断上下文）
 * @reentrant    可重入
 * @synchronous  同步调用
 * @swsid        SWS_Os_00258
 **********************************************************************************************************************/
StatusType TerminateTask(void);

/*!*********************************************************************************************************************
 * @brief        链式调用任务
 * @details      将调用任务从运行态转为挂起态，随后立刻激活指定任务
 * @param[in]    TaskID         目标任务ID
 * @retval       E_OK           操作成功
 * @retval       E_OS_ID        无效的目标任务ID
 * @retval       E_OS_RESOURCE  调用任务占用资源
 * @retval       E_OS_CALLEVEL  在ISR上下文中调用
 * @reentrant    可重入
 * @synchronous  同步
 * @swsid        SWS_Os_00052
 **********************************************************************************************************************/
StatusType ChainTask(TaskType TaskID);

/*!*********************************************************************************************************************
 * @brief          调度函数
 * @details        如果存在更高优先级的任务已准备就绪，则释放任务的内部资源，将当前任务放入就绪状态，
 *                 保存其上下文并执行更高优先级的任务。否则，继续调用任务。
 * @retval         执行结果的状态
 * @retval         E_OK     调用函数成功
 * @retval         E_NOT_OK 调用函数失败
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00641
 **********************************************************************************************************************/
StatusType Schedule(void);

/*!*********************************************************************************************************************
 * @brief          返回当前正在运行的任务的TaskID信息
 * @details        返回当前正在运行的任务的TaskID信息
 * @param[in]      TaskIDRef      指向当前正在运行的任务的引用
 * @retval         E_OK           操作成功
 * @retval         E_OS_CALLEVEL  在错误的上下文中调用
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00643
 **********************************************************************************************************************/
StatusType GetTaskID(TaskRefType TaskIDRef);

/*!*********************************************************************************************************************
 * @brief          返回任务的状态
 * @details        在调用GetTaskState时返回任务的状态（运行中，就绪，等待，挂起）。
 * @param[in]      TaskID           任务引用
 * @param[out]     taskStateRef     指向任务TaskID状态的引用
 * @retval         执行结果的状态
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid
 **********************************************************************************************************************/
StatusType GetTaskState(TaskType TaskID, TaskStateRefType taskStateRef);

/*!*********************************************************************************************************************
 * @brief          返回应用程序的状态
 * @details        在调用GetApplicationState时返回应用程序的状态(APPLICATION_TERMINATED、APPLICATION_RESTARTING、APPLICATION_ACCESSIBLE)。
 * @param[in]      AppID           应用程序引用
 * @param[out]     AppStateRef     应用程序AppID的状态引用
 * @retval         执行结果的状态
 * @retval         E_OK:           [标准:] 无错误
 * @retval         E_OS_ID:        [扩展:] 应用程序AppID无效
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00499
 **********************************************************************************************************************/
StatusType GetApplicationState(ApplicationType AppID, ApplicationStateRefType AppStateRef);

/*!*********************************************************************************************************************
 * @brief          确定调用者最初所属
 * @details        确定调用者最初所属(被配置为所属)的0S-Application(必须为每个应用分配一个唯一标识符)。
 * @retval         调用者所属应用程序的ApplicationlD
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00016
 **********************************************************************************************************************/
ApplicationType GetApplicationID(void);

/*!*********************************************************************************************************************
 * @brief          确定服务调用者当前正在执行的OS-Application。
 * @details        请注意，如果调用者不在CallTrustedFunction()调用中，则该值等于GetApplicationID()的结果。
 * @retval         调用者所属应用程序的ApplicationID
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00797
 **********************************************************************************************************************/
ApplicationType GetCurrentApplicationID(void);

/*!*********************************************************************************************************************
 * @brief          OS服务AllowAccess()。
 * @details        该服务将当前OS-Application的状态从APPLICATION_RESTARTING设置为APPLICATION_ACCESSIBLE。
 * @retval         E_OK              无错误。
 * @retval         E_OS_STATE        应用程序不处于重新启动状态。
 * @retval         E_OS_CALLEVEL     (扩展状态:) 从无效的上下文调用。
 * @retval         E_OS_DISABLEDINT  (扩展状态:) 调用者处于中断API序列中。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00501
 **********************************************************************************************************************/
StatusType AllowAccess(void);

/*!*********************************************************************************************************************
 * @brief          任务延时函数
 * @details        使当前任务阻塞等待一段时间
 * @param[in]      ticks_to_delay          阻塞等待Tick，使用pdMS_TO_TICKS()转换为Ticks
 * @retval         E_OK:                 [标准:] 无错误
 * @retval         E_OS_CALLEVEL:        [标准:] 在ISR上下文中调用
 * @retval         E_OS_RESOURCE:        [标准:] 资源被占用
 * @retval         E_OS_VALUE:           [标准:] 参数小于0
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EthIf_00024
 **********************************************************************************************************************/
StatusType task_tickdelay(TickType ticks_to_delay);

#if defined(CONFIG_RT_FRAMEWORK_SCHTBL) && (CONFIG_RT_FRAMEWORK_SCHTBL == 1)
/*!*********************************************************************************************************************
 * @brief        相对时间启动调度表
 * @details      基于当前计数器值偏移启动
 * @param[in]    SchedTableID  调度表ID
 * @param[in]    Offset        启动偏移量
 * @retval       E_OK          启动成功
 * @retval       E_OS_ID       无效调度表ID
 * @retval       E_OS_VALUE    偏移量超过(OsCounterMaxValue - InitialOffset)或为0
 * @retval       E_OS_STATE    调度表已运行
 * @reentrant    可重入
 * @synchronous  同步调用
 * @swsid        SWS_Os_00347
 **********************************************************************************************************************/
StatusType StartScheduleTableRel(ScheduleTableType SchedTableID, TickType Offset);

/*!*********************************************************************************************************************
 * @brief        绝对时间启动调度表
 * @details      在指定计数器绝对数值点启动调度表，适用于时间精准触发的场景
 * @param[in]    SchedTableID  调度表ID（范围：1~OS_SCHTBL_NUM，参见Os_Cfg.h）
 * @param[in]    Start         启动的绝对计数值（需满足：Start % OsCounterCycle = 0）
 * @retval       E_OK          启动成功，调度表进入RUNNING状态
 * @retval       E_OS_ID       无效调度表ID（建议使用GetActiveScheduleTable进行诊断）
 * @retval       E_OS_VALUE    Start值超过OsCounterMaxAllowedValue（最大值：OS_COUNTER_MAX_VALUE）
 * @retval       E_OS_STATE    调度表已处于STARTED/RUNNING状态（需先调用StopScheduleTable）
 * @reentrant    可重入（采用自旋锁保护调度表状态机）
 * @synchronous  同步调用（阻塞直到调度表加入就绪队列）
 * @swsid        SWS_Os_00358
 * @note
 *  - 设计约束：要求底层计数器服务已初始化（参见Os_InitCounter）
 *  - 多核场景：跨核启动时需通过IPC消息同步，时延抖动<10us
 *  - 错误处理：E_OS_VALUE错误需结合GetCounterValue诊断计数器溢出问题
 **********************************************************************************************************************/
StatusType StartScheduleTableAbs(ScheduleTableType SchedTableID, TickType Start);

/*!*********************************************************************************************************************
 * @brief        立即停止调度表
 * @details      强制终止指定调度表的运行，将其状态置为STOPPED
 * @param[in]    SchedTableID  调度表ID
 * @retval       E_OK          停止成功，调度表进入STOPPED状态
 * @retval       E_OS_ID       无效调度表ID
 * @retval       E_OS_NOFUNC   调度表已处于STOPPED状态
 * @reentrant    可重入
 * @synchronous  同步调用
 * @swsid        SWS_Os_00006
 **********************************************************************************************************************/
StatusType StopScheduleTable(ScheduleTableType SchedTableID);

/*!*********************************************************************************************************************
 * @brief        获取调度表状态信息
 * @details      查询指定调度表的完整运行状态
 * @param[in]    SchedTableID  调度表ID（范围：1~OS_SCHTBL_NUM，参考Os_SchedTbl_CfgType配置）
 * @param[out]   StatusRef     状态信息结构体指针（内存需由调用方分配）
 * @retval       E_OK          查询成功，StatusRef字段已更新
 * @retval       E_OS_ID       无效调度表ID（建议结合ListActiveSchedTables接口验证）
 * @reentrant    可重入（采用RCU机制保障多核读取一致性）
 * @synchronous  同步调用（执行时间<500ns，无上下文切换）
 * @swsid        SWS_Os_00227
 **********************************************************************************************************************/
StatusType GetScheduleTableStatus(ScheduleTableType SchedTableID, ScheduleTableStatusRefType StatusRef);

/*!*********************************************************************************************************************
 * @brief        动态切换调度表
 * @details      实现运行中调度表的热切换，保持时序连续性：
 *               - 停止当前调度表（CurrSchedTableID）的到期点生成
 *               - 立即启动目标调度表（NextSchedTableID）的执行流
 *               - 保持计数器相位连续性（基于OsCounter的余数补偿）
 * @param[in]    CurrSchedTableID  当前活跃调度表ID
 * @param[in]    NextSchedTableID  目标调度表ID
 * @retval       E_OK          切换成功，NextSchedTableID进入RUNNING状态
 * @retval       E_OS_ID       无效调度表ID
 * @retval       E_OS_NOFUNC  当前调度表未启动
 * @retval       E_OS_STATE   目标调度表处于非法状态（STARTED/RUNNING/NEXT）
 * @reentrant    可重入（采用双锁机制保护状态迁移）
 * @synchronous  同步调用（最大阻塞时间50us，符合ASIL-D时序约束）
 * @swsid        SWS_Os_00191
 **********************************************************************************************************************/
StatusType NextScheduleTable(ScheduleTableType CurrSchedTableID, ScheduleTableType NextSchedTableID);

/*!*********************************************************************************************************************
 * @brief        停止态调度表链式切换
 * @details      在当前调度表处于STOPPED状态时建立调度表执行链，实现调度策略的预配置切换：
 *               - 验证当前调度表处于非活跃状态（STOPPED）
 *               - 建立NextSchedTableID与CurrSchedTableID的链式关系
 *               - 继承相位基准（当Next调度表启动时自动对齐Curr调度表的终止点）
 * @param[in]    CurrSchedTableID  已停止的调度表ID（范围：1~OS_SCHTBL_NUM）
 * @param[in]    NextSchedTableID  后续调度表ID（需配置相同counter基准）
 * @retval       E_OK          链式关系建立成功
 * @retval       E_OS_ID       无效调度表ID（建议使用VerifySchedTableChain接口预校验）
 * @retval       E_OS_NOFUNC  当前调度表未停止（需先执行StopScheduleTable）
 * @retval       E_OS_STATE   目标调度表处于非法状态（STARTED/RUNNING/NEXT）
 * @reentrant    可重入（基于每调度表的链式锁实现）
 * @synchronous  同步调用（执行时间确定，最大延迟15us）
 * @swsid        SWS_Os_00191
 **********************************************************************************************************************/
StatusType NextScheduleTableFromStop(ScheduleTableType CurrSchedTableID, ScheduleTableType NextSchedTableID);

/*!*********************************************************************************************************************
 * @brief        显式同步启动调度表
 * @details      启动配置为显式同步(EXPLICIT)模式的调度表
 * @param[in]    SchedTableID  调度表ID
 * @retval       E_OK          成功进入WAIT_SYNC状态
 * @retval       E_OS_ID       无效调度表ID或非显式同步表
 * @retval       E_OS_STATE    调度表已处于非STOPPED状态
 * @reentrant    可重入
 * @synchronous  同步调用
 * @swsid        SWS_Os_00201
 **********************************************************************************************************************/
StatusType StartScheduleTableSynchron(ScheduleTableType SchedTableID);

/*!*********************************************************************************************************************
 * @brief        调度表同步信号注入
 * @details      向显式同步调度表注入时间基准信号，实现多ECU时间精准对齐。
 * @param[in]    SchedTableID  调度表ID
 * @param[in]    TickValue     主计数器当前值（需通过安全时钟源获取）
 * @retval       E_OK          同步成功
 * @retval       E_OS_ID       无效ID或策略非显式同步
 * @retval       E_OS_VALUE    数值越界（TickValue > OsCounterMaxValue）
 * @retval       E_OS_STATE    调度表未处于WAIT_SYNC状态
 * @reentrant    可重入
 * @synchronous  同步
 * @swsid        SWS_Os_00199
 **********************************************************************************************************************/
StatusType SyncScheduleTable(ScheduleTableType SchedTableID, TickType TickValue);

/*!*********************************************************************************************************************
 * @brief        调度表异步模式切换
 * @details      将显式同步调度表转为异步自由运行模式，解除与主计数器的时间耦合。切换后调度表周期由本地时钟驱动。
 * @param[in]    SchedTableID  调度表ID
 * @retval       E_OK          模式切换成功
 * @retval       E_OS_ID       无效调度表ID
 * @reentrant    可重入
 * @synchronous  同步
 * @swsid        SWS_Os_00422
 **********************************************************************************************************************/
StatusType SetScheduleTableAsync(ScheduleTableType SchedTableID);

/*!*********************************************************************************************************************
 * @brief        调度表偏差值注入
 * @details      向显式同步调度表注入指定的偏差值，用于补偿调度表与同步计数器之间的
 *               时间漂移。
 *               偏差(deviation)定义为需要添加到（或从中减去）调度表当前值的最小滴答数，
 *               使调度表与同步计数器的值相等。
 *               正值偏差：表示同步计数器超前于调度表运行，系统将缩短延迟使到期点
 *               提前发生，实现调度表"加速"；
 *               负值偏差：表示同步计数器滞后于调度表运行，系统将延长延迟使到期点
 *               推迟发生，实现调度表"减速"。
 *               主要用于多ECU时间同步、测试验证和故障注入场景。
 * @param[in]    schedtable_id    调度表ID（必须为显式同步策略的调度表）
 * @param[in]    deviate_value    偏差值（正值=加速调度表，负值=减速调度表，
 *                                绝对值不得超过调度表duration）
 * @retval       E_OK             偏差注入成功，调度表将在后续执行中逐步消除偏差
 * @retval       E_OS_ID          无效调度表ID或同步策略非显式同步
 * @retval       E_OS_VALUE       偏差值超出允许范围（|deviate_value| > duration）
 * @retval       E_OS_STATE       调度表未处于运行状态
 * @reentrant    可重入
 * @synchronous  同步
 * @swsid        SWS_Os_00483
 **********************************************************************************************************************/
StatusType DeviateScheduleTable(ScheduleTableType schedtable_id, DeviateType deviate_value);
#endif /* CONFIG_RT_FRAMEWORK_SCHTBL */

/*!*********************************************************************************************************************
 * @brief         恢复由DisableAllInterrupts保存的状态
 * @details       该服务可从1类中断2类中断以及任务级别调用，但不能从hook调用。
 *                本服务是DisableAllInterrupts服务的对应操作（必须先调用DisableAllInterrupts），
 *                其目的是结束临界区。离开临界区时不应调用任何API服务。
 *                实现应根据目标硬件适配本服务以提供最小开销。
 *                通常，该服务会启用CPU对中断的识别。
 * @reentrant     可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00071
 **********************************************************************************************************************/
void EnableAllInterrupts(void);
/*!*********************************************************************************************************************
 * @brief          禁用所有硬件支持屏蔽的中断。在调用EnableAllInterrupts前保存状态
 * @details        该服务可从1类中断2类中断以及任务级别调用，但不能从hook调用。
 *                 本服务用于启动临界区。该临界区必须通过调用EnableAllInterrupts服务来结束。
 *                 在此临界区内不允许调用任何API服务。实现应根据目标硬件适配本服务以提供最小开销。
 *                 通常，该服务会禁用CPU对中断的识别。
 *                 注意：本服务不支持嵌套。如需临界区嵌套（如库函数场景），
 *                 应使用SuspendOSInterrupts/ResumeOSInterrupts或SuspendAllInterrupt/ResumeAllInterrupts服务。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00590
 **********************************************************************************************************************/
void DisableAllInterrupts(void);

/*!*********************************************************************************************************************
 * @brief          恢复由SuspendAllInterrupts服务保存的所有中断识别状态
 * @details        该服务可从1类中断2类中断以及任务级别调用，
 *                 但不能从所有钩子例程调用。本服务是SuspendAllInterrupts服务的对应操作
 *                 （必须先调用SuspendAllInterrupts），其目的是结束临界区。
 *                 在此临界区内，除SuspendAllInterrupts/ResumeAllInterrupts对和
 *                 SuspendOSInterrupts/ResumeOSInterrupts对外，不允许调用其他API服务。
 *                 实现应根据目标硬件适配本服务以提供最小开销。
 *                 SuspendAllInterrupts/ResumeAllInterrupts可嵌套调用。
 *                 当嵌套调用SuspendAllInterrupts和ResumeAllInterrupts时，
 *                 首次SuspendAllInterrupts调用保存的中断识别状态将由最后一次
 *                 ResumeAllInterrupts服务调用恢复。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00593
 **********************************************************************************************************************/
void ResumeAllInterrupts(void);

/*!*********************************************************************************************************************
 * @brief          保存所有中断的识别状态并禁用硬件支持屏蔽的所有中断
 * @details        该服务可从1类中断2类中断、警报回调例程以及任务级别调用，
 *                 但不能从所有钩子例程调用。本服务用于保护代码临界区免受任何中断干扰。
 *                 该临界区必须通过调用ResumeAllInterrupts服务来结束。
 *                 在此临界区内，除SuspendAllInterrupts/ResumeAllInterrupts对和
 *                 SuspendOSInterrupts/ResumeOSInterrupts对外，不允许调用其他API服务。
 *                 实现应根据目标硬件适配本服务以提供最小开销。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00592
 **********************************************************************************************************************/
void SuspendAllInterrupts(void);

/*!*********************************************************************************************************************
 * @brief          恢复由SuspendOSInterrupts服务保存的中断识别状态
 * @details        该服务可从1类中断2类中断以及任务级别调用，但不能从钩子例程调用。
 *                 本服务是SuspendOSInterrupts服务的对应操作（必须先调用SuspendOSInterrupts），
 *                 其目的是结束临界区。在此临界区内，除SuspendAllInterrupts/ResumeAllInterrupts对和
 *                 SuspendOSInterrupts/ResumeOSInterrupts对外，不允许调用其他API服务。
 *                 实现应根据目标硬件适配本服务以提供最小开销。
 *                 SuspendOSInterrupts/ResumeOSInterrupts支持嵌套调用。当嵌套调用
 *                 SuspendOSInterrupts和ResumeOSInterrupts时，首次SuspendOSInterrupts调用
 *                 保存的中断识别状态将由最后一次ResumeOSInterrupts服务调用恢复。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00595
 **********************************************************************************************************************/
void ResumeOSInterrupts(void);

/*!*********************************************************************************************************************
 * @brief          保存2类中断的识别状态并禁用其中断识别
 * @details        该服务可从ISR和任务级别调用，但不能从钩子例程调用。
 *                 本服务用于保护代码临界区，该临界区必须通过调用ResumeOSInterrupts服务来结束。
 *                 在此临界区内，除SuspendAllInterrupts/ResumeAllInterrupts对和
 *                 SuspendOSInterrupts/ResumeOSInterrupts对外，不允许调用其他API服务。
 *                 实现应根据目标硬件适配本服务以提供最小开销。本服务仅禁用类别2中断的识别，
 *                 但如果无法高效实现，则可能会禁用更多中断。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00594
 **********************************************************************************************************************/
void SuspendOSInterrupts(void);

/* OSEK 事件 API 声明 */

/*!*********************************************************************************************************************
 * @brief          向任务发送事件
 * @details        该接口将Mask指定的事件发送至目标任务。若目标任务正在等待Mask中指定的
 *                 一个或多个事件，则目标任务将转为就绪状态。
 * @param[in]      TaskID          目标任务ID
 * @param[in]      Mask            待发送的事件掩码
 * @retval         E_OK            事件发送成功
 * @retval         E_OS_ID         目标任务ID无效
 * @retval         E_OS_ACCESS     目标任务不是扩展任务
 * @retval         E_OS_STATE      目标任务处于挂起状态
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00604
 **********************************************************************************************************************/
StatusType SetEvent(TaskType TaskID, EventMaskType Mask);

/*!*********************************************************************************************************************
 * @brief         异步向任务发送事件
 * @details       该接口将Mask指定的事件发送至目标任务。若目标任务正在等待Mask中指定的一个或多个事件，
 *                则目标任务将转为就绪状态。
 * @param[in]     TaskID          目标任务ID
 * @param[in]     Mask            待发送的事件掩码
 * @reentrant     可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00604
  **********************************************************************************************************************/
void SetEventAsyn(TaskType TaskID, EventMaskType Mask);

/*!*********************************************************************************************************************
 * @brief         清除事件值
 * @details       该接口从当前任务的事件值中清除由Mask指定的事件。
 * @param[in]     Mask            待清除的事件掩码
 * @retval        E_OK            事件清除成功
 * @retval        E_OS_ACCESS     调用任务不是扩展任务
 * @retval        E_OS_CALLEVEL   在中断上下文中调用
 * @reentrant     可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00604
  **********************************************************************************************************************/
StatusType ClearEvent(EventMaskType Mask);

/*!*********************************************************************************************************************
 * @brief          获取事件值
 * @details        该接口用于获取目标任务当前的事件值
 * @param[in]      TaskID          目标任务ID
 * @param[out]     Mask            用于存储获取到的事件值的指针
 * @retval         E_OK            事件获取成功
 * @retval         E_OS_ID         目标任务ID无效
 * @retval         E_OS_ACCESS     目标任务不是扩展任务
 * @retval         E_OS_STATE      目标任务处于挂起状态
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00604
  **********************************************************************************************************************/
StatusType GetEvent(TaskType TaskID, EventMaskRefType Mask);

/*!*********************************************************************************************************************
 * @brief          等待事件
 * @details        当未发生任何事件时，调用任务将进入等待状态。当等待的一个或多个事件发生时，
 *                 任务将转为就绪状态。此调用可能触发任务重新调度。当任务处于等待状态时，
 *                 其内部资源将被释放。该接口只能由拥有事件对象的扩展任务调用。
 * @param[in]      Mask            等待的事件掩码
 * @retval         E_OK            等待事件成功
 * @retval         E_OS_ACCESS     调用任务不是扩展任务
 * @retval         E_OS_RESOURCE   调用任务占用资源
 * @retval         E_OS_CALLEVEL   在中断上下文中调用
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00604
  **********************************************************************************************************************/
StatusType WaitEvent(EventMaskType Mask);

/*!*********************************************************************************************************************
 * @brief          带超时的等待事件
 * @details        当未发生任何事件时，调用任务将进入等待状态。当等待的一
 *                 个或多个事件发生时，任务将转为就绪状态。如果在指定的超
 *                 时时间内没有事件发生，任务也会转为就绪状态。此调用可能
 *                 触发任务重新调度。当任务处于等待状态时，其内部资源将被
 *                 释放。该接口只能由拥有事件对象的扩展任务调用。
 * @param[in]      mask            等待的事件掩码
 * @param[in]      ticks_to_wait      等待的超时时间(系统节拍数)
 * @retval         E_OK            等待事件成功
 * @retval         E_OS_ACCESS     调用任务不是扩展任务
 * @retval         E_OS_RESOURCE   调用任务占用资源
 * @retval         E_OS_CALLEVEL   在中断上下文中调用
 * @retval         E_OS_TIMEOUT    等待超时
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
StatusType event_tickwait(EventMaskType mask, TickType ticks_to_wait);

#if defined(CONFIG_RT_FRAMEWORK_RESOURCE) && (CONFIG_RT_FRAMEWORK_RESOURCE == 1)
/*!*********************************************************************************************************************
 * @brief         进入代码中由 ResID 引用的资源分配的关键区域。关键区域应始终使用 ReleaseResource 离开。
 * @details       仅当内层关键区域完全在外围关键区域内执行时，才允许嵌套资源占用。同一资源的嵌套占用也是禁止的！建议
 *                 在同一函数内出现对应的 GetResource 和 ReleaseResource 调用。在关键区域内不允许使用对不可抢占任务进
 *                 行重新调度的服务。此外，必须在中断服务例程完成之前离开关键区域。一般来说，关键区域应尽量简短。本服务
 *                 可从 ISR 和任务级别调用。
 * @param[in]     ResourceID      资源引用
 * @retval        E_OK            [标准:] 无错误
 * @retval        E_OS_ID         [扩展:] 资源 ResID 无效
 * @retval        E_OS_ACCESS     [扩展:] 尝试获取已被任何任务或 ISR 占用的资源，或调用任务或中断例程的静态分配优先级高于计算的顶级优先级
 * @reentrant     可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00369
 **********************************************************************************************************************/
StatusType GetResource(ResourceType ResourceID);

/*!*********************************************************************************************************************
 * @brief         ReleaseResource 是 GetResource 的对应操作，用于离开代码中由 ResID 引用的资源分配的关键区域。
 * @details       有关嵌套条件的信息，请参阅 GetResource 的详细说明。本服务可从 ISR 和任务级别调用。
 * @param[in]     ResourceID      资源引用
 * @retval        E_OK            [标准:] 无错误
 * @retval        E_OS_ID         [扩展:] 资源 ResID 无效
 * @retval        E_OS_NOFUNC     [扩展:] 尝试释放未被任何任务或 ISR 占用的资源，或应先释放另一个资源
 * @retval        E_OS_ACCESS     [扩展:] 尝试释放具有比调用任务或中断例程的静态分配优先级更低的顶级优先级的资源
 * @reentrant     可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00801
 **********************************************************************************************************************/
StatusType ReleaseResource(ResourceType ResourceID);
#endif /* CONFIG_RT_FRAMEWORK_RESOURCE */

/* AUTOSAR 自旋锁API声明 */
#if defined(CONFIG_RT_FRAMEWORK_SPINLOCK) && (CONFIG_RT_FRAMEWORK_SPINLOCK == 1)
/*!*********************************************************************************************************************
 * @brief          尝试占用自旋锁变量。函数返回时，表示锁已成功占用或发生错误。
 *                 自旋锁机制采用主动轮询方式，本函数不会触发任务重新调度。
 * @details        若函数返回，表示锁已成功占用或发生错误。自旋锁机制采用主动轮询方式，
 *                 本函数不会触发任务重新调度。
 * @param[in]      SpinlockId       要锁定的自旋锁实例标识符
 * @retval         E_OK                         [标准:] 无错误
 * @retval         E_OS_ID                      [扩展:] 自旋锁ID无效
 * @retval         E_OS_ACCESS                  [扩展:] 无法访问自旋锁
 * @retval         E_OS_INTERFERENCE_DEADLOCK   [扩展:] 任务尝试占用已被同核任务占用的自旋锁，
 *                                                  可能导致死锁
 * @retval         E_OS_NESTING_DEADLOCK        [扩展:] 任务尝试占用自旋锁，但同核任务以可能
 *                                                  导致死锁的方式持有其他自旋锁
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00649
 **********************************************************************************************************************/
StatusType GetSpinlock(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief          释放先前占用的自旋锁变量
 * @details        在终止任务前，应释放所有通过GetSpinlock()占用的自旋锁变量。
 *                 在调用WaitEVENT前，必须释放所有自旋锁。
 * @param[in]      SpinlockId      要释放的自旋锁实例标识符
 * @retval         执行结果状态
 * @retval         E_OK:           [标准:] 无错误
 * @retval         E_OS_ID:        [扩展:] 自旋锁ID无效
 * @retval         E_OS_ACCESS:    [扩展:] 无法访问自旋锁
 * @retval         E_OS_STATE:     [扩展:] 自旋锁未被当前任务占用
 * @retval         E_OS_NOFUNC:    [扩展:] 在释放本自旋锁前尝试释放其他自旋锁(或资源)
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00695
 **********************************************************************************************************************/
StatusType ReleaseSpinlock(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief          尝试获取自旋锁并立即返回状态
 * @details        TryToGetSpinlock与GetSpinlock功能相同，区别在于：如果自旋锁已被不同核的任务占用，
 *                 本函数将设置OUT参数"Success"并返回E_OK。
 * @param[in]      SpinlockId      要锁定的自旋锁实例标识符
 * @param[out]     Success         返回是否成功占用锁的布尔值
 * @retval         执行结果状态
 * @retval         E_OK:           [标准:] 无错误
 * @retval         E_OS_ID:        [扩展:] 自旋锁ID无效
 * @retval         E_OS_ACCESS:    [扩展:] 无法访问自旋锁
 * @retval         E_OS_INTERFERENCE_DEADLOCK: [扩展:] 任务尝试占用已被同核任务占用的自旋锁，
 *                                                  可能导致死锁
 * @retval         E_OS_NESTING_DEADLOCK:      [扩展:] 任务尝试占用自旋锁，但同核任务以可能
 *                                                  导致死锁的方式持有其他自旋锁
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00703
 **********************************************************************************************************************/
StatusType TryToGetSpinlock(SpinlockIdType SpinlockID, TryToGetSpinlockType *Success);

/*!*********************************************************************************************************************
 * @brief          尝试占用读写锁中的读锁。函数返回时，表示锁已成功占用或发生错误。
 *                 读写锁机制采用主动轮询方式，本函数不会触发任务重新调度。
 * @details        若函数返回，表示锁已成功占用或发生错误。读写锁机制采用主动轮询方式，
 *                 本函数不会触发任务重新调度。
 * @param[in]      SpinlockId       要锁定的读写锁实例标识符
 * @retval         E_OK                         [标准:] 无错误
 * @retval         E_OS_ID                      [扩展:] 读写锁ID无效
 * @retval         E_OS_ACCESS                  [扩展:] 无法访问读写锁
 * @retval         E_OS_INTERFERENCE_DEADLOCK   [扩展:] 任务尝试占用读锁，但该任务已持有
 *                                                  同一读写锁（读/写模式），可能导致死锁
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid         SWS_Os_00649
 **********************************************************************************************************************/
StatusType GetSpinlockRead(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief          尝试占用读写锁中的写锁。函数返回时，表示锁已成功占用或发生错误。
 *                 读写锁机制采用主动轮询方式，本函数不会触发任务重新调度。
 * @details        若函数返回，表示锁已成功占用或发生错误。读写锁机制采用主动轮询方式，
 *                 本函数不会触发任务重新调度。
 * @param[in]      SpinlockId       要锁定的读写锁实例标识符
 * @retval         E_OK                         [标准:] 无错误
 * @retval         E_OS_ID                      [扩展:] 读写锁ID无效
 * @retval         E_OS_ACCESS                  [扩展:] 无法访问读写锁
 * @retval         E_OS_INTERFERENCE_DEADLOCK   [扩展:] 任务尝试占用写锁，但该任务已持有
 *                                                  同一读写锁（读/写模式），可能导致死锁
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00649
 **********************************************************************************************************************/
StatusType GetSpinlockWrite(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief          释放先前占用的读锁，减少读锁计数器
 * @details        在终止任务前，应释放所有通过GetSpinlockRead()占用的读写锁变量。
 *                 在调用WaitEVENT前，必须释放所有读写锁。
 * @param[in]      SpinlockId      要释放的读写锁实例标识符
 * @retval         执行结果状态
 * @retval         E_OK:           [标准:] 无错误
 * @retval         E_OS_ID:        [扩展:] 读写锁ID无效
 * @retval         E_OS_ACCESS:    [扩展:] 无法访问读写锁
 * @retval         E_OS_STATE:     [扩展:] 读写锁未被当前任务占用
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00695
 **********************************************************************************************************************/
StatusType ReleaseSpinlockRead(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief           释放先前占用的写锁，并将写锁计数器清零
 * @details         在终止任务前，应释放所有通过GetSpinlockWrite()占用的读写锁变量。
 *                  在调用WaitEVENT前，必须释放所有读写锁。
 * @param[in]       SpinlockId       要释放的读写锁实例标识符
 * @retval          执行结果状态
 * @retval          E_OK:           [标准:] 无错误
 * @retval          E_OS_ID:        [扩展:] 读写锁ID无效
 * @retval          E_OS_ACCESS:    [扩展:] 无法访问读写锁
 * @retval          E_OS_STATE:     [扩展:] 读写锁未被当前任务占用
 * @retval          E_OS_NOFUNC:    [扩展:] 在释放本读写锁前尝试释放其他读写锁(或资源)
 * @reentrant       可重入
 * @synchronous     同步
 * @swsid           SWS_Os_00695
 **********************************************************************************************************************/
StatusType ReleaseSpinlockWrite(SpinlockIdType SpinlockID);

/*!*********************************************************************************************************************
 * @brief           尝试获取读锁并立即返回状态
 * @details         TryToGetSpinlockRead与GetSpinlockRead功能相同，区别在于：如果读写锁已被不同核的任务占用，
 *                  本函数将设置OUT参数"Success"并返回E_OK。
 * @param[in]       SpinlockId       要锁定的读写锁实例标识符
 * @param[out]      Success          返回是否成功占用读锁的布尔值
 * @retval          执行结果状态
 * @retval          E_OK:           [标准:] 无错误
 * @retval          E_OS_ID:        [扩展:] 读写锁ID无效
 * @retval          E_OS_ACCESS:    [扩展:] 无法访问读写锁
 * @retval          E_OS_INTERFERENCE_DEADLOCK: [扩展:] 任务尝试占用读锁，但该锁已被同核任务占用，
 *                                                  可能导致死锁
 * @reentrant       可重入
 * @synchronous     同步
 * @swsid           SWS_Os_00703
 **********************************************************************************************************************/
StatusType TryToGetSpinlockRead(SpinlockIdType SpinlockID, TryToGetSpinlockType *Success);

/*!*********************************************************************************************************************
 * @brief           尝试获取写锁并立即返回状态
 * @details         TryToGetSpinlockWrite与GetSpinlockWrite功能相同，区别在于：如果读写锁已被不同核的任务占用，
 *                  本函数将设置OUT参数"Success"并返回E_OK。
 * @param[in]       SpinlockId       要锁定的读写锁实例标识符
 * @param[out]      Success          返回是否成功占用写锁的布尔值
 * @retval          执行结果状态
 * @retval          E_OK:           [标准:] 无错误
 * @retval          E_OS_ID:        [扩展:] 读写锁ID无效
 * @retval          E_OS_ACCESS:    [扩展:] 无法访问读写锁
 * @retval          E_OS_INTERFERENCE_DEADLOCK: [扩展:] 任务尝试占用写锁，但该锁已被同核任务占用，
 *                                                  可能导致死锁
 * @reentrant       可重入
 * @synchronous     同步
 * @swsid           SWS_Os_00703
 **********************************************************************************************************************/
StatusType TryToGetSpinlockWrite(SpinlockIdType SpinlockID, TryToGetSpinlockType *Success);
#endif /* (CONFIG_RT_FRAMEWORK_SPINLOCK == 1) */

#if defined(CONFIG_RT_FRAMEWORK_COUNTER) && (CONFIG_RT_FRAMEWORK_COUNTER == 1)
/*!*********************************************************************************************************************
 * @brief          OS服务IncrementCounter()。
 * @details        该服务增加一个软件计数器。
 * @param[in]      CounterID           要增加的计数器。
 * @retval         E_OK                无错误。
 * @retval         E_OS_ID             (扩展状态:) CounterID不是有效的软件计数器ID。
 * @retval         E_OS_CALLEVEL       (扩展状态:) 从无效的上下文调用。
 * @retval         E_OS_CORE           (扩展状态:) 给定对象属于外部核心。
 * @retval         E_OS_ACCESS         (服务保护:) 调用者可以使用
 * @retval         E_OS_DISABLEDINT    (扩展状态:) 调用者在中断API序列中。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00399
 **********************************************************************************************************************/
StatusType IncrementCounter(CounterType CounterID);

/*!*********************************************************************************************************************
 * @brief          读取计数器的当前计数值
 * @details        读取计数器的当前计数值(如果计数器由硬件驱动，则返回硬件计时器的计数值，如果由用户驱动，则返回软件计时器的计数值)。
 * @param[in]      CounterID  应读取其计数值的计数器
 * @param[out]     TickValue  包含计数器的当前计数值
 * @retval         E_OK       [标准:] 无错误
 * @retval         E_OS_ID    [扩展:] CounterID无效
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00383
 **********************************************************************************************************************/
StatusType GetCounterValue(CounterType CounterID, TickRefType TickValue);

/*!*********************************************************************************************************************
 * @brief          获取当前计数值与先前读取的计数值之间的计数值。
 * @details        获取当前计数值与先前读取的计数值之间的计数值。
 * @param[in]      CounterID 计数器Id号
 * @param[in]      PreValue 计数器先前值
 * @param[in]      ElapsedValue 当前时刻的时间差
 * @retval         E_OK       无错误
 * @retval         E_OS_ID    CounterID无效
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00392
 **********************************************************************************************************************/
StatusType GetElapsedValue(CounterType CounterID, TickRefType PreValue, TickRefType ElapsedValue);
#endif

/*!*********************************************************************************************************************
 * @brief           OS服务GetCoreID()
 * @details         返回调用核的唯一逻辑核标识符。从物理核到逻辑CoreID的映射由具体实现决定。
 *                  本API仅允许在AUTOSAR核上使用。如需在非AUTOSAR核上使用该API，
 *                  可将该核配置为AUTOSAR核，但使用StartNonAutosarCore() API以非AUTOSAR核方式启动。
 * @retval          调用核的唯一ID
 * @reentrant       可重入
 * @synchronous     同步
 * @swsid           SWS_Os_00674
 **********************************************************************************************************************/
CoreIdType GetCoreID(void);

/*!*********************************************************************************************************************
 * @brief         用户可以调用此系统服务以特定模式启动操作系统。
 * @details       只能在操作系统之外调用，因此可能适用特定实现的限制。
 *                另见第11.3章，系统启动, 特别是关于OSEK和OSEKtime共存的系统。
 *                此调用不需要返回。每个核上都调用一次。
 * @param[in]     Mode           应用模式，包括：
 *                                   - DONOTCARE
 *                                   - OS_APPMODE_无
 *                                   - OSDEFAULTAPPMODE
 *                                   - OS_APPMODE_ANY
 * @reentrant     不可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00678
 **********************************************************************************************************************/
void StartOS(AppModeType Mode);

/*!*********************************************************************************************************************
 * @brief         启动操作系统之前的系统相关初始化
 * @details       必须在使用任何操作系统服务之前调用
 * @reentrant     不可重入
 * @synchronous   同步
 **********************************************************************************************************************/
void OsPreStartInit(void);

/*!*********************************************************************************************************************
 * @brief         用户可以调用此系统服务以终止整个系统（例如，紧急关闭）。如果操作系统达到未定义的内部状态且不再准备运行，
 *                操作系统也会内部调用此功能。如果配置了ShutdownHook，
 *                则在关闭操作系统之前总是调用钩子例程ShutdownHook（以Error作为参数）。
 *                如果ShutdownHook返回，ShutdownOS的进一步行为是实现特定的。如果OSEK OS和OSEKtime OS共存，
 *                则ShutdownHook必须返回。Error需要是OSEK OS支持的有效错误代码。在OSEK OS和OSEKtime OS共存的系统中，
 *                Error也可能是OSEKtime OS接受的值。在这种情况下，如果启用OSEKtime配置参数，在OSEK OS关闭后，
 *                OSEKtime OS也将关闭。
 * @details       在此服务之后，操作系统将关闭。允许在任务级别、中断服务例程（ISR）级别、ErrorHook和StartupHook中调用，
 *                也可以由操作系统内部调用。如果操作系统调用ShutdownOS，则永远不会使用E_OK作为传递的参数值。
 * @param[in]     Error         发生的错误
 * @reentrant     不可重入
 * @synchronous   同步
 * @swsid         SWS_Os_00299
 **********************************************************************************************************************/
void ShutdownOS(StatusType Error);

/*!*********************************************************************************************************************
 * @brief          关闭所有核心
 * @details        传播关机请求到所有已启动的AUTOSAR核心，然后执行自身关机。仅允许在受信任的应用程序中使用。
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_Os_00713
 **********************************************************************************************************************/
void ShutdownAllCores(StatusType Error);

#ifdef __cplusplus
}
#endif

#endif /* AUTOSAROS_INC_OS_H */
