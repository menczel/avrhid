#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// define what pin the built-in LED is connected to.
// it is D13 on the board = physical pin 32 = PC7
#define LED_PIN     PC7

// Some macros that make the code more readable
#define set_dir_out(portdir,pin)  (portdir) |= (1 << (pin))
#define out_low(port,pin)         (port) &= ~(1 << (pin))
#define out_high(port,pin)        (port) |= (1 << (pin))

#define DELAY_COUNT  5  // x 100 msec

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
  UDIEN = (1 << EORSTE)|(1 << SOFE);
  sei();
}

//======================================================================
// Blinks the built-in LED:
// 200 ms ON, 200 ms OFF, 200 ms ON, 1 sec OFF

int
main(void)
{
  int i;

  usb_init();

  set_dir_out(DDRC, LED_PIN);  

  while (1)
  {
    out_high(PORTC, LED_PIN);
    for (i = 0; i < 10; i++)
      _delay_ms(DELAY_COUNT);

    out_low(PORTC, LED_PIN);
    for (i = 0; i < 10; i++)
      _delay_ms(DELAY_COUNT);
  }
}
