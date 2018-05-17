#include "uart.h"

int uart_putchar(char c, FILE *stream) {
  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}

int uart_readline() {

    uint8_t ccount=0;

    while (1) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	char c = UDR0;
	if ( c=='\r' || c=='\n') break;
	if ( c==0x08 || c==0x7F ) {
	    if ( ccount>0 ) {
		ccount--;
		if (_term_echo ) {
		    uart_putchar(0x08,NULL);
		    uart_putchar(' ',NULL);
		    uart_putchar(0x08,NULL);
		}
	    } else {	 
		if (_term_echo ) uart_putchar('\a',NULL);
	    }
	} else {
	    uart_line_buffer[ccount++]=c;
	    if (_term_echo ) uart_putchar(c,NULL);
	}
    }
    uart_line_buffer[ccount]='\0';
    if (_term_echo ) {
	uart_putchar('\r',NULL);
	uart_putchar('\n',NULL);
    }
    return ccount;
}

void init_uart( void ) {
  
  UBRR0L = UBRRL_VALUE;
  UBRR0H = UBRRH_VALUE;
#ifdef USE_2X
  UCSR0A |= (1<<U2X0);
#endif

  UCSR0B |= (1<<TXEN0);
  UCSR0B |= (1<<RXEN0);
  //UCSR0C = (1<<URSEL)|(1 << UCSZ1)|(1 << UCSZ0);


}


