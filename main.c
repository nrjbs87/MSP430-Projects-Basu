#include <msp430g2553.h>

//EC 450 Homework 3, Neeraj Basu
/*
 * main.c
 */
#define RED_LED 	 0x01
#define GREEN_LED 	 0x60
#define MEM 		 50		// max size of memory array
#define MAX_TIME	 5000	// maximum time to record
#define WAIT_TIME	 500	// max time to wait since last button press
#define BUTTON 		 0x08

volatile unsigned int counter;		//Interupt counter
volatile unsigned int transition_counter;		//when was the button pressed
volatile unsigned int trans[MEM]; 	//Array containing ints monitoring when the button is pressed
volatile unsigned char prev_button;	//Monitors what the last button press was
volatile unsigned char mode;		//current mode

int main(void)
{
	 // setup the watchdog timer as an interval timer
		 WDTCTL =(WDTPW +  WDTTMSEL + WDTCNTCL +  0  +1);
		 IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

		 P1DIR |= RED_LED + GREEN_LED;		//Red & Green LEDs are outputs
		 P1DIR &= ~BUTTON;					//Declare button as input

		 P1OUT &= ~GREEN_LED;				//Green LED off
		 P1OUT |= RED_LED;					//Red LED on

		 P1OUT |= BUTTON;		//pull up resistors
		 P1REN |= BUTTON;


		  //Initializing variables
		  counter = 0;
		  transition_counter = 0;
		  prev_button = BUTTON;
		  mode = 'r';
		  int i;		//ensures array is empty to start
		  for (i = 0; i < MEM; i++){
			  trans[i]= 0;
		  }


		  _bis_SR_register(GIE+LPM0_bits);  // enable interrupts and also turn the CPU off!



	return 0;
}

interrupt void WDT_interval_handler(){

switch (mode){

//Record Mode
case 'r':
{
	unsigned char current_button  = (P1IN & BUTTON);//reading input from button

	//If counter exceed max time, transition to playback
	if (counter > MAX_TIME){		//record time has exceeded max time
		if (!current_button){
		trans[transition_counter] = counter;
		}
		//Transition
		mode = 'a';
		P1OUT &= ~(GREEN_LED + RED_LED);
		counter = 0;
		transition_counter = 0;

		}

	//If no more memory space, transition to playback
	else if (transition_counter > MEM-1){
		//Transition
		mode = 'a';
		P1OUT &= ~(GREEN_LED + RED_LED);
		counter = 0;
		transition_counter = 0;
	}

	//if user has not touched button in certain amount of time, transition to playback
	else if (prev_button && (counter- trans[transition_counter - 1]) > WAIT_TIME && (transition_counter >0 )){
		//Transition
		mode = 'a';
		P1OUT &= ~(GREEN_LED + RED_LED);
		counter = 0;
		transition_counter = 0;
	}

	else{
		//record
				counter += 1;

				if (prev_button != current_button)//button state has changed, so we need to store it in the trans array
				{
					trans[transition_counter] = counter;
					transition_counter += 1;
				}

				if (!current_button)//turn on green LED every time the button is pressed
				{
					P1OUT |= GREEN_LED;
				}
				else
				{
					P1OUT &= ~GREEN_LED;//other wise shut it off
				}
				prev_button = current_button;

	}
	break;
}
//Playback Mode
case 'p':

	//If counter counter exceed Max time, transition to
	if (counter == MAX_TIME){
		//Transition to record mode
				mode = 'b';
				P1OUT &= ~(GREEN_LED + RED_LED);
				counter = 0;
				transition_counter = 0;
				 int i;
						  for (i = 0; i < MEM; i++){
							  trans[i]= 0;
						  }

	}

	else if (trans[transition_counter] == 0){
		//Transition to record mode
				mode = 'b';
				P1OUT &= ~(GREEN_LED + RED_LED);
				counter = 0;
				transition_counter = 0;
				 int i;
						  for (i = 0; i < MEM; i++){
							  trans[i]= 0;
						  }

	}

	else if (transition_counter == MEM - 1){
		//Transition to record mode
				mode = 'b';
				P1OUT &= ~(GREEN_LED + RED_LED);
				counter = 0;
				transition_counter = 0;
				 int i;
						  for (i = 0; i < MEM; i++){
							  trans[i]= 0;
						  }

	}
	counter += 1;

	if (counter == trans[transition_counter])
	{
		P1OUT ^= GREEN_LED;		//Xor green led in same pattern as recorded
		transition_counter += 1;
	}


		break;

//Transition Mode from record to playback
case 'a':
if (transition_counter > 5){		//blink red LED three times to let the user knows the mode is changing
	if (trans[0] > 500) {
		counter = trans[0] - 100;
	}
	transition_counter = 0;
	mode = 'p';
	P1OUT &= ~(GREEN_LED + RED_LED);

}

else{
	counter += 1;
	if (counter == 40){
		counter = 0;
		P1OUT ^= RED_LED;
		transition_counter += 1;
	}
}
break;

//Transition Mode from playback to record
case 'b':
if (transition_counter > 5){		//blink red LED three times to let the user knows the mode is changing
	counter = 0;
	transition_counter = 0;
	mode = 'r';
	P1OUT |= (RED_LED);


}

else{
	counter += 1;
	if (counter == 40){
		counter = 0;
		P1OUT ^= RED_LED;
		transition_counter += 1;
	}
}


break;
} // switch
} // interrupt

// DECLARE function WDT_interval_handler as handler for interrupt 10
// using a macro defined in the msp430g2553.h include file
ISR_VECTOR(WDT_interval_handler, ".int10")
