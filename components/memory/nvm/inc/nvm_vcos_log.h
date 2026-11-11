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

#ifndef NVM_VCOS_LOG_H
#define NVM_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_DLT_CANCEL_WRITEALL "NvM WriteAll canceled at Block ID %d\r\n" ///< 取消NvM的WriteAll操作
#define NVM_DLT_MULTIBLOCK_FAILED "NvM Multiblock Job Failed %u\r\n" ///< NvM多块操作失败
#define NVM_DLT_RESTORE_RESULT "Block %u Restore result %u\r\n" ///< 块恢复结果
#define NVM_DLT_JOB_PENDING "Block %u Job result : %u Keep on waiting\r\n" ///< 任务结果：继续等待
#define NVM_DLT_JOB_DONE "NvM block done, Block: %d, job type: %d \r\n" ///< NvM块完成，任务类型
#define NVM_DLT_JOB_FAILED "NvM read/write failed, job result : %u \r\n" ///< NvM读/写失败，任务结果
#define NVM_DLT_JOB_START "Block ID %d BaseNumber:%d Length:%d Num:%d\r\n" ///< 任务开始
#define NVM_DLT_JOB_CANCEL "Block ID %d Job has been canceld\r\n" ///< 任务已被取消
#define NVM_DLT_BLOCK_STATUS "NvM block %u Status result %u\r\n" ///< 块状态结果
#define NVM_DLT_JOB_CANCEL_FAIL "Block ID %d Job not exist in queue\r\n" ///< 任务在队列中不存在
#define NVM_DLT_WRITEBLOCK_FAILED "NvM Write Failed, Block %u Write Failed, result %u\r\n" ///< 写入失败
#define NVM_DLT_WRITE_FAILED "Block %u Write Failed, result %u\r\n" ///< 写入失败
#define NVM_DLT_READ_FAILED "Block %u Read Failed, result %u\r\n" ///< 读取失败
#define NVM_DLT_CRC_FAILED "Block %u CRC check Failed\r\n" ///< CRC校验失败
#define NVM_DLT_SYS_READALL "[NvM] Read All.\r\n" ///< 读取所有
#define NVM_DLT_SYS_WRITEALL "[NvM] Write All.\r\n" ///< 写入所有
#define NVM_DLT_SYS_WRITEBLOCK "[NvM] Write Block:%d, Ret=%d, Cnt=%lu.\r\n" ///< 写入块
#define NVM_DLT_SYS_CANCEL_WRITEALL "[NvM] Cancel Write All.\r\n" ///< 取消写入所有

#ifdef __cplusplus
}
#endif

#endif /* NVM_VCOS_LOG_H */
