/*
 * control.h
 *
 */

#ifndef CONTROL_H_
#define CONTROL_H_
#include <stdbool.h>

typedef enum {
    CE_BT_CONN = 0x0001,
    CE_BATT_IN = 0x0002, // Battery measurement complete
    CE_CFGCHG = 0x0004, // Config changed - reload timer and servos
    CE_DELALL = 0x0008, // Delete all records
    CE_OLEDVAL = 0x0010, // Write value to OLED
} ControlEventType;

typedef struct {
    ControlEventType type;
    union {
        void* ptr;
        uint8_t* ptr8;
        uint16_t* ptr16;
        bool b;
        uint16_t val16;
    };
} ControlEvent;

typedef void (*ControlEvtCb)(const ControlEvent* pEvt);

void ControlPost(const ControlEvent* pEvt);
void ControlInit(void);

// Register callback for specific event
void ControlRegisterCb(ControlEventType type, ControlEvtCb cb);
// ToDo: De-register?

#endif /* CONTROL_H_ */
