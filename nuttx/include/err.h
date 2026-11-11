/****************************************************************************
 * include/err.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_ERR_H
#define __INCLUDE_ERR_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdarg.h>
#include <nuttx/compiler.h>
#include <errno.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* Append _func suffix to avoid the penitential symbol collision */

#define warn   warn_func
#define vwarn  vwarn_func
#define warnx  warnx_func
#define vwarnx vwarnx_func

#define err    err_func
#define verr   verr_func
#define errx   errx_func
#define verrx  verrx_func

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Print "pid: ", FORMAT, ": ", the standard error string for errno,
 * and a newline, on stderr.
 */

void warn(FAR const char *fmt, ...) printf_like(1, 2);
void vwarn(FAR const char *fmt, va_list ap) printf_like(1, 0);

/* Likewise, but without ": " and the standard error string.  */

void warnx(FAR const char *fmt, ...) printf_like(1, 2);
void vwarnx(FAR const char *fmt, va_list ap) printf_like(1, 0);

/* Likewise, and then exit with STATUS.  */

void err(int status, FAR const char *fmt, ...) printf_like(2, 3);
void verr(int status, FAR const char *fmt, va_list ap) printf_like(2, 0);
void errx(int status, FAR const char *fmt, ...) printf_like(2, 3);
void verrx(int status, FAR const char *fmt, va_list ap) printf_like(2, 0);

#ifndef __ASSEMBLY__

/**
 * IS_ERR_VALUE - Detect an error pointer.
 * @x: The pointer to check.
 *
 * Like IS_ERR(), but does not generate a compiler warning
 * if result is unused.
 */

#define IS_ERR_VALUE(x) unlikely((unsigned long)(void *)(x) >= (unsigned long)-__ELASTERROR)

/**
 * ERR_PTR - Create an error pointer.
 * @error: A negative error code.
 *
 * Encodes @error into a pointer value. Users should consider the result
 * opaque and not assume anything about how the error is encoded.
 *
 * Return: A pointer with @error encoded within its value.
 */

static inline void *ERR_PTR(long error)
{
  return (void *) error;
}

/**
 * PTR_ERR - Extract the error code from an error pointer.
 * @ptr: An error pointer.
 * Return: The error code within @ptr.
 */

static inline long PTR_ERR(const void *ptr)
{
  return (long) ptr;
}

/**
 * IS_ERR - Detect an error pointer.
 * @ptr: The pointer to check.
 * Return: true if @ptr is an error pointer, false otherwise.
 */

static inline bool IS_ERR(const void *ptr)
{
  return IS_ERR_VALUE((unsigned long)ptr);
}

/**
 * IS_ERR_OR_NULL - Detect an error pointer or a null pointer.
 * @ptr: The pointer to check.
 *
 * Like IS_ERR(), but also returns true for a null pointer.
 */

static inline bool IS_ERR_OR_NULL(const void *ptr)
{
  return unlikely(!ptr) || IS_ERR_VALUE((unsigned long)ptr);
}

/**
 * ERR_CAST - Explicitly cast an error-valued pointer to another pointer type
 * @ptr: The pointer to cast.
 *
 * Explicitly cast an error-valued pointer to another pointer type in such a
 * way as to make it clear that's what's going on.
 */

static inline void *ERR_CAST(const void *ptr)
{
  /* cast away the const */

  return (void *) ptr;
}

/**
 * PTR_ERR_OR_ZERO - Extract the error code from a pointer if it has one.
 * @ptr: A potential error pointer.
 *
 * Convenience function that can be used inside a function that returns
 * an error code to propagate errors received as error pointers.
 * For example, ``return PTR_ERR_OR_ZERO(ptr);`` replaces:
 *
 * .. code-block:: c
 *
 *  if (IS_ERR(ptr))
 *    return PTR_ERR(ptr);
 *  else
 *    return 0;
 *
 * Return: The error code within @ptr if it is an error pointer; 0 otherwise.
 */

static inline int  PTR_ERR_OR_ZERO(const void *ptr)
{
  if (IS_ERR(ptr))
    return PTR_ERR(ptr);
  else
    return 0;
}

#endif

#ifdef __cplusplus
#undef EXTERN
}
#endif

#endif /* __INCLUDE_ERR_H */
