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

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 * @return error code from BLE stack initialization, NRF_SUCCESS if init was ok
 */

#include "bluetooth_core.h"
#include "ble_advdata.h"
#include "ble_advertising.h"

#define NRF_LOG_MODULE_NAME "BLE_CORE"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#ifndef CENTRAL_LINK_COUNT
    #error "Please define CENTRAL_LINK_COUNT in bluetooth_config.h"
#endif
#ifndef PERIPHERAL_LINK_COUNT
    #error "Please define PERIPHERAL_LINK_COUNT in bluetooth_config.h"
#endif
#ifndef PERIPHERAL_LINK_COUNT
    #error "Please define PERIPHERAL_LINK_COUNT in bluetooth_config.h"
#endif
#ifndef BLE_COMPANY_IDENTIFIER
    #error "Please define BLE_COMPANY_IDENTIFIER in bluetooth_config.h"
#endif

uint32_t ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    return err_code;
}

/**
 * @brief Function to setsBLE transmission power
 *  
 * @details set the BLE transmission power in dBm
 * @param int8_t power power in dBm, must be one of -40, -30, -20, -16, -12, -8, -4, 0, 4
 * @return error code, 0 if operation was success.
 */
uint32_t ble_tx_power_set(int8_t power)
{
    uint32_t err_code = sd_ble_gap_tx_power_set(power);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

/**@brief Function for advertising data. 
 *
 * @details Initializes the BLE advertisement with given data as manufacturer specific data.
 * Companyt ID is included by default and doesn't need to be included.  
 *
 * @param data pointer to data to advertise, maximum length 24 bytes
 * @param length length of data to advertise
 *
 * @return error code from BLE stack initialization, NRF_SUCCESS if init was ok
 */
uint32_t bluetooth_advertise_data(uint8_t *data, uint8_t length)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    
    // Variables used for manufacturer specific data
    ble_advdata_manuf_data_t adv_manuf_data;
    uint8_array_t            adv_manuf_data_array;
    uint8_t                  adv_manuf_data_data[2];

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));
    
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
    advdata.flags                   = flags;

    
    // Configuration of manufacturer specific data
    adv_manuf_data_data[0] = 0x12;
    adv_manuf_data_data[1] = 0x13;
    
    adv_manuf_data_array.p_data = adv_manuf_data_data;
    adv_manuf_data_array.size = 2;
    
    adv_manuf_data.company_identifier = BLE_COMPANY_IDENTIFIER;
    adv_manuf_data.data = adv_manuf_data_array;
    
    advdata.p_manuf_specific_data = &adv_manuf_data;
    // ---------------------------------------------
    
    err_code = ble_advertising_init(&advdata, NULL, NULL, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    return err_code;
    
}


