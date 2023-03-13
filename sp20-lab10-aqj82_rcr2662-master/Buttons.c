//for button input which will control rotate clockwise, rotate counterclockwise, hard drop, and soft drop
/* 
button connections
hard drop = PE0 
soft drop = PE1 
counterclockwise rotate = PE2 
clockwise rotate = PE3 
*/

#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "Sound.h"

extern uint8_t fire;

void Button_Init(void){volatile long delay;                            
  SYSCTL_RCGCGPIO_R |= 0x30;           // activate port E
  while((SYSCTL_PRGPIO_R&0x30)==0){}; // allow time for clock to start
  delay = 100;                  //    allow time to finish activating
  GPIO_PORTE_AMSEL_R &= ~0xF;  // 3) disable analog on PE3-0
                                // 4) configure PF4,0 as GPIO
  GPIO_PORTE_PCTL_R &= ~0x0000FFFF;
  GPIO_PORTE_DIR_R &= ~0xF;    // 5) make PE3-0 in
  GPIO_PORTE_AFSEL_R &= ~0xF;  // 6) disable alt funct on PE3-0
  GPIO_PORTE_DEN_R |= 0xF;     // 7) enable digital I/O on PE3-0
  GPIO_PORTE_IS_R &= ~0xF;     //    PE3-0 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0xF;    //    PE3-0 is not both edges
  GPIO_PORTE_IEV_R &= ~0xF;     //    PFE3-0 falling edge event (Neg logic)
  GPIO_PORTE_ICR_R = 0xF;      //    clear flag1-0
  GPIO_PORTE_IM_R |= 0xF;      // 8) arm interrupt on PE3-0
                                // 9) GPIOE priority 2
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF0F)|0x00000040;
  NVIC_EN0_R = 1<<4;   // 10)enable interrupt 4 in NVIC
		
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0                              // 2) GPIO Port F needs to be unlocked
  GPIO_PORTF_AMSEL_R &= ~0x11;  // 3) disable analog on PF4,0
                                // 4) configure PF4,0 as GPIO
  GPIO_PORTF_PCTL_R &= ~0x000F000F;
  GPIO_PORTF_DIR_R &= ~0x11;    // 5) make PF4,0 in
  GPIO_PORTF_AFSEL_R &= ~0x11;  // 6) disable alt funct on PF4,0
	GPIO_PORTF_PUR_R |= 0x11;     
  GPIO_PORTF_DEN_R |= 0x11;     // 7) enable digital I/O on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     //    PF4,0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //    PF4,0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;     //    PF4,0 falling edge event (Neg logic)
  GPIO_PORTF_ICR_R = 0x11;      //    clear flag1-0
  GPIO_PORTF_IM_R &= ~0x11;      // 8) disarm interrupt on PF4,0
                                // 9) GPIOF priority 2
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00400000;
}

void GPIOPortE_Handler(void){
	if ((GPIO_PORTE_RIS_R & 0x01)==0x01){ //PE0
		//fire regular bullet
		fire = 1;
		
	}
	if ((GPIO_PORTE_RIS_R & 0x02)==0x02){ //PE1
		//fire laser
		fire = 2;
	}

  GPIO_PORTE_ICR_R = 0xF;      // acknowledge flags
}
