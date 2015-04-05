#include <msp430g2553.h>

#define REDLED 0x01 //Name of the pin
#define UNIT 30	//Length of the interupt so it is perceivable to us

volatile unsigned int blink_interval;  // number of WDT interrupts per blink of LED
volatile unsigned int blink_counter;   // down counter for interrupt handler
volatile unsigned int short_blink;
volatile unsigned int long_blink;
volatile unsigned int pause;
volatile unsigned char state;


int main(void) {
	  // setup the watchdog timer as an interval timer
	  WDTCTL =(WDTPW + // (bits 15-8) password
	                   // bit 7=0 => watchdog timer on
	                   // bit 6=0 => NMI on rising edge (not used here)
	                   // bit 5=0 => RST/NMI pin does a reset (not used here)
	           WDTTMSEL + // (bit 4) select interval timer mode
	           WDTCNTCL +  // (bit 3) clear watchdog timer counter
	  		          0 // bit 2=0 => SMCLK is the source
	  		          +1 // bits 1-0 = 01 => source/8K
	  		   );
	  IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

	  P1DIR |= REDLED;					// Set P1.0 to output direction
	  P1OUT = 0; //Every pin is set to zero.

	  // initialize the state variables
	  blink_counter= 0;     // initialize the counter
	  short_blink= UNIT;  	//Defining the length of the short blink
	  long_blink = 3 * UNIT;	//Defining the length of the short blink
	  pause = 7 * UNIT;			//Defining the length of the pause
	  blink_interval=short_blink;	//Setting the first interval equal to a short blink
	  state = 1; // first S


	  _bis_SR_register(GIE+LPM0_bits);  // enable interrupts and also turn the CPU off!
}

// ===== Watchdog Timer Interrupt Handler =====
// This event handler is called to handle the watchdog timer interrupt,
//    which is occurring regularly at intervals of about 8K/1.1MHz ~= 7.4ms.

interrupt void WDT_interval_handler(){
  if (--blink_interval==0){          // decrement the counter and act only if it has reached 0
	  switch(state){

	  case 1: // first S
		  P1OUT ^= 1;                   // toggle LED on P1.0
		  if(blink_counter < 5) {
			  blink_interval=short_blink; // reset the down counter
			  blink_counter += 1;
		  }
		  else {
			  blink_interval = long_blink;
			  blink_counter = 0;
			  state = 2;
		  }
		  break;

	  case 2: // first short pause
		  blink_interval=short_blink;
		  state = 3;
		  break;

	  case 3:
		  P1OUT ^= 1;                   // toggle LED on P1.0
		  if(blink_counter < 5) {

			  if(blink_counter % 2 == 0 || blink_counter == 0){
				  blink_interval = long_blink;
				  blink_counter+=1;
			  }

			  else {
				  blink_interval = short_blink;
				  blink_counter += 1;
			  }

		  }
		  else {
			  blink_interval = long_blink;
			  blink_counter = 0;
			  state = 4;
		  }
		  break;

	  case 4: // second short pause
			  blink_interval=short_blink;
			  state = 5;
			  break;

	  case 5: // second S
			  P1OUT ^= 1;                   // toggle LED on P1.0
			  if(blink_counter < 5) {
				  blink_interval=short_blink; // reset the down counter
				  blink_counter += 1;
			  }
			  else {
				  blink_interval = pause;
				  blink_counter = 0;
				  state = 6;
			  }
			  break;

	  case 6: //Long pause
		  state = 1;
		  blink_interval = pause;

	  }


  }
}
// DECLARE function WDT_interval_handler as handler for interrupt 10
// using a macro defined in the msp430g2553.h include file
ISR_VECTOR(WDT_interval_handler, ".int10")
