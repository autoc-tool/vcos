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
#ifndef VCOS_LOG_H
#define VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define VCOS_LOG_OFF 0 ///< VCOS_LOG等级-关闭
#define VCOS_LOG_FATAL 1 ///< VCOS_LOG等级-FATAL
#define VCOS_LOG_ERROR 2 ///< VCOS_LOG等级-ERROR
#define VCOS_LOG_WARN 3 ///< VCOS_LOG等级-WARN
#define VCOS_LOG_INFO 4 ///< VCOS_LOG等级-INFO
#define VCOS_LOG_DEBUG 5 ///< VCOS_LOG等级-DEBUG
#define VCOS_LOG_VERBOSE 6 ///< VCOS_LOG等级-VERBOSE

#define LOG(...)

#ifdef __cplusplus
}
#endif

#include "vcos_log_cfg.h"
#include "vcos_log_cbk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VCOS_LOG(appid, level, ...) VCOS_LOG_##level##_##appid(__VA_ARGS__) ///< LOG宏展开中间态

#ifndef SHELL_PRINT
#if ((defined VCOS_OS_CONFIG_MODULE_PANIC_SHELL) && (VCOS_OS_CONFIG_MODULE_PANIC_SHELL == 1))
#define SHELL_PRINT ShellPanicPrint ///< SHELL_PRINT宏展开中间态
#else
#define SHELL_PRINT VCOS_PRINTF_SHELL ///< SHELL_PRINT宏展开中间态
#endif
#endif

#ifndef SHELL_LOG
#if ((defined VCOS_OS_CONFIG_MODULE_PANIC_SHELL) && (VCOS_OS_CONFIG_MODULE_PANIC_SHELL == 1))
#define SHELL_LOG ShellPanicPrint ///< SHELL_LOG宏展开中间态
#else
#define SHELL_LOG VCOS_PRINTF_SHELL ///< SHELL_LOG宏展开中间态
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* VCOS_LOG_H */
