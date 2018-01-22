#include "nfc.h"

#include <stdint.h>
#include "nfc_t2t_lib.h"
#include "nfc_ndef_msg.h"
#include "nfc_text_rec.h"
//#include "nfc_uri_msg.h" for URLs, remember to adjust makefile
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "NFC"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "sdk_config.h"

//Do not compile RAM-consumin buffers if they're not used
#if NFC_HAL_ENABLED

uint8_t m_ndef_msg_buf[256];
uint8_t binary_record_buf[128];
NFC_NDEF_MSG_DEF(nfc_msg, MAX_REC_COUNT);
bool field = false;

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
            field = true;
            break;
        case NFC_T2T_EVENT_FIELD_OFF:
            bsp_board_led_off(BSP_BOARD_LED_0);
            NRF_LOG_INFO("NFC Field lost \r\n");
            field = false;
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

    //id_record_add(&NFC_NDEF_MSG(nfc_msg));

    /** @snippet [NFC text usage_2] */
    uint32_t err_code = nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_msg),
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

    id_record_add();

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
void nfc_binary_record_set(uint8_t* data, uint32_t data_length)
{
  //Do not update while NFC is connected
  if(field) { return; }
  ret_code_t error_code = NRF_SUCCESS;
  uint32_t length = sizeof(m_ndef_msg_buf);

  //TODO: define max length, return error if exceeded.

  //Stop NFC
  NRF_LOG_INFO("Stopping NFC \r\n");
  nfc_t2t_emulation_stop();
  nfc_t2t_done();
  error_code |= nfc_t2t_setup(nfc_callback, NULL);
  nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_msg));
  id_record_add(&NFC_NDEF_MSG(nfc_msg));

  //TODO: #define data length
  static char data_string[60] = { 0 };
  for (uint8_t ii = 0; ii < data_length; ii++){
    sprintf(data_string+2*ii, "%02x", data[ii]);
  }
  uint8_t* data_bytes = (void*)&data_string;
  static const uint8_t us_code[] = {'v', '3'};

  NFC_NDEF_TEXT_RECORD_DESC_DEF(data_text_rec,
                                  UTF_8,
                                  us_code,
                                  sizeof(us_code),
                                  data_bytes,
                                  sizeof(data_string));
   /** @snippet [NFC text usage_1] */
  error_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_msg),
                                    &NFC_NDEF_TEXT_RECORD_DESC(data_text_rec));

  NRF_LOG_INFO("Payload set status: %d\r\n", error_code);

  /* Encode welcome message */
  initial_msg_encode(m_ndef_msg_buf, &length);

  /* Set created message as the NFC payload */
  error_code |= nfc_t2t_payload_set(m_ndef_msg_buf, length);

  /* Start sensing NFC field */
  error_code |= nfc_t2t_emulation_start();
  NRF_LOG_INFO("Emulation start status: %d\r\n", error_code);
  //APP_ERROR_CHECK(error_code);
}



/**
 * @brief Function for creating a record.
 */
void id_record_add()
{
    /** @snippet [NFC text usage_1] */
    uint32_t err_code = NRF_SUCCESS;
    unsigned int mac0 = NRF_FICR->DEVICEID[0];
    unsigned int mac1 = NRF_FICR->DEVICEID[1];
    //8 hex bytes
    static char name[17] = { 0 };
    sprintf(name, "%x%x", mac0, mac1);
    uint8_t* name_bytes = (void*)&name;
    static const uint8_t en_code[] = {'i', 'd'};

    NFC_NDEF_TEXT_RECORD_DESC_DEF(id_text_rec,
                                  UTF_8,
                                  en_code,
                                  sizeof(en_code),
                                  name_bytes,
                                  sizeof(name));
   /** @snippet [NFC text usage_1] */
    err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_msg),
                                       &NFC_NDEF_TEXT_RECORD_DESC(id_text_rec));
    APP_ERROR_CHECK(err_code);
}

#endif