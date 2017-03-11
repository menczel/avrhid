/*
  hid_kbd.h
  Copyright (C) 2017 Laszlo Menczel

  Permission to use, copy, modify, and/or distribute this software for  
  any purpose with or without fee is hereby granted, provided that the  
  above copyright notice and this permission notice appear in all copies.  
  
  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
  WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
  WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
  BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
  SOFTWARE.  
*/

#if ! defined HID_MOUSE_H
#define HID_MOUSE_H

void copy_report_to_buf(void);

// Functions for testing the device
extern void move_mouse(int8_t x, int8_t y);

#define DATA_EP_BUF_SIZE     8
#define AVR_DATA_BUF_CODE    0      // Atmega code for 8 bytes  
#define IF_PROTOCOL          2
#define IF_SUBCLASS          1

#define HID_DESC_SIZE        51

typedef struct
{
  uint8_t but_left;
  uint8_t but_right;
  uint8_t but_mid;
  int8_t  wheel;
  int8_t  x;
  int8_t  y;
} mouse_state_t;

extern mouse_state_t mouse_state;
extern const uint8_t PROGMEM hid_report_desc[];

#endif /* HID_MOUSE_H */
