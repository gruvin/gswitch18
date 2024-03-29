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

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x27DC  /* Obdev's free shared PID for use with Joystick class HID devices */

// USB report descriptor, size must match USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH in usbconfig.h
PROGMEM const char usbHidReportDescriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE_ID (GamePad=0x05)
    0xA1, 0x00,                    // COLLECTION (Physical)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (button 1)
    0x29, 0x12,                    //   USAGE_MAXIMUM (button 18)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x12,                    //   REPORT_COUNT (18)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    // pad out remaining 6 bits for missing buttons 19 to 24 (18-23 0-based)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x03,                    //   INPUT (Const,Var,Abs)

    0xC0                           // END COLLECTION
};

typedef struct{ // USB 1.1 low speed devices limited to max 8 bytes
    uchar   buttonMask0; // buttons 1 - 8
    uchar   buttonMask1; // buttons 9 - 16
    uchar   buttonMask2; // buttons 17 - 18 (plus 6 unused, total 24)
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
  uchar i, tb, tc, td;

  DDRB = 0x00;  PORTB = 0x3f;
  DDRC = 0x00;  PORTC = 0x3f;
  DDRD = 0x00;  PORTD = 0xf9;

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

      tb = ~PINB & 0x3f;
      tc = ~PINC & 0x3f;
      td = ~PIND & 0xf9;

      reportBuffer.buttonMask0 = 
          (tb & 0x3f)         // SW 1-6, bits 0-5 
        | ((td & 0x01) << 6)  // SW 7, bit 6
        | ((td & 0x08) << 4); // SW 8, bit 7

      reportBuffer.buttonMask1 = 
          ((td & 0xf0) >> 4)  // SW 9-12, bits 0-3
        | ((tc & 0x0f) << 4); // SW 13-16, bits 4-7

      reportBuffer.buttonMask2 = 
          ((tc & 0x30) >> 4); // SW 17/18, bits 0-1 (plus 6x 0-padding bits)

      usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
    }
  }

  return 0;
}
