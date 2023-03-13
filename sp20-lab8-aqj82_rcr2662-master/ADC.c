// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/17/2020
// Student names: Alison Jin, Roberto Reyes
// Last modification date: 4/18/2020

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	SYSCTL_RCGCADC_R |= 0x1;
	SYSCTL_RCGCGPIO_R |= 0x8;
	while((SYSCTL_PRGPIO_R & 0x8) != 0x8){};
	GPIO_PORTD_DIR_R &= ~0x4;
	GPIO_PORTD_AFSEL_R |= 0x4;
	GPIO_PORTD_DEN_R &= ~0x4;
	GPIO_PORTD_AMSEL_R |= 0x4;
	ADC0_PC_R &= ~0xF;
	ADC0_PC_R |= 0x1;
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~0x8;
	ADC0_EMUX_R &= ~0xF000;
	ADC0_SSMUX3_R &= ~0xF;
	ADC0_SSMUX3_R += 0x5;
	ADC0_SSCTL3_R = 0x6;
	ADC0_IM_R &= ~0x8;
	ADC0_ACTSS_R |= 0x8;
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	uint32_t result;
	ADC0_PSSI_R = 0x8;
	while((ADC0_RIS_R & 0x8) == 0){};
	result = ADC0_SSFIFO3_R & 0xFFF;
	ADC0_ISC_R = 0x8;
  return result; 
}


