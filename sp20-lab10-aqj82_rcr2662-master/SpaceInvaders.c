// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2019

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2019

 Copyright 2019 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Buttons.h"
#include "Timer2.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void end(void);

uint32_t points = 0;

typedef enum {English, Spanish} Language_t;
Language_t myLanguage;
typedef enum {GAMEOVER, SCORE, NICETRY, EARTHLING} phrase_t;
const char Gameover_English[] ="Game Over";
const char Gameover_Spanish[] ="Fin";		
const char Score_English[]="Score:";
const char Score_Spanish[]="Puntos:";				
const char NiceTry_English[] = "Nice try,";
const char NiceTry_Spanish[] = "Buen Intento,";
const char Earthling_English[] = "Earthling!";
const char Earthling_Spanih[] = "Terr\xa1""cola!";

const char *Phrases[4][2]={
  {Gameover_English,Gameover_Spanish},
  {Score_English,Score_Spanish},
	{NiceTry_English,NiceTry_Spanish},
	{Earthling_English, Earthling_Spanih}
};


typedef enum {unfired, fired} state_t;

typedef struct{
	const unsigned short *image;
	const unsigned short *black;
	uint8_t w;
	uint8_t h;
	state_t current;
	uint32_t x;
	int32_t y;
	uint8_t vy;
} projectile_t;

typedef enum {dead, alive, missileHit, laserHit} status_t;
struct sprite { 
	int32_t x;		//x-coordinate
	int32_t y;		//y-coordinate
	int32_t vx, vy; //pixels/30Hz
	const unsigned short *image;		//Prt -> image
	const unsigned short *black;
	status_t life;
	uint32_t w;
	uint32_t h;
	uint32_t needDraw;
	projectile_t ammo;
};
typedef struct sprite sprite_t;
sprite_t Enemy[20];
int Flag;
int Anyalive;

uint8_t fire = 0; // 0 don't fire, 1 fire missile, 2 fire laser
uint8_t shoot = 4; // 0 - shoot missile 1, 1 - shoot missile 2, 2 - shoot missile 3, 3 - shoot laser, 4 - nothing
uint8_t numMissiles = 0; //number currently using. can't go above 3
uint8_t numLasers = 0; //number currently using. can't go above 1
projectile_t inventory [4] = {
	{Missile1,blackMissile, 4,9,unfired, 0, 0, 1},
	{Missile1,blackMissile, 4,9,unfired, 0, 0, 1},
	{Missile1,blackMissile, 4,9,unfired, 0, 0, 1},
	{Laser0, blackLaser, 2,9,unfired, 0, 0, 1}
};

sprite_t ship = {52, 159, 0, 0, PlayerShip0, PlayerShip4,1, 18,8, 1};


//laser explosion
const unsigned short *bigExplosion = BigExplosion0;
uint8_t xExplosion = 127;
uint8_t yExplosion = 159;
uint8_t wExplosion = 32;
uint8_t hExplosion = 10;
uint8_t explosionKill = 0; // 0 none, 1 yes

//projectile_t enemyFire[20] = {Missile1,blackMissile, 4,9,unfired, 0, 0, 1};

uint32_t Convert(uint32_t input){
  return 173*input/4096+12; 
}
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 8000000; 
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 7;
}

int32_t ADCMail;
uint32_t ADCStatus; // 1 means new data
void SysTick_Handler(void){
  ADCMail = Convert(ADC_In());        // Sample ADC
  ADCStatus = 1;            // Synchronize with other threads
}

void WelcomeScreen(void){
	//welcome screen
  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("SPACE INVADERS");
	ST7735_SetCursor(1,3);
	ST7735_OutString("Press SW1 for");
	ST7735_SetCursor(1, 4);
	ST7735_OutString("English");	
	ST7735_SetCursor(1,6);
	ST7735_OutString("Presione SW2 para");		
	ST7735_SetCursor(1, 7);
	ST7735_OutString("Espa\xA4ol");					
	while((GPIO_PORTF_RIS_R & 0x11)==0x00){}
	if ((GPIO_PORTF_RIS_R & 0x01)==0x01){ //PF0 sw2
		//spanish
		myLanguage = Spanish;
	}
	if ((GPIO_PORTF_RIS_R & 0x10)==0x10){ //PF4 sw1
		//english
		myLanguage = English;
	}
	GPIO_PORTF_ICR_R = 0x11;      // acknowledge flags
}

void GameInit(void){ int i;
	Flag = 0;
	for(i=0; i<5; i++){
		Enemy[i].x = 20*i;
		Enemy[i].y = 10;
		Enemy[i].vx = 1;
		Enemy[i].vy = 0;
		Enemy[i].image = SmallEnemy10pointA;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16;
		Enemy[i].h = 10;
		Enemy[i].needDraw = 1;
		Enemy[i].ammo.image = Missile1;
		Enemy[i].ammo.black = blackMissile;
		Enemy[i].ammo.w = 4;
		Enemy[i].ammo.h = 9;
		Enemy[i].ammo.current = unfired;
		Enemy[i].ammo.x = 0;
		Enemy[i].ammo.y = 0;
		Enemy[i].ammo.vy = 1;
	}
	for(i=5; i<10; i++){
		Enemy[i].x = 20*(i-5);
		Enemy[i].y = 0;
		Enemy[i].vx = 1;
		Enemy[i].vy = 0;
		Enemy[i].image = SmallEnemy10pointA;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16;
		Enemy[i].h = 10;
		Enemy[i].needDraw = 0;
		Enemy[i].ammo.image = Missile1;
		Enemy[i].ammo.black = blackMissile;
		Enemy[i].ammo.w = 4;
		Enemy[i].ammo.h = 9;
		Enemy[i].ammo.current = unfired;
		Enemy[i].ammo.x = 0;
		Enemy[i].ammo.y = 0;
		Enemy[i].ammo.vy = 1;
	}
		for(i=10; i<15; i++){
		Enemy[i].x = 20*(i-10);
		Enemy[i].y = -10;
		Enemy[i].vx = 1;
		Enemy[i].vy = 0;
		Enemy[i].image = SmallEnemy10pointA;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16;
		Enemy[i].h = 10;
		Enemy[i].needDraw = 0;
		Enemy[i].ammo.image = Missile1;
		Enemy[i].ammo.black = blackMissile;
		Enemy[i].ammo.w = 4;
		Enemy[i].ammo.h = 9;
		Enemy[i].ammo.current = unfired;
		Enemy[i].ammo.x = 0;
		Enemy[i].ammo.y = 0;
		Enemy[i].ammo.vy = 1;
	}
	for(i=15; i<20; i++){
		Enemy[i].x = 20*(i-15);
		Enemy[i].y = -20;
		Enemy[i].vx = 1;
		Enemy[i].vy = 0;
		Enemy[i].image = SmallEnemy10pointA;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16;
		Enemy[i].h = 10;
		Enemy[i].needDraw = 0;
		Enemy[i].ammo.image = Missile1;
		Enemy[i].ammo.black = blackMissile;
		Enemy[i].ammo.w = 4;
		Enemy[i].ammo.h = 9;
		Enemy[i].ammo.current = unfired;
		Enemy[i].ammo.x = 0;
		Enemy[i].ammo.y = 0;
		Enemy[i].ammo.vy = 1;
	}
}

uint32_t previous = 100; //previous position of slide pot
void GameMove(void){ int i;
	Anyalive = 0;
	for(i = 0; i < 4; i++){
		if(inventory[i].current == fired)
			inventory[i].y -= inventory[i].vy;
		if(inventory[i].y < 0){
			inventory[i].current = unfired;
			inventory[i].y = 0;
			if(i < 3){
				if(numMissiles > 0)
					numMissiles--;
			}else
				numLasers = 0;
		}
	}
	for(i=0; i<20; i++){
		if(Enemy[i].life == alive){
			Anyalive = 1;
			if(Enemy[i].needDraw == 0){
				if(Enemy[i].y > 0)
					Enemy[i].needDraw = 1;
			}
			if(Enemy[i].needDraw == 1){
				if(Enemy[i].y >= ship.y - ship.h){
					Anyalive = 0;//If enemy y is the same or greater than the highest point of the ship, game ends
					ship.life = dead;
					i = 20;
				}else if(Enemy[i].y > (ship.y - ship.h) && Enemy[i].x > ship.x && Enemy[i].x < (ship.x + ship.w)){
					Anyalive = 0;
					ship.life = dead;
					i = 20;
				}else if(Enemy[i].x < 0){
					Enemy[i].life = dead;
				}else if(Enemy[i].x > 112){
					Enemy[i].life = dead;
				}else{
					uint32_t spritePos = i - (i/5) *5;
					switch(spritePos){
						case 0: if(Enemy[i].x > 30){			//Check for rightmost bound
												Enemy[i].vx = -1;				//Invert the velocity
										}else if(Enemy[i].x < 10)		//Check for leftmost bound
												Enemy[i].vx = 1;					//Invert the velocity
										break;
						case 1:	if(Enemy[i].x > 50){			//Check for rightmost bound
												Enemy[i].vx = -1;				//Invert the velocity
										}else if(Enemy[i].x < 30)		//Check for leftmost bound
												Enemy[i].vx = 1;					//Invert the velocity
										break;
						case 2: if(Enemy[i].x > 70){			//Check for rightmost bound
												Enemy[i].vx = -1;				//Invert the velocity
										}else if(Enemy[i].x < 50)		//Check for leftmost bound
												Enemy[i].vx = 1;					//Invert the velocity
										break;
						case 3: if(Enemy[i].x > 90){			//Check for rightmost bound
												Enemy[i].vx = -1;				//Invert the velocity
										}else if(Enemy[i].x < 70)		//Check for leftmost bound
												Enemy[i].vx = 1;					//Invert the velocity
										break;
						case 4:	if(Enemy[i].x > 110){			//Check for rightmost bound
												Enemy[i].vx = -1;				//Invert the velocity
										}else if(Enemy[i].x < 90)		//Check for leftmost bound
												Enemy[i].vx = 1;					//Invert the velocity
										break;
					}
				}
				if(explosionKill == 1){
					if((((Enemy[i].x + 2) < (xExplosion + wExplosion) && ((Enemy[i].x + 2) > xExplosion)) 																// if right inside explosion's right boundary
						&& (((Enemy[i].y - 1) > (yExplosion - hExplosion) && ((Enemy[i].y - 1) < yExplosion))																//  and (if right inside explosion's top boundary
						|| (((Enemy[i].y - Enemy[i].h + 1) < yExplosion) && ((Enemy[i].y - Enemy[i].h + 1) > (yExplosion - hExplosion)))))	// or right inside explosion's bottom boundary)
					
						|| (((Enemy[i].x + Enemy[i].w - 2) > xExplosion && (((Enemy[i].x + 2) < (xExplosion + wExplosion))))								//if right inside explosion's left boundary
						&& (((Enemy[i].y - 1) > (yExplosion - hExplosion) && ((Enemy[i].y - 1) < yExplosion))																// and (if right insdie explosion's top boundary
						|| (((Enemy[i].y - Enemy[i].h + 1) < yExplosion) && ((Enemy[i].y - Enemy[i].h + 1) > (yExplosion - hExplosion)))))){// or right inside explosion's bottom boundary
					
						Enemy[i].life = missileHit;
					}
				}else if(shoot != 10){
					for(int j = 0; j < 3; j++){
						if(inventory[j].current == fired){
							if(inventory[j].x + 4 > Enemy[i].x + 1 && inventory[j].x + 1 < (Enemy[i].x+Enemy[i].w - 2) && (inventory[j].y - inventory[j].h) == Enemy[i].y){
								Enemy[i].life = missileHit;
								inventory[j].current = unfired;
								if(numMissiles > 0)
									numMissiles--;
							}
						}
					}
					if(inventory[3].current == fired){
						if(inventory[3].x > Enemy[i].x && inventory[3].x < (Enemy[i].x+Enemy[i].w - 2) && (inventory[3].y - inventory[3].h) == Enemy[i].y){
								Enemy[i].life = laserHit;
								inventory[3].current = unfired;
								numLasers = 0;
						}
					}
				}
				
				/*//enemies shooting missiles
				if(Enemy[i].ammo.current == unfired){
					Enemy[i].ammo.x = Enemy[i].x + 8;
					Enemy[i].ammo.y = Enemy[i].y;
					Enemy[i].ammo.current = fired;
				}
				if(Enemy[i].ammo.current == fired){
					Enemy[i].ammo.y += Enemy[i].ammo.vy;
				}*/
			}
			if(Enemy[i].life == alive){	
				if(Enemy[i].y > 0)
					Enemy[i].x += Enemy[i].vx;
				Enemy[i].y += Enemy[i].vy;
				Enemy[i].vy = 0;
			}else{
				if(ship.life == alive)	
					Sound_Killed();
			}
		}
	}
	//move ship
	if(ADCStatus == 1){
		if(ADCMail > previous + 20)
			ship.vx = -1;
		else if(ADCMail < previous - 20)
			ship.vx = 1;
		else
			ship.vx = 0;
		if((ship.x + ship.vx) > 109 || (ship.x + ship.vx) < 0)
			ship.vx = 0;
		ship.x += ship.vx;
	}
	//check if ship is hit
	
}


void GameDraw(void){ int i;
	if(fire != 0){
		if(fire == 2){
			if(numLasers < 1)
				shoot = 3;
			else
				shoot = 4;
			fire = 0;
		}else{
			if(numMissiles < 3){
				uint8_t notFound = 1;
				i = 0;
				while(notFound){
					if(inventory[i].current == unfired){
						shoot = i;
						notFound = 0;
					}
					i++;
				}
			}else
				shoot = 4;
			fire = 0;
		}
		if(shoot != 4){
			inventory[shoot].x = ship.x + 7;
			inventory[shoot].y = ship.y - 8;	
			inventory[shoot].current = fired;
			ST7735_DrawBitmap(inventory[shoot].x, inventory[shoot].y, inventory[shoot].image, inventory[shoot].w, inventory[shoot].h);
			Sound_Shoot();
			if(shoot < 3)
				numMissiles++;
			else
				numLasers++;
		}
		shoot = 4;
	}
	for(i = 0; i < 4; i++){
		if(inventory[i].current == fired)
			ST7735_DrawBitmap(inventory[i].x, inventory[i].y, inventory[i].image, inventory[i].w, inventory[i].h);
		else
			ST7735_DrawBitmap(inventory[i].x, inventory[i].y, inventory[i].black, inventory[i].w, inventory[i].h);			
	}

	for(i=0;i<20;i++){
			if(Enemy[i].needDraw){
				if(Enemy[i].life == alive){
					ST7735_DrawBitmap(Enemy[i].x, Enemy[i].y, Enemy[i].image, Enemy[i].w, Enemy[i].h);
					Enemy[i].needDraw = 0;
				}else if(Enemy[i].life == missileHit){
					ST7735_DrawBitmap(Enemy[i].x, Enemy[i].y, SmallExplosion0, Enemy[i].w, Enemy[i].h);
					Sound_Explosion();
					Enemy[i].life = dead;
					points += 5;
				}else if(Enemy[i].life == laserHit){
					ST7735_DrawBitmap(Enemy[i].x - 4, Enemy[i].y, BigExplosion0, wExplosion, hExplosion);
					Sound_Explosion();
					if(Enemy[i].x - 4 < 0)
						xExplosion = 0;
					else
						xExplosion = Enemy[i].x - 4;
					yExplosion = Enemy[i].y;
					Enemy[i].life = dead;
					explosionKill = 1;
					points += 10;
				}else{
					if(explosionKill == 1){
						ST7735_DrawBitmap(xExplosion, yExplosion, BigExplosionBlack, wExplosion, hExplosion);
						explosionKill = 0;
						xExplosion = 127;
						yExplosion = 159;
					}else
						ST7735_DrawBitmap(Enemy[i].x, Enemy[i].y, Enemy[i].black, Enemy[i].w, Enemy[i].h);
					Enemy[i].needDraw = 0;
				}
			}
			/*if(Enemy[i].ammo.current == fired){
				ST7735_DrawBitmap(Enemy[i].ammo.x, Enemy[i].ammo.y, Enemy[i].ammo.image, Enemy[i].ammo.w, Enemy[i].ammo.h);
			}*/
	}
	
	if(ship.needDraw){
		if(ship.life == alive)
			ST7735_DrawBitmap(ship.x, ship.y, ship.image, ship.w, ship.h);
		else if(ship.life == missileHit){
				ST7735_DrawBitmap(ship.x, ship.y, SmallExplosion0, ship.w, ship.h);
				ship.life = dead;
		}else{
			ST7735_DrawBitmap(ship.x, ship.y, ship.black, ship.w, ship.h);
			Anyalive = 0;
		}
	}
}
void GameTask(void){	//30 Hz
		GameMove();
		Flag = 1;
	}
void end(void){								 //Game over Display
	ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char *)Phrases[GAMEOVER][myLanguage]);
  ST7735_SetCursor(1, 2);
  ST7735_OutString((char *)Phrases[NICETRY][myLanguage]);
  ST7735_SetCursor(1, 3);
  ST7735_OutString((char *)Phrases[EARTHLING][myLanguage]);
  ST7735_SetCursor(2, 4);
	ST7735_OutString((char *)Phrases[SCORE][myLanguage]);
  LCD_OutDec(points);
}

void AdvanceTask(void){
	for(int i = 0; i < 20; i++)
		if(Enemy[i].life == alive)
			Enemy[i].vy = 1;
}
int main(void){
  DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
  Button_Init();
	Output_Init();
	WelcomeScreen();
	GameInit();
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
	ADC_Init();
	SysTick_Init();
	Sound_Init();
	Timer1_Init(&GameTask, 80000000/30);
	Timer2_Init(&AdvanceTask, 80000000/5);
	EnableInterrupts();
	
	
	do{
		while(Flag == 0){};
			Flag = 0;
			GameDraw();
		}while(ship.life == alive && Anyalive);
	
  Delay100ms(50);              // delay 5 sec at 80 MHz
	end();
  while(1){
  }

}



// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}


