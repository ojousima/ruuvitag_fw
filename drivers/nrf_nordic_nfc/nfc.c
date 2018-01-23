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

//Do not compile RAM-consuming buffers if they're not used
#if NFC_HAL_ENABLED


uint8_t m_ndef_msg_buf[256];
bool nfc_is_init = false;
volatile bool field_on = false;

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
            field_on = true;
            break;
        case NFC_T2T_EVENT_FIELD_OFF:
            bsp_board_led_off(BSP_BOARD_LED_0);
            NRF_LOG_INFO("NFC Field lost \r\n");
            field_on = false;
            break;
        case NFC_T2T_EVENT_DATA_READ:
            NRF_LOG_INFO("Data read\r\n");
        default:
            break;
    }
}

/**
 * @brief Function for encoding the nfc message.
 */
void nfc_msg_encode(nfc_ndef_msg_desc_t* nfc_msg, uint8_t * p_buffer, uint32_t * p_len)
{

    //id_record_add(&NFC_NDEF_MSG(nfc_msg));

    /** @snippet [NFC text usage_2] */
    uint32_t error_code = nfc_ndef_msg_encode(nfc_msg,
                                            p_buffer,
                                            p_len);
    NRF_LOG_INFO("Encode msg status: %d\r\n", error_code);
    /** @snippet [NFC text usage_2] */
}

uint32_t nfc_init(uint8_t* data, uint32_t data_length)
{
    if(field_on) { return NRF_ERROR_INVALID_STATE; }
    NFC_NDEF_MSG_DEF(nfc_msg, MAX_REC_COUNT);
    uint32_t  len = sizeof(m_ndef_msg_buf);
    uint32_t  error_code = NRF_SUCCESS;

    //Deinit NFC if it has been previously initialized
    if(nfc_is_init){
      NRF_LOG_INFO("Deinit NFC\r\n");
      nrf_delay_ms(10);
      nfc_t2t_emulation_stop();
      nfc_t2t_done();
      NRF_LOG_INFO("Deinit NFC ok\r\n");
      nrf_delay_ms(10);
    }

    /* Set up NFC */
    error_code |= nfc_t2t_setup(nfc_callback, NULL);
    NRF_LOG_INFO("NFC setup status: %d\r\n", error_code);

    nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_msg));
    id_record_add(&NFC_NDEF_MSG(nfc_msg));
    address_record_add(&NFC_NDEF_MSG(nfc_msg));
    data_record_add(&NFC_NDEF_MSG(nfc_msg), data, data_length);

    /* Encode welcome message */
    nfc_msg_encode(&NFC_NDEF_MSG(nfc_msg), m_ndef_msg_buf, &len);

    /* Set created message as the NFC payload */
    error_code |= nfc_t2t_payload_set(m_ndef_msg_buf, len);
    NRF_LOG_INFO("Payload set status: %d\r\n", error_code);

    /* Start sensing NFC field */
    error_code |= nfc_t2t_emulation_start();
    NRF_LOG_INFO("Emulation start status: %d\r\n", error_code);

    nfc_is_init = true;

    return error_code;
}

/**
 * Update NFC payload with given data. Data is converted to hex and printed as a string.
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Fnfc_ndef_format_dox.html
 */
void data_record_add(nfc_ndef_msg_desc_t* nfc_msg, uint8_t* data, uint32_t data_length)
{
  //uint32_t error_code = NRF_SUCCESS;
  uint8_t empty[] = {0};
  if(0 == data_length || NULL == data) { data = empty; data_length = 1;}
  //TODO: #define data length
  static char data_string[60] = { 0 };
  memset(data_string, 0, sizeof(data_string));
  for (uint8_t ii = 0; ii < data_length; ii++){
    sprintf(data_string+2*ii, "%02x", data[ii]);
  }
  uint8_t* data_bytes = (void*)&data_string;
  static const uint8_t data_code[] = {'d', 'a', 't', 'a'};

  NFC_NDEF_TEXT_RECORD_DESC_DEF(data_text_rec,
                                  UTF_8,
                                  data_code,
                                  sizeof(data_code),
                                  data_bytes,
                                  data_length*2);
   /** @snippet [NFC text usage_1] */
  nfc_ndef_msg_record_add(nfc_msg, &NFC_NDEF_TEXT_RECORD_DESC(data_text_rec));
}



/**
 * @brief Function for creating a record.
 */
void id_record_add(nfc_ndef_msg_desc_t* nfc_msg)
{
    /** @snippet [NFC text usage_1] */
    uint32_t err_code = NRF_SUCCESS;
    unsigned int mac0 = NRF_FICR->DEVICEID[0];
    unsigned int mac1 = NRF_FICR->DEVICEID[1];
    //8 hex bytes
    static char name[17] = { 0 };
    sprintf(name, "%x%x", mac0, mac1);
    uint8_t* name_bytes = (void*)&name;
    static const uint8_t id_code[] = {'i', 'd'};

    NFC_NDEF_TEXT_RECORD_DESC_DEF(id_text_rec,
                                  UTF_8,
                                  id_code,
                                  sizeof(id_code),
                                  name_bytes,
                                  sizeof(name));
   /** @snippet [NFC text usage_1] */
    err_code = nfc_ndef_msg_record_add(nfc_msg,
                                       &NFC_NDEF_TEXT_RECORD_DESC(id_text_rec));
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for creating a record.
 */
void address_record_add(nfc_ndef_msg_desc_t* nfc_msg)
{
    /** @snippet [NFC text usage_1] */
    uint32_t err_code = NRF_SUCCESS;
    //MACs are LSB first in FICR
    unsigned int addr0 = ((NRF_FICR->DEVICEADDR[1]>>16)&0xFF) | 0xC000; //2 MSB must be 11;;
    unsigned int addr1 = NRF_FICR->DEVICEADDR[0];
    //8 hex bytes
    static char name[17] = { 0 };
    sprintf(name, "%x%x", addr0, addr1);
    uint8_t* name_bytes = (void*)&name;
    static const uint8_t addr_code[] = {'a', 'd', 'd', 'r'};

    NFC_NDEF_TEXT_RECORD_DESC_DEF(id_text_rec,
                                  UTF_8,
                                  addr_code,
                                  sizeof(addr_code),
                                  name_bytes,
                                  sizeof(name));
   /** @snippet [NFC text usage_1] */
    err_code = nfc_ndef_msg_record_add(nfc_msg,
                                       &NFC_NDEF_TEXT_RECORD_DESC(id_text_rec));
    APP_ERROR_CHECK(err_code);
}

#endif