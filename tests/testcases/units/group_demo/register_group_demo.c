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

#include "ut_register.h"
#include "register_suite_demo.h"

void setup_group(void) {
	return;
}

void teardown_group(void) {
	return;
}

void register_group_demo(char *group_name) {
	add_group_to_registry(group_name, setup_group, teardown_group);
	REGISTER_SUITE(group_name, "suite_demo", register_suite_demo);
}