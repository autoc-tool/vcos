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

/* Including File */
#if (CONFIG_RT_FRAMEWORK == 1)
#include "Os.h"
#endif
#if (VCOS_MODULE_CONFIG_SHELL == 1)
#include "Shell.h"
#endif
#include "sysm_init.h"
#include "sysm_init_internals.h"
#include "vcos_log.h"
#include "sysm_init_vcos_log.h"

#define ECUM_START_SEC_CODE
#include "EcuM_MemMap.h"
/* coverity[misra_c_2012_rule_8_6_violation:SUPPRESS] */
extern FUNC(void, ECUM_CODE) print_multicore_startup_dot(void);
/* coverity[misra_c_2012_rule_8_6_violation:SUPPRESS] */
extern FUNC(void, ECUM_CODE) print_master_startup_dot(void);

/* Local Macro Definition */
#if (VCOS_MODULE_CONFIG_SHELL == 1u)
static FUNC(void, ECUM_CODE) totle_startuptime(void)
{
#if ((CONFIG_RT_FRAMEWORK == 1) && (EcuM_INITPHASEDOT == 1))
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_STARTUP2_TIME,
		       ecum_calculate_time(ecum_rte_end_time_value, ecum_startup_two_start_time));
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_INITZERO_TIME,
		       ecum_calculate_time(ecum_init_zero_end_time, ecum_init_start_time));
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_INITONE_TIME,
		       ecum_calculate_time(ecum_init_one_end_time, ecum_init_one_start_time));
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_INITTWO_TIME,
		       ecum_calculate_time(ecum_init_two_end_time, ecum_init_two_start_time));
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_INITTHREE_TIME,
		       ecum_calculate_time(ecum_init_three_end_time, ecum_init_three_start_time));
	(void)vcos_log(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_RTESYNC_TIME,
		       ecum_calculate_time(ecum_rte_sync_time_value, ecum_init_three_end_time));
#endif
}

/* coverity[misra_c_2012_rule_2_7_violation:SUPPRESS] */
/* coverity[misra_c_2012_rule_4_6_violation:SUPPRESS] */
/* coverity[misra_c_2012_rule_8_13_violation:SUPPRESS] */
/* coverity[misra_c_2012_rule_8_4_violation:SUPPRESS] */
FUNC(int, ECUM_CODE) do_startuptime(struct cmd_tbl_s *cmdtp, UINT32Type_t flag, UINT32Type_t argc, char *const argv[])
{
#if (CONFIG_RT_FRAMEWORK == 1)
	if (argc < 2u) {
		totle_startuptime();
	} else {
		/* coverity[misra_c_2012_rule_5_3_violation:SUPPRESS] */
		/* coverity[misra_c_2012_rule_21_7_violation:SUPPRESS] */
		// coverity[cert_int31_c_violation : SUPPRESS]
		// coverity[cert_err34_c_violation : SUPPRESS]
		uint8 case_flag = (uint8)atoi(argv[1]);

		switch (case_flag) {
		case 1u:
			print_master_startup_dot();
			break;
		case 2u:
#if (ECUM_NUMBER_OF_CORES > 1)
			print_multicore_startup_dot();
#endif
			break;
		default:
			totle_startuptime();
			break;
		}
	}

#endif
	return 0;
}

/* coverity[misra_c_2012_rule_17_12_violation:SUPPRESS] */
/* coverity[misra_c_2012_rule_2_8_violation:SUPPRESS] */
// coverity[cert_str30_c_violation : SUPPRESS]
SHELL_CMD_REGISTER(startuptime, CONFIG_SYS_MAXARGS, 2, do_startuptime, "StartUp time.", "\n");
#endif
#define ECUM_STOP_SEC_CODE
#include "EcuM_MemMap.h"