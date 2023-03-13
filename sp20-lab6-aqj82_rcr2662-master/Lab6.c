// Lab6.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// MOOC lab 13 or EE319K lab6 starter
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
key 0 = PE0 G
key 1 = PE1 E
key 2 = PE2 D
key 3 = PE3 C

PF3 - heartbeat
PF0 - switch for extra credit

*/

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"
#include "music.h"

note_t morning[25] = {
	{C, quarter},
	{A, quarter},
	{G, quarter},
	{F, quarter},
	{G, quarter},
	{A, quarter},
	{C, quarter},
	{A, quarter},
	{G, quarter},
	{F, quarter},
	{G, eighth},
	{A, eighth},
	{G, eighth},
	{A, eighth},
	{C, quarter},
	{A, quarter},
	{C, quarter},
	{D1, quarter},
	{A, quarter},
	{D1, quarter},
	{C, quarter},
	{A, quarter},
	{G, quarter},
	{F, half},
	{0,0}
};

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts


int main(void){      
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);    // bus clock at 80 MHz
  Piano_Init();
  Sound_Init();
	Music_Init();
  // PORT F initialization for heartbeat PF3 and extra credit switch PF0
  SYSCTL_RCGCGPIO_R |= 0x20;
  volatile uint8_t delay = 0;
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;       	// allow changes to PF4-0
	GPIO_PORTF_DIR_R |= 0x8;
	GPIO_PORTF_DIR_R &= !0x1;
	GPIO_PORTF_PUR_R = 0x1;
  GPIO_PORTF_DEN_R |= 0x9;
  
  uint8_t pressed;
	extern uint8_t playing;
 EnableInterrupts();
  while(1){ 
		if((GPIO_PORTF_DATA_R & 0x1) == 1 && playing == 0){
			//read keys from piano
			pressed = Piano_In();
			//call Sound_Play for the note
			if(pressed != 0){
				NVIC_ST_CTRL_R = 7;
				Sound_Play(keymap[pressed]);
			//quiet by flag = 0 or disarming Systick if no key is pressed
			}else
					NVIC_ST_CTRL_R = 5;
		}else if((GPIO_PORTF_DATA_R & 0x1) == 0 && playing == 0){
			NVIC_ST_CTRL_R = 7;
			Music_Play(morning);
		}	
  }    
}

/*
//test code for static testing of DAC only
#include "DAC.h"
uint8_t Testdata;

int main(void){ // use this if you have a voltmeter
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);  // bus clock at 80 MHz
  DAC_Init(); // your lab 6 solution
  Testdata = 15;
  EnableInterrupts();
  while(1){
    Testdata = (Testdata+1)&0x0F;
    DAC_Out(Testdata);  // your lab 6 solution
  }
}*/
