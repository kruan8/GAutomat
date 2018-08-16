/* -----------------------------------------------------------------------------

XPT2046 based touchscreen driver

Copyright (C) 2013  Fabio Angeletti - fabio.angeletti89@gmail.com

Part of this code is an adaptation from souce code provided by
		WaveShare - http://www.waveshare.net

I'm not the owner of the whole code

------------------------------------------------------------------------------*/

#ifndef _TOUCHPANEL_H_
#define _TOUCHPANEL_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/
typedef	struct POINT 
{
  uint16_t x;
  uint16_t y;
} Coordinate;

typedef struct Matrix 
{						
/* long double */ int32_t An,
            Bn,     
            Cn,   
            Dn,    
            En,    
            Fn,     
            Divider ;
} Matrix ;

typedef bool(*PtrXPT2046ClickCallback) (Coordinate* pScreenCoordinate);

void XPT2046_Init(void);
void XPT2046_Calibrate(void);  // calibration routine
bool XPT2046_Delay(void);   // returns if a pressure is present
Coordinate *Read_XPT2046(void);  // returns coordinates of the pressure
void XPT2046_GetAdXY(uint16_t *x, uint16_t *y);
void XPT2046_SetClickCallback(PtrXPT2046ClickCallback pFunction);

// function to calibrate touchscreen and use calibration matrix
FunctionalState XPT2046_SetCalibrationMatrix(Coordinate * displayPtr,Coordinate * screenPtr);
FunctionalState getDisplayPoint(Coordinate * displayPtr,Coordinate * screenPtr);

#endif
