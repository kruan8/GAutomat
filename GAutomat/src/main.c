/**
  ******************************************************************************
  * @file    main.c
  * @author  Priesol Vladimir
  * @version
  * @date    16.8.2018
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f4xx.h"
			
#include "ILI9163.h"
#include "SPI1.h"
#include "XPT2046.h"
#include "timer.h"
#include "DHT.h"
#include "App.h"

#include "rtcf4.h"

#include "wnd_manager.h"
#include "windows/wnd_start.h"

/*
 * v0.0 - verze bez window manageru
 * v0.1 - prepracovana sprava oken (window manager)
 *        opraveno zpracovani kliknuti na LCD displej
 *        ikony prevedeny do monochrome bitmapy
 *
 * Todo: pridat hysterezi na teploty
 *
 *
 */

int main(void)
{
  // kontrola hodin
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks); // Get system clocks


  // Start Watchdog
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_64);      // 32kHz / 64
  IWDG_SetReload(0xFFF);                    //  WD interval 4096 ms
  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);

  DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP, ENABLE);   // WD stop in debug mode

  Timer_Init();
  SPI1_Init();
  ILI9163_Init();
  XPT2046_Init();
  DHT_Init(RCC_Clocks.PCLK2_Frequency);  // PCLK2 for TIM9
  RTCF4_Init();
  App_Init();

//  RTCF4_Test();

  Wm_Init();
  Wm_AddNewWindow(WndStart_GetTemplate());
  Wm_Exec();
}
