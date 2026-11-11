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

#include <stdio.h>

#include "ut_register.h"
#if defined(CONFIG_UNIT_GROUP_YOUR_GROUP) && (CONFIG_UNIT_GROUP_YOUR_GROUP == 1)
#include "register_group_demo.h"
#endif

void register_groups(void)
{
#if defined(CONFIG_UNIT_GROUP_YOUR_GROUP) && (CONFIG_UNIT_GROUP_YOUR_GROUP == 1)
	REGISTER_GROUP("group_demo", register_group_demo);
#endif
}
