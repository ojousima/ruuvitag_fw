#ifndef NFC_H
#define NFC_H

#include <stdint.h>
#include "nfc_t2t_lib.h"
#include "nfc_ndef_msg.h"
#include "nfc_text_rec.h"

#define MAX_REC_COUNT      3     /**< Maximum records count. */

/**
 * Initializes NFC with ID message.
 *
 * ID message is uniques_id in UTF-8 numbers string, i.e. "032523523"
 *
 * @return error  code from NFC init, 0 on success
 *
 */
uint32_t nfc_init(void);

/**
 * @brief Function for encoding the initial message.
 */
void initial_msg_encode(uint8_t * p_buffer, uint32_t * p_len);

/**
 * @brief Creates UTF-8 string record of device ID.
 *
 * @param p_ndef_mmmmmsg_desc pointer to array containing text records.
 */
void id_record_add();

/**
 * Update NFC payload with given data. Datr is converted to hex and printed as a string.
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Fnfc_ndef_format_dox.html
 */
void nfc_binary_record_set(uint8_t* data, uint32_t data_length);

/**
 * @brief Callback function for handling NFC events.
 */
void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length);

#endif