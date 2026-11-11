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
#include "sysm_init.h"
#if (CONFIG_RT_FRAMEWORK == 1)
#include "Os.h"
#include "rtfw_atomic.h"
#endif

#include "sysm_init_generated_types.h"
#include "sysm_init_internals.h"
#include "SchM_EcuM.h"

#if (VCOS_MODULE_CONFIG_MCU == 1)
#include "Mcu.h"
#endif
#if (CONFIG_MODULE_COM == 1)
#include "comstack_types.h"
#endif

#if (CONFIG_SERVM_BSWM == 1)
#include "bswm_bsw.h"
#endif
#if (CONFIG_MODULE_COMM == 1)
#include "comm_ecum.h"
#endif
#if (CONFIG_MODULE_DET == 1)
#include "Det.h"
#endif
#if (CONFIG_MODULE_DEM == 1)
#include "dem.h"
#endif
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#endif
#if (VCOS_MODULE_CONFIG_RTE == 1)
#include "Rte_Main.h"
#endif

#include "sysm_init_vcos_log.h"
#include "vcos_log.h"

#ifndef MIN
#define MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x, _y) (((_x) > (_y)) ? (_x) : (_y))
#endif

#define ECUM_COMBINEDWAKEUPS (ecum_get_pending_wakeup_events() | ecum_get_validated_wakeup_events())
extern void bswm_ecum_current_wakeup(ecum_wakeup_source_t source, ecum_wakeup_status_t state);
extern void bswm_process_immediate_request_on_init(void);
extern void bswm_ecum_current_state(ecum_state_t current_state);

/* Local Variable Definition */
#define ECUM_START_SEC_SYSTEMAPPLICATION_CORE0_SEMISHARED_DATA
#include "ecum_memmap.h"
#if defined(VCOS_MODULE_CONFIG_RTE) && (VCOS_MODULE_CONFIG_RTE == 1)
static VAR(uint8, ECUM_SYSTEMAPPLICATION_CORE0_SEMISHARED_DATA) ecum_current_mode;
#endif
static VAR(uint32,
	   ECUM_SYSTEMAPPLICATION_CORE0_SEMISHARED_DATA) pending_wkup_mask; // 待验证的唤醒源
#define ECUM_STOP_SEC_SYSTEMAPPLICATION_CORE0_SEMISHARED_DATA
#include "ecum_memmap.h"

/* Global Variable Definition */
#define ECUM_START_SEC_MULTI_APP_BSS
#include "ecum_memmap.h"
VAR(ecum_global_t, ECUM_MULTI_APP_BSS) ecum_world;
/*!*********************************************************************************************************************
 * @brief  启动各阶段时间戳定义
 **********************************************************************************************************************/
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_startup_two_start_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_bswm_end_time_value;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_rte_end_time_value;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_rte_sync_time_value;
/*!*********************************************************************************************************************
 * @brief  EcuM各个初始化阶段起始时间点
 **********************************************************************************************************************/
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_start_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_one_start_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_two_start_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_three_start_time;
/*!*********************************************************************************************************************
 * @brief  EcuM各个初始化阶段结束时间点
 **********************************************************************************************************************/
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_zero_end_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_one_end_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_two_end_time;
VAR(uint32, ECUM_MULTI_APP_BSS) ecum_init_three_end_time;
#define ECUM_STOP_SEC_MULTI_APP_BSS
#include "ecum_memmap.h"

#define ECUM_START_SEC_CODE
#include "ecum_memmap.h"
/* Local Fuction Implementation */
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
static FUNC(boolean, ECUM_CODE) ecum_validate_wakeup_source(ecum_wakeup_source_t sources, uint8 function_id)
{
	boolean res = TRUE;
	ecum_wakeup_source_t wk_source;
	/* Get user defined sources */
	wk_source = ecum_world.config->ecum_sleep_mode_config[ecum_world.sleep_mode].ecum_wakeup_source_mask;
	/* Predefined source does not require validation */
	wk_source |= VALIED_WAKEUP_SOURCES;

	if (!((sources | wk_source) == wk_source)) {
		ecum_det_report_error(ecum_get_errorid(function_id, ECUM_E_UNKNOWN_WAKEUP_SOURCE));
		res = FALSE;
	}
	return res;
}
#endif

static FUNC(boolean, ECUM_CODE) check_pb_configuration(const ecum_config_t *config)
{
	if (config == NULL) {
		return FALSE;
	}

	if (config->post_build_config == NULL) {
		return FALSE;
	}

	if ((uint32)(0x5A5A5A5A) != config->post_build_config->start_pattern) {
		return FALSE;
	}

	if (config->ecum_post_build_variant != config->post_build_config->post_build_variant) {
		return FALSE;
	}

	if ((config->ecum_config_consistency_hash_low != config->post_build_config->pre_compile_hash_low) ||
	    (config->ecum_config_consistency_hash_high != config->post_build_config->pre_compile_hash_high)) {
		return FALSE;
	}

	return TRUE;
}

static FUNC(void, ECUM_CODE) go_sleep_sequence(void)
{
	uint32 c_mask;
	uint32 source;
	const ecum_sleep_mode_t *sleep_mode_ptr;

	/* Get the current sleep mode */
	sleep_mode_ptr = &ecum_world.config->ecum_sleep_mode_config[ecum_world.sleep_mode];
	c_mask = sleep_mode_ptr->ecum_wakeup_source_mask;

	/* Loop over the WKSOURCE for this sleep mode */
	for (uint8 i = 0u; i < 32u; i++) {
		if ((c_mask & 1u) != 0u) {
			source = ((uint32)1u) << i;
#if (CONFIG_SERVM_BSWM == 1)
			bswm_ecum_current_wakeup((ecum_wakeup_source_t)source, ECUM_WKSTATUS_NONE);
#endif
			ecum_enable_wakeup_sources((ecum_wakeup_source_t)source);
			(void)VCOS_LOG(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_WAKEUPSOURCE_ID, source);
		}
		c_mask >>= 1u;
	}
}

static FUNC(void, ECUM_CODE) halt_sequence(void)
{
	const ecum_sleep_mode_t *sleep_mode_ptr;

	/* coverity[misra_c_2012_rule_2_2_violation:SUPPRESS] */
	ecum_generate_ram_hash();

	do {
		DisableAllInterrupts();

		sleep_mode_ptr = &ecum_world.config->ecum_sleep_mode_config[ecum_world.sleep_mode];
		(void)sleep_mode_ptr;
#if 0
	Mcu_SetMode(sleep_mode_ptr->ecum_sleep_mode_mcu_mode);
#endif
		EnableAllInterrupts();
	} while (ECUM_COMBINEDWAKEUPS == ECUM_WKSOURCE_NONE);
	if (ecum_check_ram_hash() == 0u) {
#if (CONFIG_MODULE_DEM == 1)
		ecum_error_hook(ecum_world.config->ecum_dem_ram_check_failed_event_id);
#endif
	}
}

static FUNC(void, ECUM_CODE) poll_sequence(void)
{
	uint8_least loop_count;
	const ecum_sleep_mode_t *sleep_mode_ptr;

	ecum_wakeup_source_t ecum_wakeup_source_list;

	DisableAllInterrupts();
	sleep_mode_ptr = &ecum_world.config->ecum_sleep_mode_config[ecum_world.sleep_mode];
	ecum_wakeup_source_list = sleep_mode_ptr->ecum_wakeup_source_mask;
#if 0
Mcu_SetMode(sleep_mode_ptr->ecum_sleep_mode_mcu_mode);
#endif
	EnableAllInterrupts();

	while (ECUM_COMBINEDWAKEUPS == ECUM_WKSOURCE_NONE) {
		/* Call of EcuM_SleepActivity to control the polling sleep phase. */
		ecum_sleep_activity();

		for (loop_count = 0u; loop_count < (uint32)EcuM_SizeOfWakeupSourceList; loop_count++) {
			if ((((ecum_wakeup_source_t)1u << (ecum_wakeup_source_t)loop_count) &
			     (ecum_wakeup_source_list)) != 0u) {
				ecum_check_wakeup(((ecum_wakeup_source_t)1u << (ecum_wakeup_source_t)loop_count));
			} else {
				/* do nothing */
			}
		}
	}
}

static FUNC(void, ECUM_CODE) wakeup_restart_sequene(void)
{
	ecum_wakeup_source_t w_mask = 0;

	DisableAllInterrupts();
#if 0
Mcu_SetMode(ecum_world.config->ecum_normal_mcu_mode);
#endif
	EnableAllInterrupts();
	/* Assigning 0 as default value for w_mask */
	w_mask = ecum_get_pending_wakeup_events();

	ecum_disable_wakeup_sources(w_mask);

	ecum_al_driver_restart(ecum_world.config);

	if (ecum_get_validated_wakeup_events() != 0u) {
		set_current_state(ECUM_STATE_WAKEUP_REACTION);
	} else {
		ecum_set_validation_timer();
		set_current_state(ECUM_STATE_WAKEUP_VALIDATION);
	}
}

#if (ECUM_NUMBER_OF_CORES > 1)
static inline FUNC(void, ECUM_CODE) go_off_on_slave(uint8 core_id)
{
#if (VCOS_MODULE_CONFIG_RTE == 1)
	(void)Rte_Stop();
	SchM_Deinit();
#endif
	SchM_Enter_ECUM_EXCLUSIVE_AREA_1();
	ecum_set_core_status_value(core_id, ECUM_CORESTATUS_OFF);
	SchM_Exit_ECUM_EXCLUSIVE_AREA_1();
}

static inline FUNC(void, ECUM_CODE) go_to_sleep_on_slave(uint8 slave_core)
{
	uint8 status;

	SchM_Enter_ECUM_EXCLUSIVE_AREA_1();
	ecum_set_core_status_value(slave_core, ECUM_CORESTATUS_SLEEP);
	SchM_Exit_ECUM_EXCLUSIVE_AREA_1();

	DisableAllInterrupts();
	for (;;) {
		EnableAllInterrupts();
		/*sleep */
		DisableAllInterrupts();
		if ((ecum_wakeup_source_t)ECUM_WKSOURCE_NONE != ECUM_COMBINEDWAKEUPS) {
			break;
		}
	}
	EnableAllInterrupts();

	for (;;) {
		SchM_Enter_ECUM_EXCLUSIVE_AREA_1();
		status = ecum_get_core_status_value(ECUM_CORE_ID_BSW);
		SchM_Exit_ECUM_EXCLUSIVE_AREA_1();

		if (status == ECUM_CORESTATUS_RUN) {
			break;
		}
	}

	ecum_set_core_status_value(slave_core, ECUM_CORESTATUS_RUN);
}
#endif

#if ((CONFIG_RT_FRAMEWORK == 1u) && (EcuM_INITPHASEDOT == 1u))
FUNC(float32, ECUM_CODE) ecum_calculate_time(uint32 ecum_t1, uint32 ecum_t0)
{
	/* coverity[misra_c_2012_rule_10_8_violation:SUPPRESS] */
	// coverity[cert_flp36_c_violation : SUPPRESS]
	float32 timediff = (1000.0f * (float32)(ecum_t1 - ecum_t0) / (float32)up_timer_freq());
	return timediff;
}
#endif

FUNC(void, ECUM_CODE) platform_init(void)
{
	uint32_t reset_reason;
	boolean ret = TRUE;

	(void)reset_reason;
#if (ECUM_NUMBER_OF_CORES > 1u)
	uint32 core_id = (uint32)GetCoreID();

	if (core_id == ECUM_CORE_ID_BSW) {
#endif
		Std_ReturnType status;

		set_current_state(ECUM_STATE_STARTUP_ONE);
		/* Initialize drivers that are needed to determine PostBuild conf */
		ecum_al_driver_init_zero();

		/* Determine PostBuild configuration */
		ecum_world.config = ecum_determine_pb_configuration();

		if (check_pb_configuration(ecum_world.config) == FALSE) {
#if (CONFIG_MODULE_DEM == 1)
			ecum_error_hook(ecum_world.config->ecum_dem_inconsistency_event_id);
#endif
			ret = FALSE;
		}

		if (ret == TRUE) {
			/* Initialize drivers needed before the OS-starts */
			ecum_al_driver_init_one(ecum_world.config);
#if (VCOS_MODULE_CONFIG_MCU == 1)
			reset_reason = Mcu_GetResetReason();
			uint8 ecum_init_flag = 0;
			/* Determine the reset/wakeup reason */
#if (ECUM_MCURESETREFNUM != 0)
			for (uint32 i = 0; i < ECUM_MCURESETREFNUM; i++) {
				ecum_reset_reason_t reset_reason_config;

				reset_reason_config = ecum_world.config->ecum_reset_reason_config[i];
				if (reset_reason == reset_reason_config.reset_reason) {
					ecum_validate_wakeup_event(ereset_reason_config.wkup_source);
					ecum_init_flag = 1;
					break;
				} else {
					;
				}
			}
#endif
			if (ecum_init_flag == 0u) {
				ecum_remember_wakeup_event((uint32)reset_reason);
			} else {
				;
			}
#endif
			ecum_world.initiated = TRUE;

			/* Set default shutdown target */
			status = ecum_select_shutdown_target(ecum_world.config->ecum_default_shutdown_target,
							     ecum_world.config->ecum_default_sleep_mode);
			if (status == E_OK) {
				ecum_world.run_requests = 0;
				ecum_world.postrun_requests = 0;
				ecum_world.kill_all_request = FALSE; /* Enable run request again */
				ecum_world.kill_all_postrun_request = FALSE; /* Enable post_run request again */
#if (ECUM_RESET_LOOP_DETECTION == STD_ON)
				ecum_loop_detection();
#endif
			}
		}
#if (ECUM_NUMBER_OF_CORES > 1)
	} else {
		/* 从核初始化的模块 */
		ecum_al_driver_init_zero();
	}
	ecum_set_core_status_value(core_id, ECUM_CORESTATUS_RUN);
#else
	ecum_set_core_status_value(ECUM_CORE_ID_BSW, ECUM_CORESTATUS_RUN);
#endif

	ecum_start_os(ECUM_DEFAULT_APPMODE);
}

FUNC(void, ECUM_CODE) system_init(void)
{
#if (ECUM_NUMBER_OF_CORES > 1u)
	uint8 core_id = (uint8)GetCoreID();

	if (core_id == (uint8)ECUM_CORE_ID_BSW) {
#endif
		// coverity[cert_int31_c_violation : SUPPRESS]
		ecum_get_timmertick(ecum_startup_two_start_time);

#if (CONFIG_SERVM_BSWM == 1)
		bswm_init(ecum_world.config->post_build_config->bswm_config_ptr);
#endif
		// coverity[cert_int31_c_violation : SUPPRESS]
		ecum_get_timmertick(ecum_bswm_end_time_value);
		if (ecum_world.current_state == ECUM_STATE_STARTUP_ONE) {
			set_current_state(ECUM_STATE_STARTUP_TWO);
		}

		if (ecum_world.current_state == ECUM_STATE_STARTUP_TWO) {
#if ((CONFIG_SERVM_BSWM == 1) && (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0))
			bswm_process_immediate_request_on_init(); /* 模块初始化和EcuM状态机仲裁*/
#endif
		}
#if (ECUM_NUMBER_OF_CORES > 1)
	}
#endif
	ecum_world.initiated = TRUE;
	ecum_al_driver_init_multi_core();
}

FUNC(Std_ReturnType, ECUM_CODE)
ecum_select_shutdown_target(ecum_shutdown_target_t target, ecum_shutdown_mode_t sleep_mode)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_SELECTSHUTDOWNTARGET_ID_ECUM_E_UNINIT, E_NOT_OK);

	switch (target) {
	case ECUM_SHUTDOWN_TARGET_OFF:
	case ECUM_SHUTDOWN_TARGET_RESET:
	case ECUM_SHUTDOWN_TARGET_SLEEP:
		ecum_world.shutdown_target = target;
		ecum_world.sleep_mode = sleep_mode;
		break;
	default:
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
		(void)det_report_error_with_info(ECUM_ECUM_SELECTSHUTDOWNTARGET_ID_ECUM_E_STATE_PAR_OUT_OF_RANGE,
						 NULL_PTR, 0);
#endif
		return E_NOT_OK;
	}

	return E_OK;
}

FUNC(Std_ReturnType, ECUM_CODE)
ecum_get_shutdown_target(ecum_shutdown_target_t *target, ecum_shutdown_mode_t *sleep_mode)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_GETSHUTDOWNTARGET_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(target != NULL, ECUM_ECUM_GETSHUTDOWNTARGET_ID_ECUM_E_NULL_POINTER, E_NOT_OK);
	ecum_validate_rv(sleep_mode != NULL, ECUM_ECUM_GETSHUTDOWNTARGET_ID_ECUM_E_NULL_POINTER, E_NOT_OK);

	*target = ecum_world.shutdown_target;
	*sleep_mode = ecum_world.sleep_mode;

	return E_OK;
}

FUNC(ecum_wakeup_source_t, ECUM_CODE) ecum_get_validated_wakeup_events(void)
{
	return ecum_world.valid_events;
}

FUNC(ecum_wakeup_source_t, ECUM_CODE) ecum_get_pending_wakeup_events(void)
{
	return ecum_world.wakeup_events;
}

FUNC(void, ECUM_CODE) ecum_set_wakeup_event(ecum_wakeup_source_t sources)
{
	ecum_wakeup_source_t wk_source;
	ecum_wakeup_source_t new_sources;

	new_sources = sources;

	wk_source = VALIED_WAKEUP_SOURCES; /*Validate at once for pre-defined sources */

	wk_source &= new_sources; /* Sources that should be validated at once */
	if (wk_source != 0u) {
		ecum_validate_wakeup_event(wk_source); /* Validate pre-defined source at once */
	}

	new_sources ^= wk_source; /* Don't add pre-defined source to PENDING */

	if (new_sources != 0u) {
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
		if (ecum_validate_wakeup_source(new_sources, ECUM_SETWAKEUPEVENT_ID) == FALSE) {
			return;
		}
#endif
		/* coverity[misra_c_2012_rule_11_3_violation:SUPPRESS] */
		(void)Os_AtomicOr((Atomic *)&ecum_world.wakeup_events, (sint32)new_sources);
#if (CONFIG_SERVM_BSWM == 1)
		bswm_ecum_current_wakeup(new_sources, ECUM_WKSTATUS_PENDING);
#endif
	}
	ecum_world.validation_timer =
		ECUM_VALIDATION_TIMEOUT / ECUM_MAIN_FUNCTION_PERIOD; // Allow zero value as left parameter for
	// configuration
}

FUNC(void, ECUM_CODE) ecum_clear_wakeup_event(ecum_wakeup_source_t source)
{
	/* coverity[misra_c_2012_rule_11_3_violation:SUPPRESS] */
	/* coverity[misra_c_2012_rule_10_8_violation:SUPPRESS] */
	// coverity[cert_int31_c_violation : SUPPRESS]
	(void)Os_AtomicAnd((Atomic *)&ecum_world.wakeup_events, (sint32)(~(uint32)source)); /* Clear pending events */
	/* coverity[misra_c_2012_rule_11_3_violation:SUPPRESS] */
	/* coverity[misra_c_2012_rule_10_8_violation:SUPPRESS] */
	// coverity[cert_int31_c_violation : SUPPRESS]
	(void)Os_AtomicAnd((Atomic *)&ecum_world.valid_events, (sint32)(~(uint32)source)); /* Clear validated events */
}

FUNC(void, ECUM_CODE) ecum_validate_wakeup_event(ecum_wakeup_source_t sources)
{
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
	if (ecum_validate_wakeup_source(sources, ECUM_VALIDATE_WAKEUP_EVENT_ID) == FALSE) {
		return;
	}
#endif
	/* coverity[misra_c_2012_rule_11_3_violation:SUPPRESS] */
	(void)Os_AtomicOr((Atomic *)&ecum_world.valid_events, (sint32)sources);
#if (CONFIG_MODULE_COMM == 1)
	const ecum_wakeup_source_config_t *wkup_cfg_ptr;

	for (uint32 i = 0; i < (uint32)ECUM_WKSOURCE_USER_CNT; i++) {
		wkup_cfg_ptr = &ecum_world.config->ecum_wakeup_source_config[ecum_wk_source_id(i)];
		if (((sources & wkup_cfg_ptr->ecum_wakeup_source_id) != 0u) &&
		    (wkup_cfg_ptr->ecum_comm_channel != ECUM_COMM_CHANNEL_ILL)) {
			comm_ecum_wake_up_indication(wkup_cfg_ptr->ecum_comm_channel);
		}
	}
#endif

#if (CONFIG_SERVM_BSWM == 1)
	if (ecum_world.current_state > ECUM_STATE_STARTUP_TWO) {
		bswm_ecum_current_wakeup(sources, ECUM_WKSTATUS_VALIDATED);
	}
#endif
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_go_down(uint16 caller)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_GODOWN_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_clear_wakeup_event(ECUM_WKSOURCE_ALL_SOURCES);
	Std_ReturnType rv = E_OK;

	ecum_on_go_off_one();

#if (ECUM_NUMBER_OF_CORES > 1u)
	ecum_set_core_status_value(ECUM_CORE_ID_BSW, ECUM_CORESTATUS_OFF);
	ecum_shutdown_synchronization(ECUM_CORESTATUS_OFF);
#endif

	ecum_wakeup_source_t wakeup_events;

	wakeup_events = ecum_get_pending_wakeup_events();

	if (wakeup_events != 0u) {
		rv = ecum_select_shutdown_target(ECUM_SHUTDOWN_TARGET_RESET, Conf_ECUM_RESET_MCU);
	}

	ecum_shutdown_os(E_OK);

	return rv;
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_go_halt(void)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_GOHALT_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_clear_wakeup_event(ECUM_WKSOURCE_ALL_SOURCES);
	go_sleep_sequence();
	halt_sequence();
	wakeup_restart_sequene();
	return E_OK;
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_go_poll(void)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_GOPOLL_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_clear_wakeup_event(ECUM_WKSOURCE_ALL_SOURCES);
	go_sleep_sequence();
	poll_sequence();
	wakeup_restart_sequene();
	return E_OK;
}

/*
 * Typically called from OS shutdown hook
 */
FUNC(void, ECUM_CODE) ecum_shutdown(void)
{
	set_current_state(ECUM_STATE_GO_OFF_TWO);

	ecum_on_go_off_two(); /* Let the last drivers do a nice shutdown */

	if (ecum_world.shutdown_target == ECUM_SHUTDOWN_TARGET_OFF) {
		ecum_al_switch_off();
	} else {
		uint8 ResetMode = ecum_world.sleep_mode;

		ecum_al_reset(ResetMode);
	}
}

static inline FUNC(void, ECUM_CODE) ecum_swich_rtemode(ecum_state_t state)
{
#if (VCOS_MODULE_CONFIG_RTE == 1)
	uint8 newMode = ecum_current_mode;

	switch (state) {
	case ECUM_STATE_WAKEUP_ONE:
	case ECUM_STATE_WAKEUP_VALIDATION:
	case ECUM_STATE_WAKEUP_REACTION:
	case ECUM_STATE_WAKEUP_TWO:
	case ECUM_STATE_GO_SLEEP:
	case ECUM_STATE_SLEEP:
		newMode = RTE_MODE_EcuM_Mode_SLEEP;
		break;
	case ECUM_STATE_GO_OFF_ONE:
	case ECUM_STATE_GO_OFF_TWO:
		newMode = RTE_MODE_EcuM_Mode_SHUTDOWN;
		break;
	case ECUM_STATE_PREP_SHUTDOWN:
	case ECUM_STATE_APP_POST_RUN:
		newMode = RTE_MODE_EcuM_Mode_POST_RUN;
		break;
	case ECUM_STATE_STARTUP_TWO:
		newMode = RTE_MODE_EcuM_Mode_STARTUP;
		break;
	case ECUM_STATE_APP_RUN:
		newMode = RTE_MODE_EcuM_Mode_RUN;
		break;
	default:
		/*other state*/
		break;
	}

	if (newMode != ecum_current_mode) {
		ecum_current_mode = newMode;
		(void)Rte_Switch_currentMode_currentMode(ecum_current_mode);
	}
#endif
}

FUNC(void, ECUM_CODE) set_current_state(ecum_state_t state)
{
	ecum_world.current_state = state;
	ecum_swich_rtemode(state);
#if (CONFIG_SERVM_BSWM == 1)
	if (ECUM_STATE_STARTUP_ONE != state) {
		bswm_ecum_current_state(state);
	}
#endif
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_get_state(ecum_state_t *state)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_GETSTATE_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(state != NULL, ECUM_ECUM_GETSTATE_ID_ECUM_E_NULL_POINTER, E_NOT_OK);

	*state = ecum_world.current_state;

	return E_OK;
}

static inline void set_req_flag(uint32 *flags, uint8 pos)
{
	SchM_Enter_ECUM_EXCLUSIVE_AREA_0();
	*flags |= (1UL << pos);
	SchM_Exit_ECUM_EXCLUSIVE_AREA_0();
}

static inline void clear_req_flag(uint32 *flags, uint8 pos)
{
	SchM_Enter_ECUM_EXCLUSIVE_AREA_0();
	*flags &= ~(1UL << pos);
	SchM_Exit_ECUM_EXCLUSIVE_AREA_0();
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_request_run(ecum_user_t user)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_REQUESTRUN_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(user < ECUM_USER_ENDMARK, ECUM_ECUM_REQUESTRUN_ID_ECUM_E_INVALID_PAR, E_NOT_OK);

	if (FALSE == ecum_world.kill_all_request) {
		set_req_flag(&ecum_world.run_requests, user);
	}

	return E_OK;
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_release_run(ecum_user_t user)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_RELEASERUN_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(user < ECUM_USER_ENDMARK, ECUM_ECUM_RELEASERUN_ID_ECUM_E_INVALID_PAR, E_NOT_OK);

	clear_req_flag(&ecum_world.run_requests, user);
	return E_OK;
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_request_post_run(ecum_user_t user)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_REQUESTPOSTRUN_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(user < ECUM_USER_ENDMARK, ECUM_ECUM_REQUESTPOSTRUN_ID_ECUM_E_INVALID_PAR, E_NOT_OK);

	if (FALSE == ecum_world.kill_all_postrun_request) {
		set_req_flag(&ecum_world.postrun_requests, user);
	}

	return E_OK;
}

FUNC(Std_ReturnType, ECUM_CODE) ecum_release_post_run(ecum_user_t user)
{
	ecum_validate_rv(ecum_world.initiated, ECUM_ECUM_RELEASEPOSTRUN_ID_ECUM_E_UNINIT, E_NOT_OK);
	ecum_validate_rv(user < ECUM_USER_ENDMARK, ECUM_ECUM_RELEASEPOSTRUN_ID_ECUM_E_INVALID_PAR, E_NOT_OK);

	clear_req_flag(&ecum_world.run_requests, user);
	return E_OK;
}

FUNC(ecum_wakeup_status_t, ECUM_CODE)
ecum_get_status_of_wakeup_source(ecum_wakeup_source_t sources)
{
	uint32 valid_events = ecum_get_validated_wakeup_events();
	uint32 pending_events = ecum_get_pending_wakeup_events();

	if ((valid_events & sources) != 0u) {
		return ECUM_WKSTATUS_VALIDATED;
	}
	if (pending_events & sources) {
		return ECUM_WKSTATUS_PENDING;
	}
	return ECUM_WKSTATUS_NONE;
}

#if (CONFIG_MODULE_COMM == 1)
FUNC(void, ECUM_CODE)
set_comm_communication_allowed(ecum_comm_communication_groups_t group, boolean Allowed)
{
	for (uint8 i = 0u; i < (uint8)ECUM_COMM_NETWORKS_CNT; i++) {
		const ecum_comm_config_t *comm_config_ptr = &ecum_world.config->ecum_comm_config[i];

		if ((NULL != ecum_world.config->ecum_comm_config) &&
		    ((ALL_COM == group) ||
		     ((ALL_WO_LIN == group) && (comm_config_ptr->ecum_com_bus_t != COMM_BUS_TYPE_LIN)) ||
		     ((ONLY_LIN == group) && (comm_config_ptr->ecum_com_bus_t == COMM_BUS_TYPE_LIN)))) {
			comm_communication_allowed(comm_config_ptr->ecum_comm_network_handle, Allowed);
		}
	}
}
#endif

FUNC(void, ECUM_CODE) ecum_enter_run_mode(void)
{
	/* Clear all wakeup events each time entering RUN */
	ecum_clear_wakeup_event(ECUM_WKSOURCE_ALL_SOURCES);

#if (CONFIG_MODULE_COMM == 1)
	set_comm_communication_allowed(ALL_COM, TRUE);
#endif

	set_current_state(ECUM_STATE_APP_RUN);
	(void)VCOS_LOG(ECUM_MODULE_ID, VCOS_LOG_INFO, ECUM_DLT_ENTER_RUN);
}

FUNC(boolean, ECUM_CODE) has_run_requests(void)
{
	return ((ecum_world.run_requests != 0u) ? TRUE : FALSE);
}

FUNC(boolean, ECUM_CODE) has_post_run_requests(void)
{
	return ((ecum_world.postrun_requests != 0u) ? TRUE : FALSE);
}

FUNC(void, ECUM_CODE) ecum_set_validation_timer(void)
{
	uint32 max_timeout = 0U;
	uint32 mask = ecum_get_pending_wakeup_events();
	uint32 idx;

	pending_wkup_mask = mask;
	ecum_start_wakeup_sources(mask);

	for (idx = 0U; idx < ECUM_WKSOURCE_USER_CNT; ++idx) {
		const ecum_wakeup_source_config_t *cfg =
			&ecum_world.config->ecum_wakeup_source_config[ecum_wk_source_id(idx)];

		if ((mask & cfg->ecum_wakeup_source_id) == 0U) {
			continue;
		}

		uint32 timeout = cfg->ecum_validation_timeout;

		if (timeout == ECUM_VALIDATION_TIMEOUT_ILL) {
			continue;
		}

		uint32 period_ticks = timeout / ECUM_MAIN_FUNCTION_PERIOD;

		if (period_ticks > max_timeout) {
			max_timeout = period_ticks;
		}
	}

	if (max_timeout > 0U) {
		ecum_world.validation_timer = max_timeout;
	} else {
		ecum_world.validation_timer = 1U;
	}
}

/*
 * Call ecum_check_validation() while all events have not been validated and
 * timeout have not expired. The call to ecum_check_validation(..) triggers a
 * call to ecum_validate_wakeup_event(..) from the driver when validated.
 */
FUNC(void, ECUM_CODE) ecum_internal_validation(void)
{
	const uint32 validated_wkup_events = ecum_get_validated_wakeup_events();
	const uint32 diff_mask = validated_wkup_events ^ pending_wkup_mask;

	if (ecum_world.validation_timer == 0u) {
		if (0u != diff_mask) {
			ecum_stop_wakeup_sources(diff_mask);

#if (CONFIG_SERVM_BSWM == 1)
			bswm_ecum_current_wakeup(diff_mask, ECUM_WKSTATUS_EXPIRED);
#endif
		}
		set_current_state(ECUM_STATE_WAKEUP_REACTION);
		return;
	} else {
		ecum_check_validation(diff_mask);

		if (0u != validated_wkup_events) {
			set_current_state(ECUM_STATE_WAKEUP_REACTION);
		}
	}

	if (ecum_world.validation_timer > 0u) {
		ecum_world.validation_timer--;
	}
}

FUNC(void, ECUM_CODE) ecum_mainfunction(void)
{
#if (ECUM_NUMBER_OF_CORES > 1u)
	uint8 core_id = (uint8)GetCoreID();

	if (core_id == (uint8)ECUM_CORE_ID_BSW) {
#endif
		if (TRUE == ecum_world.initiated) {
			if (ECUM_STATE_WAKEUP_VALIDATION == ecum_world.current_state) {
				ecum_internal_validation();
			}
		}

#if (ECUM_NUMBER_OF_CORES > 1)
	} else {
		/*slave core only perform the shutdown/sleep process */
		if (ecum_get_core_status_value(ECUM_CORE_ID_BSW) == ECUM_CORESTATUS_OFF) {
			go_off_on_slave(core_id);
		} else if (ecum_get_core_status_value(ECUM_CORE_ID_BSW) == ECUM_CORESTATUS_SLEEP) {
			go_to_sleep_on_slave(core_id);
		} else {
			/* do nothing */
		}
	}
#endif
}

#define ECUM_STOP_SEC_CODE
#include "ecum_memmap.h"