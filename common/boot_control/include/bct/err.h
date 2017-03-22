/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * Copyright (c) 2014-2016, NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */


#ifndef __ERR_H
#define __ERR_H

#include <sys/types.h> // for status_t

#define NO_ERROR 0
#define ERR_GENERIC -1
#define ERR_NOT_FOUND -2
#define ERR_NOT_READY -3
#define ERR_NO_MSG -4
#define ERR_NO_MEMORY -5
#define ERR_ALREADY_STARTED -6
#define ERR_NOT_VALID -7
#define ERR_INVALID_ARGS -8
#define ERR_NOT_ENOUGH_BUFFER -9
#define ERR_NOT_SUSPENDED -10
#define ERR_OBJECT_DESTROYED -11
#define ERR_NOT_BLOCKED -12
#define ERR_TIMED_OUT -13
#define ERR_ALREADY_EXISTS -14
#define ERR_CHANNEL_CLOSED -15
#define ERR_OFFLINE -16
#define ERR_NOT_ALLOWED -17
#define ERR_BAD_PATH -18
#define ERR_ALREADY_MOUNTED -19
#define ERR_IO -20
#define ERR_NOT_DIR -21
#define ERR_NOT_FILE -22
#define ERR_RECURSE_TOO_DEEP -23
#define ERR_NOT_SUPPORTED -24
#define ERR_TOO_BIG -25
#define ERR_THREAD_DETACHED -26
#define ERR_NOT_CONFIGURED -27
#define ERR_DEPENDENCY_FAIL -28
#define ERR_XUSB_DEV_NOT_ATTACHED -29
#define ERR_XUSB_PORT_RESET_FAILED -30
#define ERR_XUSB_EP_STALL -31
#define ERR_XUSB_EP_NOT_READY -32
#define ERR_XUSB_EP_ERR -33
#define ERR_XUSB_DEV_RESPONSE_ERR -34
#define ERR_XUSB_RETRY -35
#define ERR_THREAD_TERMINATED -36
#define ERR_ROLLBACK_AVOIDANCE -37

#define GOTO_FAIL_ERROR(expr)										\
	do {																\
		err = (expr);													\
		if (err != NO_ERROR)											\
			goto fail;													\
	} while (0)

#define RETURN_ERROR(expr)											\
	do {																	\
		err = (expr);														\
		if (err != NO_ERROR)												\
			return err;														\
	} while (0)

#define PRINT_ERROR(expr, err_msg)									\
	do {																	\
		err = (expr);														\
		if (err != NO_ERROR) { 												\
			dprintf(CRITICAL, "%s with error %d in %s func at %d line\n",	\
					err_msg, err, __func__, __LINE__);						\
			goto fail;														\
		}																	\
	} while (0)

#endif
