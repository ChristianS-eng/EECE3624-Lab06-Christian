/*
 * Lab06.c
 *
 * Created: 10/4/2022 9:16:57 AM
 * Initial Coder: jfhutton
 * Current Coder: Christian Sorensen
 * Modified:      10/12/2026
 *
 * This lab uses hardware LEDs wired to PORTA, an interrupt from the joystick center
 * button, and a timer interrupt to build a game loop program that can display 
 * different patterns on the LEDs.
 *
 * While there are many "down and dirty" ways to get this coding done, try to 
 * remember your coding and data structures classes.  Things like ENUM, Arrays, Functions
 * could help make for more elegant coding.
 *
 */ 

#include <avr/io.h>              // Needed for AVR IO defines
#include <avr/interrupt.h>       // Needed for AVR interrupt devices

#define LEDS            PORTA    // alias PORTA

	
// global variables for communication between ISRs and main
const unsigned char TCNT0_COUNT_SET = 0x8E;// Count for 1ms loop (Provided by Prof Hutton)

volatile enum Mode {
	zero,
	one,
	two,
	three,
} mode;

volatile enum Mode mode = zero;  
volatile int tick = 0;
volatile int next_led;
volatile int lock = 1;
volatile int sub_pattern_1 = 1;
volatile int sub_pattern_2 = 0;

int main(void){
	
	DDRA = 0xFF;  // Set the Direction for all PORTA pints to be outputs
	LEDS = 0xFF;  // Set the PORTA for all pins to be high (i.e. OFF)
	
	// Initialization for Timer Interrupt
	// With a 7.3MHz crystal we have a 0.137us period
	// To build a 1ms timer "tick" we
	// - set the pre-scaler to 1/64 (8.77us)
	// - set the TCNT1 to count 114 counts (0-114=0x8E)
	// - enable the interrupt on overflow
	// (Prof Note: these are the commands you need, BUT they are commented
	//  out to start.  Be sure you fully understand them (book, datasheet, etc)
	//  before you enable them...  You also have to have the proper ISR routine
	//  ready for this to work.)
	// (Prof Note Two:  You will need to show your own calculations in the 
	//  Lab Report to verify the TCNT0_COUNT_SET value!)
	
	TCCR0 = (1<<CS02);
	TCNT0 = TCNT0_COUNT_SET;
	TIMSK = (1<<TOIE0);
	
	// Port Initialization
	// Using the LED PORTA initialization above and your Lab05 code, 
	// Configure the joystick button.
	// (Prof Note: you will need to add another jumper wire, or 
	// change the ones you have, to get the center button working!
	
	DDRB = 0x00;	// Set Port B to be input
	PORTB = 0x10;	// Pin 4 (joystick pushbutton)
	
	// Interrupt Enable Block
	// Using your Lab05 code, you will need to update these
	// lines from 0x00 to have the appropriate mask.
	
	EICRA = (1 << ISC01) | (1 << ISC00); // Trigger INT0 on the falling edge
	EIMSK = (1 << INT0);				 // Enable INT0

	// Enable Global Interrupts
	sei();
	
	// Main Loop
	while (1) {
		
	while(1) {
		
		while (tick != 50) {}	// Stall for 50 mS
			
		tick = 0;	
		
		if (mode == zero) {
			PORTA = ~(1 << next_led);
			next_led = (next_led + 1) % 8;			
		}
		
		if (mode == one) {	
			PORTA = ~(1 << next_led);
			next_led--;
			if (next_led == -1)
				next_led = 7;	
		}
		
		if (mode == two) {
			PORTA = ~(1 << next_led);
			
			if (0 <= next_led && next_led < 7 && lock)
				next_led++;
			else {
				next_led--;
				lock = 0;
				if (next_led == 0)
					lock = 1;
			}
		}
		
		if (mode == three) {
			if (sub_pattern_1) {
				LEDS ^= (1<<next_led) | (1<<(7-next_led));
				next_led++;
				
				if (next_led == 4) {
					next_led = 0;
					lock = 0;
				}	
				
				if (LEDS == 0xFF) {
					sub_pattern_1 = 0;
					sub_pattern_2 = 1;	
					next_led = 3;
				}	
			}
			
			if (sub_pattern_2) {
				LEDS ^= ((1<<next_led) | (1<<(7-next_led)));				
				next_led--;
				
				if (next_led == -1) {
					next_led = 3;
					lock = 0;
				}
					
				if (LEDS == 0xFF) {
					sub_pattern_1 = 1;
					sub_pattern_2 = 0;	
					next_led = 0;
				}	
			}
		}
	}
		
		
	}
}

// This is the required format of an ISR routine for a 
// Timer.  Your ISR code should go inside
// (Prof Note:  When you define this correctly, the JUMP TABLE
//  will be properly updated by the compiler.  (Thank you!) 
ISR(TIMER0_OVF_vect) {
	tick++;
	TCNT1 = 0x00;
}

// This is the required format of an ISR routine for a
// Hardware Pin Interrupt.  Format is "PIN#_vect" where 
// '#' is the number of the interrupt...  PD0->0, PD1->1, etc.
ISR(INT0_vect){

	if (mode == zero)
	{
		next_led = 7;
		mode = one;	
	}		
	else if (mode == one)
	{
		next_led = 0;
		lock = 1;
		mode = two;
	}
	else if (mode == two)
	{
		next_led = 0;
		LEDS = 0xFF;
		lock = 1;
		mode = three;
		sub_pattern_1 = 1;	
		sub_pattern_2 = 0;
	}
	else if (mode == three)
	{
		next_led = 0;
		mode = zero;
	}	
}