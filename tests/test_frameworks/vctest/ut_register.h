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

#ifndef __UT_REGISTER_H
#define __UT_REGISTER_H

typedef void (*TestFunction)(void);

typedef enum { UNIT_CASE_R0 = 0, UNIT_CASE_R1 = 1, UNIT_CASE_R2 = 2, UNIT_CASE_R3 = 3 } unity_radius;

typedef struct test_case_s {
	const char *name;
	TestFunction test;
	int radius;
	int testcase_id;
	int core_id;
	struct test_case_s *next;
} test_case_s;

typedef struct test_suite_s {
	const char *name;
	TestFunction setup;
	TestFunction teardown;
	int suite_id;
	int total_testcases_in_suite;
	test_case_s *test_cases;
	struct test_suite_s *next;
} test_suite_s;

typedef struct test_group_s {
	const char *name;
	TestFunction setup;
	TestFunction teardown;
	int group_id;
	int total_suites_in_group;
	test_suite_s *test_suites;
	struct test_group_s *next;
} test_group_s;

extern int total_groups;

extern test_group_s *g_test_groups;

void unity_init(void);
void unity_deinit(void);
void add_group_to_registry(const char *name, TestFunction setup, TestFunction teardown);
test_suite_s *add_suite_to_group(const char *group_name, const char *suite_name, TestFunction setup,
				 TestFunction teardown);
void register_testcase(test_suite_s *suite, const char *name, TestFunction func, int radius, int core_id);

#define REGISTER_GROUP(group_name, group_func) group_func(group_name)
#define REGISTER_SUITE(group_name, suite_name, suite_func) suite_func(group_name, suite_name)
#define REGISTER_TESTCASE(suite_ptr, testcase_name, testcase_func, radius, core_id)                                    \
	register_testcase(suite_ptr, testcase_name, testcase_func, radius, core_id)

#endif
