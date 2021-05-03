/** @file       main.c
 *  @brief      Beacon/Scanner BLE Program.
 *  @author     Evren Kenanoglu
 *  @date       1/03/2021
 */
/**
 * @brief Beacon/Scanner BLE Program.
 *
 * This file contains the source code for an Beacon transmitter/Scanner application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "bsp.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "app_timer.h"
#include "task_manager.h"
#include "nrf_pwr_mgmt.h"
#include "ble_advertising.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_error.h"

#include "nrf_delay.h"

#include "boardinit.h"
#include "bleall.h"

#include "parameters.h"
/** CONSTANTS *****************************************************************/
/** MACROS ********************************************************************/

NRF_BLE_SCAN_DEF(bleScanModule); /**< Scanning Module instance. */
NRF_BLE_GATT_DEF(gattModule);    /**< GATT module instance. */

/** VARIABLES *****************************************************************/

tsBleScanParams bleScanParams;
tsBleParams BLEParams;
tsProgramParams programParams = {.programStatus  = eModeFirstStart,
                                 .programCounter = 0};

/**< Scan parameters requested for scanning */
static ble_gap_scan_params_t const bleGapScanParams =
    {
        .active        = 0x01,
        .interval      = NRF_BLE_SCAN_SCAN_INTERVAL,
        .window        = NRF_BLE_SCAN_SCAN_WINDOW,
        .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
        .timeout       = BLE_SCAN_DURATION,
        .scan_phys     = BLE_GAP_PHY_1MBPS,
};


// Dummy packages for advertising
uint16_t dummyValue = 0xFF;

uint8_t advertisingDataPacket[] =
    {
        0xFF, /// Dummy Values
        0xAA, /// Dummy Values
        0xFF, /// Dummy Values

};
uint8_t advertisingDataPacket2[] =
    {
        0xAA, /// Dummy Values
        0xFF, /// Dummy Values
        0xAA, /// Dummy Values
};
uint8_t advertisingDataPacket3[] =
    {
        0xBB, /// Dummy Values
        0xBB, /// Dummy Values
        0xBB, /// Dummy Values
};

uint8_t dataPacketExpected[] =
{
    APP_COMPANY_IDENTIFIER,
    0xAA,
    0xFF,
    0xAA,
};


/** LOCAL FUNCTION DECLARATIONS ***********************************************/
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name);
static void bleEventHandler(ble_evt_t const *p_ble_evt, void *p_context); 
static void idle_state_handle(void);
static void createTimers();
static void timerCBRefreshAdvData();
static char compareArray(uint8_t *arrayFirst, uint8_t *arraySecond, uint8_t size);
static void deviceDetectionHandler(void);


#if MASTER_ENABLE
APP_TIMER_DEF(timerProgramMaster);
#else
APP_TIMER_DEF(timerProgramSlave);
#endif
APP_TIMER_DEF(timerRefreshAdvDataBLE);


static void tcbProgramMasterHandler();
static void startTimers();

uint32_t counter = 0;

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    boardInit();
    createTimers();

#if BLE_ENABLE
    //BLEParams.bleEventHandler = bleEventHandler;
    BLEParams.gatt           = &gattModule;
    bleScanParams.scanModule = &bleScanModule;
    bleScanParams.scanParam  = bleGapScanParams;

    ble_params_init(&BLEParams);
    ble_stack_init(&BLEParams);
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, bleEventHandler, NULL);

    gap_params_init(DEVICE_NAME);
    gattInit(&BLEParams);

#if ADVERTISEMENT_ENABLE
    advertising_init(&BLEParams);
#endif

#if SCANNING_ENABLE
    bleScanInit(&bleScanParams);
#endif

#endif
    startTimers();

    NRF_LOG_INFO("Program started.");
    
    // Enter main loop. This is for power management. 
    for (;;)
    {
        idle_state_handle();
    }
}

/** FUNCTIONS *****************************************************************/

/**
 * @brief Refresh Ble Advertising Data
 * 
 */
static void timerCBRefreshAdvData()
{
    static uint8_t i = 0;
    ret_code_t errCode;

    ///> TX Power Lever Supported: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
    switch (i)
    {
            // case 0:

            //     BLEParams.txPower = POWER_TX_LEVEL_MINUS_40_DB;
            //     errCode =bleAdvUpdateData(&BLEParams, advertisingDataPacket, sizeof(advertisingDataPacket));
            //     APP_ERROR_CHECK(errCode);
            //     printf("Now First Package is being advertised.\n");

            //     break;
            // case 1:
            // {
            //     BLEParams.txPower = POWER_TX_LEVEL_0_DB;
            //     errCode = bleAdvUpdateData(&BLEParams, advertisingDataPacket2, sizeof(advertisingDataPacket2));
            //     APP_ERROR_CHECK(errCode);
            //     printf("Now Second Package is being advertised.\n");

            // }
            //     break;
            // case 2:
            //     BLEParams.txPower = POWER_TX_LEVEL_4_DB;
            //     errCode = bleAdvUpdateData(&BLEParams, advertisingDataPacket3, sizeof(advertisingDataPacket3));
            //     APP_ERROR_CHECK(errCode);
            //     printf("Now Third Package is being advertised.\n");
            // break;

        default:
            break;
    }

    if (i > 1)
    {
        i = 0;
    }
    else
    {
        i++;
    }
}


#if MASTER_ENABLE
/**
 * @brief Master Program Handler
 *      
 * @details Master program is always in a loop of scanning in duration of SCAN_TIMEOUT and advertising in duration of ADVERTISEMENT_TIMEOUT.
 * 
 */
static void tcbProgramMasterHandler()
{
    ret_code_t errCode;
    ///> TX Power Lever Supported: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
    switch (programParams.programStatus)
    {
/****************************************************************************************************
* 											    FIRST START
*****************************************************************************************************/
        case eModeFirstStart:
        {
            programParams.programStatus  = eModeScanning;
            programParams.programCounter = 0;
            app_timer_start(timerProgramMaster, APP_TIMER_TICKS(1000), NULL);
#if JLINK_DEBUG_PRINT_ENABLE
            printf("Program Started!\n");
#endif
        }
        break;
/****************************************************************************************************
* 											    FIRST START END
*****************************************************************************************************/
/****************************************************************************************************
* 											    SLEEP
*****************************************************************************************************/
        case eModeSleep:
        {
            if(programParams.programCounter >= SLEEP_DURATION) // Sleeping Timeout... Mode Changing to Scanning Mode... 
            {
                programParams.programStatus = eModeScanning; // eModeScanning
                programParams.programCounter = 0;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);

            }
            else // Sleeping...
            {
                programParams.programCounter += TCB_PROGRAM_SLEEP_MODE_INTERVAL;
                programParams.programStatus = eModeSleep;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_SLEEP_MODE_INTERVAL), NULL);                
            }
        }
        break;
/****************************************************************************************************
* 											    SLEEP END
*****************************************************************************************************/
    
/****************************************************************************************************
* 											    SCANNING
*****************************************************************************************************/
        case eModeScanning:
        {
            if (programParams.programCounter >= SCAN_TIMEOUT) // Scanning Timeout... Mode Changing to Advertisement....
            {
                bleScanStop(&bleScanParams);

                BLEParams.bleAdvStatus       = eBleIdle;
                programParams.programCounter = 0;
                programParams.programStatus  = eModeAdvertising;
                app_timer_start(timerProgramMaster, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Scanning Timeout!\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_off(SCANNING_LED);
#endif
            }
            else // Scanning...
            {
                if (!(BLEParams.bleAdvStatus == eBleScanning))
                {
                    errCode = bleScanStart(&bleScanParams);
                    APP_ERROR_CHECK(errCode);
                }
                BLEParams.bleAdvStatus = eBleScanning;
                programParams.programCounter += TCB_PROGRAM_TASK_INTERVAL;
                app_timer_start(timerProgramMaster, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Scanning...\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_on(SCANNING_LED);
#endif
            }
        }
        break;
/****************************************************************************************************
* 											    SCANNING END
*****************************************************************************************************/

/****************************************************************************************************
* 											    ADVERTISING
*****************************************************************************************************/
        case eModeAdvertising:
        {
            if (programParams.programCounter >= ADVERTISEMENT_TIMEOUT) // Advertising Timeout... Mode Changing to Scanning....
            {
                bleAdvertisingStop(&BLEParams);
                BLEParams.bleAdvStatus       = eBleIdle;
                programParams.programCounter = 0;
                programParams.programStatus  = eModeScanning;
                app_timer_start(timerProgramMaster, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Advertising Timeout!\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_off(ADVERTISEMENT_LED);
#endif
            }
            else // Advertising...
            {
                errCode                = bleAdvUpdateData(&BLEParams, advertisingDataPacket2, sizeof(advertisingDataPacket2));
                BLEParams.bleAdvStatus = eBleAdvertising;
                programParams.programCounter += TCB_PROGRAM_TASK_INTERVAL;
                app_timer_start(timerProgramMaster, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Advertising...!\n");
#endif

#if LED_INDICATORS_ENABLE

                bsp_board_led_on(ADVERTISEMENT_LED);
#endif
            }
        }
        break;
/****************************************************************************************************
* 											    ADVERTISING END
*****************************************************************************************************/

        default:
            break;
    }
}

#else

/**
 * @brief Slave Program Handler
 *      
 * @details Slave program starts with scanning mode in SCAN_TIMEOUT ms.
 *          If master device is detected in the environment program modes is changed to advertising(ADVERTISEMENT_TIMEOUT).
 *          if master device is not detected in the environment, program goes back to the sleep.(SLEEP_DURATION)
 *          After that program starts to scan again.
 * 
 * @warning FILTER_DEVICE_NAME_ENABLE has to be setted 1 for detecting master device.
 * 
 */
static void tcbProgramSlaveHandler()
{
    ret_code_t errCode;
    ///> TX Power Lever Supported: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
    switch (programParams.programStatus)
    {
/****************************************************************************************************
* 											    FIRST START
*****************************************************************************************************/
        case eModeFirstStart:
        {
            programParams.programStatus  = eModeScanning; // eModeScanning
            programParams.programCounter = 0;
            app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_INIT_DELAY), NULL);
#if JLINK_DEBUG_PRINT_ENABLE
            printf("Program Started!\n");
#endif
        }
        break;

/****************************************************************************************************
* 											    FIRST START END
*****************************************************************************************************/

/****************************************************************************************************
* 											    SLEEP
*****************************************************************************************************/
        case eModeSleep:
        {
            if(programParams.programCounter >= SLEEP_DURATION) // Sleeping Timeout... Mode Changing to Scanning Mode... 
            {
                programParams.programStatus = eModeScanning; // eModeScanning
                programParams.programCounter = 0;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);

            }
            else // Sleeping...
            {
                programParams.programCounter += TCB_PROGRAM_SLEEP_MODE_INTERVAL;
                programParams.programStatus = eModeSleep;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_SLEEP_MODE_INTERVAL), NULL);                
            }
        }
        break;
/****************************************************************************************************
* 											    SLEEP END
*****************************************************************************************************/

/****************************************************************************************************
* 											    SCANNING
*****************************************************************************************************/
        case eModeScanning:
        {
            if (programParams.programCounter >= SCAN_TIMEOUT) // Scanning Timeout... Mode Changing to Advertisement/Sleep....
            {
                bleScanStop(&bleScanParams);

                BLEParams.bleAdvStatus       = eBleIdle;
                programParams.programCounter = 0;

                if(programParams.deviceDetectionStatus == eDeviceDetected) // if master device is detected in the environment during scanning...
                {
                    programParams.programStatus = eModeAdvertising;
                    app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);
                    programParams.deviceDetectionStatus = eDeviceNotDetected; // Clear detection Status...
                }
                else // if master device is not detected in the environment during scanning...
                {
                    programParams.programStatus = eModeSleep; ///eModeSleep
                    app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);
                }

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Scanning Timeout!\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_off(SCANNING_LED);
#endif
            }
            else // Scanning...
            {
                if (!(BLEParams.bleAdvStatus == eBleScanning))
                {
                    errCode = bleScanStart(&bleScanParams);
                    APP_ERROR_CHECK(errCode);
                }
                BLEParams.bleAdvStatus = eBleScanning;
                programParams.programCounter += TCB_PROGRAM_TASK_INTERVAL;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Scanning...\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_on(SCANNING_LED);
#endif
            }
        }
        break;

/****************************************************************************************************
* 											    SCANNING END
*****************************************************************************************************/

/****************************************************************************************************
* 											    ADVERTISING
*****************************************************************************************************/
        case eModeAdvertising:
        {
            if (programParams.programCounter >= ADVERTISEMENT_TIMEOUT) // Advertising Timeout... Mode Changing to Scanning....
            {
                bleAdvertisingStop(&BLEParams);
                BLEParams.bleAdvStatus       = eBleIdle;
                programParams.programCounter = 0;
                programParams.programStatus  = eModeScanning; // eModeScanning
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_SWITCH_MODE_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Advertising Timeout!\n");
#endif

#if LED_INDICATORS_ENABLE
                bsp_board_led_off(ADVERTISEMENT_LED);
#endif
            }
            else // Advertising...
            {
                errCode                = bleAdvUpdateData(&BLEParams, advertisingDataPacket2, sizeof(advertisingDataPacket2));
                BLEParams.bleAdvStatus = eBleAdvertising;
                programParams.programCounter += TCB_PROGRAM_TASK_INTERVAL;
                app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);

#if JLINK_DEBUG_PRINT_ENABLE
                printf("Advertising...!\n");
#endif

#if LED_INDICATORS_ENABLE

                bsp_board_led_on(ADVERTISEMENT_LED);
#endif
            }
        }
        break;
/****************************************************************************************************
* 											    ADVERTISING END 
*****************************************************************************************************/

        default:
            break;
    }
}

#endif

/**@brief Create App Timer Objects */
void createTimers()
{
    ret_code_t errCode;
//errCode = app_timer_create(&timerRefreshAdvDataBLE, APP_TIMER_MODE_REPEATED, timerCBRefreshAdvData);
#if MASTER_ENABLE
    errCode = app_timer_create(&timerProgramMaster, APP_TIMER_MODE_SINGLE_SHOT, tcbProgramMasterHandler);
    NRF_LOG_INFO(" Program Started as Master!");
#else
    errCode = app_timer_create(&timerProgramSlave, APP_TIMER_MODE_SINGLE_SHOT, tcbProgramSlaveHandler);
    NRF_LOG_INFO(" Program Started as Slave!");
    APP_ERROR_CHECK(errCode);
#endif
}


/**@brief Starts App Timers*/
void startTimers()
{
    ret_code_t errCode;
    //errCode = app_timer_start(timerRefreshAdvDataBLE,   APP_TIMER_TICKS(ADVERTISEMENT_PACKET_UPDATE_INTERVAL), NULL);
#if MASTER_ENABLE
    errCode = app_timer_start(timerProgramMaster, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);
#else
    errCode = app_timer_start(timerProgramSlave, APP_TIMER_TICKS(TCB_PROGRAM_TASK_INTERVAL), NULL);
    APP_ERROR_CHECK(errCode);

#endif
}


/**
 * @brief All BLE Events are handled by this function
 * 
 * @param p_ble_evt BLE Event Pointer 
 * @param p_context Context Pointer
 * 
 * @details In this function, all ble device in the environment are scanned and reported. 
 *          Also filtering with the device name and filtering with RSSI are available.
 *          
 *          After device detection, program calls deviceDetectionHandler() function.
 */
static void bleEventHandler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code;
    ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
            nrf_ble_scan_t *p_scan_data                  = (nrf_ble_scan_t *)p_context;
            ble_gap_evt_adv_report_t const *p_adv_report = &p_ble_evt->evt.gap_evt.params.adv_report;

            //if(124==p_adv_report->peer_addr.addr[0])
            {
                
#if FILTER_DEVICE_NAME_ENABLE

                char *p_msg         = "Name: %s\n\r";
                uint8_t *deviceName = ble_advdata_parse(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);

                if (!strcmp(deviceName, FILTER_DEVICE_NAME))
                {
                    // Name
                    counter++;
                    printf("%d\n\r", counter);
                    if (NULL == deviceName)
                    {
                        char *p_msg = "Name: %s\n\r";

                        deviceName = ble_advdata_parse(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME);
                    }
                    if (NULL != deviceName)
                    {
                        printf(p_msg, deviceName);
                    }
                    else
                    {
                        printf("Name: No Name\n\r");
                    }

                    /// Address
                    printf("Address: ");
                    for (int i = 0; i < sizeof(p_adv_report->peer_addr.addr); i++)
                    {
                        if (i == sizeof(p_adv_report->peer_addr.addr) - 1)
                        {
                            printf("%02x", p_adv_report->peer_addr.addr[i]);
                        }
                        else
                        {
                            printf("%02x:", p_adv_report->peer_addr.addr[i]);
                        }
                    }
                    printf("\n\r");

                    /// Manufacturer Data 
                    p_msg                     = "Manufacturer Data : ";
                    uint8_t *manufacturerData = ble_advdata_parse(p_adv_report->data.p_data, p_adv_report->data.len, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
                    if (manufacturerData == NULL)
                    {
                    }
                    else
                    {
                        printf(p_msg);
                        for (int i = 0; i < sizeof(manufacturerData) + 1; i++)
                        {
                            if (i == sizeof(manufacturerData))
                            {
                                printf("%02x", manufacturerData[i]);
                            }
                            else
                            {
                                printf("%02x:", manufacturerData[i]);
                            }
                        }
                        printf("\n\r");
                    }
                    
                    /// RSSI POWER
                    printf("RSSI: %d\n\r", p_adv_report->rssi);

#if RSSI_FILTER_ENABLE
                    if (p_adv_report->rssi > RSSI_FILTER_VALUE)
                    {
                        deviceDetectionHandler(); // Filtered Device Detected Handler
                    }
#else
                    deviceDetectionHandler(); // Filtered Device Detected Handler

#endif

                    // if(!compareArray(manufacturerData, dataPacketExpected, sizeof(manufacturerData + 1) ))
                    // {
                    //     // TO DO: Verifying... 
                    // }

                    //printf("data: ");
                    //for (int i = 0; i < p_adv_report->data.len; i++)
                    //{
                    //
                    //    printf("%02x:", p_adv_report->data.p_data[i]);
                    //}
                }

#else
                char *p_msg         = "Name: %s\n\r";
                uint8_t *deviceName = ble_advdata_parse(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);

                if (NULL == deviceName)
                {
                    char *p_msg = "Name: %s\n\r";

                    deviceName = ble_advdata_parse(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME);
                }
                if (NULL != deviceName)
                {
                    printf(p_msg, deviceName);
                }
                else
                {
                    printf("Name: No Name\n\r");
                }

                /// Address
                printf("Address: ");
                for (int i = 0; i < sizeof(p_adv_report->peer_addr.addr); i++)
                {
                    printf("%02x:", p_adv_report->peer_addr.addr[i]);
                }
                printf("\n\r");

                p_msg                     = "Manufacturer Data : ";
                uint8_t *manufacturerData = ble_advdata_parse(p_adv_report->data.p_data, p_adv_report->data.len, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
                if (manufacturerData == NULL)
                {
                }
                else
                {
                    printf(p_msg);
                    for (int i = 0; i < sizeof(manufacturerData) + 1; i++)
                    {
                        if (i == sizeof(manufacturerData))
                        {
                            printf("%02x", manufacturerData[i]);
                        }
                        else
                        {
                            printf("%02x:", manufacturerData[i]);
                        }
                    }
                    printf("\n\r");
                }

                counter++;

                //printf("data: ");
                //for (int i = 0; i < p_adv_report->data.len; i++)
                //{
                //
                //    printf("%02x:", p_adv_report->data.p_data[i]);
                //}

                printf("\n\r");
#endif
            }

            
        }

        break;
    }
}
/**
 * @brief Handler after detection master device in the environment
 * 
 */
static void deviceDetectionHandler(void)
{
    programParams.deviceDetectionStatus = eDeviceDetected;
}

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
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

char compareArray(uint8_t *arrayFirst, uint8_t *arraySecond, uint8_t size)
{
    if (arrayFirst == NULL || arraySecond == NULL)
    {
        return 0;
    }
    {
        uint8_t i;
        for (i = 0; i < size; i++)
        {
            if (arrayFirst[i] != arrayFirst[i])
                return 1;
        }
        return 0;
    }
}

/**
 * @}
 */
