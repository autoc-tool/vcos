/*
 * copyright (c) 2025 li auto inc. and its affiliates
 * licensed under the apache license, version 2.0(the "License");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef E2E_INTERNAL_H
#define E2E_INTERNAL_H

#if (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (VCOS_MODULE_CONFIG_DET == 1)
#define E2E_DET_REPORT(errid) \
	do { \
		(void)det_report_error_with_info(errid, NULL, 0); \
	} while(0)
#else
#define E2E_DET_REPORT(errid)
#endif

#if (VCOS_MODULE_CONFIG_AUTOTEST == 1)
#define E2E_STATIC
#else
#define E2E_STATIC static
#endif

#ifdef __cplusplus
}
#endif

#endif /* E2E_INTERNAL_H */
