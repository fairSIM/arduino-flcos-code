#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "uart.h"

static FILE uartstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


// read the camera 'exposure' pin
inline uint8_t read_camera() {
    return (PINC & (1<<PC0));	// A0 on the Arduino
}


// read the SLM 'LED enable' pin
inline uint8_t read_ledenable() {
    return (PINC & (1<<PC5));	// A5 on the Arduino
}


// set the laser on / off
inline void set_laser( uint8_t state ) {
    if (!state) {
	PORTD |= (1<<PD7);	// digital 7 on the Arduino
    } else {
	PORTD &= ~(1<<PD7);
    }
}


// set control LED on pin 4 on / ogg
inline void set_cLed4( uint8_t state ) {
    if (state) {
	PORTD |= (1<<PD4);	// digital 7 on the Arduino
    } else {
	PORTD &= ~(1<<PD4);
    }
}

// set control LED on pin 5 on / ogg
inline void set_cLed5( uint8_t state ) {
    if (state) {
	PORTD |= (1<<PD5);	// digital 7 on the Arduino
    } else {
	PORTD &= ~(1<<PD5);
    }
}

// send a pulse to 'trigger' SLM
static inline void send_trigger() {
    PORTD |= (1<<PD2);
    _delay_us(50);
    PORTD &= ~(1<<PD2);
}


// send a pulse to 'finish' SLM
static inline void send_finish() {
    PORTD |= (1<<PD3);
    _delay_us(50);
    PORTD &= ~(1<<PD3);
}



int main() {

    // start the UART
    init_uart();
    _term_echo = 1;
    stdout=&uartstdout;
  
    // set PORTC to input, no pullup
    PORTC=0x00;
    DDRC=0x00;
    
    // set PORTD to output, all off
    PORTD=0x00;
    DDRD=0xff; 

    set_laser(0);


    // endless loop running our commands
    while (1) {

	// wait for the camera to go to 'exposure'
	set_cLed4(1);
	while ( ! read_camera() ) {};	
	set_cLed4(0);

	// trigger the SLM
	send_trigger();
	UDR0='T';


	// while the camera is on exposure ....
	
	uint8_t last_led_state = read_ledenable();
	uint16_t light_count   = 0;

	while ( read_camera() ) {
	    
	    // read out the LED_ENABLE state from the SLM
	    uint8_t led_state = read_ledenable();   
	    
	    // if the state has changed, mirror it to the laser output
	    if (led_state != last_led_state ) {
		set_laser( led_state);
		last_led_state = led_state;
		UDR0 = (led_state)?('1'):('0');
	    
		// if the laser has been turned on twice, retrigger the SLM
		light_count++;
		if (light_count == 4) {
		    light_count =0;
		    _delay_us(75);
		    send_trigger(); 
		}
	    }
	}

	// turn off laser, finish SLM sequence
	set_laser(0);
	send_finish();
	
	// output some status
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0='\n';
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0='\r';
	
    }

}
