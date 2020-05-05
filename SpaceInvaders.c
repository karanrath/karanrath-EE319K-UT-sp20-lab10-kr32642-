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
#include "Timer2.h"
#include "Buttons.h"
#include "DAC.h"

const unsigned short BlackEnemy[160] = {0};
const unsigned short BlackPlayerShip[170] = {0};
const unsigned short BlackBullet[21] = {0};	

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
//void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint32_t delay2;
extern uint32_t ADCStatus;
extern int Shot;
extern int PlayAgain;
extern int lang;
extern int pick;
int EndGame;	// check if game over
int NumDead;
int Score = 0;
uint8_t oldyb = 0;

typedef enum {dead, alive} status_t;
struct sprite {
	int32_t x;	//x-coordinate
	int32_t y;	//y-coordinate
	int32_t vy;	//pixels/30Hz
	const unsigned short *image;	// ptr->img
	const unsigned short *black;	
	status_t life; //0=dead, 1=alive
	uint32_t w;	//width
	uint32_t h;	//height
	uint32_t needDraw;	//true if need to draw
}; typedef struct sprite sprite_t;

struct ship {
	int32_t x; //x-coordinate
	int32_t y; //y-coordinate
	const unsigned short *image;
	const unsigned short *black;
	status_t life; //0=dead, 1=alive
	uint32_t w;	//width
	uint32_t h;	//height
}; typedef struct ship ship_t;

struct bullet {
	int32_t x;	//x-coordinate
	int32_t y;	//y-coordinate
	int32_t vy;	//pixels/30Hz
	const unsigned short *image;	// ptr->img
	const unsigned short *black;	
	status_t life; //0=dead, 1=alive
	uint32_t w;	//width
	uint32_t h;	//height
	uint32_t needDraw;	//true if need to draw
}; typedef struct bullet bullet_t;

sprite_t Enemy[6];
ship_t Ship;
bullet_t Bullet;

void EnemyInit(void) { 
	int i;
	for(i = 0; i < 6; i++){
		Enemy[i].x = 20*i;
		Enemy[i].y = 8;
		Enemy[i].vy = 1;
		Enemy[i].image = SmallEnemy30pointA;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16;
		Enemy[i].h = 12;
		Enemy[i].needDraw = 1;
	}
}

void ShipInit(void) {
	Ship.x = 50;
	Ship.y = 156;
	Ship.image = PlayerShip0;
	Ship.black = BlackPlayerShip;
	Ship.life = alive;
	Ship.w = 20;
	Ship.h = 8;
}

void BulletInit(void){
	for (int i = 0; i < 7; i++){
		Bullet.x = Ship.x + 8;
		Bullet.y = 150;
		Bullet.vy = 2;
		Bullet.image = Bullets; 
		Bullet.black = BlackBullet; 
		Bullet.life = alive;
		Bullet.w = 3; 
		Bullet.h = 7;
	}
}

//enemies
void EnemyDraw(void){
	
	for (int i = 0; i < 6; i++){
		if(Enemy[i].needDraw){
			if(Enemy[i].life == alive){
				ST7735_DrawBitmap(Enemy[i].x, Enemy[i].y, 
				Enemy[i].image, Enemy[i].w, Enemy[i].h);
			}else{
				ST7735_DrawBitmap(Enemy[i].x, Enemy[i].y, 
				Enemy[i].black, Enemy[i].w, Enemy[i].h);
			}
			Enemy[i].needDraw = 0;
		}
	}
}

//ship
void ShipDraw(void) {
	if(Ship.life == alive){
		ST7735_DrawBitmap(Ship.x, Ship.y, 
		Ship.image, Ship.w, Ship.h);
	}
}
void BulletMove(void){

			if (Bullet.y < 10){
				Bullet.life = dead;
				Shot=0;
			}else{
				Bullet.y -= Bullet.vy; 
	}
}
			
		
void BulletDraw(void){
		if(Bullet.life == alive){
			for(int i = 0; i < 6; i++){
				if (((Bullet.y >= (Enemy[i].y - Enemy[i].h)) && (Bullet.y <= Enemy[i].y))
					&& (((Bullet.x) >= Enemy[i].x)&&(Bullet.x <= Enemy[i].x + Enemy[i].w))){
					if(Enemy[i].life==alive){
					Enemy[i].life = dead;
					Bullet.life = dead;
					playsound(1);	
					ST7735_DrawBitmap(Bullet.x, Bullet.y, 
					Bullet.black, Bullet.w, Bullet.h);
					Shot = 0;
					NumDead++; }
				}
					if (NumDead == 6) {
						EndGame = 1;
						Ship.life=alive;
					}
			}
			if (oldyb == 0){	
				ST7735_DrawBitmap(Bullet.x, Bullet.y, 
				Bullet.image, Bullet.w, Bullet.h);
			}
			ST7735_DrawBitmap(Bullet.x, oldyb, 
			Bullet.black, Bullet.w, Bullet.h);
			ST7735_DrawBitmap(Bullet.x, Bullet.y, 
			Bullet.image, Bullet.w, Bullet.h);
			}
		else{
			ST7735_DrawBitmap(Bullet.x, oldyb, 
			Bullet.black, Bullet.w, Bullet.h);
			Bullet.y=150;//reset y
			ST7735_DrawBitmap(Bullet.x, Bullet.y, 
			Bullet.black, Bullet.w, Bullet.h);
			
		}
		oldyb = Bullet.y;
		BulletMove();
}



//draws bullets

//move logic for  enemies
void EnemyMove(void){
	
	for(int i = 0; i < 6; i++){
			Enemy[i].needDraw = 1;
			if (Enemy[i].y > 150){
				//game over  
				Enemy[i].life = dead;	
				Ship.life = dead;  
				EndGame = 1;
				
			}else{
				Enemy[i].y += Enemy[i].vy;
			}
		}
	}

//void GameTask(void){	//30Hz
//	//BUTTONS, PLAYSOUND, SLIDEPOT
//	EnemyMove();
//	}

void Inits(void){
	DisableInterrupts();
	NumDead=0;
	Score=0;
  Timer0_Init(&EnemyMove, (80000000/15)); //game time-step timer
	EnemyInit();
	ShipInit();
	EndGame = 0;
	ADC_Init();        
	SysTick_Init();
	Output_Init();
	EnemyDraw();
	Button_Init();
	ShipDraw();
	DAC_Init();
	EnableInterrupts();	
}

int main (void){
	DisableInterrupts();
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 

	//initializations for language
	DisableInterrupts();
	Output_Init();
	Button_Init();
	EnableInterrupts();
	ST7735_FillScreen(0x0000);
	ST7735_OutString("switch 1:english");
	ST7735_SetCursor(0, 2);
	ST7735_OutString("switch 2:spanish");
//	ST7735_OutString("1e 2s");
	while(lang == 0){}
	Inits();
	
	while(1){
		if(EndGame == 0){
				if(ADCStatus == 1){
				int32_t oldx = Ship.x;
				Ship.x = NewShip();
				ST7735_DrawBitmap(oldx, Ship.y, Ship.black, Ship.w, Ship.h);
				ShipDraw(); 
				}
			EnemyDraw();
		}			
		else{ // EndGame = 1
			int EnemyScore=10;
			for(int i=0;i<NumDead;i++){//final score
				Score += EnemyScore;	//10,30,60,100,150,210
				EnemyScore+=10;			//	20,30,40,50,60
			}
			if(NumDead==0){
				Score=0;
			}
			DisableInterrupts();
			ST7735_FillScreen(0);	// set screen to black		
			ST7735_SetCursor(1, 2);
			LCD_OutDec(Score);
			if (pick == 1){ //english	
				ST7735_SetCursor(1, 3);
				ST7735_OutString("GAME OVER");
			if(NumDead!=6){
					ST7735_SetCursor(1, 4);
					ST7735_OutString("You Lose");
					ST7735_SetCursor(1, 5);
					ST7735_OutString("Human!");
				} else {
					ST7735_SetCursor(1, 4);
					ST7735_OutString("Congrats!");
					ST7735_SetCursor(1,5);
					ST7735_OutString("Human!");
				}	
					ST7735_SetCursor(1, 6);
					ST7735_OutString("Play Again?");
					EnableInterrupts();
					while (PlayAgain == 0){}
					PlayAgain = 0;	//reset 
					Inits();
					}
				if (pick == 2){	//spanish		
				ST7735_SetCursor(1, 3);
				ST7735_OutString("Fin del Juego");
				if(NumDead!=6){
					ST7735_SetCursor(1, 4);
					ST7735_OutString("Has perdido");
					ST7735_SetCursor(1, 5);
					ST7735_OutString("El Ser Humano!");
	
				} else {
					ST7735_SetCursor(1, 4);
					ST7735_OutString("Usted Gana");
					ST7735_SetCursor(1,5);
					ST7735_OutString("El Ser Humano!");
				}	
					ST7735_SetCursor(1, 6);
					ST7735_OutString("Jugar de Nuevo?");
					EnableInterrupts();
					while (PlayAgain == 0){}
					PlayAgain = 0;//reset 
					
					Inits();
					}
		}		
	}
}
