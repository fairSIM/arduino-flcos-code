#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "uart.h"

static FILE uartstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

// ------ main loop ------

int main() {

    // start the UART
    init_uart();
    _term_echo = 1;
    stdout=&uartstdout;
   

 
    DDRD=0xff;
    DDRB=0xff;


    printf_P(PSTR("\n\n----- micro-tics: Welcome -----\n\n"));

    while (1) {

	int res=1;

	while (res) {

	    printf_P(PSTR("> "));

	    uart_readline();

	    
	}


    }

}
