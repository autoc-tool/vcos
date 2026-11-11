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

#include "dem_internal.h"
#include "Compiler.h"

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

FUNC(boolean, DEM_CODE)
dem_atomic_cmp_swap(volatile dem_atomic_word_type *address_ptr, const dem_atomic_word_type old_value,
		    const dem_atomic_word_type new_value)
{
	return (boolean)Os_AtomicCmpSwap((Atomic *)address_ptr, (sint32)old_value, (sint32)new_value);
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"