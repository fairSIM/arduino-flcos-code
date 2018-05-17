#ifndef _INC_SIM_ATMEL_UART
#define _INC_SIM_ATMEL_UART

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

#define F_CPU 16000000
#define BAUD 57600
#include <util/setbaud.h>

// if true, the UART-readline will echo
uint8_t _term_echo ;

// store a line while its being read from UART
char uart_line_buffer[140];

// read a line from UART
int uart_readline();

// write to UART
int uart_putchar(char c, FILE *stream);

// initialize the UART
void init_uart();

#endif
