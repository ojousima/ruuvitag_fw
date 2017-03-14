/* This file was generated by plugin 'Nordic Semiconductor nRF5x v.1.2.2' (BDS version 1.1.3135.0) */

#ifndef BLE_TPS_H__
#define BLE_TPS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "app_util_bds.h"


// Error codes 
#define REPLACE_TABLE_BY__THIS_SERVICE_DOES_NOT_DEFINE_ANY_APPLICATION_ERROR_CODES_THAT_ARE_USED_IN_ATTRIBUTE_PROTOCOL_ 0xA0; /*  */

/**@brief Tx Power event type. */
typedef enum
{ 
    BLE_TPS_TX_POWER_LEVEL_EVT_NOTIFICATION_ENABLED,  /**< Tx Power Level value notification enabled event. */
    BLE_TPS_TX_POWER_LEVEL_EVT_NOTIFICATION_DISABLED, /**< Tx Power Level value notification disabled event. */
} ble_tps_evt_type_t;

// Forward declaration of the ble_tps_t type.
typedef struct ble_tps_s ble_tps_t;








/**@brief Tx Power Level structure. */
typedef struct
{
    int8_t tx_power;
} ble_tps_tx_power_level_t;

/**@brief Tx Power Service event. */
typedef struct
{
    ble_tps_evt_type_t evt_type;    /**< Type of event. */
    union {
        uint16_t cccd_value; /**< Holds decoded data in Notify and Indicate event handler. */
    } params;
} ble_tps_evt_t;

/**@brief Tx Power Service event handler type. */
typedef void (*ble_tps_evt_handler_t) (ble_tps_t * p_tps, ble_tps_evt_t * p_evt);

/**@brief Tx Power Service init structure. This contains all options and data needed for initialization of the service */
typedef struct
{
    ble_tps_evt_handler_t     evt_handler; /**< Event handler to be called for handling events in the Tx Power Service. */
    ble_tps_tx_power_level_t ble_tps_tx_power_level_initial_value; /**< If not NULL, initial value of the Tx Power Level characteristic. */ 
} ble_tps_init_t;

/**@brief Tx Power Service structure. This contains various status information for the service.*/
struct ble_tps_s
{
    ble_tps_evt_handler_t evt_handler; /**< Event handler to be called for handling events in the Tx Power Service. */
    uint16_t service_handle; /**< Handle of Tx Power Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t tx_power_level_handles; /**< Handles related to the Tx Power Level characteristic. */
    uint16_t conn_handle; /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
};

/**@brief Function for initializing the Tx Power.
 *
 * @param[out]  p_tps       Tx Power Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_tps_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_tps_init(ble_tps_t * p_tps, const ble_tps_init_t * p_tps_init);

/**@brief Function for handling the Application's BLE Stack events.*/
void ble_tps_on_ble_evt(ble_tps_t * p_tps, ble_evt_t * p_ble_evt);

/**@brief Function for setting the Tx Power Level.
 *
 * @details Sets a new value of the Tx Power Level characteristic. The new value will be sent
 *          to the client the next time the client reads the Tx Power Level characteristic.
 *          This function is only generated if the characteristic's Read property is not 'Excluded'.
 *
 * @param[in]   p_tps                 Tx Power Service structure.
 * @param[in]   p_tx_power_level  New Tx Power Level.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_tps_tx_power_level_set(ble_tps_t * p_tps, ble_tps_tx_power_level_t * p_tx_power_level);

#endif //_BLE_TPS_H__
