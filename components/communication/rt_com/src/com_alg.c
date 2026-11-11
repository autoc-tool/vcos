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

/* including file */
#include <string.h>
#include "com.h"
#include "com_internal.h"
#include "com_alg.h"
#include "Compiler.h"
#include "com_vcos_log.h"
#include "vcos_log.h"
#include "SchM_Com.h"
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur.h"
#include "pdur_com.h"
#endif

/****************************************************************************
 * name: COM_SIGNAL_BITOFFSET
 *
 * description: COM_SIGNAL_BITOFFSET
 *
 ****************************************************************************/
#define COM_SIGNAL_BITOFFSET(pos) (uint8)((pos)&7u)

/****************************************************************************
 * name: COM_SIGNAL_STARTBYTE
 *
 * description: COM_SIGNAL_STARTBYTE
 *
 ****************************************************************************/
#define COM_SIGNAL_STARTBYTE(pos) (uint8)((pos) >> 3u)

#define COM_ALG_SIGNAL_64BIT (64u)

#define COM_ALG_SIGNAL_32BIT (32u)

#define COM_ALG_SIGNAL_SHIFT_8BIT (8u)

#define COM_ALG_SIGNAL_8BYTE (8u)

#define COM_ALG_SIGNAL_8BYTE_VALUE (0xFFFFFFFFFFFFFFFFu)

#define COM_ALG_SIGNAL_4BYTE_VALUE (0xFFFFFFFFu)

/****************************************************************************
 * name: com_alg_rx_process_check_invalid_signal
 *
 * description: check signal value == invalid value in rx process.
 *
 ****************************************************************************/
#define COM_START_SEC_CODE
#include "com_memmap.h"

#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
static FUNC(com_invalid_signal_action_type, COM_CODE)
	com_alg_rx_process_check_invalid_signal(const com_rx_signal_type *signal,
						const com_self_rx_ipdu_type *self_ipdu, const com_rx_ipdu_type *ipdu,
						const PduInfoType *pdu_info_ptr);
#endif

/**
 * Determines whether a communication algorithm should be triggered
 * based on the transfer property and whether the data has changed.
 *
 * @param transfer_property  The property type governing the transfer behavior.
 *                          Possible values:
 *                          - COM_TRIGGERED
 *                          - COM_TRIGGERED_WITHOUT_REPETITION
 *                          - COM_TRIGGERED_ON_CHANGE
 *                          - COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION
 * @param data_changed      Indicates if the relevant data has changed since last transfer.
 * @return                  TRUE if the communication should be triggered, FALSE otherwise.
 *
 * Transfer behaviors:
 * - If the transfer property is COM_TRIGGERED or COM_TRIGGERED_WITHOUT_REPETITION,
 *   always trigger the transfer.
 * - If the transfer property is COM_TRIGGERED_ON_CHANGE or COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION,
 *   only trigger the transfer if the data has changed.
 * - For any other transfer property, do not trigger the transfer.
 */
static boolean com_alg_should_trigger(com_transfer_property_type transfer_property, boolean data_changed)
{
	switch (transfer_property) {
	case COM_TRIGGERED:
	case COM_TRIGGERED_WITHOUT_REPETITION:
		return TRUE;
	case COM_TRIGGERED_ON_CHANGE:
	case COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION:
		return data_changed;
	default:
		return FALSE;
	}
}

/**
 * Gets the number of repetitions for a given transfer property and transmission mode.
 *
 * For transfer properties COM_TRIGGERED and COM_TRIGGERED_ON_CHANGE, the number of repetitions
 * is taken from the transmission mode configuration (if specified as greater than 0).
 * For all other transfer properties, the function returns 1 by default (no repetition).
 *
 * @param tx_mode_ptr         Pointer to the transmission mode configuration structure.
 * @param transfer_property   Transfer property type indicating the transmission trigger behavior.
 * @return                    Number of repetitions to perform for this transmission cycle.
 */
static uint8 com_alg_get_repetitions(const com_tx_mode_type *tx_mode_ptr, com_transfer_property_type transfer_property)
{
	uint8 repetitions = 1u;

	if ((transfer_property == COM_TRIGGERED) || (transfer_property == COM_TRIGGERED_ON_CHANGE)) {
		if (tx_mode_ptr->com_tx_mode_repetitions_num > 0u) {
			repetitions = tx_mode_ptr->com_tx_mode_repetitions_num;
		}
	}
	return repetitions;
}

/**
 * Handles the triggered transmission for a Tx I-PDU, updating repetition and deadline counters.
 *
 * If the current transmission mode is COM_DIRECT or COM_MIXED,
 * and the remaining repetition count is less than or equal to the required repetitions,
 * this function updates the repetition counter, confirmation counter (if deadline monitoring is used),
 * and resets the deadline monitoring timer.
 *
 * @param self_ipdu      Pointer to the Tx I-PDU state structure.
 * @param tx_mode_ptr    Pointer to the transmission mode configuration structure.
 * @param repetitions    Number of repetitions required for this triggered transmission.
 */
static void com_alg_handle_triggered_tx(com_self_tx_ipdu_type *self_ipdu, const com_tx_mode_type *tx_mode_ptr,
					uint8 repetitions)
{
	com_tx_mode_mode_type mode = tx_mode_ptr->com_tx_mode_mode;

	if ((mode == COM_DIRECT) || (mode == COM_MIXED)) {
		if (self_ipdu->com_txipdu_repetitions_left_num <= repetitions) {
			self_ipdu->com_txipdu_repetitions_left_num = repetitions;
			if (self_ipdu->com_tx_deadline_cnt > 0u) {
				self_ipdu->com_txipdu_tx_confirmations_num = repetitions;
				self_ipdu->com_tx_dm_timer = self_ipdu->com_tx_deadline_cnt;
			}
		}
	}
}

/**
 * Checks the conditions and, if appropriate, triggers transmission for the specified Tx I-PDU.
 *
 * This function performs several checks:
 * - Ensures that configuration pointers and runtime objects are valid.
 * - Checks that the I-PDU transmission has been started.
 * - Determines if transmission should be triggered based on the transfer property and data change flag.
 * - If transmission should be triggered, calculates the repetition count and handles the triggered transmission.
 *
 * @param pdu_handle_id      Handle ID of the Tx I-PDU to check/trigger.
 * @param data_changed       Flag indicating whether the data to be transmitted has changed.
 * @param transfer_property  Type of transfer property that dictates transmission behavior.
 * @return                   TRUE if transmission was triggered, FALSE otherwise.
 */
FUNC(boolean, COM_CODE)
com_alg_trigger_tx_on_conditions(uint16 pdu_handle_id, boolean data_changed,
				 com_transfer_property_type transfer_property)
{
	const com_tx_ipdu_type *ipdu = COM_GET_TXIPDU(pdu_handle_id);
	com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(pdu_handle_id);
	boolean ret = FALSE;

	if ((ipdu == NULL) || (self_ipdu == NULL) || (ipdu->com_tx_ipdu == NULL)) {
		return ret;
	}

	if (self_ipdu->com_ipdu_started != TRUE) {
		return ret;
	}

	const com_ipdu_tx_type *com_ipdu_tx_ram = ipdu->com_tx_ipdu;
	const com_tx_mode_type *tx_mode_ptr =
		self_ipdu->com_txipdu_tx_mode ? com_ipdu_tx_ram->com_tx_mode_true : com_ipdu_tx_ram->com_tx_mode_false;

	boolean trigger = com_alg_should_trigger(transfer_property, data_changed);

	if (trigger) {
		uint8 repetitions = com_alg_get_repetitions(tx_mode_ptr, transfer_property);

		com_alg_handle_triggered_tx(self_ipdu, tx_mode_ptr, repetitions);
		ret = TRUE;
	}

	return ret;
}

/**
 * Copies signal group data from the shadow buffer to the Rx PDU, performing masked merging.
 *
 * For each byte in the Rx PDU, this function merges data from the PDU and the shadow buffer using a mask:
 * - Where the mask bit is set, data is taken from the shadow buffer.
 * - Where the mask bit is clear, data is taken from the current PDU data.
 *
 * If the copied value is different from the original PDU data, and @p data_changed is not NULL,
 * sets *data_changed to TRUE to indicate that the data actually changed as a result of this copy.
 *
 * @param signal_group_id               Signal group ID whose shadow buffer will be copied to the PDU.
 * @param deferred_buffer_destination   (Unused) Indicates if destination buffer is deferred.
 * @param data_changed                  If not NULL, is set to TRUE if any byte in PDU data changed.
 */
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
FUNC(void, COM_CODE)
com_alg_copy_sg_data_from_sb_to_rx_pdu(const com_signal_id_type signal_group_id, boolean deferred_buffer_destination,
				       boolean *data_changed)
{
	(void)deferred_buffer_destination;
	const com_rx_signal_type *signal = COM_GET_RXSIGNAL(signal_group_id);
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;

	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(com_ipdu_handle_id_ram);
	uint16 com_ipdu_size_ram = ipdu->com_ipdu_size;
	const com_self_rx_signal_type *self_signal = com_get_self_rx_signal(signal_group_id);

	uint8 *pdu_data_ptr = NULL;

	pdu_data_ptr = COM_GET_SELF_RXIPDU(com_ipdu_handle_id_ram)->com_ipdu_data_ptr;

	const uint8 *buf = (const uint8 *)self_signal->com_self_shadow_buffer;
	uint8 data = 0u;

	if (data_changed != NULL) {
		*data_changed = FALSE;
	}
	for (uint16 i = 0u; i < com_ipdu_size_ram; i++) {
		data = (~signal->com_self_shadow_buffer_mask[i] & (*pdu_data_ptr)) |
		       (signal->com_self_shadow_buffer_mask[i] & (*buf));
		if ((*pdu_data_ptr != data) && (data_changed != NULL)) {
			*data_changed = TRUE;
		}
		*pdu_data_ptr = data;
		buf++;
		pdu_data_ptr++;
	}
}
#endif

/**
 * Copies signal group data from the shadow buffer to the Tx PDU, using a byte mask for each byte.
 *
 * For each byte in the PDU, merge the current value and the value from the shadow buffer
 * according to the mask: bits set in the mask are replaced by those from the shadow buffer,
 * and other bits remain unchanged from the original PDU.
 *
 * If any byte in the PDU is changed, sets *data_changed to TRUE; otherwise sets it to FALSE.
 *
 * The copy operation is done in a critical section to avoid concurrent access issues.
 *
 * @param signal_group_id  The signal group whose shadow buffer is to be copied to the Tx PDU.
 * @param data_changed     Pointer to boolean; set to TRUE if any data in the PDU changes after the copy.
 */
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
FUNC(void, COM_CODE)
com_alg_copy_sg_data_from_sb_to_tx_pdu(const com_signal_id_type signal_group_id, boolean *data_changed)
{
	const com_tx_signal_type *signal = COM_GET_TXSIGNAL(signal_group_id);
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;
	const com_tx_ipdu_type *ipdu = COM_GET_TXIPDU(com_ipdu_handle_id_ram);
	uint16 com_ipdu_size_ram = ipdu->com_ipdu_size;
	const com_self_tx_signal_type *self_signal = com_get_self_tx_signal(signal_group_id);

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	uint8 *pdu_data_ptr = COM_GET_SELF_TXIPDU(com_ipdu_handle_id_ram)->com_ipdu_data_ptr;
	const uint8 *buf = (const uint8 *)self_signal->com_self_shadow_buffer;
	*data_changed = FALSE;

	uint16 i = 0u;

	while (i < com_ipdu_size_ram) {
		uint8 mask = signal->com_self_shadow_buffer_mask[i];
		uint8 orig = pdu_data_ptr[i];
		uint8 new_val = (orig & ~mask) | (buf[i] & mask);

		if (orig != new_val) {
			*data_changed = TRUE;
		}
		pdu_data_ptr[i] = new_val;
		i++;
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
}
#endif

/**
 * Processes each group signal within a signal group by copying its value from the shadow buffer.
 *
 * This function iterates over all group signals referenced by the `signal_group` parameter,
 * and for each group signal, extracts its value from the shadow buffer. The actual signal
 * processing is delegated to com_alg_processing_rx_signal_data_from_pdu.
 * The call is performed within a critical section to protect against concurrent access.
 *
 * @param signal_group   Pointer to the signal group configuration structure.
 * @param self_signal    Pointer to the runtime information (shadow buffer) of the signal group.
 * @param core_map_idx   Core index for multi-core mapping.
 */
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
static FUNC(void, COM_CODE)
	com_alg_process_group_signal_from_shadow_buffer(const com_rx_signal_type *signal_group,
							com_self_rx_signal_type *self_signal, uint8 core_map_idx)
{
	const uint8 *buf = (const uint8 *)self_signal->com_self_shadow_buffer;
	const com_rx_signal_type *group_signal = NULL;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	for (uint16 i = 0u; signal_group->com_group_signal[i] != COM_GROUPSIGNALREFS_END; i++) {
		group_signal = COM_GET_RXSIGNAL(signal_group->com_group_signal[i]);
		com_alg_processing_rx_signal_data_from_pdu(buf, group_signal, core_map_idx);
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
	return;
}

/**
 * Copies data for a signal group from the Rx PDU buffer to the corresponding shadow buffer.
 *
 * For each byte in the PDU, only the bits enabled by the shadow buffer mask are copied to the shadow buffer,
 * with other bits remaining as-is. The update is performed in a critical section to ensure thread safety.
 *
 * When signal update bit feature is enabled, the copy is only performed if the update bit is valid
 * or not used for the signal group.
 *
 * After copying, the function further processes and updates each group signal value
 * via `com_alg_process_group_signal_from_shadow_buffer`.
 *
 * @param signal_group_id  Signal group ID whose content should be copied from the PDU to the shadow buffer.
 */
FUNC(void, COM_CODE) com_alg_copy_signal_group_data_from_pdu_to_shadow_buffer(const com_signal_id_type signal_group_id)
{
	const com_rx_signal_type *signal = COM_GET_RXSIGNAL(signal_group_id);
	com_self_rx_signal_type *self_signal = com_get_self_rx_signal(signal_group_id);
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;
	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(com_ipdu_handle_id_ram);
	uint16 com_ipdu_size_ram = ipdu->com_ipdu_size;
	uint8 *pdu_data_ptr = COM_GET_SELF_RXIPDU(com_ipdu_handle_id_ram)->com_ipdu_data_ptr;
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(com_ipdu_handle_id_ram);
#endif
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	boolean update_bit_ok = (FALSE == signal->com_signal_self_use_update_bit) ||
				((TRUE == signal->com_signal_self_use_update_bit) &&
				 (0u < COM_TESTBIT(pdu_data_ptr, signal->com_update_bit_pos)));
	if (update_bit_ok) {
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
		uint8 *buf = (uint8 *)self_signal->com_self_shadow_buffer;
		uint16 i = 0u;

		while (i < com_ipdu_size_ram) {
			uint8 mask = signal->com_self_shadow_buffer_mask[i];
			*buf = (*buf & ~mask) | (mask & *pdu_data_ptr);
			pdu_data_ptr++;
			buf++;
			i++;
		}
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
	}
#else
	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	uint8 *buf = (uint8 *)self_signal->com_self_shadow_buffer;
	uint16 i = 0u;

	while (i < com_ipdu_size_ram) {
		uint8 mask = signal->com_self_shadow_buffer_mask[i];
		*buf = (*buf & ~mask) | (mask & *pdu_data_ptr);
		pdu_data_ptr++;
		buf++;
		i++;
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
#endif
	com_alg_process_group_signal_from_shadow_buffer(signal, self_signal, core_map_idx);
}
#endif

/**
 * Reads and converts signal data from a PDU buffer, handling sign extension and signal type width.
 *
 * This function masks the input PDU data according to the signal's bit size,
 * and then casts or sign-extends it to the correct C data type based on the signal type.
 * For signed types, sign extension is performed if necessary.
 *
 * @param pdu_data     Input PDU data as a 32-bit value, already aligned and shifted.
 * @param bit_size     Number of valid bits in the signal.
 * @param signal_type  Data type of the signal (COM_BOOLEAN, COM_UINT8, COM_SINT16, etc.)
 * @param signal_data  Pointer to the destination buffer for the extracted signal value.
 * @param dest_length  Number of bytes to copy (for types wider than 8 bits).
 */
static inline void com_alg_read_signal_data_from_pdu_signal_type(uint32 pdu_data, uint16 bit_size,
								 com_signal_type signal_type, uint8 *signal_data,
								 uint32 dest_length)
{
	uint32 mask = (bit_size > 0u) ? (COM_ALG_SIGNAL_4BYTE_VALUE >> (32u - bit_size)) : 0u;

	pdu_data &= mask;
	uint32 signmask = ~(mask >> 1u);

	switch (signal_type) {
	case COM_BOOLEAN:
	case COM_UINT8:
		*signal_data = (uint8)pdu_data;
		break;
	case COM_UINT16:
	case COM_UINT32:
		(void)memcpy((void *)signal_data, (const void *)&pdu_data, dest_length);
		break;
	case COM_SINT8:
		if ((pdu_data & signmask) != 0u) {
			pdu_data |= signmask;
		}
		*signal_data = (uint8)pdu_data;
		break;
	case COM_SINT16:
	case COM_FLOAT32:
	case COM_SINT32:
		if ((pdu_data & signmask) != 0u) {
			pdu_data |= signmask;
		}
		(void)memcpy((void *)signal_data, (const void *)&pdu_data, dest_length);
		break;
	default:
		/* do nothing */
		break;
	}
}

/**
 * Extracts up to 32 bits of a signal's data from a PDU buffer, considering endianess and bit position.
 *
 * This function reads 4 (or sometimes 5) bytes from the PDU buffer at the given position,
 * assembles these bytes into a 32-bit value according to the signal's byte order (big/little endian),
 * then right-shifts by the start bit, and handles the case where the signal crosses a byte boundary
 * at the MSB side.
 *
 * Used for extracting arbitrary-length signals that may not be byte-aligned.
 *
 * @param signal_source_ptr  Pointer to the byte of PDU data containing the *start bit* of the signal.
 * @param bit_position       Bit offset of the signal within the buffer.
 * @param bit_size           Number of bits in the signal.
 * @param endian             Endianess of the signal in the PDU (COM_BIG_ENDIAN/COM_LITTLE_ENDIAN).
 * @return                   Up to 32 bits of the extracted value, right-aligned and not sign-extended.
 */
static inline uint32 com_alg_read_signal_data_from_pdu_endian(const uint8 *signal_source_ptr, uint16 bit_position,
							      uint16 bit_size, com_signal_endianess_type endian)
{
	uint32 pdu_data = 0U;
	uint8 bit_shift = (uint8)(bit_position % COM_ALG_SIGNAL_SHIFT_8BIT);

	switch (endian) {
	case COM_BIG_ENDIAN: {
		const uint8 *pdu_data_ptr = signal_source_ptr - 3u;

		for (uint8 i = 0u; i < 4u; i++) {
			pdu_data = (pdu_data << COM_ALG_SIGNAL_SHIFT_8BIT) | pdu_data_ptr[i];
		}
		pdu_data >>= bit_shift;
		if ((COM_ALG_SIGNAL_32BIT - bit_shift) < bit_size) {
			pdu_data |= (uint32)pdu_data_ptr[-1] << (COM_ALG_SIGNAL_32BIT - bit_shift);
		}
		break;
	}
	case COM_LITTLE_ENDIAN: {
		for (sint8 i = 3; i >= 0; i--) {
			pdu_data = (pdu_data << COM_ALG_SIGNAL_SHIFT_8BIT) | signal_source_ptr[i];
		}
		pdu_data >>= bit_shift;
		if ((COM_ALG_SIGNAL_32BIT - bit_shift) < bit_size) {
			pdu_data |= (uint32)signal_source_ptr[4] << (COM_ALG_SIGNAL_32BIT - bit_shift);
		}
		break;
	}
	default:
		break;
	}
	return pdu_data;
}

/**
 * Extracts up to 64 bits of a signal's data from a PDU buffer, considering endianess, bit position, and type.
 *
 * This function reads 8 (possibly 9) bytes from the PDU buffer at the given position,
 * assembles these bytes into a 64-bit value according to the signal's endianness (big/little endian),
 * then right-shifts by the offset, and handles the case where the signal is not byte-aligned
 * at the MSB side.
 *
 * After extracting the raw value, it applies a mask to get only the relevant bits, and for
 * signed/float64 types, performs sign extension if the sign bit is set.
 *
 * @param signal_source_ptr  Pointer to the byte of PDU data containing the *start bit* of the signal.
 * @param bit_position       Bit offset of the signal within the buffer.
 * @param bit_size           Number of bits in the signal (up to 64).
 * @param endian             Endianess of the signal in the PDU (COM_BIG_ENDIAN/COM_LITTLE_ENDIAN).
 * @param signal_type        Signal data type (e.g. COM_SINT64, COM_FLOAT64).
 * @param signal_data        Pointer to the destination buffer for the extracted signal value.
 * @param dest_length        Number of bytes to copy (according to type).
 */
static inline void com_alg_read_signal_data_from_pdu64bits(const uint8 *signal_source_ptr, uint16 bit_position,
							   uint16 bit_size, com_signal_endianess_type endian,
							   com_signal_type signal_type, uint8 *signal_data,
							   uint32 dest_length)
{
	uint64 pdu_data = 0U;
	const uint8 *pdu_data_ptr;
	uint8 bit_shift = (uint8)(bit_position % 8u);

	switch (endian) {
	case COM_BIG_ENDIAN:
		pdu_data_ptr = signal_source_ptr - 7;
		for (uint8 i = 0u; i < COM_ALG_SIGNAL_8BYTE; i++) {
			pdu_data = (pdu_data << COM_ALG_SIGNAL_8BYTE) | pdu_data_ptr[i];
		}
		pdu_data >>= bit_shift;
		if ((COM_ALG_SIGNAL_64BIT - bit_shift) < bit_size) {
			pdu_data |= (uint64)pdu_data_ptr[-1] << (COM_ALG_SIGNAL_64BIT - bit_shift);
		}
		break;
	case COM_LITTLE_ENDIAN:
		pdu_data_ptr = signal_source_ptr;
		for (sint8 i = 7; i >= 0; i--) {
			pdu_data = (pdu_data << COM_ALG_SIGNAL_8BYTE) | pdu_data_ptr[i];
		}
		pdu_data >>= bit_shift;
		if ((COM_ALG_SIGNAL_64BIT - bit_shift) < bit_size) {
			pdu_data |= (uint64)pdu_data_ptr[8] << (COM_ALG_SIGNAL_64BIT - bit_shift);
		}
		break;
	default:
		break;
	}

	uint64 mask = (COM_ALG_SIGNAL_8BYTE_VALUE >> (COM_ALG_SIGNAL_64BIT - bit_size));

	pdu_data &= mask;
	uint64 signmask = ~(mask >> 1u);

	if ((signal_type == COM_FLOAT64) || (signal_type == COM_SINT64)) {
		if ((pdu_data & signmask) != 0u) {
			pdu_data |= signmask;
		}
	}
	(void)memcpy((void *)signal_data, (const void *)&pdu_data, dest_length);
}

/**
 * Extracts and converts a signal's value from a PDU buffer, applying bit position, size, type, and endianess.
 *
 * This function determines the signal's bit position, size, and endian type from its configuration, then:
 * - For signals narrower than 64 bits, extracts up to 32 bits into a temporary value,
 *   and further converts it to the target type, possibly applying sign extension, etc.
 * - For 64-bit signals (COM_UINT64, COM_SINT64, COM_FLOAT64), extracts 64 bits and does the conversion.
 * The result is written into the application buffer pointed to by @p signal_appl_dest_ptr.
 *
 * @param com_signal            Pointer to the COM signal configuration information.
 * @param signal_type           Data type of the signal (e.g. COM_UINT8, COM_FLOAT64).
 * @param signal_appl_dest_ptr  Pointer to the application buffer to store the extracted value.
 * @param signal_source_ptr     Pointer to the raw PDU buffer containing the signal's data.
 * @param dest_length           Number of bytes to copy into the destination buffer (based on type).
 */
static inline FUNC(void, COM_CODE)
	com_alg_read_signal_from_pdu(const com_rx_signal_type *com_signal, uint8 *signal_appl_dest_ptr,
				     const uint8 *signal_source_ptr, uint32 dest_length)
{
	com_signal_type signal_type = com_signal->com_signal_type;
	uint16 bit_position = com_signal->com_bit_pos;
	uint16 bit_size = com_signal->com_bit_size;
	com_signal_endianess_type endian = com_signal->com_signal_endianess;

	if (signal_type < COM_UINT64) {
		uint32 pdu_data =
			com_alg_read_signal_data_from_pdu_endian(signal_source_ptr, bit_position, bit_size, endian);
		com_alg_read_signal_data_from_pdu_signal_type(pdu_data, bit_size, signal_type, signal_appl_dest_ptr,
							      dest_length);
	} else {
		com_alg_read_signal_data_from_pdu64bits(signal_source_ptr, bit_position, bit_size, endian, signal_type,
							signal_appl_dest_ptr, dest_length);
	}
	return;
}

/**
 * Parses and extracts a uint8 signal value from a PDU buffer at the specified position.
 *
 * This function calculates the actual start byte of the signal in the PDU,
 * reads the value via the generic signal extraction function, and writes it either
 * to the application buffer (if supplied) or to the internal COM signal buffer.
 * In multicore configurations, the value is written to the corresponding core buffer.
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer.
 * @param com_signal          Pointer to the COM signal configuration structure.
 * @param sig_type            Signal type (usually COM_UINT8 or COM_BOOLEAN).
 * @param signal_data         Destination pointer for the extracted signal value (optional).
 *                            If NULL, value is written to signal buffer instead.
 * @param core_map_idx        Core mapping index (used in multicore configurations).
 */
#if (COM_HAVE_UINT8_TYPE == STD_ON)
static inline FUNC(void, COM_CODE)
	com_alg_rx_signal_parse_uint8(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *com_signal,
				       com_signal_type sig_type, uint8 *signal_data, uint8 core_map_idx)
{
	uint8 tmp_new_value = 0;
	uint8 signal_start_byte = COM_SIGNAL_STARTBYTE(com_signal->com_bit_pos);

	com_alg_read_signal_from_pdu(com_signal, (void *)&tmp_new_value, com_ipdu_data_ptr + signal_start_byte, 1);
	if (signal_data == NULL) {
		uint16 idx_buffer = com_signal->com_buff_idx;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u8[idx_buffer] = tmp_new_value;
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint8ref[idx_buffer] = tmp_new_value;
#endif
	} else {
		*signal_data = tmp_new_value;
	}
	(void)core_map_idx;
}
#endif

/**
 * Parses and extracts a uint16 signal value from a PDU buffer at the specified position.
 *
 * This function calculates the actual start byte of the signal in the PDU,
 * extracts the signal value with appropriate conversion (endian, bit position, etc.)
 * via the generic extraction function, and writes the value either into the COM internal buffer
 * or the application buffer provided.
 * When running in multicore mode, updates the core-specific buffer.
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer.
 * @param com_signal          Pointer to the COM signal configuration structure.
 * @param sig_type            Signal type (usually COM_UINT16, COM_SINT16, etc.).
 * @param signal_data         Destination pointer for the extracted signal value (optional).
 *                            If NULL, value is written to the COM internal buffer.
 * @param core_map_idx        Core mapping index (used only if multicore is enabled).
 */
#if (COM_HAVE_UINT16_TYPE == STD_ON)
static inline FUNC(void, COM_CODE)
	com_alg_rx_signal_parse_uint16(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *com_signal,
					com_signal_type sig_type, uint8 *signal_data, uint8 core_map_idx)
{
	uint16 tmp_new_value = 0;
	uint8 signal_start_byte = COM_SIGNAL_STARTBYTE(com_signal->com_bit_pos);

	com_alg_read_signal_from_pdu(com_signal, (void *)&tmp_new_value, com_ipdu_data_ptr + signal_start_byte, 2);
	if (signal_data == NULL) {
		uint16 idx_buffer = com_signal->com_buff_idx;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u16[idx_buffer] = tmp_new_value;
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint16ref[idx_buffer] = tmp_new_value;
#endif
	} else {
		*((uint16 *)signal_data) = tmp_new_value;
	}
	(void)core_map_idx;
}
#endif

/**
 * 从PDU缓冲区中解析并提取一个 uint32 信号值。
 *
 * 本函数先计算信号在PDU中的起始字节位置，通过通用信号读取接口获取32位信号，
 * 并根据 signal_data 是否为空分别写入内部COM缓冲区或应用层结果缓冲区。
 * 多核配置下会写入各自核对应的缓冲区。
 *
 * @param com_ipdu_data_ptr   PDU原始数据缓冲区指针
 * @param com_signal          信号配置结构体指针
 * @param sig_type            信号类型（通常为COM_UINT32、COM_SINT32等）
 * @param signal_data         应用层缓冲指针；如为NULL则写入COM内部缓冲
 * @param core_map_idx        多核映射索引（单核下忽略）
 */
#if (COM_HAVE_UINT32_TYPE == STD_ON)
static inline FUNC(void, COM_CODE)
	com_alg_rx_signal_parse_uint32(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *com_signal,
					com_signal_type sig_type, uint8 *signal_data, uint8 core_map_idx)
{
	uint32 tmp_new_value = 0;
	uint8 signal_start_byte = COM_SIGNAL_STARTBYTE(com_signal->com_bit_pos);

	com_alg_read_signal_from_pdu(com_signal, (void *)&tmp_new_value, com_ipdu_data_ptr + signal_start_byte, 4);
	if (signal_data == NULL) {
		uint16 idx_buffer = com_signal->com_buff_idx;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u32[idx_buffer] = tmp_new_value;
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint32ref[idx_buffer] = tmp_new_value;
#endif
	} else {
		*((uint32 *)signal_data) = tmp_new_value;
	}
	(void)core_map_idx;
}
#endif

/**
 * Parses and extracts a uint64 signal value from a PDU buffer at the specified position.
 *
 * This function computes the signal's start byte within the PDU, extracts the 64-bit value
 * using the generic signal reading/conversion function, and writes it either to the internal COM buffer
 * or to the provided application buffer.
 * In multicore configurations, the value is written to the corresponding core-specific buffer.
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer.
 * @param com_signal          Pointer to the COM signal configuration structure.
 * @param sig_type            Signal type (usually COM_UINT64, COM_SINT64, COM_FLOAT64, etc.).
 * @param signal_data         Destination pointer for the extracted signal value (optional).
 *                            If NULL, value is written to the COM internal buffer.
 * @param core_map_idx        Core mapping index (used in multicore mode).
 */
#if (COM_HAVE_UINT64_TYPE == STD_ON)
static inline FUNC(void, COM_CODE)
	com_alg_rx_signal_parse_uint64(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *com_signal,
					com_signal_type sig_type, uint8 *signal_data, uint8 core_map_idx)
{
	uint64 tmp_new_value = 0;
	uint8 signal_start_byte = COM_SIGNAL_STARTBYTE(com_signal->com_bit_pos);

	com_alg_read_signal_from_pdu(com_signal, (void *)&tmp_new_value, com_ipdu_data_ptr + signal_start_byte, 8);
	if (signal_data == NULL) {
		uint16 idx_buffer = com_signal->com_buff_idx;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u64[idx_buffer] = tmp_new_value;
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint64ref[idx_buffer] = tmp_new_value;
#endif
	} else {
		*((uint64 *)signal_data) = tmp_new_value;
	}
	(void)core_map_idx;
}
#endif

/**
 * Parses and extracts a signal value from a PDU buffer according to its configured data type.
 *
 * This function serves as a universal dispatch for all supported signal types,
 * delegating the extraction and conversion to the appropriate parsing routine depending on signal type
 * (uint8, uint16, uint32, uint64, float, etc.).
 * The result is written into the application buffer or COM internal buffer, as implemented in each parser.
 * This function automatically handles core mapping for multicore configurations.
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer containing the signal.
 * @param signal              Pointer to the COM signal configuration structure.
 * @param signal_type         Signal type (COM_UINT8, COM_SINT16, COM_FLOAT32, etc.).
 * @param signal_data         Pointer to the destination buffer for the extracted value (optional).
 * @param core_map_idx        Core mapping index (used for multicore operation).
 */
static inline FUNC(void, COM_CODE)
	com_rx_signal_parse(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *signal,
				 com_signal_type signal_type, uint8 *signal_data, uint8 core_map_idx)
{
	switch (signal_type) {
#if (COM_HAVE_UINT8_TYPE == STD_ON)
	case COM_BOOLEAN:
	case COM_UINT8:
	case COM_SINT8:
			com_alg_rx_signal_parse_uint8(com_ipdu_data_ptr, signal, signal_type, signal_data,
						       core_map_idx);
		break;
#endif
#if (COM_HAVE_UINT16_TYPE == STD_ON)
	case COM_UINT16:
	case COM_SINT16:
			com_alg_rx_signal_parse_uint16(com_ipdu_data_ptr, signal, signal_type, signal_data,
							core_map_idx);
		break;
#endif
#if (COM_HAVE_UINT32_TYPE == STD_ON)
	case COM_UINT32:
	case COM_FLOAT32:
	case COM_SINT32:
			com_alg_rx_signal_parse_uint32(com_ipdu_data_ptr, signal, signal_type, signal_data,
							core_map_idx);
		break;
#endif
#if (COM_HAVE_UINT64_TYPE == STD_ON)
	case COM_UINT64:
	case COM_FLOAT64:
	case COM_SINT64:
			com_alg_rx_signal_parse_uint64(com_ipdu_data_ptr, signal, signal_type, signal_data,
							core_map_idx);
		break;
#endif
	default:
		/* do nothing */
		break;
	}
}

/**
 * Processes and extracts a single Rx signal's data from the PDU buffer and writes it to the COM internal buffer.
 *
 * - For standard scalar signal types (except array/opaque), delegates extraction to the general parser.
 * - For signals of type UINT8_N (array-based signals), extracts the correct array region based on endianess
 *   and signal configuration, and copies the extracted bytes to the COM buffer (handling multicore mode if required).
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer containing the signal.
 * @param signal              Pointer to the COM signal configuration structure.
 * @param core_map_idx        Core mapping index (used for multicore environments).
 */
FUNC(void, COM_CODE)
com_alg_processing_rx_signal_data_from_pdu(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *signal,
					   uint8 core_map_idx)
{
	com_signal_type signal_type_ram = signal->com_signal_type;

	if ((signal_type_ram != COM_UINT8_N) && (COM_OPAQUE != signal->com_signal_endianess)) {
		com_rx_signal_parse(com_ipdu_data_ptr, signal, signal_type_ram, NULL, core_map_idx);
	}
#if (COM_HAVE_UINT8_N_TYPE == STD_ON)
	else {
		uint16 bit_position = 0;

		if (signal->com_signal_endianess == COM_BIG_ENDIAN) {
			bit_position = signal->com_end_bit_pos;
		} else {
			bit_position = signal->com_bit_pos;
		}
		uint16 idx_buffer = signal->com_buff_idx;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		memcpy(&com_rx_sig_buf_u8_n[idx_buffer],
				       &com_ipdu_data_ptr[bit_position >> 3u], signal->com_bit_size >> 3u);
#else
		memcpy(
			&(com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_array_based_ref[idx_buffer]),
			&com_ipdu_data_ptr[bit_position >> 3u], signal->com_bit_size >> 3u);
#endif
	}
#endif
	return;
}

/**
 * Extracts a signal's value from the PDU raw buffer into a user/application buffer.
 *
 * For scalar signals (non-array, non-opaque types), this function delegates extraction and conversion
 * to the universal parsing function, considering signal configuration options (bit position, type, endianess, etc).
 * For array (COM_UINT8_N) or opaque types, it performs a direct memory copy of the signal bytes from the PDU buffer.
 *
 * All operations are performed inside a critical section to ensure data consistency during extraction.
 *
 * @param com_ipdu_data_ptr   Pointer to the raw PDU data buffer.
 * @param signal              Pointer to the signal configuration structure.
 * @param signal_data         Pointer to the user/application buffer to store the extracted signal value.
 */
FUNC(void, COM_CODE)
com_alg_read_signal_data_from_pdu(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *signal, uint8 *signal_data)
{
	com_signal_type signal_type_ram = signal->com_signal_type;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	if ((signal_type_ram != COM_UINT8_N) && (COM_OPAQUE != signal->com_signal_endianess)) {
		com_rx_signal_parse(com_ipdu_data_ptr, signal, signal_type_ram, signal_data, COM_NO_COREMAPIDX);
	} else {
		uint16 bit_position = 0;

		if (signal->com_signal_endianess == COM_BIG_ENDIAN) {
			bit_position = signal->com_end_bit_pos;
		} else {
			bit_position = signal->com_bit_pos;
		}
		memcpy(signal_data, &com_ipdu_data_ptr[bit_position >> 3u], signal->com_bit_size >> 3u);
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
}

/**
 * Reads the value of a received signal from the COM internal buffer and copies it into the provided application buffer.
 *
 * Depending on the signal's type, this function selects and reads the appropriate internal buffer.
 * For scalar types (boolean, uint8, sint8, uint16, etc.), the value is simply assigned.
 * For array (COM_UINT8_N, COM_UINT8_DYN) and multi-byte types, the function copies the required bytes.
 * Critical sections protect access for 32/64-bit and array types to ensure thread safety.
 * In multicore configurations, the value is read from the corresponding core-specific buffer.
 *
 * @param com_signal        Pointer to the signal configuration structure.
 * @param signal_data_ptr   Destination pointer where the value will be copied to.
 * @param core_map_idx      Core mapping index (used in multicore mode).
 */
FUNC(void, COM_CODE)
com_receive_sig_read_buffer(const com_rx_signal_type *com_signal, uint8 *signal_data_ptr, uint8 core_map_idx)
{
	com_signal_type signal_type_ram = com_signal->com_signal_type;
	uint16 idx_buffer = com_signal->com_buff_idx;

	switch (signal_type_ram) {
#if (COM_HAVE_UINT8_TYPE == STD_ON)
	case COM_BOOLEAN:
	case COM_UINT8:
	case COM_SINT8:
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		*signal_data_ptr = com_rx_sig_buf_u8[idx_buffer];
#else
			*signal_data_ptr =
				com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint8ref[idx_buffer];
#endif
		break;
#endif
#if (COM_HAVE_UINT16_TYPE == STD_ON)
	case COM_UINT16:
	case COM_SINT16:
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		*((uint16 *)signal_data_ptr) = com_rx_sig_buf_u16[idx_buffer];
#else
		*((uint16 *)signal_data_ptr) =
			com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint16ref[idx_buffer];
#endif
		break;
#endif
#if (COM_HAVE_UINT32_TYPE == STD_ON)
	case COM_UINT32:
	case COM_FLOAT32:
	case COM_SINT32:
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		*((uint32 *)signal_data_ptr) = com_rx_sig_buf_u32[idx_buffer];
#else
		*((uint32 *)signal_data_ptr) =
			com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint32ref[idx_buffer];
#endif
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
		break;
#endif
#if (COM_HAVE_UINT64_TYPE == STD_ON)
	case COM_UINT64:
	case COM_FLOAT64:
	case COM_SINT64:
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		*((uint64 *)signal_data_ptr) = com_rx_sig_buf_u64[idx_buffer];
#else
		*((uint64 *)signal_data_ptr) =
			com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint64ref[idx_buffer];
#endif
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
		break;
#endif
#if (COM_HAVE_UINT8_N_TYPE == STD_ON)
	case COM_UINT8_N:
	case COM_UINT8_DYN:
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		memcpy(signal_data_ptr, &com_rx_sig_buf_u8_n[idx_buffer],
				       com_signal->com_bit_size >> 3u);
#else
		memcpy(
			signal_data_ptr,
			&com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_array_based_ref[idx_buffer],
			com_signal->com_bit_size >> 3u);
#endif
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
		break;
#endif
	default:
		/* do nothing */
		break;
	}
	(void)core_map_idx;
	return;
}

/****************************************************************************
 * name: com_rx_processing_init_rx_signal
 *
 * description: init rx signal value.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_rx_processing_init_rx_signal(const com_rx_signal_type *signal, uint8 core_map_idx)
{
	const uint8 *signal_data_ptr = signal->com_signal_init_value;
	uint16 idx_buffer = signal->com_buff_idx;
	com_signal_type signal_type = signal->com_signal_type;

	switch (signal_type) {
#if (COM_HAVE_UINT8_TYPE == STD_ON)
	case COM_BOOLEAN:
	case COM_UINT8:
	case COM_SINT8:
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u8[idx_buffer] = *signal_data_ptr;
#else
			com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint8ref[idx_buffer] =
				*signal_data_ptr;
#endif
		break;
#endif
#if (COM_HAVE_UINT16_TYPE == STD_ON)
	case COM_UINT16:
	case COM_SINT16:
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u16[idx_buffer] = *((uint16 *)signal_data_ptr);
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint16ref[idx_buffer] =
			*((uint16 *)signal_data_ptr);
#endif
		break;
#endif
#if (COM_HAVE_UINT32_TYPE == STD_ON)
	case COM_UINT32:
	case COM_FLOAT32:
	case COM_SINT32:
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u32[idx_buffer] = *((uint32 *)signal_data_ptr);
#else
		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint32ref[idx_buffer] =
			*((uint32 *)signal_data_ptr);
#endif
		break;
#endif
#if (COM_HAVE_UINT64_TYPE == STD_ON)
	case COM_UINT64:
	case COM_FLOAT64:
	case COM_SINT64:
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		com_rx_sig_buf_u64[idx_buffer] = *((uint64 *)signal_data_ptr);
#else

		com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_uint64ref[idx_buffer] =
			*((uint64 *)signal_data_ptr);
#endif
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
		break;
#endif
#if (COM_HAVE_UINT8_N_TYPE == STD_ON)
	case COM_UINT8_N:
	case COM_UINT8_DYN:
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
#if (COM_SUPPORT_MUTICORE == STD_OFF)
		memcpy(&com_rx_sig_buf_u8_n[idx_buffer], signal_data_ptr,
				       signal->com_bit_size >> 3u);
#else
		memcpy(
			&com_core_map_info_struct[core_map_idx].com_rx_sig_buffer_array_based_ref[idx_buffer],
			signal_data_ptr, signal->com_bit_size >> 3u);
#endif
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
		break;
#endif
	default:
		/* do nothing */
		break;
	}
	(void)core_map_idx;
	return;
}

/****************************************************************************
 * name: com_alg_compose_64bits_big_endian
 *
 * description: com compose signal according 64bits big endian.
 *
 ****************************************************************************/
static inline uint64 com_alg_compose_64bits_big_endian(const uint8 *pdu_data_ptr)
{
	uint64 orig_val = 0u;

	for (uint8 idx = 0u; idx < COM_ALG_SIGNAL_8BYTE; idx++) {
		orig_val = (orig_val << COM_ALG_SIGNAL_SHIFT_8BIT) | pdu_data_ptr[idx];
	}
	return orig_val;
}

/****************************************************************************
 * name: com_alg_write_64bits_big_endian
 *
 * description: com write signal according 64bits big endian.
 *
 ****************************************************************************/
static inline void com_alg_write_64bits_big_endian(uint8 *pdu_data_ptr, uint64 value)
{
	for (sint8 idx = 7; idx >= 0; idx--) {
		pdu_data_ptr[idx] = (uint8)value;
		value >>= COM_ALG_SIGNAL_SHIFT_8BIT;
	}
}

/****************************************************************************
 * name: com_alg_handle_high_bits_big_endian
 *
 * description: com handle high bits big endian.
 *
 ****************************************************************************/
static inline void com_alg_handle_high_bits_big_endian(uint8 *pdu_data_ptr, uint64 mask, uint64 sig_val,
						       uint16 bit_size, uint8 shift, boolean *data_changed)
{
	uint8 bits_written = COM_ALG_SIGNAL_64BIT - shift;

	if (bits_written < bit_size) {
		pdu_data_ptr--;
		uint64 orig_val = *pdu_data_ptr;
		uint64 mask_high = ~(mask >> bits_written);
		uint64 sig_high = sig_val >> bits_written;

		uint64 updated_val = (orig_val & mask_high) | sig_high;
		*data_changed = *data_changed || (updated_val != orig_val);
		*pdu_data_ptr = (uint8)updated_val;
	}
}

/****************************************************************************
 * name: com_alg_write_signal_64bits_big_endian
 *
 * description: com write signal value 64bits big endian.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_64bits_big_endian(uint64 sig_val, uint64 mask, uint8 *com_ipdu_data_ptr,
					       com_bit_pos_type bit_position, uint16 bit_size, boolean *data_changed)
{
	uint8 *pdu_data_ptr = &com_ipdu_data_ptr[COM_SIGNAL_STARTBYTE(bit_position)] - 7;
	uint64 orig_val = com_alg_compose_64bits_big_endian(pdu_data_ptr);

	uint8 shift = COM_SIGNAL_BITOFFSET(bit_position);
	uint64 sig_part = sig_val << shift;
	uint64 mask_part = ~(mask << shift);
	uint64 updated_val = (orig_val & mask_part) | sig_part;

	*data_changed = (updated_val != orig_val);

	com_alg_write_64bits_big_endian(pdu_data_ptr, updated_val);

	com_alg_handle_high_bits_big_endian(pdu_data_ptr, mask, sig_val, bit_size, shift, data_changed);
	return;
}

/****************************************************************************
 * name: com_alg_compose_64bits_little_endian
 *
 * description: com compose 64bits signal little endian.
 *
 ****************************************************************************/
static inline uint64 com_alg_compose_64bits_little_endian(const uint8 *pdu_data_ptr)
{
	uint64 orig_val = 0u;

	for (sint8 idx = 7; idx >= 0; idx--) {
		orig_val = (orig_val << COM_ALG_SIGNAL_SHIFT_8BIT) | pdu_data_ptr[idx];
	}
	return orig_val;
}

/****************************************************************************
 * name: com_alg_write_64bits_little_endian
 *
 * description: com write 64bits signal little endian.
 *
 ****************************************************************************/
static inline void com_alg_write_64bits_little_endian(uint8 *pdu_data_ptr, uint64 value)
{
	for (uint8 idx = 0u; idx < COM_ALG_SIGNAL_8BYTE; idx++) {
		pdu_data_ptr[idx] = (uint8)value;
		value >>= COM_ALG_SIGNAL_8BYTE;
	}
}

/****************************************************************************
 * name: com_alg_handle_high_bits_little_endian
 *
 * description: com handle high bits according little endian.
 *
 ****************************************************************************/
static inline void com_alg_handle_high_bits_little_endian(uint8 *pdu_data_ptr, uint64 mask, uint64 sig_val,
							  uint16 bit_size, uint8 shift, boolean *data_changed)
{
	uint8 bits_written = COM_ALG_SIGNAL_64BIT - shift;

	if (bits_written < bit_size) {
		pdu_data_ptr = &pdu_data_ptr[COM_ALG_SIGNAL_8BYTE];
		uint64 orig_val = *pdu_data_ptr;
		uint64 mask_high = ~(mask >> bits_written);
		uint64 sig_high = sig_val >> bits_written;

		uint64 updated_val = (orig_val & mask_high) | sig_high;
		*data_changed = *data_changed || (updated_val != orig_val);
		*pdu_data_ptr = (uint8)updated_val;
	}
}

/****************************************************************************
 * name: com_alg_write_signal_64bits_little_endian
 *
 * description: com write signal 64bits according little endian.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_64bits_little_endian(uint64 sig_val, uint64 mask, uint8 *com_ipdu_data_ptr,
						  com_bit_pos_type bit_position, uint16 bit_size, boolean *data_changed)
{
	uint8 *pdu_data_ptr = &com_ipdu_data_ptr[COM_SIGNAL_STARTBYTE(bit_position)];
	uint64 orig_val = com_alg_compose_64bits_little_endian(pdu_data_ptr);

	uint8 shift = COM_SIGNAL_BITOFFSET(bit_position);
	uint64 sig_part = sig_val << shift;
	uint64 mask_part = ~(mask << shift);
	uint64 updated_val = (orig_val & mask_part) | sig_part;

	*data_changed = (updated_val != orig_val);

	com_alg_write_64bits_little_endian(pdu_data_ptr, updated_val);

	com_alg_handle_high_bits_little_endian(pdu_data_ptr, mask, sig_val, bit_size, shift, data_changed);
	return;
}

/****************************************************************************
 * name: com_alg_write_signal_data_to_pdu64bits
 *
 * description: write signal data to pdu buffer for 64 bit signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_data_to_pdu64bits(const uint8 *signal_data_ptr, com_signal_type signal_type,
					       uint8 *com_ipdu_data_ptr, com_bit_pos_type bit_position, uint16 bit_size,
					       com_signal_endianess_type endian, boolean *data_changed)
{
	uint64 sig_val = 0u;
	(void)memcpy((void *)&sig_val, (const void *)signal_data_ptr, sizeof(uint64));
	uint64 mask = COM_ALG_SIGNAL_8BYTE_VALUE >> (64u - bit_size);

	sig_val &= mask;
	switch (endian) {
	case COM_BIG_ENDIAN:
		com_alg_write_signal_64bits_big_endian(sig_val, mask, com_ipdu_data_ptr, bit_position, bit_size,
						       data_changed);
		break;
	case COM_LITTLE_ENDIAN:
			com_alg_write_signal_64bits_little_endian(sig_val, mask, com_ipdu_data_ptr, bit_position,
								  bit_size, data_changed);
		break;
	default:
		break;
	}
	return;
}

/****************************************************************************
 * name: com_alg_write_signal_data_to_pdu_opaque
 *
 * description: write signal data to pdu buffer for opaque signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_data_to_pdu_opaque(const uint8 *signal_data_ptr, uint8 *com_ipdu_data_ptr,
						com_bit_pos_type bit_position, uint16 bit_size, boolean *data_changed)
{
	uint8 *pdu_buffer_bytes = &(com_ipdu_data_ptr[bit_position >> 3u]);
	uint16 signal_length = bit_size >> 3u;
#if (COM_SIGNAL_ONCHANGE_ENABLE == STD_ON)
	*data_changed = (0 != memcmp(pdu_buffer_bytes, signal_data_ptr, signal_length));
#else
	*data_changed = FALSE;
#endif
	(void)memcpy((void *)pdu_buffer_bytes, (const void *)signal_data_ptr, signal_length);
	return;
}

/****************************************************************************
 * name: read_big_endian_32
 *
 * description: read big endian 32 bit signal.
 *
 ****************************************************************************/
static inline FUNC(uint32, COM_CODE) read_big_endian_32(const uint8 *data)
{
	uint32 value = 0;

	for (uint8 i = 0; i < 4; i++) {
		value = (value << 8) | data[i];
	}
	return value;
}

/****************************************************************************
 * name: write_big_endian_32
 *
 * description: write big endian 32 bit signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE) write_big_endian_32(uint8 *data, uint32 value)
{
	for (int8_t i = 3; i >= 0; i--) {
		data[i] = (uint8)value;
		value >>= 8;
	}
}

/****************************************************************************
 * name: handle_remaining_bits
 *
 * description: handle remaining bits.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE) handle_remaining_bits(uint8 *base_ptr, uint16 bit_size, uint8 shift, uint32 signal,
							 uint32 mask, boolean *changed)
{
	uint8 written_bits = 32 - shift;

	if (written_bits < bit_size) {
		uint8 *prev_byte = base_ptr - 1;
		uint8 original_byte = *prev_byte;

		uint32 remaining_mask = ~(mask >> written_bits);
		uint32 remaining_signal = signal >> written_bits;

		uint8 modified_byte = (original_byte & remaining_mask) | remaining_signal;
		*changed = *changed || (modified_byte != original_byte);
		*prev_byte = modified_byte;
	}
}

/****************************************************************************
 * name: modify_bits
 *
 * description: modify signal bits.
 *
 ****************************************************************************/
static uint32 modify_bits(uint32 original, uint32 signal, uint32 mask, uint8 shift)
{
	uint32 signal_part = signal << shift;
	uint32 mask_part = ~(mask << shift);

	return (original & mask_part) | signal_part;
}

/****************************************************************************
 * name: com_alg_write_signal_data_to_pdu_big_endian
 *
 * description: write signal data to pdu buffer for big endian signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_data_to_pdu_big_endian(uint8 *com_ipdu_data_ptr, com_bit_pos_type bit_position,
						    uint16 bit_size, uint32 mask, uint32 sig_val, boolean *data_changed)
{
	uint8 *target_byte = ((&com_ipdu_data_ptr[COM_SIGNAL_STARTBYTE(bit_position)]) - 3);
	uint32 original_value = read_big_endian_32(target_byte);

	uint8 bit_shift = COM_SIGNAL_BITOFFSET(bit_position);
	uint32 modified_value = modify_bits(original_value, sig_val, mask, bit_shift);

	*data_changed = (modified_value != original_value);

	write_big_endian_32(target_byte, modified_value);

	handle_remaining_bits(target_byte, bit_size, bit_shift, sig_val, mask, data_changed);
	return;
}

/****************************************************************************
 * name: read_little_endian_32
 *
 * description: read little endian 32 bit signal.
 *
 ****************************************************************************/
static uint32 read_little_endian_32(const uint8 *data)
{
	uint32 value = 0;

	for (int i = 3; i >= 0; i--) {
		value = (value << COM_ALG_SIGNAL_SHIFT_8BIT) | data[i];
	}
	return value;
}

/****************************************************************************
 * name: write_little_endian_32
 *
 * description: write little endian 32 bit signal.
 *
 ****************************************************************************/
static void write_little_endian_32(uint8 *data, uint32 value)
{
	for (uint8 i = 0; i < 4; i++) {
		data[i] = (uint8)value;
		value >>= COM_ALG_SIGNAL_SHIFT_8BIT;
	}
}

/****************************************************************************
 * name: handle_overflow_bits
 *
 * description: handle overflow bits.
 *
 ****************************************************************************/
static void handle_overflow_bits(uint8 *data, uint16 bit_size, uint8 shift, uint32 signal, uint32 mask,
				 boolean *changed)
{
	uint8 remaining_bits = COM_ALG_SIGNAL_32BIT - shift;

	if (remaining_bits < bit_size) {
		uint8 *next_byte = &data[4];
		uint8 original_byte = *next_byte;

		uint32 remaining_mask = ~(mask >> remaining_bits);
		uint32 remaining_signal = signal >> remaining_bits;

		uint8 modified_byte = (original_byte & remaining_mask) | remaining_signal;
		*changed = *changed || (modified_byte != original_byte);
		*next_byte = modified_byte;
	}
}

/****************************************************************************
 * name: com_alg_write_signal_data_to_pdu_little_endian
 *
 * description: write signal data to pdu buffer for little endian signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_data_to_pdu_little_endian(uint8 *com_ipdu_data_ptr, com_bit_pos_type bit_position,
						       uint16 bit_size, uint32 mask, uint32 sig_val,
						       boolean *data_changed)
{
	uint8 *target_byte = &com_ipdu_data_ptr[COM_SIGNAL_STARTBYTE(bit_position)];
	uint32 original_value = read_little_endian_32(target_byte);

	uint8 bit_shift = COM_SIGNAL_BITOFFSET(bit_position);
	uint32 modified_value = modify_bits(original_value, sig_val, mask, bit_shift);

	*data_changed = (modified_value != original_value);
	write_little_endian_32(target_byte, modified_value);

	handle_overflow_bits(target_byte, bit_size, bit_shift, sig_val, mask, data_changed);
	return;
}

/****************************************************************************
 * name: com_alg_write_signal_process
 *
 * description: write signal data to pdu buffer for nbit_nbyte signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_process(const uint8 *signal_data_ptr, com_signal_type signal_type,
				     uint8 *com_ipdu_data_ptr, com_bit_pos_type bit_position, uint16 bit_size,
				     com_signal_endianess_type endian, boolean *data_changed)
{
	if (signal_type < COM_UINT64) {
		uint32 sig_val = 0u;

		switch (signal_type) {
		case COM_BOOLEAN:
		case COM_UINT8:
		case COM_SINT8:
			sig_val = *(signal_data_ptr);
			break;
		case COM_UINT16:
		case COM_SINT16:
			(void)memcpy((void *)&sig_val, (const void *)signal_data_ptr, sizeof(uint16));
			break;
		case COM_FLOAT32:
		case COM_UINT32:
		case COM_SINT32:
			(void)memcpy((void *)&sig_val, (const void *)signal_data_ptr, sizeof(uint32));
			break;
		default:
			/* do nothing */
			break;
		}
		uint32 mask = (bit_size > 0u) ? (COM_ALG_SIGNAL_4BYTE_VALUE >> (32u - bit_size)) : 0u;

		sig_val &= mask;
		if (COM_BIG_ENDIAN == endian) {
			com_alg_write_signal_data_to_pdu_big_endian(com_ipdu_data_ptr, bit_position, bit_size, mask,
								    sig_val, data_changed);
		} else if (COM_LITTLE_ENDIAN == endian) {
			com_alg_write_signal_data_to_pdu_little_endian(com_ipdu_data_ptr, bit_position, bit_size, mask,
								       sig_val, data_changed);
		} else {
			/* do nothing */
		}
	} else {
		com_alg_write_signal_data_to_pdu64bits(signal_data_ptr, signal_type, com_ipdu_data_ptr, bit_position,
						       bit_size, endian, data_changed);
	}
	return;
}

/****************************************************************************
 * name: com_alg_write_signal_data_to_pdu_sub
 *
 * description: write signal data to pdu buffer sub function.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_alg_write_signal_data_to_pdu_sub(const uint8 *signal_data_ptr, com_signal_type signal_type,
					     uint8 *com_ipdu_data_ptr, com_bit_pos_type bit_position, uint16 bit_size,
					     com_signal_endianess_type endian, boolean *data_changed)
{
	com_alg_write_signal_process(signal_data_ptr, signal_type, com_ipdu_data_ptr, bit_position, bit_size, endian,
				     data_changed);
#if (COM_SIGNAL_ONCHANGE_ENABLE == STD_OFF)
	*data_changed = FALSE;
#endif
	return;
}

/****************************************************************************
 * name: com_alg_write_tx_signal_data_to_pdu
 *
 * description: write signal data to pdu buffer function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_write_tx_signal_data_to_pdu(const uint8 *signal_data_ptr, const com_tx_signal_type *tx_signal,
				    uint8 *com_ipdu_data_ptr, boolean *data_changed)
{
	com_signal_type signal_type = tx_signal->com_signal_type;
	com_signal_endianess_type endian = tx_signal->com_signal_endianess;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	if ((signal_type != COM_UINT8_N) && (COM_OPAQUE != endian)) {
		com_alg_write_signal_data_to_pdu_sub(signal_data_ptr, signal_type, com_ipdu_data_ptr,
						     tx_signal->com_bit_pos, tx_signal->com_bit_size, endian,
						     data_changed);
	} else {
		uint16 bit_po_temp = 0;

		if (endian == COM_BIG_ENDIAN) {
			bit_po_temp = tx_signal->com_end_bit_pos;
		} else {
			bit_po_temp = tx_signal->com_bit_pos;
		}
		com_alg_write_signal_data_to_pdu_opaque(signal_data_ptr, com_ipdu_data_ptr, bit_po_temp,
							tx_signal->com_bit_size, data_changed);
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
	return;
}

/****************************************************************************
 * name: com_alg_write_rx_signal_data_to_pdu
 *
 * description: write rx signal data to pdu buffer function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_write_rx_signal_data_to_pdu(const uint8 *signal_data_ptr, const com_rx_signal_type *rx_signal,
				    uint8 *com_ipdu_data_ptr, boolean *data_changed)
{
	com_signal_type signal_type = rx_signal->com_signal_type;
	com_signal_endianess_type endian = rx_signal->com_signal_endianess;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	if ((signal_type != COM_UINT8_N) && (COM_OPAQUE != endian)) {
		com_alg_write_signal_data_to_pdu_sub(signal_data_ptr, signal_type, com_ipdu_data_ptr,
						     rx_signal->com_bit_pos, rx_signal->com_bit_size, endian,
						     data_changed);
	} else {
		uint16 bit_po_temp = 0;

		if (endian == COM_BIG_ENDIAN) {
			bit_po_temp = rx_signal->com_end_bit_pos;
		} else {
			bit_po_temp = rx_signal->com_bit_pos;
		}
		com_alg_write_signal_data_to_pdu_opaque(signal_data_ptr, com_ipdu_data_ptr, bit_po_temp,
							rx_signal->com_bit_size, data_changed);
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
	return;
}

/****************************************************************************
 * name: com_alg_rx_process_invalid_signal
 *
 * description: com process rx invalid signal.
 *
 ****************************************************************************/
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
static FUNC(com_invalid_signal_action_type, COM_CODE)
	com_alg_rx_process_invalid_signal(const com_rx_signal_type *signal, const com_self_rx_ipdu_type *self_ipdu,
					  const com_rx_ipdu_type *ipdu)
{
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;

	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(com_ipdu_handle_id_ram);
#endif
	if (INVALID_NOTIFY == signal->com_data_invalid_action) {
		if (COM_NO_FUNCTION_CALLOUT != signal->com_invalid_notification) {
			com_rx_signal_invalid_notification[signal->com_invalid_notification]();
		}
		return COM_INVALID_ACTION_NOTIFY;
	} else if (INVALID_REPLACE == signal->com_data_invalid_action) {
		if (self_ipdu->com_self_ipdu_has_ipdu_buff == TRUE) {
			com_alg_write_rx_signal_data_to_pdu((const uint8 *)signal->com_signal_init_value, signal,
							    self_ipdu->com_ipdu_data_ptr, &data_changed);
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
			if (COM_IMMEDIATE == ipdu->com_ipdu_signal_processing) {
				if ((FALSE == signal->com_self_is_signal_group) &&
				    (FALSE == signal->com_self_is_group_signal)) {
					com_rx_processing_init_rx_signal(signal, core_map_idx);
				}
#endif
			}
		} else {
			com_rx_processing_init_rx_signal(signal, core_map_idx);
		}
		return COM_INVALID_ACTION_REPLACE;
	} else {
		/* do nothing */
	}
	return COM_NONE_INVALID_VALUE;
}
#endif

/****************************************************************************
 * name: com_alg_rx_process_check_invalid_signal
 *
 * description: check signal value == invalid value in rx process.
 *
 ****************************************************************************/
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
static FUNC(com_invalid_signal_action_type, COM_CODE)
	com_alg_rx_process_check_invalid_signal(const com_rx_signal_type *signal,
						const com_self_rx_ipdu_type *self_ipdu, const com_rx_ipdu_type *ipdu,
						const PduInfoType *pdu_info_ptr)
{
	if (signal->com_signal_data_invalid_value == NULL) {
		return COM_NONE_INVALID_VALUE;
	}

	uint64 signal_data = 0u;
	uint64 signal_invalid_data = 0u;

	com_alg_read_signal_data_from_pdu(pdu_info_ptr->SduDataPtr, signal, (uint8 *)&signal_data);

	if ((signal->com_signal_type == COM_UINT8_N) || (signal->com_signal_type == COM_UINT8_DYN)) {
		(void)memcpy(&signal_invalid_data, signal->com_signal_data_invalid_value, signal->com_bit_size / 8u);
	} else if (signal->com_bit_size <= 8u) {
		signal_invalid_data = *((const uint8 *)signal->com_signal_data_invalid_value);
	} else if (signal->com_bit_size <= 16u) {
		signal_invalid_data = *((const uint16 *)signal->com_signal_data_invalid_value);
	} else if (signal->com_bit_size <= 32u) {
		if ((signal->com_signal_type == COM_UINT32) || (signal->com_signal_type == COM_SINT32)) {
			signal_invalid_data = (uint64)(*((const uint32 *)signal->com_signal_data_invalid_value));
		} else {
			signal_invalid_data = (uint64)(*((const float32 *)signal->com_signal_data_invalid_value));
		}
	} else if (signal->com_bit_size <= 64u) {
		if ((signal->com_signal_type == COM_UINT64) || (signal->com_signal_type == COM_SINT64)) {
			signal_invalid_data = *((const uint64 *)signal->com_signal_data_invalid_value);
		}
	} else {
		/* do nothing */
	}
	if (signal_invalid_data == signal_data) {
		return com_alg_rx_process_invalid_signal(signal, self_ipdu, ipdu);
	}
	return COM_NONE_INVALID_VALUE;
}
#endif

/****************************************************************************
 * name: com_alg_rx_process_signals
 *
 * description: rx process signal.
 *
 ****************************************************************************/
#if ((COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON) || (COM_SIG_GATEWAY_ENABLE == STD_ON) ||                           \
	 (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF) || (COM_IPDU_ALL_DEFERRED == STD_OFF) ||                              \
	 (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON))

static FUNC(void, COM_CODE) com_alg_rx_process_signals(const com_rx_ipdu_type *ipdu, com_self_rx_ipdu_type *self_ipdu,
						       PduIdType rx_pdu_id, const PduInfoType *pdu_info_ptr)
{
	const com_rx_signal_type *com_signal;
	uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
	uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
	com_invalid_signal_action_type invalid_signal_action = COM_NONE_INVALID_VALUE;
#if (COM_IPDU_ALL_DEFERRED == STD_OFF)
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(rx_pdu_id);
#endif
#endif
	for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
		com_signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		boolean com_signal_self_use_update_bit_ram = com_signal->com_signal_self_use_update_bit;
#endif

#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
		uint32 com_timeout_factor_ram = com_signal->com_timeout_factor;
#endif
#if ((COM_SIG_GATEWAY_ENABLE == STD_ON) || (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
		com_self_rx_signal_type *self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);
#endif

#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		if ((FALSE == com_signal_self_use_update_bit_ram) ||
		    ((TRUE == com_signal_self_use_update_bit_ram) &&
		     (COM_TESTBIT(pdu_info_ptr->SduDataPtr, com_signal->com_update_bit_pos) > 0u))) {
#endif
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
			invalid_signal_action =
				com_alg_rx_process_check_invalid_signal(com_signal, self_ipdu, ipdu, pdu_info_ptr);
			if (invalid_signal_action == COM_INVALID_ACTION_NOTIFY) {
				continue;
			}
#endif

#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
			if (com_timeout_factor_ram > 0u) {
				self_signal->com_rx_deadline_cnt = com_timeout_factor_ram;
			}
#endif

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
			if ((TRUE == ipdu->com_ipdu_gw_routing_req) && (TRUE == com_signal->com_sig_gw_routing_req)) {
				self_signal->com_signal_updated_gw_routing = TRUE;
			}
#endif

#if (COM_IPDU_ALL_DEFERRED == STD_OFF)
			if (COM_IMMEDIATE == ipdu->com_ipdu_signal_processing) {
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
				if ((FALSE == com_signal->com_self_is_signal_group) &&
				    (FALSE == com_signal->com_self_is_group_signal)) {
					if (invalid_signal_action != COM_INVALID_ACTION_REPLACE) {
						com_alg_processing_rx_signal_data_from_pdu(pdu_info_ptr->SduDataPtr,
											   com_signal, core_map_idx);
					}
				}
#else
				if (invalid_signal_action != COM_INVALID_ACTION_REPLACE) {
					com_alg_processing_rx_signal_data_from_pdu(pdu_info_ptr->SduDataPtr, com_signal,
										   core_map_idx);
				}
#endif
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
				self_signal->com_signal_routing_req = self_signal->com_signal_updated_gw_routing;
				self_signal->com_signal_updated_gw_routing = FALSE;
#endif
				uint32 com_notification_ram = com_signal->com_notification;

				if (COM_NO_FUNCTION_CALLOUT != com_notification_ram) {
					com_rx_signal_notification[com_notification_ram]();
				}
			}
#endif
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		}
#endif
	}
	(void)rx_pdu_id;
	(void)pdu_info_ptr;
	(void)invalid_signal_action;
	return;
}
#endif

/****************************************************************************
 * name: com_alg_rx_process
 *
 * description: rx process in rx indication.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_rx_process(PduIdType rx_pdu_id, const PduInfoType *pdu_info_ptr)
{
	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(rx_pdu_id);
	com_self_rx_ipdu_type *self_ipdu = COM_GET_SELF_RXIPDU(rx_pdu_id);

	/* user local buffer */
	PduInfoType pdu_info_local;

	pdu_info_local.SduDataPtr = pdu_info_ptr->SduDataPtr;
	pdu_info_local.SduLength = pdu_info_ptr->SduLength;
#if (COM_RXIPDU_ALL_IMMEDIATE == STD_OFF) || (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	if (self_ipdu->com_self_ipdu_has_ipdu_buff == TRUE) {
		memcpy(self_ipdu->com_ipdu_data_ptr, pdu_info_local.SduDataPtr, ipdu->com_ipdu_size);
	}
#endif

	self_ipdu->com_ipdu_updated = TRUE;

#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
	uint16 rx_tout_info_idx_ram = ipdu->rx_tout_info_idx;

	if (rx_tout_info_idx_ram != COM_NO_REF_INDEX) {
		self_ipdu->com_rx_deadline_cnt = com_rx_pdu_tout_struct[rx_tout_info_idx_ram].com_timeout_factor;
	}
#endif

#if ((COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON) || (COM_SIG_GATEWAY_ENABLE == STD_ON) ||                           \
	 (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF) || (COM_IPDU_ALL_DEFERRED == STD_OFF) ||                              \
	 (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON))
	com_alg_rx_process_signals(ipdu, self_ipdu, rx_pdu_id, &pdu_info_local);
#endif
	return;
}

/****************************************************************************
 * name: com_alg_tx_handle_dm
 *
 * description: rx process in rx indication.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_tx_handle_dm(const com_tx_ipdu_type *ipdu, com_self_tx_ipdu_type *self_ipdu)
{
	if ((ipdu == NULL) || (self_ipdu == NULL)) {
		return;
	}

	const com_tx_mode_type *tx_mode_ptr = self_ipdu->com_txipdu_tx_mode ? ipdu->com_tx_ipdu->com_tx_mode_true :
									      ipdu->com_tx_ipdu->com_tx_mode_false;

	com_tx_mode_mode_type mode = tx_mode_ptr->com_tx_mode_mode;

	if ((mode == COM_DIRECT) || (mode == COM_MIXED)) {
		if (self_ipdu->com_txipdu_tx_confirmations_num > 0u) {
			self_ipdu->com_txipdu_tx_confirmations_num--;
			if (self_ipdu->com_txipdu_tx_confirmations_num == 0u) {
				self_ipdu->com_tx_dm_timer = 0u;
			}
		} else {
			self_ipdu->com_tx_dm_timer = 0u;
		}
	} else if (mode == COM_PERIODIC) {
		self_ipdu->com_tx_dm_timer = 0u;
	} else if (mode == COM_NONE) {
		self_ipdu->com_tx_dm_timer = self_ipdu->com_tx_deadline_cnt;
	} else {
		/* do nothing */
	}
	return;
}

/****************************************************************************
 * name: com_alg_update_shadow_tx_signal
 *
 * description: update shadow tx signal.
 *
 ****************************************************************************/
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
FUNC(void, COM_CODE) com_alg_update_shadow_tx_signal(com_signal_id_type signal_id, const void *signal_data_ptr)
{
	const com_self_tx_signal_type *self_group_signal = com_get_self_tx_signal(signal_id);
	const com_tx_signal_type *group_signal = COM_GET_TXSIGNAL(signal_id);
	boolean data_changed = FALSE;

	com_alg_write_tx_signal_data_to_pdu(signal_data_ptr, group_signal,
					    (uint8 *)self_group_signal->com_self_shadow_buffer, &data_changed);
	return;
}

/****************************************************************************
 * name: com_alg_update_shadow_rx_signal
 *
 * description: update shadow rx signal.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_update_shadow_rx_signal(com_signal_id_type signal_id, const void *signal_data_ptr)
{
	const com_self_rx_signal_type *self_group_signal = com_get_self_rx_signal(signal_id);
	const com_rx_signal_type *group_signal = COM_GET_RXSIGNAL(signal_id);
	boolean data_changed = FALSE;

	com_alg_write_rx_signal_data_to_pdu(signal_data_ptr, group_signal,
					    (uint8 *)self_group_signal->com_self_shadow_buffer, &data_changed);
	return;
}
#endif

/****************************************************************************
 * name: com_alg_set_tx_confirmation_status
 *
 * description: set tx confirmation status.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_set_tx_confirmation_status(com_self_tx_ipdu_type *self_ipdu, boolean value)
{
	if (NULL != self_ipdu) {
		self_ipdu->com_ipdu_updated = value;
	}
	return;
}

/****************************************************************************
 * name: com_alg_get_tx_confirmation_status
 *
 * description: get tx confirmation status.
 *
 ****************************************************************************/
FUNC(boolean, COM_CODE) com_alg_get_tx_confirmation_status(com_self_tx_ipdu_type *self_ipdu)
{
	boolean tc_status = FALSE;

	if (self_ipdu != NULL) {
		tc_status = self_ipdu->com_ipdu_updated;
	}
	return tc_status;
}

#if (COM_SUPPORT_MUTICORE == STD_ON)
/****************************************************************************
 * name: com_alg_get_rx_ipdu_core_map_idx_by_pdu_id
 *
 * description: get rx ipdu core map idx by pdu id.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(uint16 pdu_id)
{
	uint8 core_map_idx = 0;

	for (uint8 i = 0u; i < COM_CORE_NUM; i++) {
		if (pdu_id < com_core_map_info_struct[i].end_rx_ipdu_idx) {
			core_map_idx = i;
			break;
		}
	}
	return core_map_idx;
}

/****************************************************************************
 * name: com_alg_get_ipdu_core_map_idx_by_core_id
 *
 * description: get core map idx by core id.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_alg_get_ipdu_core_map_idx_by_core_id(uint32 core_id)
{
	uint8 core_map_idx = 0;

	for (uint8 i = 0u; i < COM_CORE_NUM; i++) {
		if (core_id == com_core_map_info_struct[i].core_id) {
			core_map_idx = i;
			break;
		}
	}
	return core_map_idx;
}
#endif

/****************************************************************************
 * name: com_alg_trigger_ipdu_send_sub
 *
 * description: trigger ipdu send sub function.
 *
 ****************************************************************************/
FUNC(com_tx_trigger_status_type, COM_CODE)
com_alg_trigger_ipdu_send_sub(com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu, PduInfoType *pdu_info_ptr)
{
	com_tx_trigger_status_type tx_trigger_status = COM_TX_NOT_TRIGGERED;
	const com_ipdu_tx_type *com_ipdu_tx_ram = ipdu->com_tx_ipdu;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const com_tx_signal_type *com_signal = NULL;
#endif
	if (E_OK == pdur_com_transmit(ipdu->self_ipdu_outgoing_id, pdu_info_ptr)) {
		tx_trigger_status = COM_TX_TRIGGERED;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		if (COM_TRANSMIT == com_ipdu_tx_ram->com_tx_ipdu_clear_update_bit) {
			uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
			uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;

			for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
				com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
				if (TRUE == com_signal->com_signal_self_use_update_bit) {
					COM_CLEARBIT(self_ipdu->com_ipdu_data_ptr, com_signal->com_update_bit_pos);
				}
			}
		}
#endif
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
		if (TRUE == com_ipdu_tx_ram->enable_retry_failed_transmit_requests) {
			self_ipdu->com_txipdu_min_delay_timer = com_ipdu_tx_ram->com_tx_ipdu_minimum_delay_factor;
		}
#endif
	}
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
	if (FALSE == com_ipdu_tx_ram->enable_retry_failed_transmit_requests) {
		self_ipdu->com_txipdu_min_delay_timer = com_ipdu_tx_ram->com_tx_ipdu_minimum_delay_factor;
	}
#else
	self_ipdu->com_txipdu_min_delay_timer = com_ipdu_tx_ram->com_tx_ipdu_minimum_delay_factor;
#endif
	return tx_trigger_status;
}

/****************************************************************************
 * name: com_alg_trigger_ipdu_send
 *
 * description: ipdu send function.
 *
 ****************************************************************************/
FUNC(com_tx_trigger_status_type, COM_CODE)
com_alg_trigger_ipdu_send(PduIdType pdu_id, com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu)
{
	boolean ts_status;

	ts_status = TRUE;
	com_tx_trigger_status_type tx_trigger_status = COM_TX_NOT_TRIGGERED;
	SchM_Enter_COM_EXCLUSIVE_AREA_1();
	if (0u == self_ipdu->com_txipdu_min_delay_timer) {
		PduInfoType pdu_info_package;

		pdu_info_package.SduDataPtr = (uint8 *)(self_ipdu->com_ipdu_data_ptr);
		pdu_info_package.SduLength = ipdu->com_ipdu_size;
		uint32 com_tx_ipdu_callout_ram = ipdu->com_tx_ipdu_callout;

		if (COM_NO_FUNCTION_CALLOUT != com_tx_ipdu_callout_ram) {
			if (FALSE == com_tx_ipdu_callouts[com_tx_ipdu_callout_ram](pdu_id, &pdu_info_package)) {
				tx_trigger_status = COM_TX_NOT_TRIGGERED;
				ts_status = FALSE;
			}
		}
		if (TRUE == ts_status) {
			tx_trigger_status = com_alg_trigger_ipdu_send_sub(self_ipdu, ipdu, &pdu_info_package);
		}
	} else {
		tx_trigger_status = COM_TX_MIN_DELAY_NOT_FULFILLED;
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_1();
	return tx_trigger_status;
}

/****************************************************************************
 * name: com_alg_route_gw_destn_signals
 *
 * description: rout signal to dest signal.
 *
 ****************************************************************************/
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
FUNC(void, COM_CODE) com_alg_route_gw_destn_signals(uint8 gw_mapidx, const uint8 *signal_data_ptr)
{
	uint16 j, sig_handle;
	com_gw_signal_ref_type gw_signal_ref;

	for (j = 0u; j < com_config->com_gw_mapping_ref[gw_mapidx].com_gw_desitnation_routes_num; j++) {
		gw_signal_ref = com_config->com_gw_mapping_ref[gw_mapidx].com_gw_dest_ref[j].com_gw_dest_sig_ref;
		sig_handle = com_config->com_gw_mapping_ref[gw_mapidx].com_gw_dest_ref[j].com_gw_dest_sig_id;

		switch (gw_signal_ref) {
		case COM_SIGNAL_REFERENCE:
		case COM_GROUP_SIGNAL_REFERENCE:
			(void)com_send_signal(sig_handle, signal_data_ptr);
			break;
		default:
			break;
		}
	}
	return;
}

/****************************************************************************
 * name: com_alg_extract_gw_src_sig_data_sub
 *
 * description: extract gateway src signal data subfunction.
 *
 ****************************************************************************/
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_alg_extract_gw_src_sig_data_sub(uint16 ipdu_handle, uint8 *sig_data_ptr,
								const com_rx_signal_type *signal_info)
{
	const com_self_rx_ipdu_type *self_ipdu_src;
	uint16 start_from_pdu_byte;

	self_ipdu_src = COM_GET_SELF_RXIPDU(ipdu_handle);

	const uint8 *pdu_data_ptr = (uint8 *)self_ipdu_src->com_ipdu_data_ptr;

	if ((COM_UINT8_N != signal_info->com_signal_type) && (COM_UINT8_DYN != signal_info->com_signal_type)) {
		com_alg_read_signal_data_from_pdu(pdu_data_ptr, signal_info, sig_data_ptr);
	} else {
		com_bit_pos_type bit_position = 0;

		if (signal_info->com_signal_endianess == COM_BIG_ENDIAN) {
			bit_position = signal_info->com_end_bit_pos;
		} else {
			bit_position = signal_info->com_bit_pos;
		}
		start_from_pdu_byte = bit_position / 8u;
		SchM_Enter_COM_EXCLUSIVE_AREA_0();
		(void)memcpy((void *)sig_data_ptr, (const void *)(&pdu_data_ptr[start_from_pdu_byte]),
			     (uint32)(signal_info->com_bit_size) / 8u);
		SchM_Exit_COM_EXCLUSIVE_AREA_0();
	}
}
#endif

/****************************************************************************
 * name: com_alg_extract_gw_src_sig_data
 *
 * description: extract gateway src signal data function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_extract_gw_src_sig_data(const com_rx_signal_type *com_signal_src, uint16 ipdu_handle, uint8 *sig_data_ptr)
{
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(ipdu_handle);
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	if (FALSE == com_signal_src->com_self_is_group_signal) {
		com_receive_sig_read_buffer(com_signal_src, sig_data_ptr, core_map_idx);
	} else {
		com_alg_extract_gw_src_sig_data_sub(ipdu_handle, sig_data_ptr, com_signal_src);
	}
#else
	(void)ipdu_handle;
	com_receive_sig_read_buffer(com_signal_src, sig_data_ptr, core_map_idx);
#endif
	return;
}
#endif
#define COM_STOP_SEC_CODE
#include "com_memmap.h"
