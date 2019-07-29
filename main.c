#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "uart.h"

static FILE uartstdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

uint8_t needs_reset = 1;

// read the camera 'exposure' pin
uint8_t read_camera() {
    return (PINC & (1<<PC0));	// A0 on the Arduino
}


// read the SLM 'LED enable' pin
uint8_t read_ledenable() {
    return (PINC & (1<<PC5));	// A5 on the Arduino
}


// set the laser on / off
void set_laser( uint8_t state ) {
    if (state) {
	PORTD |= (1<<PD7);	// digital 7 on the Arduino
    } else {
	PORTD &= ~(1<<PD7);
    }
}

// read SPO_3 (last in seq).
uint8_t read_spo3() {
    return (PINC & (1<<PC4));
}


// set control LED on pin 5 on / ogg
void set_cLed5( uint8_t state ) {
    if (state) {
	PORTD |= (1<<PD5);	// digital 5 on the Arduino
    } else {
	PORTD &= ~(1<<PD5);
    }
}

// set control LED on pin 5 on / ogg
void set_cLed6( uint8_t state ) {
    if (state) {
	PORTD |= (1<<PD6);	// digital 5 on the Arduino
    } else {
	PORTD &= ~(1<<PD6);
    }
}

// send a pulse to 'trigger' SLM
static void send_trigger() {
    PORTD |= (1<<PD2);		// digital 2 on the Arduino
    _delay_us(50);
    PORTD &= ~(1<<PD2);
}


// send a pulse to 'finish' SLM
static void send_finish() {
    PORTD |= (1<<PD3);		// digital 3 on the Arduino
    _delay_us(50);
    PORTD &= ~(1<<PD3);
}


// forward definition of a function to reset the SLM
static void reset_SLM_seq();

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
	set_cLed6(1);
	uint32_t cam_timeout = 0;
	while ( ! read_camera() ) {
	    cam_timeout++;
	    asm( "nop\n nop\n nop\n nop\n");	    
	    asm( "nop\n nop\n nop\n nop\n");	    
	    asm( "nop\n nop\n nop\n nop\n");	    
	    if (cam_timeout > 200000 && needs_reset ) {
		reset_SLM_seq();
	    } 
	};	
	set_cLed6(0);

	needs_reset = 1;

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


static void reset_SLM_seq() {

set_cLed5(1);

    UDR0='R';
    _delay_us(100);
    uint8_t loop=1;

    while (loop++) {
	
	send_trigger();
	UDR0='t';
	_delay_us(100);
	if (loop%4==0) {
	    send_finish();
	}
	
	for (uint16_t i=0; i<600; i++) {
	    if (!read_spo3()) {
		loop=0;
	    }
	    _delay_us(10);	
	}    
    }
    UDR0='d';
    _delay_us(50);
    UDR0='\n';
    _delay_us(50);

    needs_reset = 0;

set_cLed5(0);
}
