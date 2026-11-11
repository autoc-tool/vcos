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

#include "unity.h"

void test_demo_01(void)
{
	int a = 1;
	printf("this is test demo 01\n");
	TEST_ASSERT_TRUE(a == 1);
}

void test_demo_02(void)
{
	int b = 2;
	printf("this is test demo 02\n");
	TEST_ASSERT_FALSE(b == 1);
}
