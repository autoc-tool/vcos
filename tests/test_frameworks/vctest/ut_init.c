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

test_group_s *g_test_groups = NULL;

int total_groups = 1;

extern void register_groups(void);
#if defined(CONFIG_UNIT_EXTERNTION_CASE) && (CONFIG_UNIT_EXTERNTION_CASE == 1)
extern void register_externtion_groups(void);
#endif

void unity_init(void)
{
	register_groups();
#if defined(CONFIG_UNIT_EXTERNTION_CASE) && (CONFIG_UNIT_EXTERNTION_CASE == 1)
	register_externtion_groups();
#endif
}

void unity_deinit(void)
{
	test_group_s *current = g_test_groups;

	while (current != NULL) {
		test_suite_s *suite = current->test_suites;

		while (suite != NULL) {
			test_case_s *testcase = suite->test_cases;

			while (testcase != NULL) {
				test_case_s *testcase_ptr = testcase;

				testcase = testcase->next;
				free(testcase_ptr);
			}
			test_suite_s *suite_ptr = suite;

			suite = suite->next;
			free(suite_ptr);
		}
		test_group_s *group_ptr = current;

		current = current->next;
		free(group_ptr);
	}

	g_test_groups = NULL;
	total_groups = 1;
}

void add_group_to_registry(const char *name, TestFunction setup, TestFunction teardown)
{
	test_group_s *tg = (test_group_s *)malloc(sizeof(test_group_s));

	tg->name = name;
	tg->setup = setup;
	tg->teardown = teardown;
	tg->group_id = total_groups++;
	tg->total_suites_in_group = 1;
	tg->test_suites = NULL;
	tg->next = NULL;

	if (g_test_groups == NULL) {
		g_test_groups = tg;
	} else {
		test_group_s *current = g_test_groups;

		while (current->next != NULL) {
			current = current->next;
		}
		current->next = tg;
	}
}

test_suite_s *add_suite_to_group(const char *group_name, const char *suite_name, TestFunction setup,
				 TestFunction teardown)
{
	test_group_s *current_group = g_test_groups;

	while (current_group != NULL) {
		if (strcmp(current_group->name, group_name) == 0) {
			test_suite_s *ts = (test_suite_s *)malloc(sizeof(test_suite_s));

			ts->name = suite_name;
			ts->setup = setup;
			ts->teardown = teardown;
			ts->suite_id = current_group->total_suites_in_group;
			ts->total_testcases_in_suite = 1;
			ts->test_cases = NULL;
			ts->next = NULL;
			current_group->total_suites_in_group++;

			if (current_group->test_suites == NULL) {
				current_group->test_suites = ts;
			} else {
				test_suite_s *current = current_group->test_suites;

				while (current->next != NULL) {
					current = current->next;
				}
				current->next = ts;
			}
			return ts;
		}
		current_group = current_group->next;
	}
	return NULL;
}

void register_testcase(test_suite_s *suite, const char *name, TestFunction func, int radius, int core_id)
{
	test_case_s *tc = (test_case_s *)malloc(sizeof(test_case_s));

	tc->name = name;
	tc->test = func;
	tc->radius = radius;
	tc->testcase_id = suite->total_testcases_in_suite;
	tc->core_id = core_id;
	tc->next = NULL;
	suite->total_testcases_in_suite++;

	if (suite->test_cases == NULL) {
		suite->test_cases = tc;
	} else {
		test_case_s *current = suite->test_cases;

		while (current->next != NULL) {
			current = current->next;
		}
		current->next = tc;
	}
}
