/**
 * Project: gswitch18 USB HID Game Controller
 * Author: Gruvin <gruvin@gmail.com>
 * Based on Joonas Pihlajamaa's AVR ATtiny USB HID Tutorial, published at his 
 * Code and Life blog, http://codeandlife.com.
 * Copyright: (c) 2012 by Bryan J. Rentoul (gruvin)
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v3 (see License.txt)
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "usbdrv.h"

#include <util/delay.h>

/*
PROGMEM const char usbHidReportDescriptor[52] = { // USB report descriptor, size must match usbconfig.h
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Mouse=0x02, GamePad=0x05)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM
    0x29, 0x03,                    //     USAGE_MAXIMUM
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)

    // pad out the 5 remaining bits of unused buttons (constants)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Const,Var,Abs)

    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xC0,                          //   END_COLLECTION
    0xC0,                          // END COLLECTION
};
*/

// USB report descriptor, size must match USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH in usbconfig.h
PROGMEM const char usbHidReportDescriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE_ID (GamePad=0x05)
    0xA1, 0x00,                    // COLLECTION (Physical)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (button 1)
    0x29, 0x14,                    //   USAGE_MAXIMUM (button 20)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x14,                    //   REPORT_COUNT (20)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    // pad out remaining 4 bits for missing buttons 21 to 24
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x04,                    //   REPORT_SIZE (4)
    0x81, 0x03,                    //   INPUT (Const,Var,Abs)

    0xC0                           // END COLLECTION
};

typedef struct{ // USB 1.1 low speed devices limited to max 8 bytes
    uchar   buttonMask0; // buttons 0 - 7
    uchar   buttonMask1; // buttons 8 - 15
    uchar   buttonMask2; // buttons 16 - 19 (plus 4 unused)
} report_t;

static report_t reportBuffer;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    // The following requests are never used. But since they are required by
    // the specification, we implement them in this example.
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        if(rq->bRequest == USBRQ_HID_GET_REPORT) {  
            // wValue: ReportType (highbyte), ReportID (lowbyte)
            usbMsgPtr = (void *)&reportBuffer; // we only have this one
            return sizeof(reportBuffer);
        } else if(rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = &idleRate;
            return 1;
        } else if(rq->bRequest == USBRQ_HID_SET_IDLE) {
            idleRate = rq->wValue.bytes[1];
        }
    }
    
    return 0; // by default don't return any data
}

int main() {
  uchar i, tg, tc, tb;

  // Set up I/O port data directions and initial states
  // G9X v4.x PCB being used as devlopment platform for now
  DDRA = 0xff;  PORTA = 0x00; // LCD data
  DDRB = 0;     PORTB = 0x30; // pull-ups for Train_Sw and IDL2
  DDRC = 0x3f;  PORTC = 0xc0; // 7:AilDR_Sw, 6:EleDr_Sw, LCD[5,4,3,2,1], 0:LED_back-light
  DDRL = 0x80;  PORTL = 0x7f; // 7: Hold_PWR_On (1=On, default Off), 6:Jack_Presence_TTL, 5-0: User Button inputs
  DDRE = 0x04;  PORTE = 0x04; // Bit 3=BUZZER
  DDRG = 0;     PORTG = 0x2d; // Pull-ups for RudDr_Sw, TCut_Sw, IDL1_Sw, Gear_Sw
  DDRJ = 0;     PORTJ = 0xff; // Pull-ups for all trim switches

  _delay_us(10000); // let the I/O ports settle

  wdt_enable(WDTO_1S); // enable 1s watchdog timer

  usbInit();

  usbDeviceDisconnect(); // enforce re-enumeration
  for(i = 0; i<250; i++) { // wait 500 ms
    wdt_reset(); // keep the watchdog happy
    _delay_ms(2);
  }
  usbDeviceConnect();

  sei(); // Enable interrupts after re-enumeration

  while(1) {
    wdt_reset(); // keep the watchdog happy
    usbPoll();

    if(usbInterruptIsReady()) { // if the interrupt is ready, feed data

      tg = ~PING & 0x2d;
      tc = ~PINC & 0xc0;
      tb = ~PINB & 0x30;

      reportBuffer.buttonMask0 = 
          ((~tg & 0x04)<<5)    // bit  7 Thr_Cut
        | ((~tg & 0x01)<<6)    // bit  6 Rud_Sw
        | ((~tc & 0x40)>>1)    // bit  5 Ele_DR
        | ((tg & 0x08)<<1)     // bit  4 IDL1
        | ((tb & 0x10)>>1)     // bit  3 IDL2
        | ((~tc & 0x80)>>5)    // bit  2 Ail_DR
        | ((~tg & 0x20)>>4)    // bit  1 Gear_Sw
        | ((~tb & 0x20)>>5);   // bit  0 Trainer_Sw

      reportBuffer.buttonMask1 = ~PINJ; // front panel trim switches
      reportBuffer.buttonMask2 = (~PINL & 0x3f); // front panel key-pad buttons (G9X)

      usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
    }
  }

  return 0;
}
