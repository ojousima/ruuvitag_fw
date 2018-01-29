/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

/**
 * Firmware for the RuuviTag B with weather-station functionality.
 */

// STDLIB
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

// Nordic SDK
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_gap.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "MAIN"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

// BSP
//#define BSP_SIMPLE
#include "bsp.h"

// Drivers
#include "lis2dh12.h"
#include "bme280.h"
#include "battery.h"
#include "bluetooth_core.h"
#include "eddystone.h"
#include "pin_interrupt.h"
#include "rtc.h"
#include "rng.h"
#include "nfc.h"

// Libraries
#include "base64.h"
#include "sensortag.h"

// Init
#include "init.h"

// Configuration
#include "bluetooth_config.h"

// Constants
#define DEAD_BEEF               0xDEADBEEF    //!< Value used as error code on stack dump, can be used to identify stack location on stack unwind.

// ID for main loop timer.
APP_TIMER_DEF(main_timer_id);                 // Creates timer id for our program.

// milliseconds until main loop timer function is called. Other timers can bring
// application out of sleep at higher (or lower) interval.
#define MAIN_LOOP_INTERVAL_RAW 1000u
#define ADVERTISEMENT_INTERVAL_RAW 1000u

#ifndef SYMMETRIC_KEY 
#define SYMMETRIC_KEY {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}
#endif 

static uint8_t symmetric_key[] = SYMMETRIC_KEY;
static uint8_t data_buffer[24] = { 0 };

static ruuvi_sensor_t data;

static void main_timer_handler(void * p_context);


/**@brief Function for doing power management.
 */
static void power_manage(void)
{
  // Clear both leds before sleep.
  nrf_gpio_pin_set(LED_GREEN); 
  nrf_gpio_pin_set(LED_RED);       
  
  uint32_t err_code = sd_app_evt_wait();
  APP_ERROR_CHECK(err_code);

  // Signal mode by led color.
  //if (highres) { nrf_gpio_pin_clear(LED_RED); }
  //else { nrf_gpio_pin_clear(LED_GREEN); }
}


static void updateAdvertisement(void)
{
  ret_code_t err_code = NRF_SUCCESS;
  err_code |= bluetooth_set_manufacturer_data(data_buffer, sizeof(data_buffer));
  NRF_LOG_DEBUG("Applying configuration, data status %d\r\n", err_code);
  bluetooth_apply_configuration();
}


/**@brief Timeout handler for the repeated timer
 */
void main_timer_handler(void * p_context)
{
    static int32_t  raw_t  = 0;
    static uint32_t raw_p = 0;
    static uint32_t raw_h = 0;
    static lis2dh12_sensor_buffer_t buffer;
    static int32_t acc[3] = {0};

    // Get raw environmental data.
    raw_t = bme280_get_temperature();
    raw_p = bme280_get_pressure();
    raw_h = bme280_get_humidity();
      
    // Get accelerometer data.
    lis2dh12_read_samples(&buffer, 1);  
    acc[0] = buffer.sensor.x;
    acc[1] = buffer.sensor.y;
    acc[2] = buffer.sensor.z;

    // Get battery voltage every 30.th cycle
    static uint32_t vbat_update_counter;
    static uint16_t vbat = 0;
    if(!(vbat_update_counter++%30)) { vbat = getBattery(); }
    //NRF_LOG_INFO("temperature: , pressure: , humidity: ");
    // Embed data into structure for parsing.
    parseSensorData(&data, raw_t, raw_p, raw_h, vbat, acc);
    NRF_LOG_DEBUG("temperature: %d, pressure: %d, humidity: %d x: %d y: %d z: %d\r\n", raw_t, raw_p, raw_h, acc[0], acc[1], acc[2]);
    NRF_LOG_DEBUG("VBAT: %d send %d \r\n", vbat, data.vbat);

    memset(data_buffer, 0, sizeof(data_buffer));
    encodeToSensorDataFormat(data_buffer, &data);
    nfc_init(data_buffer, sizeof(data_buffer));
    encodeToCryptedSensorDataFormat(data_buffer, &data, symmetric_key);

    updateAdvertisement();
    watchdog_feed();
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
  ret_code_t err_code = 0; // counter, gets incremented by each failed init. It is 0 in the end if init was ok.

  // Initialize log.
  err_code |= init_log();

  // Setup leds. LEDs are active low, so setting high them turns leds off.
  err_code |= init_leds();      // INIT leds first and turn RED on.
  nrf_gpio_pin_clear(LED_RED);  // If INIT fails at later stage, RED will stay lit.

  //Init NFC ASAP for reading out ID on wakeup.
  err_code |= init_nfc();

  // Initialize BLE Stack. Required in all applications for timer operation.
  err_code |= init_ble();
  bluetooth_tx_power_set(BLE_TX_POWER);
  bluetooth_configure_advertising_interval(ADVERTISEMENT_INTERVAL_RAW);
  bluetooth_configure_advertisement_type(BLE_GAP_ADV_TYPE_ADV_NONCONN_IND);
  err_code |= bluetooth_apply_configuration();

  // Initialize the application timer module.
  err_code |= init_timer(main_timer_id, MAIN_LOOP_INTERVAL_RAW, main_timer_handler);

  // Initialize BME 280 and lis2dh12. Requires timer running and pin interrups init.
  err_code |= pin_interrupt_init();
  err_code |= init_sensors();

  // Clear memory.
  lis2dh12_reset();
  // Wait for reboot.
  nrf_delay_ms(10);
  // Enable XYZ axes.
  lis2dh12_enable();
  lis2dh12_set_scale(LIS2DH12_SCALE2G);
  // Sample rate 10 for activity detection.
  lis2dh12_set_sample_rate(LIS2DH12_RATE_1);
  lis2dh12_set_resolution(LIS2DH12_RES12BIT);

  // Setup BME280 - oversampling must be set for each used sensor.
  bme280_set_oversampling_hum(BME280_OVERSAMPLING_1);
  bme280_set_oversampling_temp(BME280_OVERSAMPLING_1);
  bme280_set_oversampling_press(BME280_OVERSAMPLING_1);
  bme280_set_iir(BME280_IIR_16);
  bme280_set_interval(BME280_STANDBY_1000_MS);
  bme280_set_mode(BME280_MODE_NORMAL);
  NRF_LOG_DEBUG("BME280 configuration done\r\n");

  // Visually display init status. Hangs if there was an error, waits 3 seconds on success.
  init_blink_status(err_code);

  nrf_gpio_pin_set(LED_RED);  // Turn RED led off.
  // Turn green led on to signal model +
  // LED will be turned off in power_manage.
  nrf_gpio_pin_clear(LED_GREEN); 

  // Delay for green ledf
  nrf_delay_ms(1000);

  // Init ok, start watchdog with default wdt event handler (reset).
  init_watchdog(NULL);

  // Enter main loop.
  for (;;)
  {
    app_sched_execute();
    power_manage();
  }
}
