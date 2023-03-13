// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: Alison Jin, Roberto Reyes
// Last modification date: 4/17/2020
// Last Modified: 1/17/2020 

// Specifications:
// Measure distance using slide pot, sample at 10 Hz
// maximum distance can be any value from 1.5 to 2cm
// minimum distance is 0 cm
// Calculate distance in fixed point, 0.01cm
// Analog Input connected to PD2=ADC5
// displays distance on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats (use them in creative ways)
// 

#include <stdint.h>
#include "PLL.h"
#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "../inc/tm4c123gh6pm.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on ST7735
// main3 adds your convert function, position data is no ST7735

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20;
	uint32_t volatile delay = 0;
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;       	// allow changes to PF4-0
	GPIO_PORTF_DIR_R |= 0xE;
	GPIO_PORTF_DEN_R |= 0xE;
}
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
/*int main(void){       // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 5
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 5
  }
}*/

uint32_t time0,time1,time2,time3;
uint32_t ADCtime,OutDectime; // in usec
/*int main(void){
  TExaS_Init();   	// Bus clock is 80 MHz
  NVIC_ST_RELOAD_R = 0x00FFFFFF; // maximum reload value
  NVIC_ST_CURRENT_R = 0;      	// any write to current clears it
  NVIC_ST_CTRL_R = 5;
  ADC_Init();     	// turn on ADC, set channel to 5
  ADC0_SAC_R = 4;   // 16-point averaging, move this line into your ADC_Init()
  ST7735_InitR(INITR_REDTAB);
  while(1){       	// use SysTick 
    time0 = NVIC_ST_CURRENT_R;
    Data = ADC_In();  // sample 12-bit channel 5
    time1 = NVIC_ST_CURRENT_R;
    ST7735_SetCursor(0,0);
    time2 = NVIC_ST_CURRENT_R;
    LCD_OutDec(Data);
    ST7735_OutString("	");  // spaces cover up characters from last output
    time3 = NVIC_ST_CURRENT_R;
    ADCtime = ((time0-time1)&0x0FFFFFF)/80;	// usec
    OutDectime = ((time2-time3)&0x0FFFFFF)/80; // usec
  }
}*/

// your function to convert ADC sample to distance (0.01cm)
uint32_t Convert(uint32_t input){
  return 173*input/4096+12; 
}

/*int main(void){ 
  TExaS_Init();         // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  ADC_Init();         // turn on ADC, set channel to 5
  while(1){  
    PF2 ^= 0x04;      // Heartbeat
    Data = ADC_In();  // sample 12-bit channel 5
    PF3 = 0x08;       // Profile Convert
    Position = Convert(Data); 
    PF3 = 0;          // end of Convert Profile
    PF1 = 0x02;       // Profile LCD
    ST7735_SetCursor(0,0);
    LCD_OutDec(Data); 
		ST7735_OutString("    "); 
    ST7735_SetCursor(6,0);
    LCD_OutFix(Position);
    PF1 = 0;          // end of LCD Profile
  }
}   */


//Part E
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 8000000; 
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 7;
}
//Part G
uint32_t ADCMail; // 0 to 4095
uint32_t ADCStatus; // 1 means new data
void SysTick_Handler(void){
  GPIO_PORTF_DATA_R ^= 0x02; // toggle PF1
  ADCMail = Convert(ADC_In());        // Sample ADC
  ADCStatus = 1;            // Synchronize with other threads
}

//Part H
uint32_t Sample;

int main(void){
  TExaS_Init();
	ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
	ADC_Init();
	SysTick_Init();
	
	while(1){
		if(ADCStatus == 1){
			Sample = ADCMail;
			ADCStatus = 0;
			LCD_OutFix(Sample);
			ST7735_OutString(" cm");
			ST7735_SetCursor(0,0);
		
		}	
	}
}
