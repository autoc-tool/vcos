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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <nuttx/config.h>
#include <nuttx/note/handshake_driver.h>
#include <nuttx/note/noteram_driver.h>

#include "Compiler.h"
#include "Std_Types.h"
#include "syslog.h"
#include "trace.h"
#include "trace_eth_wrapper.h"

/****************************************************************************
 * Private Struct
 ****************************************************************************/

typedef struct {
	uint16_t start_flag;   /* start flag */
	uint16_t version;      /* protocol version */
	uint16_t data_length;  /* data length */
	uint16_t sequence;     /* sequence number */
	uint16_t reserved;     /* reserve */
	uint16_t header_crc;   /* crc value */
} ProtocolHeader;

typedef struct {
	bool is_open;  /* file status*/
	int fd;        /* file descriptor*/
	uint8_t *buf;  /* trace buffer */
} TraceContext;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TRACE_DATA_BUFFER_SIZE   1472    /* trace data buffer max size*/
#define TRACE_SEND_THRESHOLD     1024     /* trace data sen threshold */
#define TRACE_START_MAGIC        0xA5A5   /* trace data header start flag*/
#define TRACE_HEADER_SIZE        sizeof(ProtocolHeader)
#define TRACE_PROTOCOL_VERSION   0x0201   /* trace protocol version ,current version 2.1 */
#define HANDSHAKE_FORMAT         "IDX                      NAME CPU PID PRI PER TYPE CMDID LEN\n"
#define HANDSHAKE_VALUE          "%3d %25s %3d %3" PRIu32 " %3d %3" PRIu32 " %4d %5d %3d\n"
#define TRACE_HANDSHAKE_BUFFER_SIZE  (CONFIG_NOTE_HANDSHAKE_BUFFSIZE + TRACE_HEADER_SIZE)

/****************************************************************************
 * Private Data
 ****************************************************************************/
#ifdef TRACE_VAR_SECTION
#define TRACE_START_SEC_MULTI_APP_BSS
#include "trace_memmap.h"
VAR(int, TRACE_MULTI_APP_BSS) handshake_idx;
VAR(uint8_t, TRACE_MULTI_APP_BSS) handshake_data[TRACE_HANDSHAKE_BUFFER_SIZE];
VAR(bool, TRACE_MULTI_APP_BSS) handshake_data_exist;
VAR(int, TRACE_MULTI_APP_BSS) handshake_data_len;
static VAR(uint16_t, TRACE_MULTI_APP_BSS) sequence;
#if defined(__TASKING__)
#pragma align 4
#endif
static VAR(uint8_t, TRACE_MULTI_APP_BSS) tracedata[TRACE_DATA_BUFFER_SIZE];
#if defined(__TASKING__)
#pragma align restore
#endif
#define TRACE_STOP_SEC_MULTI_APP_BSS
#include "trace_memmap.h"

#define TRACE_START_SEC_MULTI_APP_DATA
#include "trace_memmap.h"
static VAR(TraceContext, TRACE_MULTI_APP_DATA) context = {
	.is_open = false,
	.fd = -1,
	.buf = tracedata
};
#define TRACE_STOP_SEC_MULTI_APP_DATA
#include "trace_memmap.h"
#else
int handshake_idx = 0;
uint8_t handshake_data[TRACE_HANDSHAKE_BUFFER_SIZE] = {0};
bool handshake_data_exist = false;
int handshake_data_len = 0;


/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint16_t sequence = 0;
static uint8_t tracedata[TRACE_DATA_BUFFER_SIZE] = {0};
static TraceContext context = {
	.is_open = false,
	.fd = -1,
	.buf = tracedata
};
#endif

/****************************************************************************
 * Name: trace_open_device
 *
 * Description: Open trace device and initialize context.
 *
 ****************************************************************************/

static int trace_open_device(void)
{
	/* file is opened */
	if (context.is_open == true) {
		return 0;
	}

	/* open file */
	context.fd = open("/dev/note/ram", O_RDONLY);
	if (context.fd < 0) {
		syslog(LOG_ERR, "trace: cannot open /dev/note/ram\n");
		return ERROR;
	}

	/* set read mode */
	unsigned int mode = NOTERAM_MODE_READ_BINARY;

	if (ioctl(context.fd, NOTERAM_SETREADMODE, &mode) < 0) {
		syslog(LOG_ERR, "trace: set read mode failed\n");
		close(context.fd);
		return ERROR;
	}

	context.is_open = true;
	sequence = 0;
	return OK;
}

/****************************************************************************
 * Name: trace_close_device
 *
 * Description:
 *   Close the trace device and reset the context status.
 *
 ****************************************************************************/

static void trace_close_device(void)
{
	if (context.is_open == true) {
		close(context.fd);
		context.fd = -1;
		context.is_open = false;
	}
}

/****************************************************************************
 * Name: trace_build_header
 *
 * Description:
 *   Build the protocol header for trace data packets.
 *   Fills the ProtocolHeader structure with the start flag, version,
 *   data length, and sequence number.
 *
 ****************************************************************************/

bool trace_build_header(ProtocolHeader *hdr, uint16_t data_len, uint16_t *seq)
{
	if (hdr == NULL || seq == NULL) {
		return false;
	}
	hdr->start_flag = TRACE_START_MAGIC;
	hdr->version = TRACE_PROTOCOL_VERSION;
	hdr->data_length = data_len;
	hdr->sequence = (*seq)++;
	return true;
}

/****************************************************************************
 * Name: trace_send_encapsulated_packet
 *
 * Description:
 *   Build the protocol header, encapsulate the trace data, and send it via UDP.
 *   This function fills the header, appends the data, and transmits the packet.
 *
 ****************************************************************************/

int trace_send_encapsulated_packet(uint8_t *buff, uint16_t data_len, uint16_t *seq)
{
	int ret = OK;

	if (buff == NULL || seq == NULL) {
		return ERROR;
	}
	if (trace_build_header((ProtocolHeader *)buff, data_len, seq) != true) {
		syslog(LOG_ERR, "trace: build header error\n");
		ret = ERROR;
	}
	ret = eth_udpdata_send(buff, data_len);
	if (ret != OK) {
		syslog(LOG_ERR, "trace: send udp data error, error code: %d\n", ret);
	}
	return ret;
}

/****************************************************************************
 * Name: trace_handshake_out
 *
 * Description:
 *   dump and send handshake info
 *
 ****************************************************************************/
static Std_ReturnType trace_handshake_out(uint8_t *buff, int len)
{
	if (buff == NULL) {
		return E_NOT_OK;
	}

	Std_ReturnType ret = E_OK;
#ifdef CONFIG_NOTE_HANDSHAKE_OUT_UART
	handshake_idx = 0;
	int len_done = 0;
	struct ftrace_taskinfo_cmd_type *handshake_info =
		(struct ftrace_taskinfo_cmd_type *)(buff + TRACE_HEADER_SIZE);

	while (true) {
		if (len_done >= len) {
			break;
		}

		handshake_info++;
		len_done += sizeof(struct ftrace_taskinfo_cmd_type);
	}
#endif

#ifdef CONFIG_NOTE_HANDSHAKE_OUT_ETH

	ret = trace_send_encapsulated_packet(buff, len + TRACE_HEADER_SIZE, &sequence);

#endif
	return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * Name: handshake_main_func
 *
 * Description:
 *   collect and send trace handshake info
 *
 ****************************************************************************/

void handshake_main_func(void)
{
	struct lib_memoutstream_s stream;

	if (handshake_core_get() == up_cpu_index() &&
		handshake_data_exist != true) {
		lib_memoutstream(&stream, (FAR char *)(handshake_data + TRACE_HEADER_SIZE),
						CONFIG_NOTE_HANDSHAKE_BUFFSIZE);

		/* total_len = 0 if not work */

		int total_len = handshake_task_info_get(&stream);

		if (total_len > 0) {
			handshake_data_len = total_len;
			handshake_data_exist = true;
		}
	}

	if (up_cpu_index() == 0 && handshake_data_exist == true) {
		if (trace_handshake_out(handshake_data, handshake_data_len) == E_OK) {
			handshake_data_len = 0;
			handshake_data_exist = false;
		}
	}
}

/****************************************************************************
 * Name: trace_dumpudp_main_func
 *
 * Description:
 *   read and send trace event data
 *
 ****************************************************************************/

void trace_dumpudp_main_func(void)
{
	bool is_transmitting = trace_get_transmitting();
	int note_len;
	int ret = OK;
	size_t s_accumulate_len = 0;
	uint8_t send_cycle = 0;

	/* Open note for read */

	if (is_transmitting == true) {
		ret = trace_open_device();
	}

	/* Open file failed */

	if (ret != OK) {
		return;
	}

	/* Read and output all notes */

	while ((is_transmitting == true) && (ret == 0) && (send_cycle <= CONFIG_TRACE_READ_TIMES_MAX)) {
		send_cycle++;
		/* remain space*/
		size_t reminning_space = TRACE_DATA_BUFFER_SIZE - TRACE_HEADER_SIZE - s_accumulate_len;

		note_len = read(context.fd, context.buf + TRACE_HEADER_SIZE + s_accumulate_len, reminning_space);

		if (note_len < 0 || note_len > reminning_space) {
			syslog(LOG_ERR, "trace: read error: %d, errno:%d\n", note_len, errno);
			continue;
		} else if (note_len == 0) {
			break;
		}

		/* update accumulate length */

		s_accumulate_len += note_len;

		/* update accumulate length */

		if (s_accumulate_len >= TRACE_SEND_THRESHOLD) {
			ret = trace_send_encapsulated_packet(context.buf,
				s_accumulate_len + TRACE_HEADER_SIZE, &sequence);
			s_accumulate_len = 0;
		}
	}

	/* send legacy data */

	if (s_accumulate_len > 0) {
		ret = trace_send_encapsulated_packet(context.buf,
			s_accumulate_len + TRACE_HEADER_SIZE, &sequence);
		s_accumulate_len = 0;
	}

	/* Close note */

	if (is_transmitting == false) {
		trace_close_device();
	}

	return;
}
