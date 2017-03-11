/*
  hid.c

  Keyboard example for Teensy USB Development Board
  http://www.pjrc.com/teensy/usb_keyboard.html
  Copyright (c) 2008 PJRC.COM, LLC
   
  Modified for implementing HID devices on Arduino boards
  Implements: keyboard, mouse and joystick
  Modifications Copyright (C) 2017 Laszlo Menczel

  This is free software distributed under the terms of the MIT license
  (see the file LICENSE or https://opensource.org/licenses/MIT).
*/

#define PRIVATE_INCLUDE

#include "usb.h"

//======================================================================
//  Configurable Options
//======================================================================

#define STR_VENDOR      L"Arduino LLC"
#define STR_PRODUCT     L"Arduino Leonardo"

#define VENDOR_ID       0x2143

// PRODUCT_ID is defined in the Makefile

// USB devices are supposed to implement a halt feature, which is
// rarely (if ever) used.  If you comment this line out, the halt
// code will be removed, saving 102 bytes of space (gcc 4.3.0).
// This is not strictly USB compliant, but works with all major
// operating systems.
// #define SUPPORT_ENDPOINT_HALT

//======================================================================
//  Endpoint Buffer Configuration
//======================================================================

#define INTERFACE_NUM          0
#define IF_CLASS               0x03
#define CTRL_EP_BUF_SIZE       32
#define AVR_CTRL_BUF_CODE      0x20   // Atmega code for 32 bytes  

static const uint8_t PROGMEM endpoint_config_table[] =
{
  0,
  0,
  1, EP_TYPE_INTERRUPT_IN, AVR_DATA_BUF_CODE | EP_DOUBLE_BUFFER,
  0
};

//======================================================================
//  Descriptor Data
//======================================================================

// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo). The most commonly
// changed items are editable at the top of this file. Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of the USB HID 1.1 class specifications!

const static uint8_t PROGMEM device_desc[] =
{
  18,                 // bLength
  1,                  // bDescriptorType
  0x00, 0x02,         // bcdUSB
  0,                  // bDeviceClass
  0,                  // bDeviceSubClass
  0,                  // bDeviceProtocol
  CTRL_EP_BUF_SIZE,   // bMaxPacketSize0
  LSB(VENDOR_ID),     // idVendor
  MSB(VENDOR_ID),
  LSB(PRODUCT_ID),    // idProduct
  MSB(PRODUCT_ID),
  0x00, 0x01,         // bcdDevice
  1,                  // iManufacturer
  2,                  // iProduct
  0,                  // iSerialNumber
  1                   // bNumConfigurations
};

#define CONFIG_DESC_SIZE     (9+9+9+7)
#define HID_DESC_OFFSET      (9+9)

const static uint8_t PROGMEM config_desc[CONFIG_DESC_SIZE] =
{
  // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
  9,                              // bLength;
  2,                              // bDescriptorType;
  LSB(CONFIG_DESC_SIZE),          // wTotalLength
  MSB(CONFIG_DESC_SIZE),
  1,                              // bNumInterfaces
  1,                              // bConfigurationValue
  0,                              // iConfiguration
  0x80,                           // bmAttributes (bus powered)
  250,                            // bMaxPower (250 x 2 = 500 mA)

  // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
  9,                              // bLength
  4,                              // bDescriptorType
  INTERFACE_NUM,                  // bInterfaceNumber
  0,                              // bAlternateSetting
  1,                              // bNumEndpoints
  IF_CLASS,                       // bInterfaceClass
  IF_SUBCLASS,                    // bInterfaceSubClass
  IF_PROTOCOL,                    // bInterfaceProtocol
  0,                              // iInterface

  // HID interface descriptor, HID 1.11 spec, section 6.2.1
  9,                              // bLength
  0x21,                           // bDescriptorType
  0x11, 0x01,                     // bcdHID
  0,                              // bCountryCode
  1,                              // bNumDescriptors
  0x22,                           // bDescriptorType
  HID_DESC_SIZE,                  // wDescriptorLength
  0,

  // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
  7,                              // bLength
  5,                              // bDescriptorType
  ENDPOINT_NUM | 0x80,            // bEndpointAddress
  0x03,                           // bmAttributes (0x03=intr)
  DATA_EP_BUF_SIZE, 0,            // wMaxPacketSize
  0x20                            // bInterval (32 ms)
};

// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  int16_t wString[];
} string_desc_t;

const static string_desc_t PROGMEM string0 =
{
  4,
  3,
  {0x0409}
};

const static string_desc_t PROGMEM string1 =
{
  sizeof(STR_VENDOR),
  3,
  STR_VENDOR
};

const static string_desc_t PROGMEM string2 =
{
  sizeof(STR_PRODUCT),
  3,
  STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (specified in wValue and wIndex of the request).
typedef struct
{
  uint16_t       wValue;
  uint16_t       wIndex;
  const uint8_t *addr;
  uint8_t        length;
} desc_list_t;

const static desc_list_t PROGMEM desc_list[] =
{
  { 0x0100, 0x0000, device_desc, sizeof(device_desc) },
  { 0x0200, 0x0000, config_desc, sizeof(config_desc) },
  { 0x2200, INTERFACE_NUM, hid_report_desc, HID_DESC_SIZE },
  { 0x2100, INTERFACE_NUM, config_desc + HID_DESC_OFFSET, 9 },
  { 0x0300, 0x0000, (const uint8_t *) &string0, 4 },
  { 0x0301, 0x0409, (const uint8_t *) &string1, sizeof(STR_VENDOR) },
  { 0x0302, 0x0409, (const uint8_t *) &string2, sizeof(STR_PRODUCT) }
};

#define NUM_DESC_LIST (sizeof(desc_list)/sizeof(desc_list_t))

//======================================================================
//  Variables - these are the only non-stack RAM usage
//======================================================================

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_config = 0;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
static uint8_t idle_config = 125;

// count until idle timeout
static uint8_t idle_count = 0;

// protocol setting from the host.  We use exactly the same report
// either way, so this variable only stores the setting since we
// are required to be able to report which setting is in use.
static uint8_t hid_protocol = 1;

//======================================================================
// Public functions
//======================================================================

// initialize USB
void
usb_init(void)
{
  UHWCON = 0x01;
  USBCON = ((1 << USBE)|(1 << FRZCLK));   // enable USB
  PLLCSR = 0x12;                          // config PLL

  while (!(PLLCSR & (1 << PLOCK)))        // wait for PLL lock
    ;    

  USBCON = ((1 << USBE)|(1 << OTGPADE));  // start USB clock
  UDCON = 0;                              // enable attach resistor
  usb_config = 0;
  UDIEN = (1 << EORSTE)|(1 << SOFE);
  sei();
}

//======================================================================
// return 0 if the USB is not configured, or the configuration
// number selected by the HOST

uint8_t
usb_configured(void)
{
  return usb_config;
}

//======================================================================
// send an updated HID report of the device

int8_t
send_hid_report(void)
{
  uint8_t intr_state, timeout;

  if (! usb_config)
    return -1;

  timeout = UDFNUML + 50;
  do
  {
    // get ready to try checking again
    intr_state = SREG;
    cli();
    UENUM = ENDPOINT_NUM;

    // are we ready to transmit?
    if (UEINTX & (1 << RWAL))
      break;

    SREG = intr_state;

    // has the USB gone offline?
    if (! usb_config)
      return -1;

    // have we waited too long?
    if (UDFNUML == timeout)
      return -1;
  } while (1);

  copy_report_to_buf();  // device specific function

  UEINTX = 0x3A;
  idle_count = 0;
  SREG = intr_state;

  return 0;
}

//======================================================================
// Private functions
//======================================================================

// Misc inline functions to wait for ready and send/receive packets

inline void wait_in_ready(void) { while (! (UEINTX & (1 << TXINI))) ; }

inline void send_in(void) { UEINTX = ~(1 << TXINI); }

inline void wait_receive_out(void) { while (! (UEINTX & (1 << RXOUTI))) ; }

inline void ack_out(void) { UEINTX = ~(1 << RXOUTI); }

//======================================================================
// USB Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame

ISR(USB_GEN_vect)
{
  uint8_t intbits;
  static uint8_t div4 = 0;

  intbits = UDINT;
  UDINT = 0;

  if (intbits & (1 << EORSTI))
  {
    UENUM = 0;
    UECONX = 1;
    UECFG0X = EP_TYPE_CONTROL;
    UECFG1X = AVR_CTRL_BUF_CODE | EP_SINGLE_BUFFER;
    UEIENX = (1 << RXSTPE);
    usb_config = 0;
  }

  if ((intbits & (1 << SOFI)) && usb_config)
  {
    if (idle_config && (++div4 & 3) == 0)
    {
      UENUM = ENDPOINT_NUM;

      if (UEINTX & (1 << RWAL))
      {
        idle_count++;

        if (idle_count == idle_config)
        {
          idle_count = 0;
          copy_report_to_buf();
          UEINTX = 0x3A;
        }
      }
    }
  }
}

//======================================================================
// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.

ISR(USB_COM_vect)
{
  uint8_t intbits;
  const uint8_t *list;
  const uint8_t *cfg;
  uint8_t i, n, len, en;
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
  uint16_t desc_val;
  const uint8_t *desc_addr;
  uint8_t desc_length;

  UENUM = 0;
  intbits = UEINTX;

  if (intbits & (1 << RXSTPI))
  {
    bmRequestType = UEDATX;
    bRequest = UEDATX;
    wValue = UEDATX;
    wValue |= (UEDATX  <<  8);
    wIndex = UEDATX;
    wIndex |= (UEDATX  <<  8);
    wLength = UEDATX;
    wLength |= (UEDATX  <<  8);
    UEINTX = ~((1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI));

    if (bRequest == GET_DESCRIPTOR)
    {
      list = (const uint8_t *) desc_list;
      for (i = 0; ; i++)
      {
        if (i >= NUM_DESC_LIST)
        {
          UECONX = (1 << STALLRQ)|(1 << EPEN);  //stall
          return;
        }

        desc_val = pgm_read_word(list);
        if (desc_val != wValue)
        {
          list += sizeof(desc_list_t);
          continue;
        }

        list += 2;
        desc_val = pgm_read_word(list);
        if (desc_val != wIndex)
        {
          list += sizeof(desc_list_t) - 2;
          continue;
        }

        list += 2;
        desc_addr = (const uint8_t *) pgm_read_word(list);
        list += 2;
        desc_length = pgm_read_byte(list);

        break;
      }

      len = (wLength < 256) ? wLength : 255;
      if (len > desc_length)
        len = desc_length;

      do
      {
        // wait for host ready for IN packet
        do
        {
          i = UEINTX;
        } while (! (i & ((1 << TXINI)|(1 << RXOUTI))));

        if (i & (1 << RXOUTI))
          return;                             // abort

        // send IN packet
        if (len < CTRL_EP_BUF_SIZE)
          n = len;
        else
          n = CTRL_EP_BUF_SIZE;

        for (i = n; i > 0; i--)
          UEDATX = pgm_read_byte(desc_addr++);

        len -= n;
        send_in();
      } while (len || n == CTRL_EP_BUF_SIZE);

      return;
    }

    if (bRequest == SET_ADDRESS)
    {
      send_in();
      wait_in_ready();
      UDADDR = wValue | (1 << ADDEN);
      return;
    }

    if (bRequest == SET_CONFIGURATION && bmRequestType == 0)
    {
      usb_config = wValue;
      send_in();

      cfg = endpoint_config_table;
      for (i = 1; i < 5; i++)
      {
        UENUM = i;
        en = pgm_read_byte(cfg++);
        UECONX = en;
        if (en)
        {
          UECFG0X = pgm_read_byte(cfg++);
          UECFG1X = pgm_read_byte(cfg++);
        }
      }

      UERST = 0x1E;
      UERST = 0;

      return;
    }

    if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80)
    {
      wait_in_ready();
      UEDATX = usb_config;
      send_in();

      return;
    }

    if (bRequest == GET_STATUS)
    {
      wait_in_ready();
      i = 0;
      UEDATX = i;
      UEDATX = 0;
      send_in();

      return;
    }

    if (wIndex == INTERFACE_NUM)
    {
      if (bmRequestType == 0xa1)
      {
        if (bRequest == HID_GET_REPORT)
        {
          wait_in_ready();
          copy_report_to_buf();
          send_in();

          return;
        }

        if (bRequest == HID_GET_IDLE)
        {
          wait_in_ready();
          UEDATX = idle_config;
          send_in();

          return;
        }

        if (bRequest == HID_GET_PROTOCOL)
        {
          wait_in_ready();
          UEDATX = hid_protocol;
          send_in();

          return;
        }
      }

      if (bmRequestType == 0x21)
      {
        if (bRequest == HID_SET_REPORT)
        {
          wait_receive_out();
#if defined HID_KEYBOARD
          kbd_state.leds = UEDATX;
#endif
          ack_out();
          send_in();

          return;
        }

        if (bRequest == HID_SET_IDLE)
        {
          idle_config = (wValue >> 8);
          idle_count = 0;
          send_in();

          return;
        }

        if (bRequest == HID_SET_PROTOCOL)
        {
          hid_protocol = wValue;
          send_in();

          return;
        }
      }
    }
  }

  UECONX = (1 << STALLRQ) | (1 << EPEN);  // stall
}
