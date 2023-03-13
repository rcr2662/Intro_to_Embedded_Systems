// IO.c
// This software configures the switch and LED
// You are allowed to use any switch and any LED, 
// although the Lab suggests the SW1 switch PF4 and Red LED PF1
// Runs on LM4F120 or TM4C123
// Program written by: Alison Jin, Roberto Reyes
// Date Created: March 30, 2018
// Last Modified:  4/12/2020
// Lab number: 7


#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>

void Wait10ms(uint32_t n);

//------------IO_Init------------
// Initialize GPIO Port for a switch and an LED
// Input: none
// Output: none
void IO_Init(void) {
 // --UUU-- Code to initialize PF4 and PF2
	SYSCTL_RCGCGPIO_R |= 0x20;
  volatile uint8_t delay = 0;
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;       	// allow changes to PF4-0
	GPIO_PORTF_DIR_R &= !0x10;		//PF4 for switch input
	GPIO_PORTF_DIR_R |= 0x4;		//PF2 for heartbeat
	GPIO_PORTF_PUR_R = 0x10;
  GPIO_PORTF_DEN_R |= 0x14;
}

//------------IO_HeartBeat------------
// Toggle the output state of the  LED.
// Input: none
// Output: none
void IO_HeartBeat(void) {
 // --UUU-- PF2 is heartbeat
	GPIO_PORTF_DATA_R ^= 0x4;
}


//------------IO_Touch------------
// wait for release and press of the switch
// Delay to debounce the switch
// Input: none
// Output: none
void IO_Touch(void) {
 // --UUU-- wait for release; delay for 20ms; and then wait for press
	while((GPIO_PORTF_DATA_R & 0x10) == 0x10){
	} //waiting for PF4 to be pushed
	while((GPIO_PORTF_DATA_R & 0x10) == 0){
	} //waiting for PF4 to be released
	Wait10ms(2);
}  

void Wait10ms (uint32_t n){
	uint32_t volatile time;
	while(n){
    time =  72727;  // 10msec, tuned at 80 MHz
    while(time){
	  	time--;
    }
    n--;
  }
}

