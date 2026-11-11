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

#define DDS_MEMMAP_ERROR

#if defined __TASKING__

/*******************CORE0_DDS *******************/
/*code*/
#if defined START_SEC_CODE_DDS
    # pragma section code = "Code.SEC_CODE_DDS"
    # undef START_SEC_CODE_DDS /* PRQA S 0841 */ /* MD_MSR_Undef */
    # undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_CODE_DDS
    #pragma section code restore
    #undef STOP_SEC_CODE_DDS
    #undef DDS_MEMMAP_ERROR
#endif

/*const*/
#if defined START_SEC_CONST_DDS
    #pragma section farbss  "ClearFarData.START_SEC_DATA_DDS"
    #pragma section fardata "InitFarData.START_SEC_INIT_DATA_DDS"
    #pragma section farrom "ConstFarData.START_SEC_CONST_DDS"
    #undef START_SEC_CONST_DDS
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_CONST_DDS
    #pragma section farbss restore
    #pragma section fardata restore
    #pragma section farrom restore
    #undef STOP_SEC_CONST_DDS
    #undef DDS_MEMMAP_ERROR
#endif

/*init data*/
#if defined START_SEC_INIT_DATA_DDS
    #pragma section fardata "InitFarData.START_SEC_INIT_DATA_DDS"
    #pragma section farbss "ClearFarData.START_SEC_DATA_DDS"
    #pragma default_near_size 0
    #undef START_SEC_INIT_DATA_DDS
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_INIT_DATA_DDS
    #pragma default_near_size restore
    #pragma section fardata restore
    #pragma section farbss restore
    #undef STOP_SEC_INIT_DATA_DDS
    #undef DDS_MEMMAP_ERROR
#endif

/*noninit data*/
#if defined START_SEC_DATA_DDS
    #pragma section fardata "InitFarData.START_SEC_INIT_DATA_DDS"
    #pragma section farbss "ClearFarData.START_SEC_DATA_DDS"
    #pragma default_near_size 0
    #pragma clear
    #undef START_SEC_DATA_DDS
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_DATA_DDS
    #pragma clear restore
    #pragma default_near_size restore
    #pragma section fardata restore
    #pragma section farbss restore
    #undef STOP_SEC_DATA_DDS
    #undef DDS_MEMMAP_ERROR
#endif

/*init data*/
#if defined START_SEC_INIT_DATA_DDS_SH
    #pragma section fardata "InitFarData.SEC_INIT_DATA_DDS_SH"
    #pragma section farbss "ClearFarData.SEC_DATA_DDS_SH"
    #pragma default_near_size 0
    #undef START_SEC_INIT_DATA_DDS_SH
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_INIT_DATA_DDS_SH
    #pragma default_near_size restore
    #pragma section fardata restore
    #pragma section farbss restore
    #undef STOP_SEC_INIT_DATA_DDS_SH
    #undef DDS_MEMMAP_ERROR
#endif

/*noninit data*/
#if defined START_SEC_DATA_DDS_SH
    #pragma section fardata "InitFarData.SEC_INIT_DATA_DDS_SH"
    #pragma section farbss "ClearFarData.SEC_DATA_DDS_SH"
    #pragma default_near_size 0
    #pragma clear
    #undef START_SEC_DATA_DDS_SH
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_DATA_DDS_SH
    #pragma clear restore
    #pragma default_near_size restore
    #pragma section fardata restore
    #pragma section farbss restore
    #undef STOP_SEC_DATA_DDS_SH
    #undef DDS_MEMMAP_ERROR
#endif

/*cal*/
#if defined START_SEC_CAL_DDS
    #pragma section farrom "Caldata.SEC_CAL_DDS"
    #pragma default_near_size 0
    #pragma align 0
    #pragma protect
    #undef START_SEC_CAL_DDS
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_CAL_DDS
    #pragma endprotect restore
    #pragma align restore
    #pragma default_near_size restore
    #pragma section farrom restore
    #undef STOP_SEC_CAL_DDS
    #undef DDS_MEMMAP_ERROR
#endif


/*******************share data ”*******************/

/*init data*/
#if defined START_SEC_INIT_DATA_Debug
    #pragma section fardata "InitFarData.SEC_INIT_DATA_Debug"
    #pragma default_near_size 0
    #undef START_SEC_INIT_DATA_Debug
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_INIT_DATA_Debug
    #pragma default_near_size restore
    #pragma section fardata restore
    #undef STOP_SEC_INIT_DATA_Debug
    #undef DDS_MEMMAP_ERROR
#endif

/*noninit data*/
#if defined START_SEC_DATA_Debug
    #pragma section farbss "ClearFarData.SEC_DATA_Debug"
    #pragma default_near_size 0
    #pragma clear
    #undef START_SEC_DATA_Debug
    #undef DDS_MEMMAP_ERROR
#endif

#if defined STOP_SEC_DATA_Debug
    #pragma clear restore
    #pragma default_near_size restore
    #pragma section farbss restore
    #undef STOP_SEC_DATA_Debug
    #undef DDS_MEMMAP_ERROR
#endif

/*For compiler GreenHills*/
/*=================================================================================

===================================================================================*/
#elif defined __ghs__
/***************DDS *******************/
/*code of DDS*/
#ifdef START_SEC_CODE_DDS
# undef START_SEC_CODE_DDS
# pragma ghs section text= ".RZCU_SEC_CODE_DDS"
#endif
#ifdef STOP_SEC_CODE_DDS
# undef STOP_SEC_CODE_DDS
# pragma ghs section text = default
#endif
/*const of DDS*/
#ifdef START_SEC_CONST_DDS
# undef START_SEC_CONST_DDS
# pragma ghs section bss = ".RZCU_SEC_DATA_DDS"
# pragma ghs section data = ".RZCU_SEC_INIT_DATA_DDS"
# pragma ghs section rodata = ".RZCU_SEC_CONST_DDS"
#endif

#ifdef STOP_SEC_CONST_DDS
# undef STOP_SEC_CONST_DDS
# pragma ghs section bss = default
# pragma ghs section data = default
# pragma ghs section rodata = default
#endif
/*data of DDS*/
#ifdef START_SEC_DATA_DDS
# undef START_SEC_DATA_DDS
# pragma ghs section bss = ".RZCU_SEC_DATA_DDS"
# pragma ghs section data = ".RZCU_SEC_INIT_DATA_DDS"
#endif
#ifdef STOP_SEC_DATA_DDS
# undef STOP_SEC_DATA_DDS
# pragma ghs section bss = default
# pragma ghs section data = default
#endif
/*INIT data of DDS*/
#ifdef START_SEC_INIT_DATA_DDS
# undef START_SEC_INIT_DATA_DDS
# pragma ghs section bss = ".RZCU_SEC_DATA_DDS"
# pragma ghs section data = ".RZCU_SEC_INIT_DATA_DDS"
#endif
#ifdef STOP_SEC_INIT_DATA_DDS
# undef STOP_SEC_INIT_DATA_DDS
# pragma ghs section bss = default
# pragma ghs section data = default
#endif

/*INIT share data of DDS*/
#ifdef START_SEC_DATA_DDS_SH
# undef START_SEC_DATA_DDS_SH
# pragma ghs section bss = ".RZCU_SEC_DATA_SH_DDS"
# pragma ghs section data = ".RZCU_SEC_INIT_DATA_SH_DDS"
#endif
#ifdef STOP_SEC_DATA_DDS_SH
# undef STOP_SEC_DATA_DDS_SH
# pragma ghs section bss = default
# pragma ghs section data = default
#endif

/*cal of DDS*/
#ifdef START_SEC_CAL_DDS
# undef START_SEC_CAL_DDS
# pragma ghs section rodata = ".RZCU_SEC_CAL_DDS"
#endif

#ifdef STOP_SEC_CAL_DDS
#undef  STOP_SEC_CAL_DDS
# pragma ghs section rodata =  default
#endif

#elif defined __GNUC__ && defined __TRICORE__

#undef DDS_MEMMAP_ERROR

#endif

#if defined DDS_MEMMAP_ERROR
#error "!!! MemMap_DDS.h,Wrong pragma command !!!"
#endif