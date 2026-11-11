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

#ifndef __INCLUDE_NUTTX_TRACED_H
#define __INCLUDE_NUTTX_TRACED_H

# ifdef CONFIG_TRACED

/****************************************************************************
 * Name: trace_handshake_main_func
 *
 * Description:
 *   collect and send trace handshake info
 *
 ****************************************************************************/

void handshake_main_func(void);

/****************************************************************************
 * Name: trace_dumpudp_main_func
 *
 * Description:
 *   read and send trace event data
 *
 ****************************************************************************/

void trace_dumpudp_main_func(void);

# else /* CONFIG_TRACED*/

#define handshake_main_func()
#define trace_dumpudp_main_func()

# endif /* CONFIG_TRACED */

#endif /* __INCLUDE_NUTTX_TRACEd_H */