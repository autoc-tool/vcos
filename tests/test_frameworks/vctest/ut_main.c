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
#include <stdio.h>
#include <string.h>

#include "unity.h"
#include "ut_register.h"

typedef enum { STATUS_OTHER = 0, STATUS_SHOW = 1, STATUS_SHOW_VERBOSE = 2, STATUS_RUN = 3 } ut_para_status_e;

typedef enum { RET_STATUS_OK = 0, RET_STATUS_MEMORY_ALLOC_FAILED = 1, RET_STATUS_UNKNOWN_COMMAND = 2 } return_status_e;

typedef struct ut_parameters_s {
	ut_para_status_e status;
	int group_num;
	int suite_num;
	int testcase_num;
	int radius;
} ut_parameters_s;

void show_test_structure(test_group_s *p_tg, int verbose)
{
	test_group_s *current_group = p_tg;

	while (current_group != NULL) {
		printf("Group %d - %s\n", current_group->group_id, current_group->name);

		test_suite_s *current_suite = current_group->test_suites;

		while (current_suite != NULL) {
			printf("    |_ Suite %d - %s\n", current_suite->suite_id, current_suite->name);

			test_case_s *current_case = current_suite->test_cases;

			while (current_case != NULL && verbose == STATUS_SHOW_VERBOSE) {
				if (up_cpu_index() == current_case->core_id) {
					printf("        |_ Testcase %d - %s (Radius: %d)\n", current_case->testcase_id,
				       current_case->name, current_case->radius);
				}
				current_case = current_case->next;
			}

			current_suite = current_suite->next;
		}

		current_group = current_group->next;
	}
}

void setUp(void)
{
	return;
}

void tearDown(void)
{
	return;
}

void run_test_case(test_case_s *test_case)
{
	if ((test_case->test) && (up_cpu_index() == test_case->core_id)) {
		printf("        Test: %s ", test_case->name);
		UnityDefaultTestRun(test_case->test, test_case->name, 0);
	}
}

void run_test_suite(test_suite_s *test_suite, ut_parameters_s *para)
{
	if (test_suite->setup) {
		test_suite->setup();
	}

	test_case_s *current = test_suite->test_cases;

	while (current != NULL) {
		if (para->radius == -1) {
			if (para->testcase_num == 0 || para->testcase_num == current->testcase_id) {
				run_test_case(current);
			}
		} else {
			if ((para->testcase_num == 0 || para->testcase_num == current->testcase_id) &&
			    (para->radius == current->radius)) {
				run_test_case(current);
			}
		}
		current = current->next;
	}

	if (test_suite->teardown) {
		test_suite->teardown();
	}
}

void run_test_group(test_group_s *test_group, ut_parameters_s *para)
{
	if (test_group->setup) {
		test_group->setup();
	}

	test_suite_s *current = test_group->test_suites;

	while (current != NULL) {
		if (para->suite_num == 0 || current->suite_id == para->suite_num) {
			printf("    Suite %d: %s\n", current->suite_id, current->name);
			run_test_suite(current, para);
		}
		current = current->next;
	}

	if (test_group->teardown) {
		test_group->teardown();
	}
}

void run_all_tests(test_group_s *p_tg, ut_parameters_s *para)
{
	test_group_s *current = p_tg;

	printf("\n--- Running Tests ---\n\n");
	UnityBegin(" ");
	while (current != NULL) {
		if (para->group_num == 0 || para->group_num == current->group_id) {
			printf("Group %d: %s\n", current->group_id, current->name);
			run_test_group(current, para);
		}
		current = current->next;
	}
	printf("\nSummary");
	UnityEnd();
}

int print_unity_help(char *inputs)
{
	fprintf(stderr, "Error: Unknown ut command [%s]\n", inputs);
	printf("Usage: ut show [verbose]\n");
	printf("       ut run [-g group_num] [-s suite_num] [-t testcase_num] [-r testcase_radius]\n");
	free(inputs);
	return RET_STATUS_UNKNOWN_COMMAND;
}

char *get_input_commands(int argc, char *argv[])
{
	int total_length = 1;

	// 计算总长度，包括空格和字符串终止符
	for (int i = 0; i < argc; i++) {
		total_length += strlen(argv[i]) + 1;
	}

	char *concatenated_args = (char *)malloc(total_length * sizeof(char));

	if (concatenated_args == NULL) {
		fprintf(stderr, "Failed to allocate memory\n");
		return NULL;
	}

	concatenated_args[0] = '\0'; // 初始化为空字符串

	for (int i = 0; i < argc; i++) {
		strcat(concatenated_args, argv[i]);
		if (i < argc - 1) {
			strcat(concatenated_args, " "); // 添加空格（最后一个元素后不加）
		}
	}

	return concatenated_args;
}

int parse_command(int argc, char *argv[], ut_parameters_s *para)
{
	int ret = RET_STATUS_OK;
	char *input_commands = get_input_commands(argc, argv);

	para->radius = -1;

	if (input_commands != NULL) {
		if (argc > 1 && strcmp(argv[1], "run") == 0) {
			para->status = STATUS_RUN;
			if (argc % 2 == 0) {
				if (argc == 2) {
					para->group_num = 0;
				} else {
					for (int i = 2; i < argc; i += 2) {
						if (strcmp(argv[i], "-G") == 0 || strcmp(argv[i], "-g") == 0) {
							para->group_num = atoi(argv[i + 1]);
						} else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "-s") == 0) {
							para->suite_num = atoi(argv[i + 1]);
						} else if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "-t") == 0) {
							para->testcase_num = atoi(argv[i + 1]);
						} else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "-r") == 0) {
							para->radius = atoi(argv[i + 1]);
						} else {
							ret = print_unity_help(input_commands);
						}
					}
				}
			} else {
				ret = print_unity_help(input_commands);
			}
		} else if (argc > 1 && strcmp(argv[1], "show") == 0) {
			if (argc > 2) {
				if (strcmp(argv[2], "verbose") == 0) {
					para->status = STATUS_SHOW_VERBOSE;
				} else {
					ret = print_unity_help(input_commands);
				}
			} else {
				para->status = STATUS_SHOW;
			}
		} else if (argc > 1 && strcmp(argv[1], "help") == 0) {
			ret = print_unity_help(input_commands);
		} else {
			ret = print_unity_help(input_commands);
		}
	} else {
		ret = RET_STATUS_MEMORY_ALLOC_FAILED;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = RET_STATUS_OK;
	ut_parameters_s para = { 0 };

	unity_init();

	ret = parse_command(argc, argv, &para);

	if (ret == RET_STATUS_OK) {
		test_group_s *p_tg = g_test_groups;

		switch (para.status) {
			case STATUS_SHOW:
				show_test_structure(p_tg, STATUS_SHOW);
				break;

			case STATUS_SHOW_VERBOSE:
				show_test_structure(p_tg, STATUS_SHOW_VERBOSE);
				break;

			case STATUS_RUN:
				run_all_tests(p_tg, &para);
				break;

			default:
				ret = RET_STATUS_UNKNOWN_COMMAND;
				break;
		}
	}

	unity_deinit();

	return ret;
}
