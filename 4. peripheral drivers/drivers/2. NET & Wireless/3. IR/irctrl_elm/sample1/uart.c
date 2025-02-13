/*------------------------------------------------*/
/* UART functions                                 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define	SYSCLK		10000000UL
#define	BAUD		38400


typedef struct _fifo {
	uint8_t	idx_w;
	uint8_t	idx_r;
	uint8_t	count;
	uint8_t buff[64];
} FIFO;


static volatile
FIFO rxfifo, txfifo;



/* Initialize UART */

void uart_init()
{
	/* Flush FIFO */
	rxfifo.idx_r = 0;
	rxfifo.idx_w = 0;
	rxfifo.count = 0;
	txfifo.idx_r = 0;
	txfifo.idx_w = 0;
	txfifo.count = 0;

	/* Enable USART0 (N81) */
	UBRR0L = SYSCLK/BAUD/16-1;
	UCSR0B = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0);
}


/* Get a received character */

uint8_t uart_test ()
{
	return rxfifo.count;
}


uint8_t uart_get ()
{
	uint8_t d, i;


	i = rxfifo.idx_r;
	while (rxfifo.count == 0);
	d = rxfifo.buff[i++];
	cli();
	rxfifo.count--;
	sei();
	rxfifo.idx_r = i % sizeof(rxfifo.buff);

	return d;
}


/* Put a character to transmit */

void uart_put (uint8_t d)
{
	uint8_t i;


	i = txfifo.idx_w;
	while (txfifo.count >= sizeof(txfifo.buff));
	txfifo.buff[i++] = d;
	cli();
	txfifo.count++;
	UCSR0B |= _BV(UDRIE0);
	sei();
	txfifo.idx_w = i % sizeof(txfifo.buff);
}


/* UART RXC interrupt */

ISR(USART_RX_vect)
{
	uint8_t d, n, i;


	d = UDR0;
	n = rxfifo.count;
	if (n < sizeof(rxfifo.buff)) {
		rxfifo.count = ++n;
		i = rxfifo.idx_w;
		rxfifo.buff[i++] = d;
		rxfifo.idx_w = i % sizeof(rxfifo.buff);
	}
}


/* UART UDRE interrupt */

ISR(USART_UDRE_vect)
{
	uint8_t n, i;


	n = txfifo.count;
	if (n) {
		txfifo.count = --n;
		i = txfifo.idx_r;
		UDR0 = txfifo.buff[i++];
		txfifo.idx_r = i % sizeof(txfifo.buff);
	}
	if (n == 0)
		UCSR0B &= ~_BV(UDRIE0);
}


