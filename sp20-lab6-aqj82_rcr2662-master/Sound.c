// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 1/17/2020 
// Lab number: 6
// Hardware connections
/* 
DAC bit 0 = PB0
DAC bit 1 = PB1
DAC bit 2 = PB2
DAC bit 3 = PB3

Piano keys
key 0 = PE0
key 1 = PE1
key 2 = PE2
key 3 = PE3
*/

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "dac.h"
#include "../inc/tm4c123gh6pm.h"
// 4-bit 32-element sine wave
const unsigned short wave[32] = {
  8,9,11,12,13,14,14,15,15,15,14,
  14,13,12,11,9,8,7,5,4,3,2,
  2,1,1,1,2,2,3,4,5,7};
	
uint8_t Index;

// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
  //initialize systick, flags, indices, globabl variables, call DAC_Init
	Index = 0;
	DAC_Init();
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 0; //not sure yet??
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 5; //enables SysTick, clock source
	//start quiet
}


// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Play called again
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period to be determined by YOU
//           Maximum period to be determined by YOU
//           Minimum period to be determined by YOU
//         if period equals zero, disable sound output
// Output: none
void Sound_Play(uint32_t period){
	NVIC_ST_RELOAD_R = period;
}

void SysTick_Handler(void){
	GPIO_PORTF_DATA_R |= 0x08; //heartbeat on
	GPIO_PORTB_DATA_R ^= 0x10; //toggle this bit for the debugging
	DAC_Out(wave[Index]);
	Index = (Index + 1) % 32;
	GPIO_PORTF_DATA_R &= !0x08; //heartbeat off
}
