/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup test_drivers
 * @{
 * @ingroup test_drivers
 * @brief Test Cases for Ruuvitag drivers.
 *
 * This file contains some tests to ensure the Ruuvitag drivers are working correct. The output is transmitted via
 * the J-Link RTT.
 */

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_error.h"
#include "init.h"
#include "bme280.h"
#include "LIS2DH12.h"
#include "bluetooth_core.h"
#include "ble_event_handlers.h"
#include "rtc.h"
#include "rng.h"
#include "application_service_if.h"

#include "iota/iota.h"
#include "iota/constants.h"
#include "libiota.h"
#include "asciiToTrytes.h"

#include  "test_rng.h"
#include  "test_rtc.h"
#include  "test_environmental.h"
#include  "test_mam.h"

#define NRF_LOG_MODULE_NAME "MAIN"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_ERROR("ASSERT at %s:%d", (uint32_t)p_file_name, (uint32_t)line_num);
}

/**@brief Function for doing power management.
 *
 * Waits in sleep until event happens
 */
static void power_manage(void)
{
   uint32_t err_code = sd_app_evt_wait();
   APP_ERROR_CHECK(err_code);
}

/** Blocking BLE TX - TODO refactor to some place else **/
uint32_t tx_data(uint8_t* chunk, uint8_t length)
{
  if(length >  BLE_NUS_MAX_DATA_LEN){ return 1; } //TODO: proper error code
  uint8_t data_array[ BLE_NUS_MAX_DATA_LEN] = {0};
  uint32_t       err_code;
  memcpy(&data_array, chunk, length);  
  do
  {
    err_code = ble_nus_string_send(get_nus(), data_array, length);
  }while(NRF_SUCCESS != err_code);
  
  return err_code;
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
  init_err_code_t err_code = INIT_SUCCESS;
  //Start with log - note, strng representations of error codes are wrong on failures
  err_code |= init_log();
  NRF_LOG_INFO("Log init status %s\r\n", (uint32_t)ERR_TO_STR(err_code));
  NRF_LOG_FLUSH();
  nrf_delay_ms(10);  

  //Init BLE next to init timers and clocks
  err_code |= init_ble();
  NRF_LOG_INFO("BLE init status %s\r\n", (uint32_t)ERR_TO_STR(err_code));
  NRF_LOG_FLUSH();
  nrf_delay_ms(10);

  //Init RTC
  err_code |= init_rtc();
  NRF_LOG_INFO("RTC init status %s\r\n", (uint32_t)ERR_TO_STR(err_code));
  uint32_t test_start = millis();
  nrf_delay_ms(10);  
  

  //Init RNG
  err_code |= init_rng();
  NRF_LOG_INFO("RNG init status %s\r\n", (uint32_t)ERR_TO_STR(err_code));
  NRF_LOG_FLUSH();
  nrf_delay_ms(10);

  //Init LEDs 
  err_code |= init_leds();
  NRF_LOG_INFO("Led init status %s, turning LEDs on for a second\r\n", (uint32_t)ERR_TO_STR(err_code));
  NRF_LOG_FLUSH();
  nrf_delay_ms(100);
  nrf_gpio_pin_clear(LED_RED);
  nrf_gpio_pin_clear(LED_GREEN);
  nrf_delay_ms(1000);
  nrf_gpio_pin_set(LED_RED);
  nrf_gpio_pin_set(LED_GREEN);
  nrf_delay_ms(100);
  
  //Check battery reading
  uint16_t voltage = getBattery();
  NRF_LOG_INFO("Checking battery state... %d mV\r\n", (uint32_t)voltage);
  NRF_LOG_FLUSH();
  nrf_delay_ms(10);
  
  //Start BME280
  err_code |= init_bme280();
  
  NRF_LOG_INFO("BME280 init status %s\r\n", (uint32_t)ERR_TO_STR(err_code));
  
  //TODO: accelerometer
  //test_rtc();
  //test_rng();
  //test_environmental();
  NRF_LOG_INFO("Testing mam creation.\r\n");
  for(int ii = 0; ii < 1; ii++)
  {
    test_mam();
    test_byte_tryte_conversion();
    NRF_LOG_INFO("Round %d\r\n", (uint32_t)ii);
  }
  //NRF_LOG_INFO("Ok, Timing mam creation.\r\n");
  //uint32_t mam_start = millis();
  //test_mam_create_time();
  //NRF_LOG_INFO("Mam creation completed in %d ms\r\n", millis()-mam_start);
  //test_byte_tryte_conversion();
  
  bluetooth_advertising_start();  
  
  uint32_t test_end = millis();
  NRF_LOG_INFO("Automated test completed in %d milliseconds\r\n", test_end - test_start);
  nrf_delay_ms(10);  
  
  NRF_LOG_INFO("Waiting for BLE connection\r\n")
  while(!is_ble_connected())
  {
    app_sched_execute();
    power_manage();    
  }
  NRF_LOG_INFO("BLE connected, waiting for UART notifications to be registered\r\n");
  ble_nus_t* p_nus = get_nus();
  while(!(p_nus->is_notification_enabled))
  {
    app_sched_execute();
    power_manage();    
  }
  NRF_LOG_INFO("NUS connected, switching to BLE-based test.\r\n");

  //TODO: Test endpoint-communication

  const char seed[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ9ABCDEFGHIJKLMNOPQRSTUVWXYZ9";
  size_t start = MAM_START;
  size_t count = MAM_COUNT;
  size_t index = MAM_INDEX;
  size_t next_start = MAM_NEXT_START;
  size_t next_count = MAM_NEXT_COUNT;
  size_t security = MAM_SECURITY;
  static int32_t raw_t  = 0;
  static uint32_t raw_p = 0;
  static uint32_t raw_h = 0;
  err_code = NRF_SUCCESS;
  uint8_t* txmessage = calloc(60, sizeof(uint8_t));
  char* trytes = calloc(120, sizeof(uint8_t));
    
  while(1)
  {
    if(p_nus->is_notification_enabled)
    {
    NRF_LOG_INFO("Sending sensor data over MAM\r\n");

    //First sample
    err_code |= bme280_set_mode(BME280_MODE_FORCED);
    nrf_delay_ms(10);
    //read previous data, start next sample
    bme280_set_mode(BME280_MODE_FORCED);
    nrf_delay_ms(10);
    // Get raw environmental data
    raw_t = bme280_get_temperature();
    raw_p = bme280_get_pressure();
    raw_h = bme280_get_humidity();
    NRF_LOG_INFO("temperature: %d.%d, pressure: %d, humidity: %d\r\n", raw_t/100, raw_t%100, raw_p>>8, raw_h>>10); //Wrong decimals on negative values.

    char* print = (void*)txmessage;
    sprintf(print, "{temperature: %d.%d, pressure: %d, humidity: %d}", (int)raw_t/100, (int)raw_t%100, (int)raw_p>>8, (int)raw_h>>10);
    toTrytes(txmessage, trytes, 60);
    
    //Returns dynamically allocated pointer. REMEMBER TO FREE
    char* result = (char*)mam_create(seed, trytes, start, count, index, next_start, next_count, security);
    
    size_t chunk_index = 0;
    size_t message_length = strlen(result);
    while(18*chunk_index <= message_length)
    {
      uint8_t chunk[20] = {0};
      chunk[0] = 0xE0;
      chunk[1] = chunk_index;
      memcpy(&(chunk[2]), &(result[18*chunk_index]), 18); 
      tx_data(chunk, 20);
      chunk_index++;
    }
    
    uint8_t* chunk = calloc(2 + message_length%18, sizeof(uint8_t));
    chunk[0] = 0xE0;
    chunk[1] = chunk_index;
    memcpy(&(chunk[2]), &(result[18*chunk_index]), message_length%18); 
    tx_data(chunk, 2 + message_length%18);
    NRF_LOG_INFO("Sent last chunk %d with %d bytes \r\n", (uint32_t)chunk_index, (uint32_t)2 + message_length%18);
    
    free(chunk);
    free(result);
    }
    
    power_manage();
    app_sched_execute();
  }
}


/**
 * @}
 */
