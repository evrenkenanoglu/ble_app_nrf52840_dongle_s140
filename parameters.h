/** @file       parameters.h
 *  @brief      System-Wide Parameters
 *  @author     Evren Kenanoglu
 *  @date       3/9/2021
 */
#ifndef FILE_PARAMETERS_H
#define FILE_PARAMETERS_H

/** INCLUDES ******************************************************************/
#include <stdint.h>
#include <boards.h>
/** CONSTANTS *****************************************************************/

/** Enable/Disable Modules**/
#define MASTER_ENABLE 0
#define SLAVE_ENABLE  (!MASTER_ENABLE)

#if MASTER_ENABLE
#define DEVICE_NAME "NORDIC_EVREN_MASTER"
#else
#define DEVICE_NAME "NORDIC_EVREN_SLAVE"
#endif

/** Filtering Parameters **/
#define FILTER_DEVICE_NAME_ENABLE 1
#define FILTER_DEVICE_NAME        "NORDIC_EVREN_MASTER"

#define RSSI_FILTER_ENABLE 1
#define RSSI_FILTER_VALUE  (-40) // dBm

#define BLE_ENABLE 1
#define ADVERTISEMENT_ENABLE 1
#define SCANNING_ENABLE 1

#define JLINK_DEBUG_PRINT_ENABLE 1

/** LED Definitions **/
#define LED_INDICATORS_ENABLE 1

#define ADVERTISEMENT_LED   BSP_BOARD_LED_0
#define SCANNING_LED        BSP_BOARD_LED_1


/** Advertisement Constants **/
#define ADVERTISEMENT_PACKET_UPDATE_INTERVAL       5000 // ms
#define MIN_ADVERTISEMENT_INTERVAL                 100  // ms
#define NUMBER_OF_ADVERTISEMENT_DURING_ADVERTISING 2
#define ADVERTISEMENT_TIMEOUT                      (MIN_ADVERTISEMENT_INTERVAL * NUMBER_OF_ADVERTISEMENT_DURING_ADVERTISING) // ms


/** Scanning Constants **/
#define BLE_SCAN_DURATION_MS 50000                       // ms
#define BLE_SCAN_DURATION    (BLE_SCAN_DURATION_MS / 10) /**< Duration of the scanning in units of 10 milliseconds. */
#define SCAN_TIMEOUT         200                         //ms

/** Sleeping Constants **/
#define SLEEP_IDLE_MODE 500 // ms
#define SLEEP_BLE_INIT  300 // ms
#define SLEEP_DURATION (SLEEP_IDLE_MODE + SLEEP_BLE_INIT)


/** Tasks Constants **/
#define TCB_PROGRAM_INIT_DELAY 1000
#define TCB_PROGRAM_TASK_INTERVAL 100
#define TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL 0

#define TARGET_UUID BLE_UUID_GATT /**< Target device name that application is looking for. */

/** TYPEDEFS ******************************************************************/

typedef enum
{
    eModeFirstStart = 0,
    eModeSleep,
    eModeInitBle,
    eModeScanning,
    eModeAdvertising,
} teModes;

typedef enum
{
    eDeviceNotDetected = 0,
    eDeviceDetected,
} teDeviceDetectionMode;

typedef struct
{
    uint8_t programStatus;
    uint32_t programCounter;
    uint8_t deviceDetectionStatus;
} tsProgramParams;

/** MACROS ********************************************************************/

#ifndef FILE_PARAMETERS_C
#define INTERFACE extern
#else
#define INTERFACE
#endif

/** VARIABLES *****************************************************************/

/** FUNCTIONS *****************************************************************/

#undef INTERFACE // Should not let this roam free

#endif // FILE_PARAMETERS_H