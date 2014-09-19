#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ROW_DATA 6
#define STROBE 5
#define BLACK_OE 4
#define WHITE_OE 3
#define COL_CLOCK 2
#define ROW_CLOCK 1
#define COL_DATA 0

#define FLIPDOT_MASK (_BV(ROW_DATA) | _BV(STROBE) | _BV(BLACK_OE) | \
		_BV(WHITE_OE) | _BV(COL_CLOCK) | _BV(ROW_CLOCK) | _BV(COL_DATA))
#define FLIPDOT_PORT PORTB
#define FLIPDOT_DDR DDRB

#define ROW_CMD 0x20
#define COL_MASK 0x1F
#define COL_CMD 0x10
#define ROW_MASK 0x0F

volatile enum {
	READY,
	ROW1, ROW2, ROW_EXEC,
	COL1, COL2, COL3, COL_EXEC
} state = READY;

uint8_t idx = 0;
uint8_t mask[3];

static void send_byte(uint8_t value) {
	while (!( UCSRA & (1<<UDRE)));
	UDR = value;
}

ISR( USART_RX_vect  ) {
	uint8_t tmp = UDR;
	switch (state) {
		case READY:
			if (ROW_CMD & tmp) {
				state = ROW1;
				idx = (tmp & COL_MASK) + 4;
			} else if (COL_CMD & tmp) {
				state = COL1;
				idx = tmp & ROW_MASK;
			}
			break;
		case COL1: mask[0] = tmp; state = COL2; break;
		case ROW1: mask[0] = tmp; state = ROW2; break;
		case COL2: mask[1] = tmp; state = COL3; break;
		case ROW2:
			mask[1] = tmp;
			state = ROW_EXEC;
			break;
		case COL3:
			mask[2] = tmp;
			state = COL_EXEC;
			break;
		case COL_EXEC:
			break; // main thread
		case ROW_EXEC:
			break; // main thread
	}
}

static void send_ack(uint8_t payload) {
	state = READY;
	send_byte(payload);
}

int main(void) {
	FLIPDOT_DDR = FLIPDOT_MASK;
	for (uint8_t i = 0; i < 3; i++) mask[i] = 0x42;

	const uint16_t brr = F_CPU / 16 / 9600 - 1;
	UBRRL = brr & 0xFF;
	UBRRH = brr >> 8;
	UCSRB |= _BV(RXEN) | _BV(RXCIE) | _BV(TXEN);

	sei();                 // Set Enable Interrupts

	while (1) {
		switch (state) {
			case ROW_EXEC:
				for (uint8_t i = 0; i < 24; i++) {
					if (i == idx) FLIPDOT_PORT |= _BV(ROW_DATA);
					else FLIPDOT_PORT &= ~_BV(ROW_DATA);
					_delay_us(1);
					FLIPDOT_PORT |= _BV(ROW_CLOCK);
					_delay_us(1);
					FLIPDOT_PORT &= ~_BV(ROW_CLOCK);
				}
				for (uint8_t i = 0; i < 2; i++) {
					for (uint8_t bit = 0x80; bit; bit >>= 1) {
						if (mask[i] & bit) FLIPDOT_PORT |= _BV(COL_DATA);
						else FLIPDOT_PORT &= ~_BV(COL_DATA);
						_delay_us(1);
						FLIPDOT_PORT |= _BV(COL_CLOCK);
						_delay_us(1);
						FLIPDOT_PORT &= ~_BV(COL_CLOCK);
					}
				}
				FLIPDOT_PORT |= _BV(STROBE);
				_delay_us(1);
				FLIPDOT_PORT &= ~_BV(STROBE);
				FLIPDOT_PORT |= _BV(WHITE_OE);
				_delay_us(3000);
				FLIPDOT_PORT &= ~_BV(WHITE_OE);
				send_ack('R');
				break;
			case COL_EXEC:
				for (uint8_t i = 0; i < 16; i++) {
					if (i == idx) FLIPDOT_PORT |= _BV(COL_DATA);
					else FLIPDOT_PORT &= ~_BV(COL_DATA);
					_delay_us(1);
					FLIPDOT_PORT |= _BV(COL_CLOCK);
					_delay_us(1);
					FLIPDOT_PORT &= ~_BV(COL_CLOCK);
				}
				for (uint8_t i = 0; i < 3; i++) {
					for (uint8_t bit = 0x80; bit; bit >>= 1) {
						if (mask[i] & bit) FLIPDOT_PORT |= _BV(ROW_DATA);
						else FLIPDOT_PORT &= ~_BV(ROW_DATA);
						_delay_us(1);
						FLIPDOT_PORT |= _BV(ROW_CLOCK);
						_delay_us(1);
						FLIPDOT_PORT &= ~_BV(ROW_CLOCK);
					}
				}
				FLIPDOT_PORT |= _BV(STROBE);
				_delay_us(1);
				FLIPDOT_PORT &= ~_BV(STROBE);
				FLIPDOT_PORT |= _BV(BLACK_OE);
				_delay_us(1500);
				FLIPDOT_PORT &= ~_BV(BLACK_OE);
				send_ack('C');
				break;
			default:
				break; // RX interrupt
		}
	}
}
