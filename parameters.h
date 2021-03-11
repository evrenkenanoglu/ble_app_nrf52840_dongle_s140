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
#define DEVICE_NAME "BEACON_MASTER"

/** Enable/Disable Modules**/
#define MASTER_ENABLE 1
#define SLAVE_ENABLE  (!MASTER_ENABLE)

#define ADVERTISEMENT_ENABLE 1
#define SCANNING_ENABLE      1

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
#define SLEEP_DURATION ()

/** Tasks Constants **/
#define TCB_MAIN_TASK_INTERVAL 100
#define TCB_MAIN_TASK_SWITCH_MODE_INTERVAL 0

/** Filtering Parameters **/
#define FIlTER_DEVICE_NAME_ENABLE 0
#define FIlTER_DEVICE_NAME        "NORDIC_EVREN"

#define TARGET_UUID BLE_UUID_GATT /**< Target device name that application is looking for. */

/** TYPEDEFS ******************************************************************/

typedef enum
{
    eModeFirstStart = 0,
    eModeSleep,
    eModeScanning,
    eModeAdvertising,

} teModes;

typedef struct 
{
    uint8_t programStatus;
    uint32_t programCounter;
}tsProgramParams;

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
