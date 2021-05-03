/** @file       bleall.c
 *  @brief      All ble functions, definitions, parameters... 
 *  @author     Evren Kenanoglu
 *  @date       1/27/2021
 */
#define FILE_BLEALL_C

/** INCLUDES ******************************************************************/
#include "bleall.h"

/** CONSTANTS *****************************************************************/

/** TYPEDEFS ******************************************************************/

/** MACROS ********************************************************************/

/** VARIABLES *****************************************************************/

/** LOCAL FUNCTION DECLARATIONS ***********************************************/

/** INTERFACE FUNCTION DEFINITIONS ********************************************/

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 * @param params    Ble advertising parameters pointer
 */
void advertising_init(tsBleParams *params)
{
    uint32_t err_code;
    uint8_t flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data.data.p_data = (uint8_t *)m_beacon_info;
    manuf_specific_data.data.size   = sizeof(m_beacon_info);//APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&params->advdata, 0, sizeof(params->advdata));

    params->advdata.name_type             = BLE_ADVDATA_NO_NAME;
    params->advdata.flags                 = flags;
    params->advdata.p_manuf_specific_data = &manuf_specific_data;
    *(params->advdata.p_tx_power_level) = params->txPower;

    // Initialize advertising parameters (used when starting advertising).
    memset(&params->m_adv_params, 0, sizeof(params->m_adv_params));

    params->m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED; //BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    params->m_adv_params.p_peer_addr     = NULL;                                                 // Undirected advertisement.
    params->m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    params->m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    params->m_adv_params.duration        = 0; // Never time out.

    err_code = ble_advdata_encode(&params->advdata, params->m_adv_data.adv_data.p_data, &params->m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&params->m_adv_handle, &params->m_adv_data, &params->m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for starting advertising.
 * 
 * @param params    BLE advertising parameters pointer
 */
ret_code_t bleAdvertisingStart(tsBleParams *params)
{
    ret_code_t errCode;

    errCode = sd_ble_gap_adv_start(params->m_adv_handle, APP_BLE_CONN_CFG_TAG);
    VERIFY_SUCCESS(errCode);

    params->bleAdvStatus = eBleAdvertising;
    
    return errCode;
}

/**
 * @brief Function for stop advertising.
 * 
 * @param params BLE structure object pointer
 * 
 * @return errCode returns error code
 */
ret_code_t bleAdvertisingStop(tsBleParams *params)
{
    ret_code_t errCode;
    errCode = sd_ble_gap_adv_stop(params->m_adv_handle);
    VERIFY_SUCCESS(errCode);

    params->bleAdvStatus = eBleIdle;

    return errCode;
}

/**
 * @brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(tsBleParams *params)
{
    ret_code_t err_code;
  
    if(nrf_sdh_is_enabled())
    {
    }
    else
    {
      err_code = nrf_sdh_enable_request();
      APP_ERROR_CHECK(err_code);
    }

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code           = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

}

/**
 * @brief Function to initiliaze BLE params
 * 
 * @param params BLE Parameters pointer
 */
void ble_params_init(tsBleParams *params)
{
    params->m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */

    /**@brief Struct that contains pointers to the encoded advertising data. */
    params->m_adv_data.adv_data.p_data = params->m_enc_advdata;
    params->m_adv_data.adv_data.len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX;

    params->m_adv_data.scan_rsp_data.p_data = NULL;
    params->m_adv_data.scan_rsp_data.len    = 0;

    params->bleAdvStatus = eBleIdle;
    params->txPower      = POWER_TX_LEVEL_0_DB;
}

/**
 * @brief Function to update Ble advetising data
 * 
 * @details This function stop advertising and encodes required data for advertising and start it again.
 *  
 * @param params            BLE advertising parameters pointer
 * @param updateData        Data pointer to be updated
 * @param updateDataSize    Data size to be updated 
 * 
 * @return ret_code_t       returns error code
 */
ret_code_t bleAdvUpdateData(tsBleParams *params, void *updateData, uint32_t updateDataSize)
{
    ret_code_t errCode;

    if(params->bleAdvStatus == eBleAdvertising)
    {
      errCode = bleAdvertisingStop(params);
      VERIFY_SUCCESS(errCode);
      params->bleAdvStatus = eBleIdle;
    }
    else if(params->bleAdvStatus == eBleScanning)
    {
        return 0; // if it's in scanning phase, we can't advertise...
    }
    uint8_t flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_t newAdvData;
    ble_advdata_manuf_data_t manuf_specific_data;

    memset(&newAdvData, 0, sizeof(newAdvData));

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data.data.p_data        = (uint8_t *)updateData;
    manuf_specific_data.data.size          = updateDataSize;

    newAdvData.name_type             = BLE_ADVDATA_FULL_NAME;
    newAdvData.flags                 = flags;
    newAdvData.p_manuf_specific_data = &manuf_specific_data;
    *(newAdvData.p_tx_power_level) = params->txPower;


    sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, params->m_adv_handle, params->txPower);

    errCode = ble_advdata_encode(&newAdvData, params->m_adv_data.adv_data.p_data, &params->m_adv_data.adv_data.len);
    VERIFY_SUCCESS(errCode);

    errCode = sd_ble_gap_adv_set_configure(&params->m_adv_handle, &params->m_adv_data, &params->m_adv_params);
    VERIFY_SUCCESS(errCode);
  
    if(params->bleAdvStatus == eBleIdle)
    {
      errCode = bleAdvertisingStart(params);
      VERIFY_SUCCESS(errCode);
      params->bleAdvStatus = eBleAdvertising;
    }
    return errCode;
}

/**
 * @brief Function to Initialize BLE Scanning
 * 
 * @param params BLE scan parameters pointer
 * @return ret_code_t returns error code 
 */
ret_code_t bleScanInit(tsBleScanParams *params)
{
    ret_code_t errCode;
    memset(&params->initScan, 0, sizeof(params->initScan));

    params->initScan.p_scan_param     = &params->scanParam;
    params->initScan.connect_if_match = false;
    params->initScan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    errCode = nrf_ble_scan_init(params->scanModule, &params->initScan, params->scanEventHandler);
    VERIFY_SUCCESS(errCode);

    return errCode;
}

/**
 * @brief Function to Start Scanning
 * 
 * @param params BLE Scan parameters
 * @return ret_code_t returns error code
 */
ret_code_t bleScanStart(tsBleScanParams *params)
{
    ret_code_t errCode;
    errCode = nrf_ble_scan_params_set(params->scanModule, &params->scanParam);
    VERIFY_SUCCESS(errCode);

    errCode = nrf_ble_scan_start(params->scanModule);
    VERIFY_SUCCESS(errCode);

    return errCode;
}

/**
 * @brief Function to stop scanning
 */
void bleScanStop(tsBleScanParams *params)
{
    nrf_ble_scan_stop(); 
}

/**
 * @brief Function for setting Ble scan filter parameters 
 * 
 * @return errCode  returns Error Code
*/
ret_code_t bleScanFilterSet(tsBleScanParams *params, tsBleScanFilters *paramsFilter)
{
    ret_code_t errCode;
    errCode = nrf_ble_scan_filter_set(params->scanModule, paramsFilter->filterType, paramsFilter->filter);
    return errCode;
}

/**@brief Function for enabling Ble scan filters*/
ret_code_t bleScanFiltersEnable(tsBleScanParams *params)
{
    ret_code_t errCode;
    errCode = nrf_ble_scan_filters_enable(params->scanModule, NRF_BLE_SCAN_NAME_FILTER, false);
    return errCode;
}

/**@brief Function for disabling Ble scan filters*/
ret_code_t bleScanFiltersDisable(tsBleScanParams *params)
{
    ret_code_t errCode;
    errCode = nrf_ble_scan_filters_disable(params->scanModule);
    return errCode;
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name.
 */
void gap_params_init(const char *deviceName)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;

    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)deviceName,
                                          strlen(deviceName));
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module. */
void gattInit(tsBleParams *params)
{
    ret_code_t err_code = nrf_ble_gatt_init(params->gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for softdevice enabling */
ret_code_t sdEnable(void)
{
    ret_code_t errCode;
    
    nrf_clock_lf_cfg_t const clock_lf_cfg =
    {
        .source       = NRF_SDH_CLOCK_LF_SRC,
        .rc_ctiv      = NRF_SDH_CLOCK_LF_RC_CTIV,
        .rc_temp_ctiv = NRF_SDH_CLOCK_LF_RC_TEMP_CTIV,
        .accuracy     = NRF_SDH_CLOCK_LF_ACCURACY
    };

    errCode = sd_softdevice_enable(&clock_lf_cfg, app_error_fault_handler);

    return errCode;
}

/**@brief Function for softdevice disabling */
ret_code_t sdDisable(void)
{
    ret_code_t errCode;
    
    errCode = sd_softdevice_disable();

    return errCode;
}

/** LOCAL FUNCTION DEFINITIONS ************************************************/
