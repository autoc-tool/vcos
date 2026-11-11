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

#ifndef VCOS_LOG_CFG_H
#define VCOS_LOG_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#define VCOS_ASW_LOG_LEVEL VCOS_LOG_DEBUG ///< ASP日志级别
#define VCOS_BSWM_LOG_LEVEL VCOS_LOG_DEBUG ///< BSWM日志级别
#define VCOS_CANIF_LOG_LEVEL VCOS_LOG_ERROR ///< CANIF日志级别
#define VCOS_CANNM_LOG_LEVEL VCOS_LOG_INFO ///< CANNM日志级别
#define VCOS_CANSM_LOG_LEVEL VCOS_LOG_DEBUG ///< CANSM日志级别
#define VCOS_CANTP_LOG_LEVEL VCOS_LOG_INFO ///< CANTP日志级别
#define VCOS_COM_LOG_LEVEL VCOS_LOG_INFO ///< COM日志级别
#define VCOS_COMM_LOG_LEVEL VCOS_LOG_DEBUG ///< COMM日志级别
#define VCOS_CSM_LOG_LEVEL VCOS_LOG_INFO ///< CSM日志级别
#define VCOS_DCM_LOG_LEVEL VCOS_LOG_INFO ///< DCM日志级别
#define VCOS_DEM_LOG_LEVEL VCOS_LOG_INFO ///< DEM日志级别
#define VCOS_E2E_LOG_LEVEL VCOS_LOG_INFO ///< E2E日志级别
#define VCOS_EA_LOG_LEVEL VCOS_LOG_ERROR ///< EA日志级别
#define VCOS_ECUM_LOG_LEVEL VCOS_LOG_INFO ///< ECUM日志级别
#define VCOS_EEP_LOG_LEVEL VCOS_LOG_ERROR ///< EEP日志级别
#define VCOS_ETHSM_LOG_LEVEL VCOS_LOG_INFO ///< ETHSM日志级别
#define VCOS_ETHTSYN_LOG_LEVEL VCOS_LOG_INFO ///< ETHTSYN日志级别
#define VCOS_LINIF_LOG_LEVEL VCOS_LOG_INFO ///< LINIF日志级别
#define VCOS_NVM_LOG_LEVEL VCOS_LOG_INFO ///< NVM日志级别
#define VCOS_OS_LOG_LEVEL VCOS_LOG_INFO ///< OS日志级别
#define VCOS_PDUR_LOG_LEVEL VCOS_LOG_INFO ///< PDUR日志级别
#define VCOS_RTE_LOG_LEVEL VCOS_LOG_INFO ///< RTE日志级别
#define VCOS_WDGM_LOG_LEVEL VCOS_LOG_ERROR ///< WDGM日志级别
#define VCOS_XCP_LOG_LEVEL VCOS_LOG_INFO ///< XCP日志级别
#define VCOS_DET_LOG_LEVEL VCOS_LOG_ERROR ///< DET日志级别
#define VCOS_CRYPTO_LOG_LEVEL VCOS_LOG_ERROR ///< CRYPTO日志级别
#define VCOS_ETHIF_LOG_LEVEL VCOS_LOG_ERROR ///< ETHIF日志级别
#define VCOS_DLT_LOG_LEVEL VCOS_LOG_INFO ///< DLT日志级别
#define VCOS_SOAD_LOG_LEVEL VCOS_LOG_INFO ///< SoAd 模块日志级别
#define VCOS_DOIP_LOG_LEVEL VCOS_LOG_INFO ///< DoIP 模块日志级别
#define VCOS_TCPIP_LOG_LEVEL VCOS_LOG_INFO ///< TcpIp日志级别
#define VCOS_MONITOR_LOG_LEVEL VCOS_LOG_INFO ///< Monitor日志级别

#include "syslog.h"

#define VCOS_LOG_INTER_OFF(appid, ...) ///< 关闭日志信息
#define VCOS_LOG_INTER_FATAL(appid, ...) syslog(LOG_CRIT, __VA_ARGS__) ///< 输出致命错误日志信息
#define VCOS_LOG_INTER_ERROR(appid, ...) syslog(LOG_ERR, __VA_ARGS__) ///< 输出错误日志信息
#define VCOS_LOG_INTER_WARN(appid, ...) syslog(LOG_WARNING, __VA_ARGS__) ///< 输出警告日志信息
#define VCOS_LOG_INTER_INFO(appid, ...) syslog(LOG_INFO, __VA_ARGS__) ///< 输出信息日志
#define VCOS_LOG_INTER_DEBUG(appid, ...) syslog(LOG_DEBUG, __VA_ARGS__) ///< 输出调试日志信息
#define VCOS_LOG_INTER_VERBOSE(appid, ...) syslog(LOG_DEBUG, __VA_ARGS__) ///< 输出详细日志信息

#define VCOS_PRINTF(...) syslog(LOG_NOTICE, __VA_ARGS__) ///< 重新定义printf为空的LOG宏
#define VCOS_PRINTF_SHELL(...) syslog(LOG_NOTICE, __VA_ARGS__) ///< 重新定义shell printf为空的LOG宏

#define VCOS_PRINTF_SYNC(...) syslog(LOG_NOTICE, __VA_ARGS__) ///< 输出为空

#define VCOS_TEST_LOG(appid, level, ...) syslog(LOG_NOTICE, __VA_ARGS__) ///< 测试日志宏定义为空的LOG宏

#ifdef __cplusplus
}
#endif

#endif /*VCOS_LOG_CFG_H*/