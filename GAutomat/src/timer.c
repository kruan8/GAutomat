/*
 * timer.c
 *
 *  Created on: 24. 6. 2016
 *      Author: priesolv
 */

#include "timer.h"


static volatile uint32_t g_nDelayTimer;
static volatile uint32_t g_nTicks = 0;

PtrSysTickCallback pSysTickCallback = 0;

void Timer_Init()
{
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    /* Capture error */
    while (1);
  }
}

void Timer_Delay_ms(uint32_t delay_ms)
{
  g_nDelayTimer = delay_ms;
  while (g_nDelayTimer);
}

uint32_t Timer_GetTicks_ms()
{
  return g_nTicks;
}

void Timer_SetSysTickCallback(PtrSysTickCallback pFunction)
{
  pSysTickCallback = pFunction;
}

void SysTick_Handler(void)
{
  g_nTicks++;
  if (g_nDelayTimer)
  {
    g_nDelayTimer--;
  }

  if (pSysTickCallback)
  {
    pSysTickCallback();
  }
}
