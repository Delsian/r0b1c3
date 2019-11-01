/*
 *  Control signals exchange
 *
 *  Copyright (C) 2019 Eug Krashtan <ekrashtan@n-ix.com>
 *  This file may be distributed under the terms of the GNU GPLv3 license.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "control.h"
#include "app_scheduler.h"
#include "app_error.h"
#include "nrf_log.h"

#include "boards.h"
#include "custom_service.h"

typedef struct {
	ControlEventType type; // Event type mask - may handle more than one event
	ControlEvtCb cb;
} ControlCbList;

#define CB_LIST_LEN 16

static ControlCbList ptCbList[CB_LIST_LEN];

static void ControlEvtH(void * p_evt, uint16_t size) {
    ControlEvent* iEvt = (ControlEvent*) p_evt;
    //NRF_LOG_DEBUG("Evt type %d", iEvt->type);

    for (int i = 0; i < CB_LIST_LEN; i++) {
        if ((ptCbList[i].type & iEvt->type) && ptCbList[i].cb) {
            (ptCbList[i].cb)(iEvt);
        }
    }

    // Some events requires post-processing (memfree for example)
    switch (iEvt->type) {
//	case CE_PWR_OFF:
//		break;
    default:
        break;
    }
}

void ControlPost(const ControlEvent* evt) {
    app_sched_event_put(evt, sizeof(ControlEvent), ControlEvtH);
}

void ControlInit() {
    APP_SCHED_INIT(sizeof(ControlEvent), 32);

}

void ControlRegisterCb(ControlEventType type, ControlEvtCb cb) {
    for (int i = 0; i < CB_LIST_LEN; i++) {
        if (ptCbList[i].cb == 0) {
            ptCbList[i].type = type;
            ptCbList[i].cb = cb;
            return;
        }
    }
    // Error if we have no  more space for callbacks.
    APP_ERROR_CHECK(NRF_ERROR_NO_MEM);
}
