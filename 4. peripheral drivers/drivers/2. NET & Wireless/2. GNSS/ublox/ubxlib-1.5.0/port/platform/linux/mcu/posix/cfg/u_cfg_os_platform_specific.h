/*
 * Copyright 2019-2024 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _U_CFG_OS_PLATFORM_SPECIFIC_H_
#define _U_CFG_OS_PLATFORM_SPECIFIC_H_

/* Only header files representing a direct and unavoidable
 * dependency between the API of this module and the API
 * of another module should be included here; otherwise
 * please keep #includes to your .c files. */

/** @file
 * @brief This header file contains OS configuration information for
 * Linux.
 */

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS FOR LINUX: HEAP
 * -------------------------------------------------------------- */

/** \deprecated Not stricty speaking part of the OS but there's nowhere
 * better to put this. Set this to 1 if the C library does not free memory
 * that it has alloced internally when a task is deleted.
 * For instance, newlib when it is compiled in a certain way
 * does this on some platforms.
 *
 * This macro is retained for compatibility purposes but is now
 * ALWAYS SET TO 0 and may be removed in future.
 *
 * There is a down-side to setting this to 1, which is that URCs
 * received from a module will not be printed-out by the AT client
 * (since prints from a dynamic task often cause such leaks), and
 * this can be a pain when debugging, so please set this to 0 if you
 * can.
 */
#define U_CFG_OS_CLIB_LEAKS 0

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS FOR LINUX: OS GENERIC
 * -------------------------------------------------------------- */

#ifndef U_CFG_OS_PRIORITY_MIN
/** The minimum task priority. Low numbers indicate lower priority.
 * Within the porting layer this is mapped to priority -2.
 */
# define U_CFG_OS_PRIORITY_MIN 0
#endif

#ifndef U_CFG_OS_PRIORITY_MAX
/** The maximum task priority.  Within the porting layer this is
 * mapped to priority +2.
 */
# define U_CFG_OS_PRIORITY_MAX 99
#endif

#ifndef U_CFG_OS_YIELD_MS
/** The amount of time to block for to ensure that a yield
 * occurs.
 */
# define U_CFG_OS_YIELD_MS 1
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS FOR LINUX: STACK SIZES/PRIORITIES
 * -------------------------------------------------------------- */

/** How much stack the task running all the examples and tests needs
 * in bytes, plus slack for the users own code.
 */
#define U_CFG_OS_APP_TASK_STACK_SIZE_BYTES (1024 * 8)

/** The priority of the task running the examples and tests: can be
 * middling on Linux where there are few constraints.
 */
#define U_CFG_OS_APP_TASK_PRIORITY   7

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS FOR LINUX: OS RESOURCE MONITORING
 * -------------------------------------------------------------- */

#ifndef U_CFG_OS_RESOURCES_PER_SEMAPHORE
/** A semaphore requires mutex protection, hence this is 2.
 */
# define U_CFG_OS_RESOURCES_PER_SEMAPHORE 2
#endif

#ifndef U_CFG_OS_MALLOCS_PER_TASK
/** A task's heap is allocated with a call to pUPortMalloc(), hence
 * this is 1.
 */
# define U_CFG_OS_MALLOCS_PER_TASK 1
#endif

#ifndef U_CFG_OS_MALLOCS_PER_QUEUE
/** Memory for a queue is allocated with a call to pUPortMalloc(),
 * hence this is 1.
 */
# define U_CFG_OS_MALLOCS_PER_QUEUE 1
#endif

#ifndef U_CFG_OS_MALLOCS_PER_MUTEX
/** Memory for a mutex is allocated with a call to pUPortMalloc(),
 * hence this is 1.
 */
# define U_CFG_OS_MALLOCS_PER_MUTEX 1
#endif

#ifndef U_CFG_OS_MALLOCS_PER_SEMAPHORE
/** Memory for a semaphore is allocated with a call to pUPortMalloc(),
 * hence this is 1.
 */
# define U_CFG_OS_MALLOCS_PER_SEMAPHORE 1
#endif

#ifndef U_CFG_OS_MALLOCS_PER_TIMER
/** Memory for a timer is allocated with a call to pUPortMalloc(),
 * hence this is 1.
 */
# define U_CFG_OS_MALLOCS_PER_TIMER 1
#endif

#endif // _U_CFG_OS_PLATFORM_SPECIFIC_H_

// End of file
