/*
 * ble.c
 *
 *  Created on: Dec 9, 2017
 *      Author: Eug
 */

#include "bluetooth.h"
#include "ble_bas.h"
#include "app_timer.h"
#include "app_scheduler.h"

#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble_dfu.h"
#include "ble_dis.h"
#include "nrf_log.h"
#include "nrf_pwr_mgmt.h"
#include "custom_service.h"
#include "nrf_dfu_types.h"
#include "peer_manager_handler.h"
#include "bonding.h"

#define MANUFACTURER_NAME "EugKrashtan"
#define MODEL_NAME "R3"
#define HW_REVISION_STR "00.00.01"

#define APP_BLE_OBSERVER_PRIO           2
#define APP_BLE_CONN_CFG_TAG            1

#define APP_ADV_INTERVAL                64                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION                18000                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
        	NRF_LOG_INFO("Power management wants to reset to DFU mode");
            break;

        default:
            // Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }
    return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);

/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
        	NRF_LOG_INFO("Device is preparing to enter bootloader mode");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
        	NRF_LOG_INFO("Device will enter bootloader mode");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
        	NRF_LOG_INFO("Request to enter bootloader mode failed asynchroneously");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            printf("Request to send a response to client failed\n");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
        	NRF_LOG_INFO("Unknown event from ble_dfu_buttonless");
            break;
    }
}

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;                   /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];                    /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_scan_response_data,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};

void advertising_start(bool erase_bonds) {
    if (erase_bonds == true)
        delete_bonds();
    else
        sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
}

static void advertising_init(void)
{
    ble_uuid_t adv_uuids[] = {
        {gtServices.tServices->ptVars->tUuid.uuid, BLE_UUID_TYPE_VENDOR_BEGIN},
    };
    ret_code_t           err_code;
    ble_advdata_t        advdata;
    ble_advdata_t        srdata;
    ble_gap_adv_params_t adv_params;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type             = BLE_ADVDATA_FULL_NAME;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    memset(&srdata, 0, sizeof(srdata));
    srdata.name_type          = BLE_ADVDATA_NO_NAME;
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    // Start advertising.
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.p_peer_addr   = NULL;
    adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
    adv_params.interval      = APP_ADV_INTERVAL;

    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.duration        = 0; // Adv forever ToDo: APP_ADV_DURATION on activity
    adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    uint8_t pubName[DEVICE_NAME_LEN];
    CustGetDeviceName(pubName);
    err_code = sd_ble_gap_device_name_set(&sec_mode, pubName, strlen(pubName));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS); // 0.5s
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS); // 1s
    gap_conn_params.slave_latency     = 0;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS); // 4s

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void ble_evt_handler(const ble_evt_t * p_ble_evt, void * p_context)
{
    ret_code_t err_code;
    NRF_LOG_INFO("Ble evt %d", p_ble_evt->header.evt_id);

    pm_handler_secure_on_connection(p_ble_evt);

    NRF_LOG_DEBUG("Ble evt %x",p_ble_evt->header.evt_id);
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        	NRF_LOG_INFO("Connected");
            // call on connect
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            bms_on_connect(m_conn_handle);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
        	NRF_LOG_INFO("Disconnected");
        	delete_disconnected_bonds();
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            //sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
            break;
#if 0
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                   NULL,
                                                   NULL);
            APP_ERROR_CHECK(err_code);
            break;
#endif
//#ifndef S140
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
        	NRF_LOG_INFO("PHY update request");
        const ble_gap_phys_t phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
//#endif

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
        	NRF_LOG_INFO("GATT Client Timeout");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
        	NRF_LOG_INFO("GATT Server Timeout");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
#if 0
        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
#endif
        default:
            // No implementation needed.
            break;
    }
}

static void services_init(void)
{
    ret_code_t     err_code;
    ble_dis_init_t dis_init;

    ble_dfu_buttonless_init_t dfus_init =
    {
        .evt_handler = ble_dfu_evt_handler
    };

    // Initialize the async SVCI interface to bootloader.
    err_code = ble_dfu_buttonless_async_svci_init();
    if(err_code == NRF_SUCCESS) {
    	err_code = ble_dfu_buttonless_init(&dfus_init);
    	APP_ERROR_CHECK(err_code);
    }

    err_code = CustomServiceInit(gtServices.tServices);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str, MODEL_NAME);
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, HW_REVISION_STR);

    char serial[12];
    // NRF_FICR->DEVICEADDR[] contains randomly generated address. Use it as serial
    const uint32_t *addr = (const uint32_t*)NRF_FICR->DEVICEADDR;
    sprintf(serial, "%lu", *addr);
    ble_srv_ascii_to_utf8(&dis_init.serial_num_str, serial);

    nrf_dfu_settings_t* ptNrfDfuSettings = (nrf_dfu_settings_t*)BOOTLOADER_SETTINGS_ADDRESS;
    uint32_t ulVer = ptNrfDfuSettings->app_version;
    uint8_t v1 = ulVer/10000;
    uint8_t v2 = (ulVer-v1*10000)/100;
    uint8_t v3 = ulVer-v1*10000-v2*100;

    static uint8_t ubVerStr[12];
    dis_init.fw_rev_str.p_str = ubVerStr;
    dis_init.fw_rev_str.length = sprintf(ubVerStr, "%d.%d.%d", v1, v2, v3);
    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    bond_management_init();
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void ble_stack_init()
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable vendor-specific UUIDs
    ble_cfg_t ble_cfg;
        memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 2;
    err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

    gap_params_init();

    err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
    // ????
    //err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
    //APP_ERROR_CHECK(err_code);

    services_init();
    advertising_init();
    peer_manager_init();

    // Conn params
    memset(&cp_init, 0, sizeof(cp_init));
    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(20000); // 15s
    cp_init.next_conn_params_update_delay  = APP_TIMER_TICKS(5000); // 5s
    cp_init.max_conn_params_update_count   = 3;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);

    advertising_start(true);

    // Call after service initialization (set callbacks etc)
    if (gtServices.initCompl) {
    	(*gtServices.initCompl)();
    }
}
