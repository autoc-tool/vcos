/*
 * Copyright (C) 2022 理想汽车股份有限公司版权所有.
 *
 * Description:  Det 模块源文件.
 */

/* Including File */
/*! 此处放置所需要的头文件 */
#if (VCOS_MODULE_CONFIG_VCOSMTNCE == 1)
#include "vcosMtnce_Cfg.h"
#endif
#include "Det.h"
#include "Det_Internal.h"
#include "SchM_Det.h"
#include "det_cfg.h"
#if (DET_FORWARD_TO_DLT == STD_ON)
#include "Dlt.h"
#endif
#include "rtfw_atomic.h"
/*port 层 不需要解耦*/
#include "Compiler.h" // #include "portCompiler.h"
#include "vcos_log.h"
#include "Os.h"
#include <nuttx/clock.h>
/* Local Macro Definition */
/*! 此处放置仅在源文件内使用的宏定义 */
#if (configNUM_CORES > 1)
#define Det_GetCoreID  (uint32)GetCoreID()
#else
#define Det_GetCoreID  0u
#endif
/* Local Enumeration Definition */
/*! 此处放置仅在源文件内使用的枚举定义 */

#define DET_UNINITIALIZED   0u
#define DET_INITIALIZED	 1U
#define DET_STARTED		 2u
#define DET_MAX_SUPPORT_CORE_NUM 6u

#define DET_REPORT_ERROR_RECURSION_MAX		1u
#define DET_REPORT_RUNTIMEERROR_RECURSION_MAX   1u
#define DET_REPORT_TRANSIENTFAULT_RECURSION_MAX 1u

#define DET_TIME_TYPE       clock_t
#define Det_TimeGet()       perf_gettime()
#define Det_TimeFreqGet()   perf_getfreq()

#define DET_WITHINFO_MAX_LEN (20u)
#define MEM_LOG_MAX_LENGTH (64u)

#define DET_MS_CNT_FACTOR  (1000u)		 /*tick cnt 转ms*/
#define BSW_MODULE_ID_MAX  (256u)
/* Global Variable Definition */

/* Local Variable Definition */
#define DET_START_SEC_MULTI_APP_BSS
#include "det_memmap.h"
static VAR(uint8,DET_MULTI_APP_BSS) detStatus;
#if (DET_ENABLE_ERROR_HOOK == STD_ON)
static VAR(uint32, DET_MULTI_APP_BSS) detReportErrorRecurCnt[DET_MAX_SUPPORT_CORE_NUM];
#endif
#if (DET_ENABLE_RUNTIME_ERROR_CALLOUT == STD_ON)
static VAR(uint32, DET_MULTI_APP_BSS) detReportRuntimeErrorRecurCnt[DET_MAX_SUPPORT_CORE_NUM];
#endif
#if (DET_ENABLE_TRANSIENT_FAULT_CALLOUT == STD_ON)
static VAR(uint32, DET_MULTI_APP_BSS) detReportTransErrorRecurCnt[DET_MAX_SUPPORT_CORE_NUM];
#endif
#define DET_STOP_SEC_MULTI_APP_BSS
#include "det_memmap.h"

#define  DET_START_SEC_CODE
#include "det_memmap.h"
FUNC(void, DET_CODE) Det_SetTokenBucketCapacity(sint32 cap)
{
	Det_Bucket.capacity = cap;
	(void)Os_AtomicSet(&Det_Bucket.globaltokennum, Det_Bucket.capacity);
}

FUNC(sint32, DET_CODE) Det_GetTokenBucketCapacity(void)
{
	return Det_Bucket.capacity;
}

#if (DET_FLOW_CONTROL == STD_ON)
static FUNC(Std_ReturnType, DET_CODE) global_fillToken(Det_TokenBucket* bucket)
{
	sint32 detlockflagtmpvalue = bucket->globallockflag;
	DET_TIME_TYPE elapsed_time;

	if (detlockflagtmpvalue == 0) {
		if (Os_AtomicCmpSwap(&bucket->globallockflag, detlockflagtmpvalue, 1) == 0) {
			/* 上锁失败，代表当前有其他核或者线程在尝试补充令牌*/
			return E_NOT_OK;
		} else {
			/* 加锁成功 */
			DET_TIME_TYPE current_time = Det_TimeGet();
			DET_TIME_TYPE last_time = bucket->last_time;

			if(current_time < last_time) {
				elapsed_time = (DET_TIME_TYPE)(UINT64_MAX) - last_time + current_time;
			} else {
				elapsed_time = current_time - last_time;
			}
			/* 过去了多少ms秒 */
			uint32_t elapsed_ms = (uint32_t)(elapsed_time * DET_MS_CNT_FACTOR / (uint64)Det_TimeFreqGet());
			uint32_t elapsed_period = (uint32_t)(elapsed_ms >> DET_BUCKET_PERIOD_EXPONENT);

			if (elapsed_period > 0u) {
				(void)Os_AtomicSet(&bucket->globaltokennum, bucket->capacity);
				for(uint32 module_index = 0u; module_index < (uint32)MODULE_FLOW_CONTROL_NUM; module_index++) {
					(void)Os_AtomicSet(&bucket->tokens[Module_Flow_Info[module_index].module_id_info],
						(bucket->capacity));
				}
				(void)Os_AtomicDec(&bucket->globaltokennum);
				bucket->last_time = current_time;
				/* 解锁 */
				(void)Os_AtomicCmpSwap(&bucket->globallockflag, 1, detlockflagtmpvalue);
				return E_OK;
			} else {
				/* 解锁 */
				(void)Os_AtomicCmpSwap(&bucket->globallockflag, 1, detlockflagtmpvalue);
				return E_NOT_OK;
			}
		}
	} else {
		/* 上锁失败，代表当前有其他核或者线程在尝试补充令牌*/
		return E_NOT_OK;
	}
}

static FUNC(Std_ReturnType, DET_CODE) module_fillToken(Det_TokenBucket* bucket, uint16 ModuleId)
{
	sint32 detlockflagtmpvalue = bucket->detModuleLockFlag[Module_Flow_Info[ModuleId].module_id_info];
	DET_TIME_TYPE elapsed_time;

	if (detlockflagtmpvalue == 0) {
		if (Os_AtomicCmpSwap(&bucket->detModuleLockFlag[Module_Flow_Info[ModuleId].module_id_info],
			detlockflagtmpvalue, 1) == 0) {
			/* 上锁失败，代表当前有其他核或者线程在尝试补充令牌*/
			return E_NOT_OK;
		} else {
			/* 加锁成功 */
			DET_TIME_TYPE current_time = Det_TimeGet();
			DET_TIME_TYPE last_time = bucket->last_time;

			if(current_time < last_time) {
				elapsed_time = (DET_TIME_TYPE)(UINT64_MAX) - last_time + current_time;
			} else {
				elapsed_time = current_time - last_time;
			}
			/* 过去了多少ms秒 */
			uint32_t elapsed_ms = (uint32_t)(elapsed_time * DET_MS_CNT_FACTOR / (uint64)Det_TimeFreqGet());
			uint32_t elapsed_period = (uint32_t)(elapsed_ms >> DET_BUCKET_PERIOD_EXPONENT);

			if (elapsed_period > 0u) {
				(void)Os_AtomicSet(&bucket->globaltokennum, bucket->capacity);
				for(uint32 module_index = 0u; module_index < (uint32)MODULE_FLOW_CONTROL_NUM; module_index++) {
					(void)Os_AtomicSet(&bucket->tokens[Module_Flow_Info[module_index].module_id_info],
						(bucket->capacity));
				}
				(void)Os_AtomicDec(&bucket->tokens[Module_Flow_Info[ModuleId].module_id_info]);
				bucket->last_time = current_time;
				/* 解锁 */
				(void)Os_AtomicCmpSwap(&bucket->detModuleLockFlag[Module_Flow_Info[ModuleId].module_id_info], 1,
					detlockflagtmpvalue);
				return E_OK;
			} else {
				/* 解锁 */
				(void)Os_AtomicCmpSwap(&bucket->detModuleLockFlag[Module_Flow_Info[ModuleId].module_id_info], 1,
					detlockflagtmpvalue);
				return E_NOT_OK;
			}
		}
	} else {
		/* 上锁失败，代表当前有其他核或者线程在尝试补充令牌*/
		return E_NOT_OK;
	}
}
#endif

#if (DET_FLOW_CONTROL == STD_ON)
static FUNC(Std_ReturnType, DET_CODE) getToken(Det_TokenBucket* bucket, uint16 ModuleId)
{
	Atomic currCnt = 0;
	Std_ReturnType res = E_NOT_OK;

	if((ModuleId >= BSW_MODULE_ID_MAX) || (Module_Flow_Info[ModuleId].module_enable_info== (boolean)FALSE)) {
		currCnt = Os_AtomicDec(&bucket->globaltokennum);
		if (currCnt <= 0) {
		/* 原子减之前小于等于0的，需要将计数回归 */
			(void)Os_AtomicInc(&bucket->globaltokennum);
			if (global_fillToken(bucket) == E_OK) {
				res = E_OK;
			} else {
				res = E_NOT_OK;
			}
		} else {
			res = E_OK;  // 获取到令牌
		}
		return res;
	}
	currCnt = Os_AtomicDec(&bucket->tokens[Module_Flow_Info[ModuleId].module_id_info]);
	if (currCnt <= 0) {
		/* 原子减之前小于等于0的，需要将计数回归 */
		(void)Os_AtomicInc(&bucket->tokens[Module_Flow_Info[ModuleId].module_id_info]);
		if (module_fillToken(bucket,  ModuleId) == E_OK) {
			res = E_OK;
		} else {
			res = E_NOT_OK;
		}
	} else {
		res = E_OK;  // 获取到令牌
	}
	return res;
}
#endif
FUNC(void, DET_CODE) Det_Init(const Det_ConfigType* ConfigPtr)
{
	(void)ConfigPtr;
	detStatus = DET_INITIALIZED;
}

FUNC(void, DET_CODE) Det_DeInit(void) {
	detStatus = DET_UNINITIALIZED;
#if (DET_ENABLE_ERROR_HOOK == STD_ON)
	(void)memset(detReportErrorRecurCnt, 0, sizeof(detReportErrorRecurCnt));
#endif
#if (DET_ENABLE_RUNTIME_ERROR_CALLOUT == STD_ON)
	(void)memset(detReportRuntimeErrorRecurCnt, 0, sizeof(detReportRuntimeErrorRecurCnt));
#endif
#if (DET_ENABLE_TRANSIENT_FAULT_CALLOUT == STD_ON)
	(void)memset(detReportTransErrorRecurCnt, 0, sizeof(detReportTransErrorRecurCnt));
#endif
}

configOS_WEAK FUNC(Std_ReturnType, DET_CODE) Det_ReportError(uint16 ModuleId, uint8 InstanceId,
	uint8 ApiId, uint8 ErrorId)
{
#if (DET_ENABLE_ERROR_HOOK == STD_ON)
	uint32 core_id = 0u;
#endif
	Std_ReturnType rVal = E_OK;
	/* 如果未执行到Det_Start(),此时所有的模块无法通过DET 发送数据 */
	if (detStatus != DET_STARTED) {
		rVal = E_NOT_OK;
		return rVal;
	}
#if (DET_FLOW_CONTROL == STD_ON)
	rVal = getToken(&Det_Bucket, ModuleId);
	if (rVal == E_NOT_OK) {
		return rVal;
	}
#endif
	/* SWS_Det_00034 */
#if ((DET_FORWARD_TO_DLT == STD_ON) && (VCOS_MODULE_CONFIG_DLT == 1))
	Dlt_DetForwardErrorTrace(ModuleId, InstanceId, ApiId, ErrorId);
#else
	(void)VCOS_LOG(DET_MODULE_ID, VCOS_LOG_ERROR,"DetRpe:Module %d,Instance %d,Api %d,Error %d\r\n",
	ModuleId, InstanceId, ApiId, ErrorId);
#endif

#if (DET_ENABLE_ERROR_HOOK == STD_ON)
	for (uint32 i = 0; i < DET_ERROR_HOOK_TABLE_NUMBER; i++) {
		if (NULL != Det_ErrorHookTable[i]) {
			/*防止hook递归调用DET过深，给hook增加限制条件*/
			core_id = Det_GetCoreID;
			if(core_id >= DET_MAX_SUPPORT_CORE_NUM){
				rVal = E_NOT_OK;
				return rVal;
			}
			SchM_Enter_DET_EXCLUSIVE_AREA_0();
			/*coverity[cert_int30_c_violation:SUPPRESS] */
			detReportErrorRecurCnt[core_id]++;
			SchM_Exit_DET_EXCLUSIVE_AREA_0();
			if (detReportErrorRecurCnt[core_id] > DET_REPORT_ERROR_RECURSION_MAX) {
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			} else {
				(void)(*Det_ErrorHookTable[i])(ModuleId, InstanceId, ApiId, ErrorId);
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			}
		}
	}
#endif /* (DET_ENABLE_ERROR_HOOK == STD_ON) */

	return rVal;
}

FUNC(void, DET_CODE) Det_Start(void) { detStatus = DET_STARTED; }

configOS_WEAK FUNC(Std_ReturnType, DET_CODE) Det_ReportRuntimeError(uint16 ModuleId,
	uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
#if (DET_ENABLE_RUNTIME_ERROR_CALLOUT == STD_ON)
	uint32 core_id = 0u;
#endif
	Std_ReturnType rVal = E_OK;

	if (detStatus != DET_STARTED) {
		rVal = E_NOT_OK;
		return rVal;
	}
#if (DET_FLOW_CONTROL == STD_ON)
	rVal = getToken(&Det_Bucket, ModuleId);
	if (rVal == E_NOT_OK) {
		return rVal;
	}
#endif
#if ((DET_FORWARD_TO_DLT == STD_ON) && (VCOS_MODULE_CONFIG_DLT == 1))
	Dlt_DetForwardErrorTrace(ModuleId, InstanceId, ApiId, ErrorId);
#else
	(void)VCOS_LOG(DET_MODULE_ID, VCOS_LOG_ERROR,"DetRRE:Module %d,Instance %d,Api %d,Error %d\r\n",
	 ModuleId, InstanceId, ApiId, ErrorId);
#endif

#if (DET_ENABLE_RUNTIME_ERROR_CALLOUT == STD_ON)
	for (uint32 i = 0; i < DET_RUNTIMEERR_CALLOUT_TABLE_NUMBER; i++) {
		if (NULL != Det_RuntimeErrorCalloutTable[i]) {
			/*防止hook递归调用DET过深，给hook增加限制条件*/
			core_id = Det_GetCoreID;
			if(core_id >= DET_MAX_SUPPORT_CORE_NUM){
				rVal = E_NOT_OK;
				return rVal;
			}
			SchM_Enter_DET_EXCLUSIVE_AREA_0();
			detReportRuntimeErrorRecurCnt[core_id]++;
			SchM_Exit_DET_EXCLUSIVE_AREA_0();
			if (detReportRuntimeErrorRecurCnt[core_id] > DET_REPORT_ERROR_RECURSION_MAX) {
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportRuntimeErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			} else {
				(void)(*Det_RuntimeErrorCalloutTable[i])(ModuleId, InstanceId, ApiId, ErrorId);
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportRuntimeErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			}
		}
	}
#endif
	return rVal;
}

static void num_to_str(char *result, uint8 num)
{
	uint8 target_num = num >> 4;

	if (target_num <= 9) {
		result[0] = target_num + '0';
	} else {
		result[0] = target_num + - 10 + 'A';
	}
	target_num = num & 0xF;
	if (target_num <= 9) {
		result[1] = target_num + '0';
	} else {
		result[1] = target_num + - 10 + 'A';
	}
}

// coverity[misra_c_2012_rule_8_13_violation:SUPPRESS]
configOS_WEAK FUNC(Std_ReturnType, DET_CODE) Det_ReportErrorWithInfo(uint32 errorId, uint8 *info,uint8 len)
{
	Std_ReturnType rVal = E_OK;
#if (DET_FLOW_CONTROL == STD_ON)
	uint16 ModuleId = (uint16)((errorId >> 16u) & 0xFFu);

	rVal = getToken(&Det_Bucket, ModuleId);
#endif
	if (rVal == E_NOT_OK) {
		return rVal;
	}
#if ((DET_FORWARD_TO_DLT == STD_ON) && (VCOS_MODULE_CONFIG_DLT == 1))
	return Dlt_DetForwardErrorTraceWithInfo( errorId, info, len);
#else
	uint8 tempbuff[MEM_LOG_MAX_LENGTH];
	uint16 formatLogLen = 0u;

	if (len > DET_WITHINFO_MAX_LEN) {
		rVal = E_NOT_OK;
	} else {
		for (uint16 i = 0; i < len; i++) {
			num_to_str((char *)&tempbuff[formatLogLen], info[i]);
			formatLogLen += 2;
		}
		(void)VCOS_LOG(DET_MODULE_ID, VCOS_LOG_ERROR, "DetInfo:%08X%.*s#\r\n", (unsigned int)errorId,
															formatLogLen, (unsigned char *)tempbuff);
	}
	return rVal;
#endif
}

configOS_WEAK FUNC(Std_ReturnType, DET_CODE) Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId,
	uint8 ApiId, uint8 FaultId)
{
	Std_ReturnType rVal = E_OK;
#if (DET_ENABLE_TRANSIENT_FAULT_CALLOUT == STD_ON)
	uint32 core_id = 0u;
#endif
	if (detStatus != DET_STARTED) {
		rVal = E_NOT_OK;
		return rVal;
	}
#if (DET_FLOW_CONTROL == STD_ON)
	rVal = getToken(&Det_Bucket, ModuleId);
	if (rVal == E_NOT_OK) {
		return rVal;
	}
#endif
#if ((DET_FORWARD_TO_DLT == STD_ON) && (VCOS_MODULE_CONFIG_DLT == 1))
	Dlt_DetForwardErrorTrace(ModuleId, InstanceId, ApiId, FaultId);
#else
	(void)VCOS_LOG(DET_MODULE_ID, VCOS_LOG_ERROR,"DetRTF:Module %d,Instance %d,Api %d,Error %d\r\n",
	ModuleId, InstanceId, ApiId, FaultId);
#endif

#if (DET_ENABLE_TRANSIENT_FAULT_CALLOUT == STD_ON)
	for (uint32 i = 0; i < DET_TRANSIENT_FAULT_CALLOUT_NUMBER; i++) {
		if (NULL != Det_TransientFaultCalloutTable[i]) {
			/*防止hook递归调用DET过深，给hook增加限制条件*/
			core_id = Det_GetCoreID;
			if(core_id >= DET_MAX_SUPPORT_CORE_NUM){
				rVal = E_NOT_OK;
				return rVal;
			}
			SchM_Enter_DET_EXCLUSIVE_AREA_0();
			detReportTransErrorRecurCnt[core_id]++;
			SchM_Exit_DET_EXCLUSIVE_AREA_0();
			if (detReportTransErrorRecurCnt[core_id] > DET_REPORT_ERROR_RECURSION_MAX) {
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportTransErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			} else {
				(void)(*Det_TransientFaultCalloutTable[i])(ModuleId, InstanceId, ApiId, FaultId);
				SchM_Enter_DET_EXCLUSIVE_AREA_0();
				detReportTransErrorRecurCnt[core_id]--;
				SchM_Exit_DET_EXCLUSIVE_AREA_0();
			}
		}
	}
#endif

	return rVal;
}

#if (DET_VERSIONINFO_API == STD_ON)
FUNC(void, DET_CODE) Det_GetVersionInfo(Std_VersionInfoType* vi)
{
	if (vi == NULL) {
		(void)Det_ReportError(DET_MODULE_ID, 0, DET_GETVERSIONINFO_SERVICE_ID, DET_E_PARAM_POINTER);
		return;
	}
	vi->vendorID = DET_VENDOR_ID;
	vi->moduleID = DET_MODULE_ID;
	vi->sw_major_version = DET_SW_MAJOR_VERSION;
	vi->sw_minor_version = DET_SW_MINOR_VERSION;
	vi->sw_patch_version = DET_SW_PATCH_VERSION;
}
#endif
#define  DET_STOP_SEC_CODE
#include "det_memmap.h"
