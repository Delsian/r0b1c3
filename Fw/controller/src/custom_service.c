/*
 * custom_service.c
 *
 */

#include <stdlib.h>
#include "ble_gap.h"
#include "custom_service.h"
#include "sdk_macros.h"
#include "app_scheduler.h"
#include "nrf_log.h"
#include "nrf_dfu_types.h"
#include "crc32.h"
#include "control.h"

uint8_t m_dfu_settings_buffer[CODE_PAGE_SIZE]
    __attribute__((section(".bootloader_settings_page")))
    __attribute__((used));

static tCustomServiceVars* ptCustVar;
static nrf_mtx_t tMtxNotify;

uint16_t GetConnectionHandle()
{
	return ptCustVar->usConnHandle;
}

static tCustomChar const * GetByCccd(uint16_t iusCccd)
{
	tCustomService const * tServ = gtServices.tServices;
	uint8_t j = 0;
	while (tServ->ptChars[j].usUuid)
	{
		if (tServ->ptChars[j].ptHandle && tServ->ptChars[j].ptHandle->hcccd == iusCccd )
			return &(tServ->ptChars[j]);
		j++;
	}
    return NULL;
}

static tCustomChar const * GetByVal(uint16_t iusVal)
{
	tCustomService const * tServ = gtServices.tServices;
	uint8_t j = 0;
	while (tServ->ptChars[j].usUuid)
	{
		if (tServ->ptChars[j].ptHandle && tServ->ptChars[j].ptHandle->hval == iusVal )
			return &(tServ->ptChars[j]);
		j++;
	}
    return NULL;
}

static void CustSetDeviceName( void* ipName, uint16_t size)
{
	ble_gap_conn_sec_mode_t sec_mode;
	uint8_t* ipubName = (uint8_t*)ipName;
	(void)size;

	NRF_LOG_INFO("New name %s", ipubName);
	nrf_dfu_settings_t* ptSettings = malloc(sizeof(nrf_dfu_settings_t));
	memcpy((void*)ptSettings, m_dfu_settings_buffer, sizeof(nrf_dfu_settings_t));
    nrf_dfu_adv_name_t * pAdvName = &(ptSettings->adv_name);
    strncpy(pAdvName->name, ipubName, DEVICE_NAME_LEN-1);
    pAdvName->name[DEVICE_NAME_LEN-1] = '\0';
    pAdvName->len = strlen(pAdvName->name);
    pAdvName->crc = crc32_compute((uint8_t*)pAdvName + 4, sizeof(nrf_dfu_adv_name_t) - 4, NULL);
    //FsErase((uint32_t)m_dfu_settings_buffer, 1);
    //FsWriteFree((uint32_t)m_dfu_settings_buffer, (uint8_t*)ptSettings, sizeof(nrf_dfu_settings_t));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	sd_ble_gap_device_name_set(&sec_mode,(uint8_t const *) ipubName, strlen(ipubName));
    free(ipubName);
}

void CustGetDeviceName(uint8_t* opubBuf)
{
    nrf_dfu_settings_t* ptSettings = (nrf_dfu_settings_t*)m_dfu_settings_buffer;
    nrf_dfu_adv_name_t* pAdvName = &(ptSettings->adv_name);
    if (pAdvName->crc == crc32_compute((uint8_t*)pAdvName + 4, sizeof(nrf_dfu_adv_name_t) - 4, NULL)) {
    	strncpy(opubBuf, (uint8_t*)pAdvName + 4, DEVICE_NAME_LEN -1);
    	opubBuf[DEVICE_NAME_LEN-1] = '\0';
    } else {
    	uint8_t* pName = (uint8_t*) malloc(DEVICE_NAME_LEN);
    	strncpy(pName, gtServices.pubDeviceName, DEVICE_NAME_LEN);
    	APP_ERROR_CHECK(app_sched_event_put(pName, sizeof(uint8_t*), CustSetDeviceName));
    	strncpy(opubBuf, gtServices.pubDeviceName, DEVICE_NAME_LEN -1);
    }
    NRF_LOG_DEBUG("Get name: %s", opubBuf);
}

static void ble_custom_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
	const ControlEvent tBtConnEvt = {
			.type = CE_BT_CONN,
			.ptr16 = &(ptCustVar->usConnHandle)
	};
    if (p_ble_evt == NULL) return;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        	ptCustVar->usConnHandle = p_ble_evt->evt.gap_evt.conn_handle;
        	ControlPost(&tBtConnEvt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
        	ptCustVar->usConnHandle = BLE_CONN_HANDLE_INVALID;
        	ControlPost(&tBtConnEvt);
            break;

        case BLE_GATTS_EVT_WRITE:
        {
        	ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
        	//NRF_LOG_DEBUG("Evt Wr to 0x%x", p_evt_write->handle);
        	if (p_evt_write->uuid.uuid == BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME) {
        		// Device name update
        		uint8_t* pName = (uint8_t*) malloc(DEVICE_NAME_LEN);
            	strncpy(pName, p_evt_write->data, DEVICE_NAME_LEN-1);
            	pName[DEVICE_NAME_LEN-1] = '\0';
            	APP_ERROR_CHECK(app_sched_event_put(pName, sizeof(uint8_t*), CustSetDeviceName)); // writing to flash from separate thread
        	}
        	tCustomChar const * tCh = GetByCccd(p_evt_write->handle);
        	if (tCh) {
        		tCh->ptHandle->notif = ble_srv_is_notification_enabled(p_evt_write->data);
        		if (tCh->notifyEvt)
        			(*tCh->wrEvt)(p_ble_evt);
        		NRF_LOG_DEBUG("Notification %s", tCh->ptHandle->notif?"On":"Off");
        		break;
        	}
        	tCh = GetByVal(p_evt_write->handle);
        	if (tCh && tCh->wrEvt) {
        		(*tCh->wrEvt)(p_ble_evt);
        	}
        }
        break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            //notify with empty data that some tx was completed.
//        	if(RdHndlr) {
//				(*RdHndlr)(p_ble_evt);
//			}
            break;
        default:
            // No implementation needed.
            break;
    }
}

void CustomServiceSend(uint16_t iusChar, uint8_t *pubData, uint16_t iusLen)
{
	static uint16_t len;
	nrf_mtx_trylock(&tMtxNotify);
	ble_gatts_hvx_params_t hvx_params;
	uint16_t ch = GetConnectionHandle();
	len = iusLen;
	NRF_LOG_DEBUG("notif from %d len %d head 0x%02X", iusChar, len, pubData[0]);
	if (ch != BLE_CONN_HANDLE_INVALID)
	{
		hvx_params.handle = iusChar;
		hvx_params.p_data = pubData;
		hvx_params.p_len  = &len;
		hvx_params.offset = 0;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

		sd_ble_gatts_hvx(ch, &hvx_params);
	}
	nrf_mtx_unlock(&tMtxNotify);
}

ret_code_t CustomServiceValueSet(uint16_t iusChar, uint8_t* ipubData, uint8_t iubLen)
{
	ble_gatts_value_t tValue = {0};
	uint16_t ch = GetConnectionHandle();
	tValue.len = iubLen;
	tValue.p_value = ipubData;
	return sd_ble_gatts_value_set(ch, iusChar, &tValue);
}

NRF_SDH_BLE_OBSERVER(cust_obs, 2, ble_custom_on_ble_evt, NULL);

ret_code_t CustomServiceInit(const tCustomService* itServ)
{
	ret_code_t err_code;
	ptCustVar = itServ->ptVars;

	ptCustVar->usConnHandle = BLE_CONN_HANDLE_INVALID;

	err_code = sd_ble_uuid_vs_add(&(itServ->tUuid), &(ptCustVar->tUuid.type));
	VERIFY_SUCCESS(err_code);

	ptCustVar->tUuid.uuid = itServ->tUuid.uuid128[13];
	ptCustVar->tUuid.uuid <<= 8;
	ptCustVar->tUuid.uuid += itServ->tUuid.uuid128[12];
	// Add the service.
	err_code = sd_ble_gatts_service_add(itServ->ubServiceType,
										&(ptCustVar->tUuid),
										&(ptCustVar->usServiceHandle));
	VERIFY_SUCCESS(err_code);

	uint8_t ubChIndex = 0;

	// Initialize notification mutex
	nrf_mtx_init(&tMtxNotify);

	while (itServ->ptChars[ubChIndex].usUuid)
	{
		ble_gatts_char_handles_t tNewHandle;
		ble_gatts_char_md_t char_md = {0};
	    ble_gatts_attr_md_t cccd_md = {0};
		ble_gatts_attr_md_t attr_md;
		ble_gatts_attr_t    attr_char_value = {0};
		ble_uuid_t          ble_uuid;

		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

		cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	    char_md.p_cccd_md         = &cccd_md;

	    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
	    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
	    attr_md.vlen    = 1;

		switch(itServ->ptChars[ubChIndex].tMode) {
		case CCM_WRITE:
			char_md.char_props.write = 1;
			break;
		case CCM_READNOTIFY:
		    char_md.char_props.notify = 1;
		    char_md.char_props.read = 1;
			break;
		case CCM_NOTIFY:
		    char_md.char_props.notify = 1;
			break;
		case CCM_READWRITE:
			char_md.char_props.write = 1;
			char_md.char_props.read = 1;
			break;
		case CCM_READWRITENOTIFY:
			char_md.char_props.write = 1;
			char_md.char_props.read = 1;
			char_md.char_props.notify = 1;
			break;
		case CCM_WRITENOTIFY:
		    char_md.char_props.notify = 1;
		    char_md.char_props.write = 1;
			break;
		}

		ble_uuid.type = ptCustVar->tUuid.type;
		ble_uuid.uuid = itServ->ptChars[ubChIndex].usUuid;

		attr_char_value.p_attr_md = &attr_md;
		attr_char_value.p_uuid    = &ble_uuid;
		attr_char_value.init_len  = sizeof(uint8_t);
		attr_char_value.max_len   = 20;

		err_code = sd_ble_gatts_characteristic_add(ptCustVar->usServiceHandle,
		                                           &char_md,
		                                           &attr_char_value,
		                                           &tNewHandle);
		VERIFY_SUCCESS(err_code);
		NRF_LOG_INFO("Ch %x '%s' Hnd %x %x", itServ->ptChars[ubChIndex].usUuid, itServ->ptChars[ubChIndex].ubName, tNewHandle.value_handle, tNewHandle.cccd_handle);
		if (itServ->ptChars[ubChIndex].ptHandle) {
			itServ->ptChars[ubChIndex].ptHandle->hval = tNewHandle.value_handle;
			itServ->ptChars[ubChIndex].ptHandle->hcccd = tNewHandle.cccd_handle;
			itServ->ptChars[ubChIndex].ptHandle->notif = 0;
		}
		ubChIndex++;
	}
    return NRF_SUCCESS;
}
