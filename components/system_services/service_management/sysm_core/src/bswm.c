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

#include "bswm.h"
#include "bswm_internal.h"
#include "string.h"
#include "bswm_bsw.h"

#if (CONFIG_MODULE_COMM == 1)
#include "comm.h"
#endif
#if (CONFIG_MODULE_PUDR == 1)
#include "pdur.h"
#endif
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#endif
#if (CONFIG_MODULE_CANSM == 1)
#include "cansm.h"
#endif
#if (CONFIG_MODULE_LINSM == 1)
#include "linsm.h"
#endif
#if (CONFIG_MODULE_ETHSM == 1)
#include "ethsm.h"
#endif
#if (CONFIG_MODULE_COM == 1)
#include "com.h"
#endif
#if (CONFIG_MODULE_LINIF == 1)
#include "linif.h"
#endif
#if (CONFIG_SERVM_ECUM == 1)
#include "sysm_init.h"
#endif
#if (CONFIG_MODULE_DCM == 1)
#include "dcm.h"
#endif
#if (CONFIG_MODULE_DET == 1)
#include "Det.h"
#endif
#if (CONFIG_MODULE_DEM == 1)
#include "dem.h"
#endif
#if (CONFIG_MODULE_NM == 1)
#include "nm.h"
#endif

#include "SchM_BswM.h"
#include "vcos_log.h"
#include "bswm_vcos_log.h"

#define BSWM_START_SEC_CODE
#include "bswm_memmap.h"
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
static FUNC(bswm_rule_state_t, BSWM_CODE)
	bswm_evaluate_logical_expression(const uint32 id, const logical_expression_func_t *logical_expressions_tbl)
{
	bswm_rule_state_t ret_val = BSWM_FALSE;
#if (BSWM_NUMBER_OF_RULES > 0)
	logical_expression_func_t logical_expression_func = logical_expressions_tbl[id];

	ret_val = (TRUE == logical_expression_func()) ? BSWM_TRUE : BSWM_FALSE;
#endif
	return ret_val;
}

static FUNC(sint32, BSWM_CODE) bswm_get_rule_action_id(const sint16 rule_index, const bswm_rule_t *ruleset,
						       bswm_rule_state_t *bswm_rule_states, bswm_rule_state_t state,
						       const bswm_action_list_t *action_lists)
{
	sint32 action_list_index = -1;
	sint32 action_id = -1;

	if (BSWM_TRUE == state) {
		action_id = ruleset->true_list_idx;
	} else {
		action_id = ruleset->false_list_idx;
	}

	if (action_id >= 0) {
		if ((BSWM_CONDITION == action_lists[action_id].exec_mode) ||
		    // coverity[misra_c_2012_rule_10_4_violation:SUPPRESS]
		    (state != bswm_rule_states[rule_index])) {
			action_list_index = action_id;
		}
	}
	// coverity[cert_str31_c_violation:SUPPRESS]
	// coverity[misra_c_2012_rule_10_3_violation:SUPPRESS]
	bswm_rule_states[rule_index] = state;
	return action_list_index;
}

static FUNC(sint32, BSWM_CODE) bswm_internal_evaluate_rule(const sint16 rule_index, const bswm_internal_t *bswm_ctx)
{
	sint32 action_list_index = -1;

	if (rule_index < 0) {
		return action_list_index;
	}
#if (BSWM_NUMBER_OF_RULES > 0)
	bswm_rule_state_t logical_result;
	const bswm_rule_t *ruleset = &(bswm_ctx->bswm_rules[rule_index]);
	const bswm_action_list_t *action_lists = &(bswm_ctx->bswm_config_ptr->action_list_data[0]);
	bswm_rule_state_t *rule_states = bswm_ctx->bswm_rule_states;

	logical_result =
		bswm_evaluate_logical_expression(ruleset->logic_expr_idx, bswm_ctx->bswm_logical_expressions);

	action_list_index = bswm_get_rule_action_id(rule_index, ruleset, rule_states, logical_result, action_lists);
#endif
	return action_list_index;
}

static FUNC(Std_ReturnType, BSWM_CODE)
	bswm_internal_execute_atomic_action(const bswm_action_list_item_t *item,
					    const atomic_action_func_t *bswm_atomic_actions)
{
#if (BSWM_NUMBER_OF_ACTIONLISTS > 0)
	atomic_action_func_t atomic_action_func = bswm_atomic_actions[item->atomic_idx];

	Std_ReturnType action_result = atomic_action_func(item);
#else
	Std_ReturnType action_result = E_NOT_OK;
#endif

#if (CONFIG_MODULE_DEM == 1)
	if (item->dem_fail_report_ref >= 0) {
		// coverity[misra_c_2012_rule_17_7_violation:SUPPRESS]
		// coverity[misra_c_2012_directive_4_7_violation:SUPPRESS]
		bswm_report_error_status(item->dem_fail_report_ref, (E_NOT_OK == action_result) ?
										  DEM_EVENT_STATUS_FAILED :
											DEM_EVENT_STATUS_PASSED);
	}
#endif
	return ((TRUE == item->stop_if_fail) && (E_NOT_OK == action_result)) ? E_NOT_OK : E_OK;
}

static FUNC(boolean, BSWM_CODE)
	bswm_push_list_state(bswm_execute_stack_t *state_stack, sint32 curr_list_index, uint32 curr_item_index)
{
	if (state_stack->sp == BSWM_STACK_MAX) {
		return FALSE;
	}

	state_stack->action_states[state_stack->sp].item_id = curr_item_index;
	state_stack->action_states[state_stack->sp].list_id = curr_list_index;
	state_stack->sp++;

	return TRUE;
}

static FUNC(boolean, BSWM_CODE)
	bswm_pop_list_state(bswm_execute_stack_t *state_stack, sint32 *curr_list_index, uint32 *curr_item_index)
{
	if (state_stack->sp == 0u) {
		return FALSE;
	}

	state_stack->sp--;
	*curr_item_index = state_stack->action_states[state_stack->sp].item_id;
	*curr_list_index = state_stack->action_states[state_stack->sp].list_id;

	return TRUE;
}

static FUNC(void, BSWM_CODE) bswm_internal_iterate_action_list(const sint32 entry_list_index, bswm_internal_t *bswm_ctx)
{
	const bswm_action_list_t *action_lists = &(bswm_ctx->bswm_config_ptr->action_list_data[0]);
	sint32 new_list_index = 0;
	uint32 curr_item_index = 0;
	sint32 curr_list_index = entry_list_index;
	bswm_execute_stack_t list_state = { 0 };

	/* Traverse all action lists in depth‑first order.*/
	do {
		Std_ReturnType should_abort = E_OK;

		for (; ((E_OK == should_abort) && (curr_item_index < action_lists[curr_list_index].items_count));
		     curr_item_index++) {
			const bswm_action_list_item_t *action;

			action = action_lists[curr_list_index].items[curr_item_index];
			switch (action->action_t) {
			case BSWM_ATOMIC:
				should_abort =
					bswm_internal_execute_atomic_action(action, bswm_ctx->bswm_atomic_actions);
				break;
			case BSWM_LIST:
				if (TRUE == bswm_push_list_state(&list_state, curr_list_index, curr_item_index)) {
					curr_list_index = action->list_idx_ref;
					curr_item_index = 0;
				}
				break;
			case BSWM_RULE:
				new_list_index =
					bswm_internal_evaluate_rule(action->rule_idx_ref, bswm_ctx);
				if ((new_list_index >= 0) &&
				    (TRUE == bswm_push_list_state(&list_state, curr_list_index, curr_item_index))) {
					curr_list_index = new_list_index;
					curr_item_index = 0;
				}
				break;
			default:
				//action type unknow
				break;
			}
		}
		/* No more items in this list or it got aborted */
		// coverity[cert_arr30_c_violation:SUPPRESS]
	} while (TRUE == bswm_pop_list_state(&list_state, &curr_list_index, &curr_item_index));
}
#endif

#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
static FUNC(void, BSWM_CODE)
	bswm_internal_evaluate_rules_for_immediate_request(const uint32 index_start, const uint32 index_end,
							   bswm_internal_t *bswm_ctx)
{
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
	sint16 rule_index;
	sint32 action_list_index;

	for (uint32 i = index_start; i <= index_end; i++) {
		const sint16 *mode_request_rule_list = &(bswm_ctx->bswm_rule_config_ptr->immediate_rule_set[i]);

		rule_index = *mode_request_rule_list;
		action_list_index = bswm_internal_evaluate_rule(rule_index, bswm_ctx);

		if (action_list_index >= 0) {
			bswm_internal_iterate_action_list(action_list_index, bswm_ctx);
		}
	}
#endif
}

static FUNC(void, BSWM_CODE) bswm_internal_process_delayed_requests(bswm_internal_t *bswm_ctx)
{
	for (uint32 mode_req_index = 0; mode_req_index < BSWM_NUMBER_OF_IMMEDIATE_REQUESTS; mode_req_index++) {
		if (bswm_ctx->bswm_delayed_request[mode_req_index] == TRUE) {
			bswm_internal_evaluate_rules_for_immediate_request(mode_req_index, mode_req_index, bswm_ctx);
			bswm_ctx->bswm_delayed_request[mode_req_index] = FALSE;
		}
	}
}
#endif

static inline FUNC(boolean, BSWM_CODE) bswm_try_process_request(bswm_internal_t *bswm_ctx)
{
	if (bswm_ctx->bswm_request_processing_ongoing == FALSE) {
		bswm_ctx->bswm_request_processing_ongoing = TRUE;
		return TRUE;
	}
	return FALSE;
}

static inline FUNC(void, BSWM_CODE) bswm_release_process_request(bswm_internal_t *bswm_ctx)
{
	bswm_ctx->bswm_request_processing_ongoing = FALSE;
}

#if (BSWM_TIMERS_NUM > 0u)
static void bswm_decrease_timer(const bswm_internal_t *bswm_ctx)
{
	uint8_least timer_i;

	bswm_array_index timer_index = bswm_ctx->timer_index;

	for (timer_i = timer_index.begin; timer_i < timer_index.end; timer_i++) {
		if (bswm_get_timer_value(timer_i) > 0u) {
			bswm_get_timer_value(timer_i)--;
			if (bswm_get_timer_value(timer_i) == 0u) {
				bswm_set_timer_state(timer_i, BSWM_TIMER_EXPIRED);
			}
		}
	}
}
#endif

static inline FUNC(void, BSWM_CODE) handle_deadline_monitoring(bswm_internal_t *bswm_ctx)
{
#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
	if (FASLE == bswm_ctx->bswm_dm_control_performed) {
		return;
	}
	com_reception_dm_control(bswm_ctx->bswm_dm_control_group_vector);
	bswm_ctx->bswm_dm_control_performed = FALSE;
#endif
}

static inline FUNC(void, BSWM_CODE) handle_pdu_group_switch(bswm_internal_t *bswm_ctx)
{
#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
	if (FALSE == bswm_ctx->bswm_pdu_group_switch_action_performed) {
		return;
	}
	com_ipdu_group_control(bswm_ctx->bswm_com_ipdu_group_vector, bswm_ctx->bswm_com_ipdu_initialize);
	bswm_ctx->bswm_pdu_group_switch_action_performed = FALSE;
#endif
}

static inline FUNC(void, BSWM_CODE) process_lock_and_requests(bswm_internal_t *bswm_ctx, boolean lock_request)
{
	if (FALSE == lock_request) {
		return;
	}
#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
	bswm_internal_process_delayed_requests(bswm_ctx);
#endif
	bswm_release_process_request(bswm_ctx);
}

static FUNC(void, BSWM_CODE) bswm_com_common_action(boolean lock_request, bswm_internal_t *bswm_ctx)
{
	handle_pdu_group_switch(bswm_ctx);
	handle_deadline_monitoring(bswm_ctx);
	process_lock_and_requests(bswm_ctx, lock_request);
}

static FUNC(void, BSWM_CODE) enter_critical(bswm_internal_t *bswm_ctx, boolean *lock_request)
{
	SchM_Enter_BSWM_EXCLUSIVE_AREA_0();
	*lock_request = bswm_try_process_request(bswm_ctx);
#if (BSWM_TIMERS_NUM > 0u)
	bswm_decrease_timer(bswm_ctx);
#endif
	SchM_Exit_BSWM_EXCLUSIVE_AREA_0();
}

static FUNC(void, BSWM_CODE) process_rules_and_lists(bswm_internal_t *bswm_ctx)
{
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
	sint16 rule_index;
	uint32 cnt = bswm_ctx->bswm_rule_config_ptr->rules_count;

	for (uint32 i = 0; i < cnt; i++) {
		rule_index = bswm_ctx->bswm_rule_config_ptr->deferred_rule_set[i];

		if (rule_index > 0) {
			sint32 action_list_index = bswm_internal_evaluate_rule(rule_index, bswm_ctx);

			if (action_list_index < 0) {
				continue;
			}
			bswm_internal_iterate_action_list(action_list_index, bswm_ctx);
		}
	}
#endif
}

FUNC(void, BSWM_CODE) bswm_init(const bswm_config_type *config_ptr)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id, BSWM_BSWM_SERVICEID_INIT_BSWM_E_NULL_CONTROL_POINTER);
	bswm_validate_cfgpointer_norv(config_ptr, BSWM_BSWM_SERVICEID_INIT_BSWM_E_NULL_POINTER);
	bswm_validate_cfgparamrange_norv((NULL != config_ptr->rule_cfg), BSWM_BSWM_SERVICEID_INIT_BSWM_E_PARAM_CONFIG);
	bswm_validate_cfgparamrange_norv((BSWM_NUMBER_OF_RULES_MAX >= config_ptr->rule_cfg->rules_count),
					 BSWM_BSWM_SERVICEID_INIT_BSWM_E_PARAM_CONFIG);

	(void)*bswm_ctx->bswm_rule_config_ptr;
	(void)*bswm_ctx->bswm_config_ptr;
	(void)*bswm_ctx->bswm_rules;

	bswm_ctx->bswm_config_ptr = config_ptr;
	bswm_ctx->bswm_rule_config_ptr = config_ptr->rule_cfg;
	bswm_ctx->bswm_rules = config_ptr->rule_cfg->rule_list;

	uint32 i;
#if (BSWM_TIMERS_NUM > 0u)
	(void)memset((void *)bswm_ctx->timers, 0x00, BSWM_TIMERS_NUM * sizeof(bswm_timer_t));
#endif
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
	for (i = 0; i < BSWM_NUMBER_OF_RULES_MAX; i++) {
		bswm_ctx->bswm_rule_states[i] = bswm_ctx->bswm_rules[i].initial_state;
	}
#endif

#if (BSWM_NUMBER_OF_REQUESTS > 0)
	for (i = 0; i < BSWM_NUMBER_OF_REQUESTS; i++) {
		bswm_ctx->bswm_mode_req_mirrors[i] = config_ptr->mode_request_data[i];
	}
#endif

#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
	(void)memset(bswm_ctx->bswm_delayed_request, 0x00, BSWM_NUMBER_OF_IMMEDIATE_REQUESTS * sizeof(boolean));
#endif

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
	bswm_ctx->bswm_pdu_group_switch_action_performed = FALSE;
	bswm_ctx->bswm_com_ipdu_initialize = FALSE;
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
	bswm_ctx->bswm_dm_control_performed = FALSE;
#endif

	bswm_ctx->bswm_request_processing_ongoing = FALSE;
	bswm_ctx->init_status = BSWM_INIT;
	return;
}

FUNC(void, BSWM_CODE) bswm_deinit(void)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id,
					      BSWM_BSWM_SERVICEID_DEINIT_BSWM_E_NULL_CONTROL_POINTER);
	bswm_validate_init_norv(BSWM_BSWM_SERVICEID_DEINIT_BSWM_E_NO_INIT, bswm_ctx);

	bswm_ctx->init_status = BSWM_UNINIT;
	(void)VCOS_LOG(BSWM_MODULE_ID, VCOS_LOG_INFO, BSWM_DLT_DEINIT);
}

FUNC(Std_ReturnType, BSWM_CODE) bswm_get_status(bswm_init_status_t *bswm_status)
{
	uint8 core_id = bswm_get_core_id();
	const bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid(bswm_ctx, core_id,
					 BSWM_BSWM_SERVICEID_GETVERSIONINFO_BSWM_E_NULL_CONTROL_POINTER, E_NOT_OK);
	bswm_validate((bswm_status != NULL), BSWM_BSWM_SERVICEID_GETVERSIONINFO_BSWM_E_PARAM_POINTER, E_NOT_OK);

	*bswm_status = bswm_ctx->init_status;
	return E_OK;
}

FUNC(void, BSWM_CODE) bswm_mainfunction(void)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id,
					      BSWM_BSWM_SERVICEID_MAINFUNCTION_BSWM_E_NULL_CONTROL_POINTER);

	bswm_validate_init_norv(BSWM_BSWM_SERVICEID_MAINFUNCTION_BSWM_E_NO_INIT, bswm_ctx);

	boolean lock_request;

	enter_critical(bswm_ctx, &lock_request);

	bswm_internal_update_notification_mirrors();

	process_rules_and_lists(bswm_ctx);

	bswm_com_common_action(lock_request, bswm_ctx);
}

#define BSWM_STOP_SEC_CODE
#include "bswm_memmap.h"

#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
#define BSWM_START_SEC_CODE
#include "bswm_memmap.h"
static FUNC(void, BSWM_CODE) bswm_internal_process_immediate_request(const uint32 index_start, const uint32 index_end,
								     bswm_internal_t *bswm_ctx)
{
	boolean lock_request;

	SchM_Enter_BSWM_EXCLUSIVE_AREA_0();
	lock_request = bswm_try_process_request(bswm_ctx);
	SchM_Exit_BSWM_EXCLUSIVE_AREA_0();

	uint32 start = index_start;

	if (TRUE == lock_request) {
		bswm_internal_evaluate_rules_for_immediate_request(start, index_end, bswm_ctx);
	} else {
		for (; start <= index_end; start++) {
			bswm_ctx->bswm_delayed_request[start] = TRUE;
		}
	}

	bswm_com_common_action(lock_request, bswm_ctx);
}

FUNC(void, BSWM_CODE) bswm_process_immediate_request(const uint32 index_start, const uint32 index_end)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id,
					      BSWM_BSWM_SERVICEID_IMMEDIATEREQUEST_BSWM_E_NULL_CONTROL_POINTER);

	bswm_validate_init_norv(BSWM_BSWM_SERVICEID_IMMEDIATEREQUEST_BSWM_E_NO_INIT, bswm_ctx);
	bswm_validate_requestuser_norv((index_start <= index_end) && (index_end < BSWM_NUMBER_OF_IMMEDIATE_REQUESTS),
				       BSWM_BSWM_SERVICEID_IMMEDIATEREQUEST_BSWM_E_REQ_USER_OUT_OF_RANGE);

	bswm_internal_process_immediate_request(index_start, index_end, bswm_ctx);
}

FUNC(void, BSWM_CODE) bswm_process_immediate_request_on_init(void)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id, BSWM_BSWM_SERVICEID_INIT_BSWM_E_NULL_POINTER);
	bswm_validate_init_norv(BSWM_BSWM_SERVICEID_IMMEDIATEREQUEST_BSWM_E_NO_INIT, bswm_ctx);

	uint32_t index = 0;
	boolean lock_request;

	SchM_Enter_BSWM_EXCLUSIVE_AREA_0();
	lock_request = bswm_try_process_request(bswm_ctx);
	SchM_Exit_BSWM_EXCLUSIVE_AREA_0();

	for (uint32_t i = 0u; i < BSWM_NUMBER_OF_IMMEDIATE_ONINIT_REQUESTS; i++) {
		// coverity[cert_int31_c_violation:SUPPRESS]
		index = (uint16_t)bswm_immediate_rules_on_init[i];
		// coverity[cert_int31_c_violation:SUPPRESS]
		if (TRUE == lock_request) {
			bswm_internal_evaluate_rules_for_immediate_request(index, index, bswm_ctx);
		} else {
			bswm_ctx->bswm_delayed_request[index] = TRUE;
		}
	}

	bswm_com_common_action(lock_request, bswm_ctx);
}
#endif

typedef void (*group_set_func_t)(void *group_vector, sint32 id, boolean enable);

static void bswm_com_ipdu_set(void *group_vector, sint32 id, boolean enable)
{
#if (CONFIG_MODULE_COM == 1)
	com_set_ipdu_group(*(com_ipg_vec *)group_vector, (com_ipdu_group_id_type)id, enable);
#endif
}

static void bswm_enable_routing(void *dummy, sint32 id, boolean enable)
{
	(void)dummy;
#if (CONFIG_MODULE_PUDR == 1)
	if (BSWM_PDUR_ENABLE == enable)
		pdur_enable_routing((pdur_routing_path_group_id_type)id);
	else
		pdur_disable_routing((pdur_routing_path_group_id_type)id, TRUE);
#endif
}

static FUNC(void, BSWM_CODE) bswm_traverse_group(void *group_vector, const sint32 *group_ids, group_set_func_t set_func,
						 boolean *action_performed_flag, boolean enable)
{
	if (NULL == group_ids) {
		return;
	}

	while (*group_ids >= 0) {
		set_func(group_vector, *group_ids, enable);
		group_ids++;
		if (NULL != action_performed_flag) {
			*action_performed_flag = TRUE;
		}
	}
}

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
FUNC(void, BSWM_CODE) bswm_internal_dm_control_action(const bswm_action_list_item_t *item)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id, BSWM_BSWM_SERVICEID_INIT_BSWM_E_NULL_POINTER);

	bswm_traverse_group(bswm_ctx->bswm_dm_control_group_vector, item->active_pdu_group, bswm_com_ipdu_set,
			    &bswm_ctx->bswm_dm_control_performed, TRUE);

	bswm_traverse_group(bswm_ctx->bswm_dm_control_group_vector, item->inactive_pdu_group, bswm_com_ipdu_set,
			    &bswm_ctx->bswm_dm_control_performed, FALSE);
}
#endif

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
FUNC(void, BSWM_CODE) bswm_internal_pdu_group_switch_action(const bswm_action_list_item_t *item)
{
	uint8 core_id = bswm_get_core_id();
	bswm_internal_t *bswm_ctx = bswm_multicore[core_id];

	bswm_validate_pointer_withcoreid_norv(bswm_ctx, core_id, BSWM_BSWM_SERVICEID_INIT_BSWM_E_NULL_POINTER);

	bswm_traverse_group(bswm_ctx->bswm_com_ipdu_group_vector, item->active_pdu_group, bswm_com_ipdu_set,
			    &bswm_ctx->bswm_pdu_group_switch_action_performed, TRUE);

	bswm_traverse_group(bswm_ctx->bswm_com_ipdu_group_vector, item->inactive_pdu_group, bswm_com_ipdu_set,
			    &bswm_ctx->bswm_pdu_group_switch_action_performed, FALSE);

	bswm_ctx->bswm_com_ipdu_initialize = item->need_reinit_pdu_group;
}
#endif

#if (BSWM_PDUROUTERCONTROL_ACTIONS > 0)
FUNC(void, BSWM_CODE) bswm_internal_pdu_router_control_action(const bswm_action_list_item_t *item)
{
	if (NULL == item) {
		return;
	}
	bswm_traverse_group(NULL, item->inactive_pdu_group, bswm_enable_routing, NULL, item->pdu_route_type);
}
#endif

#if (BSWM_NMCONTROL_ACTIONS > 0)
FUNC(void, BSWM_CODE) bswm_nm_control(const bswm_action_list_item_t *item, const uint8 network_handle)
{
	Std_ReturnType status = E_NOT_OK;

	if (item->nm_action == BSWM_NM_DISABLE) {
		status = nm_disable_communication(network_handle);
	} else {
		status = nm_enable_communication(network_handle);
	}
	(void)vcos_test_log(BSWM_MODULE_ID, VCOS_LOG_DEBUG, BSWM_DLT_NMCONTROL_STATUS, status);
}
#endif

#if (BSWM_TIMERS_NUM > 0u)
FUNC(void, BSWM_CODE) bswm_update_timer(const bswm_action_list_item_t *item)
{
	if (item == NULL) {
		return;
	}
	const uint8 timerId = item->timer_id;
	const bswm_timer_value_t value = item->timer_value;

	if (timerId < (BSWM_TIMERS_NUM)) {
		SchM_Enter_BSWM_EXCLUSIVE_AREA_0();
		bswm_set_timer_value(timerId, value);
		bswm_set_timer_state(timerId,
				     (bswm_timer_state_t)((value != 0u) ? BSWM_TIMER_STARTED : BSWM_TIMER_STOPPED));
		SchM_Exit_BSWM_EXCLUSIVE_AREA_0();
	}

	(void)VCOS_LOG(BSWM_MODULE_ID, VCOS_LOG_DEBUG, BSWM_DLT_TIMER_ID, timerId);
	(void)VCOS_LOG(BSWM_MODULE_ID, VCOS_LOG_DEBUG, BSWM_DLT_TIMER_VALUE, value);
	// coverity[cert_str31_c_violation:SUPPRESS]
	// coverity[cert_arr30_c_violation:SUPPRESS]
	// coverity[misra_c_2012_rule_18_1_violation:SUPPRESS]
	(void)VCOS_LOG(BSWM_MODULE_ID, VCOS_LOG_DEBUG, BSWM_DLT_TIMER_STATE, bswm_get_timer_state(timerId));
}
#endif

#if (BSWM_VERSION_INFO_API == STD_ON)
FUNC(void, BSWM_CODE) bswm_get_version_info(Std_VersionInfoType *version_info)
{
	bswm_validate_pointer_norv(version_info, BSWM_BSWM_SERVICEID_GETVERSIONINFO_BSWM_E_PARAM_POINTER);

	version_info->moduleID = BSWM_MODULE_ID; /* Module ID of BSWM */
	version_info->vendorID = BSWM_VENDOR_ID; /* Vendor id(Lixiang) */

	/* return the Software Version numbers */
	version_info->sw_major_version = BSWM_SW_MAJOR_VERSION;
	version_info->sw_minor_version = BSWM_SW_MINOR_VERSION;
	version_info->sw_patch_version = BSWM_SW_PATCH_VERSION;
}
#endif /* BSWM_VERSION_INFO_API */
#define BSWM_STOP_SEC_CODE
#include "bswm_memmap.h"