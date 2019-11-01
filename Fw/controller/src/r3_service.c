/*
 * bender_service.c
 *
 */

#include <stdint.h>
#include "custom_service.h"
#include "control.h"
#include "nrf_log.h"

tCharVars tCharCoordHandle;

static tCustomServiceVars tR3Device;

static void InitComplete() {

}

const tCustomService tServDev = {
    .tUuid = {{0x12, 0x4D, 0x26, 0x70, 0x56, 0xf9, /*-*/ 0x27, 0xB0, /*-*/ 0x23, 0x46, /*-*/ 0xE0, 0xE5, /*-*/ 0xE7, 0x33, 0x1F, 0x42}},
    .ubServiceType = BLE_GATTS_SRVC_TYPE_PRIMARY,
    .ptVars = &tR3Device,
    .ptChars = {
        { 0x33E8u, "Test", 	CCM_READNOTIFY, 	&tCharCoordHandle,		NULL,				NULL },
        { 0 }
    },
};

const tDevDescription gtServices = {
		.pubDeviceName = "R3",
		.initCompl = &InitComplete,
		.tServices = &tServDev
};


