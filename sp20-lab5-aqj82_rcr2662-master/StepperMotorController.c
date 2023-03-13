// StepperMotorController.c starter file EE319K Lab 5
// Runs on TM4C123
// Finite state machine to operate a stepper motor.  
// Jonathan Valvano
// 1/17/2020

// Hardware connections (External: two input buttons and four outputs to stepper motor)
//  PA5 is Wash input  (1 means pressed, 0 means not pressed)
//  PA4 is Wiper input  (1 means pressed, 0 means not pressed)
//  PE5 is Water pump output (toggle means washing)
//  PE4-0 are stepper motor outputs 
//  PF1 PF2 or PF3 control the LED on Launchpad used as a heartbeat
//  PB6 is LED output (1 activates external LED on protoboard)

#include "SysTick.h"
#include "TExaS.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define home 0
#define wipeStep2 1
#define wipeStep3 2
#define wipeStep4 3
#define wipeStep5 4
#define wipeStep6 5
#define wipeStep7 6
#define wipeStep8 7
#define wipeStep9 8
#define wipeStep10 9
#define wipeStep11 10
#define wipeStep12 11
#define wipeStep13 12
#define wipeStep14 13
#define wipeStep15 14
#define wipeStep16 15
#define wipeStep17 16
#define wipeStep18 17
#define wipeStep19 18
#define washStep1 19
#define washStep2 20
#define washStep3 21
#define washStep4 22
#define washStep5 23
#define washStep6 24
#define washStep7 25
#define washStep8 26
#define washStep9 27
#define washStep10 28
#define washStep11 29
#define washStep12 30
#define washStep13 31
#define washStep14 32
#define washStep15 33
#define washStep16 34
#define washStep17 35
#define washStep18 36
#define washStep19 37


typedef struct State State_t;
	
struct State{
	uint8_t next [4];
	uint8_t output;
	uint32_t wait;
};

State_t FSM [38] = {
{{home, wipeStep2, washStep2, home},0x01, 50},
{{wipeStep3, wipeStep3, washStep3, washStep3},0x02, 50},
{{wipeStep4, wipeStep4, washStep4, washStep4},0x04, 50},
{{wipeStep5, wipeStep5, washStep5, washStep5},0x08, 50},
{{wipeStep6, wipeStep6, washStep6, washStep6},0x10, 50},
{{wipeStep7, wipeStep7, washStep7, washStep7},0x01, 50},
{{wipeStep8, wipeStep8, washStep8, washStep8},0x02, 50},
{{wipeStep9, wipeStep9, washStep9, washStep9},0x04, 50},
{{wipeStep10, wipeStep10, washStep10, washStep10},0x08, 50},
{{wipeStep11, wipeStep11, washStep11, washStep11},0x10, 50},
{{wipeStep12, wipeStep12, washStep12, washStep12},0x08, 50},
{{wipeStep13, wipeStep13, washStep13, washStep13},0x04, 50},
{{wipeStep14, wipeStep14, washStep14, washStep14},0x02, 50},
{{wipeStep15, wipeStep15, washStep15, washStep15},0x01, 50},
{{wipeStep16, wipeStep16, washStep16, washStep16},0x10, 50},
{{wipeStep17, wipeStep17, washStep17, washStep17},0x08, 50},
{{wipeStep18, wipeStep18, washStep18, washStep18},0x04, 50},
{{wipeStep19, wipeStep19, washStep19, washStep19},0x02, 50},
{{home, home, washStep1, home},0x01, 50},
{{wipeStep2, wipeStep2, washStep2, washStep2},0x21, 50},
{{wipeStep3, wipeStep3, washStep3, washStep3},0x22, 50},
{{wipeStep4, wipeStep4, washStep4, washStep4},0x04, 50},
{{wipeStep5, wipeStep5, washStep5, washStep5},0x08, 50},
{{wipeStep6, wipeStep6, washStep6, washStep6},0x30, 50},
{{wipeStep7, wipeStep7, washStep7, washStep7},0x21, 50},
{{wipeStep8, wipeStep8, washStep8, washStep8},0x02, 50},
{{wipeStep9, wipeStep9, washStep9, washStep9},0x04, 50},
{{wipeStep10, wipeStep10, washStep10, washStep10},0x28, 50},
{{wipeStep11, wipeStep11, washStep11, washStep11},0x30, 50},
{{wipeStep12, wipeStep12, washStep12, washStep12},0x08, 50},
{{wipeStep13, wipeStep13, washStep13, washStep13},0x04, 50},
{{wipeStep14, wipeStep14, washStep14, washStep14},0x22, 50},
{{wipeStep15, wipeStep15, washStep15, washStep15},0x21, 50},
{{wipeStep16, wipeStep16, washStep16, washStep16},0x10, 50},
{{wipeStep17, wipeStep17, washStep17, washStep17},0x08, 50},
{{wipeStep18, wipeStep18, washStep18, washStep18},0x24, 50},
{{wipeStep19, wipeStep19, washStep19, washStep19},0x22, 50},
{{home, home, washStep2, home},0x01, 50},
};

void EnableInterrupts(void);
// edit the following only if you need to move pins from PA4, PE3-0      
// logic analyzer on the real board
#define PA4       (*((volatile unsigned long *)0x40004040))
#define PE50      (*((volatile unsigned long *)0x400240FC))
void SendDataToLogicAnalyzer(void){
  UART0_DR_R = 0x80|(PA4<<2)|PE50;
}

int main(void){ 
  TExaS_Init(&SendDataToLogicAnalyzer);    // activate logic analyzer and set system clock to 80 MHz
  SysTick_Init();   
// you initialize your system here
//  PA5 is Wash input  (1 means pressed, 0 means not pressed)
//  PA4 is Wiper input  (1 means pressed, 0 means not pressed)
//  PE5 is Water pump output (toggle means washing)
//  PE4-0 are stepper motor outputs 
//  PF1 PF2 or PF3 control the LED on Launchpad used as a heartbeat
	uint16_t CS = home; //current state
	volatile uint8_t in; //input
	SYSCTL_RCGCGPIO_R |= 0x31;
	volatile uint8_t delay = 3;
	GPIO_PORTA_DIR_R &= ~0x30;
	GPIO_PORTE_DIR_R |= 0x3F;
	GPIO_PORTF_DIR_R |= 0x2;
	GPIO_PORTA_DEN_R |= 0x30;
	GPIO_PORTE_DEN_R |= 0x3F;
	GPIO_PORTF_DEN_R |= 0x2;
	
	
  EnableInterrupts();   
  while(1){
		GPIO_PORTF_DATA_R ^= 0x2;
		GPIO_PORTE_DATA_R = FSM[CS].output; //output
		SysTick_Wait1ms(FSM[CS].wait); //delay
		in = (GPIO_PORTA_DATA_R & 0x30) >> 4; //input
		CS = FSM[CS].next[in];//change state
  }
}




