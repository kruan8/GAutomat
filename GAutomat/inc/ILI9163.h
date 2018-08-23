/*
 * ILI9163.h
 *
 *  Created on: 7. 1. 2016
 *      Author: priesolv
 */

#ifndef ILI9163_H_
#define ILI9163_H_

#include "stm32f4xx.h"
#include <stdbool.h>

//#define LCD_128x160
//#define LCD_128x128
//#define LCD_240x320_V
#define LCD_240x320_H


// horizontalni poloha (connector left)
#ifdef LCD_240x320_H
 #define ILI9163_RES_X		320
 #define ILI9163_RES_Y		240
 #define ILI9163_OFFSET   0
 #define ILI9163_SET_MEM  0xE8
#endif

// vertikalni poloha (connector bottom)
#ifdef LCD_240x320_V
 #define ILI9163_RES_X    240
 #define ILI9163_RES_Y    320
 #define ILI9163_OFFSET   0
 #define ILI9163_SET_MEM  0x48
#endif

#ifdef LCD_128x160
 #define ILI9163_RES_X		128
 #define ILI9163_RES_Y		160
 #define ILI9163_OFFSET   0
 #define ILI9163_SET_MEM  0x00
#endif

#ifdef LCD_128x128
 #define ILI9163_RES_X    128
 #define ILI9163_RES_Y    128
 #define ILI9163_OFFSET   32
 #define ILI9163_SET_MEM  0x08
#endif

#define DMA_ENABLE

typedef enum
{
  white =   0xFFFF,
  silver =  0xC618,
  gray =    0x8410,
  black =   0x0000,
  red =     0xF800,
  maroon =  0x5000,
  yellow =  0xFFE0,
  olive =   0x5280,
  lime =    0x07E0,
  green =   0x0280,
  aqua =    0x07FF,
  teal =    0x0410,
  blue =    0x001F,
  navy =    0x0010,
  fuchsia = 0xF81F,
  purple =  0x8010,
}COLORS;

typedef struct{
	uint16_t xPos;
	uint16_t yPos;
	uint16_t txtColor;
	uint16_t bkgColor;
} TextParam;

typedef struct
{
   const void* pData;
   uint16_t nWidth;
   uint16_t nHeight;
   uint16_t nColor;
} BMPbpp1;

void ILI9163_Init();
void ILI9163_WritePixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9163_SetAddress(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop);
void ILI9163_WriteToReg(uint8_t value);
void ILI9163_WriteData8(uint8_t value);
void ILI9163_WriteData16(uint16_t value);
void ILI9163_WriteData16Block(uint16_t* arrBuffer, uint16_t nLength);

void ILI9163_LedOn();
void ILI9163_LedOff();

void ILI9163_FillScreen(uint16_t color);
void ILI9163_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9163_SetTextParams(	uint16_t txtColor,	uint16_t bkgColor);
void ILI9163_PrintText(uint16_t x, uint16_t y, char* text);
void ILI9163_PrintChar(uint16_t x, uint16_t y, char c);

void ILI9163_ConfigDMA();
void ILI9163_TransferDMA(uint32_t nMemAddr, uint16_t nLength, bool bMemInc);

// sada funkci pro akceleraci uGUI
void ILI9163_PixelSetRGB565(int16_t x, int16_t y, uint16_t color);
uint8_t ILI9163_FillFrame(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void ILI9163_InitFillArea(uint16_t nMemSize);
void* ILI9163_FillArea(int16_t posX, int16_t posY, int16_t nEndX, int16_t nEndY);
void ILI9163_FillAreaSetPixel(uint16_t nColor);

void ILI9163_DrawMonochromeBitmap(uint16_t xPos, uint16_t yPos, BMPbpp1* bmp);

#endif /* ILI9163_H_ */
