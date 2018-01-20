#include "nfc.h"

#include <stdint.h>
#include "nfc_t2t_lib.h"
#include "nfc_ndef_msg.h"
#include "nfc_text_rec.h"
//#include "nfc_uri_msg.h" for URLs, remember to adjust makefile
#include "boards.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME "NFC"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

uint8_t m_ndef_msg_buf[256];


/**
 * @brief Callback function for handling NFC events.
 */
void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length)
{
    (void)p_context;

    switch (event)
    {
        case NFC_T2T_EVENT_FIELD_ON:
            bsp_board_led_on(BSP_BOARD_LED_0); //TODO: Name LED GREEN / LED_RED etc or maybe configure in application config.h?
            NRF_LOG_INFO("NFC Field detected \r\n");
            break;
        case NFC_T2T_EVENT_FIELD_OFF:
            bsp_board_led_off(BSP_BOARD_LED_0);
            NRF_LOG_INFO("NFC Field lost \r\n");
            break;
        default:
            break;
    }
}

/**
 * @brief Function for encoding the initial message.
 */
void initial_msg_encode(uint8_t * p_buffer, uint32_t * p_len)
{
    NFC_NDEF_MSG_DEF(initial_msg, MAX_REC_COUNT);

    id_record_add(&NFC_NDEF_MSG(initial_msg));
    //url_record_add(&NFC_NDEF_MSG(initial_msg)); TODO: URL
    //app_record_add(&NFC_NDEF_MSG(initial_msg)); TODO: launch app

    /** @snippet [NFC text usage_2] */
    uint32_t err_code = nfc_ndef_msg_encode(&NFC_NDEF_MSG(initial_msg),
                                            p_buffer,
                                            p_len);
    APP_ERROR_CHECK(err_code);
    /** @snippet [NFC text usage_2] */
}

uint32_t nfc_init(void)
{
    uint32_t  len = sizeof(m_ndef_msg_buf);
    uint32_t  err_code;

    /* Set up NFC */
    err_code = nfc_t2t_setup(nfc_callback, NULL);
    APP_ERROR_CHECK(err_code);

    /* Encode welcome message */
    initial_msg_encode(m_ndef_msg_buf, &len);

    /* Set created message as the NFC payload */
    err_code = nfc_t2t_payload_set(m_ndef_msg_buf, len);
    APP_ERROR_CHECK(err_code);

    /* Start sensing NFC field */
    err_code = nfc_t2t_emulation_start();
    APP_ERROR_CHECK(err_code);

    return err_code;
}

/**
 * Update NFC payload with given data. Datr is converted to hex and printed as a string.
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Fnfc_ndef_format_dox.html
 */
void nfc_binary_record_set(uint8_t* data, size_t data_length)
{
  ret_code_t error_code = NRF_SUCCESS;
  uint32_t length;
  static uint8_t buffer_for_message[256];
  static uint8_t buffer_for_record[128];

  //TODO: define max length, return error if exceeded.

  //Stop NFC
  nfc_t2t_emulation_stop();

  /// Declare record descriptor by macro - create and initialize an instance of 
  ///   nfc_ndef_record_desc_t.
  NFC_NDEF_RECORD_BIN_DATA_DEF(binary_record, //NAME
    0x02, //MEDIA type TNF
    NULL, // No ID
    0, //ID len 0
    NULL, //No type
    0,    // type length 0
    buffer_for_record, 
    sizeof(buffer_for_record)
  );

  // If required, get the record size to length variable.
  error_code |= nfc_ndef_record_encode( &NFC_NDEF_RECORD_BIN_DATA(binary_record),
                                     NDEF_LONE_RECORD,
                                     NULL,
                                     &length);
  // Encode the message to buffer_for_message.
  //ASSERT(length <= 512); // make sure the message fits into the buffer
  // Declare message descriptor by macro - create and initialize an instance of 
  //   nfc_ndef_msg_desc_t and an array of pointers to nfc_ndef_record_desc_t.
  // The declared message can contain up to 1 records.
  NFC_NDEF_MSG_DEF(my_message, 1);
  // Add record_1 and record_2 to the message.
  // record_1 and record_2 are record descriptors as created in the previous
  // code example.
  error_code |= nfc_ndef_msg_record_add( &NFC_NDEF_MSG(my_message), &NFC_NDEF_RECORD_BIN_DATA(binary_record));
  error_code |= nfc_ndef_msg_encode( &NFC_NDEF_MSG(my_message),
                                    buffer_for_message,
                                    &length);

  /* Set created message as the NFC payload */
  error_code |= nfc_t2t_payload_set(buffer_for_message, sizeof(buffer_for_message));
  
  /* Start sensing NFC field */
  error_code |= nfc_t2t_emulation_start();
  APP_ERROR_CHECK(error_code);
}



/**
 * @brief Function for creating a record.
 */
void id_record_add(nfc_ndef_msg_desc_t * p_ndef_msg_desc)
{
    /** @snippet [NFC text usage_1] */
    uint32_t err_code = NRF_SUCCESS;
    unsigned int mac0 = NRF_FICR->DEVICEID[0];
    unsigned int mac1 = NRF_FICR->DEVICEID[1];
    //8 hex bytes
    char name[17] = { 0 };
    sprintf(name, "%x%x", mac0, mac1);
    uint8_t* name_bytes = (void*)&name;
    static const uint8_t en_code[] = {'e', 'n'};

    NFC_NDEF_TEXT_RECORD_DESC_DEF(id_text_rec,
                                  UTF_8,
                                  en_code,
                                  sizeof(en_code),
                                  name_bytes,
                                  sizeof(name));
   /** @snippet [NFC text usage_1] */
    err_code = nfc_ndef_msg_record_add(p_ndef_msg_desc,
                                       &NFC_NDEF_TEXT_RECORD_DESC(id_text_rec));
    APP_ERROR_CHECK(err_code);
}