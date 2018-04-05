/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f4xx.h"
			
#include "ILI9163.h"
#include "SPI1.h"
#include "ugui.h"
#include "XPT2046.h"
#include "timer.h"
#include <stdio.h>
#include "DHT.h"
#include "Windows.h"
#include "App.h"

#include "rtcf4.h"

#include "wnd_manager.h"
#include "windows/wnd_start.h"
#include "windows/wnd_main.h"


int main(void)
{
  // kontrola hodin
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks); // Get system clocks

  Timer_Init();
  SPI1_Init();
  ILI9163_Init();
  XPT2046_Init();
  DHT_Init(RCC_Clocks.PCLK2_Frequency);  // PCLK2 for TIM9
  RTCF4_Init();
  App_Init();

//  RTCF4_Test();

  Wnd_Init();

  WndStart();

  // kontrola TOUCH click -> kalibrace
  App_Calibrate();

  Wm_Init();
  Wm_AddNewWindow(WndMain_GetTemplate());

  Wm_Exec();

//  while(1)
//  {
//    App_Exec();
//  }
}






