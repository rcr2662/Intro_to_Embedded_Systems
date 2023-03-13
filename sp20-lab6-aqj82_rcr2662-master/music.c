#include <stdint.h>
#include "..//inc//tm4c123gh6pm.h"
#include "music.h"
uint8_t playing;
note_t *pt; //song pointer to which note in song array
/*timer0A_Init
activate Timer0 interrupts to run user task periodically
Inputs: task is a pointer to a user function
period in units (1/clockfreq), 32 bits

Outputs: none
*/

void Timer0A_Init(uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  volatile uint8_t delay = 0;
	TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000000;    // 7) disarm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}	

void Timer0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT; //acknowledge timer0a timeout
	//disarm and quit ISR if song is over when {0,0} or if song is null (doesn't exist)
	if((*pt).period == 0 && (*pt).length == 0){
	  playing = 0;
		TIMER0_IMR_R = 0x00000000;
	}else{
		Sound_Play((*pt).period);
		TIMER0_TAILR_R = (*pt).length;
		pt++;
	}
}	

void Music_Init(void){
	Timer0A_Init(0);
	pt = 0;
	playing = 0;
}

void Music_Play(note_t song[]){
	playing = 1;
	pt = &song[0];
	TIMER0_IMR_R = 0x00000001; //arm interrupts
}
