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
#include <nuttx/config.h>

#include "ut_register.h"
#include "register_suite_demo.h"
#include "test_demo.h"

static void suite_setup(void) {
	return;
}

static void suite_teardown(void) {
	return;
}

void register_suite_demo(const char *group_name, const char *suite_name) {
	test_suite_s *this_suite = add_suite_to_group(group_name, suite_name, suite_setup, suite_teardown);
	REGISTER_TESTCASE(this_suite, "test_demo_1", test_demo_01, UNIT_CASE_R0, 0);
	REGISTER_TESTCASE(this_suite, "test_demo_2", test_demo_02, UNIT_CASE_R0, 0);
}