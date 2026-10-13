#include <avr/io.h>
#include <avr/interrupt.h>

int main()
{
	DDRB = 1<<5;
	PORTD = 1 << 2;
	EICRA = 0x02;
	
	EIMSK = (1<<INT0);
	sei();
	
	while(1);
}

ISR (INT0_vect)
{
	PORTB ^= (1<<5);
}