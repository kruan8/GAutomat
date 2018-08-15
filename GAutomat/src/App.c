/*
 * App.c
 *
 *  Created on: 10. 8. 2017
 *      Author: priesolv
 */

#include "App.h"
#include "common.h"
#include "Windows.h"
#include "DHT.h"
#include "timer.h"

#include "rtcf4.h"
#include <string.h>

#include "windows/wnd_edit.h"

typedef enum
{
  app_re_light =    PC6,
  app_re_heat =     PC7,
  app_re_fan =      PC8,
  app_re_reserved = PC9,
}app_re_e;

typedef enum
{
  app_re_pos_light = 0,
  app_re_pos_heat,
  app_re_pos_fan,
  app_re_pos_reserved,
}app_re_position_e;

// interval kontroly stavu
#ifdef DEBUG
  #define APP_CHECK_INTERVAL_MS    1000
#elif
  #define APP_CHECK_INTERVAL_MS   10000
#endif

#define APP_LED_INTERVAL_MS    60000

app_re_e g_arrRelays[] = { app_re_light, app_re_heat, app_re_fan, app_re_reserved };

app_data_t g_appData;

uint32_t g_nCheckLastTime;
uint32_t g_nLedOffCounter;


void App_Init()
{
  PWR_BackupRegulatorCmd(ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

  // RE output
  for (uint8_t i = 0; i < sizeof (g_arrRelays); ++i)
  {
    gpio_pins_e ePin = g_arrRelays[i];
    GPIO_ClockEnable(GPIO_GetPort(ePin));
    GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(ePin);
    GPIO_Init(GPIO_GetPort(ePin), &GPIO_InitStructure);
  }

  Timer_SetSysTickCallback(App_Timer1ms);
  XPT2046_SetClickCallback(App_Click);
  g_nLedOffCounter = APP_LED_INTERVAL_MS;

  // all relays OFF
  GPIO_GetPort(app_re_light)->BSRRL = GPIO_GetPin(app_re_light);
  GPIO_GetPort(app_re_heat)->BSRRL = GPIO_GetPin(app_re_heat);
  GPIO_GetPort(app_re_fan)->BSRRL = GPIO_GetPin(app_re_fan);
  GPIO_GetPort(app_re_reserved)->BSRRL = GPIO_GetPin(app_re_reserved);
}

void App_Timer1ms()
{
  if (g_nLedOffCounter)
  {
    g_nLedOffCounter--;
    if (g_nLedOffCounter == 0)
    {
      ILI9163_LedOff();
    }
  }

  // zachyceni preteceni ticks casovace
  if (Timer_GetTicks_ms() == 0)
  {
    g_nCheckLastTime = 0;
  }
}

bool App_Click()
{
  // pri kliknuti nastavit casovac vypnuti LED podsviceni
  bool bClick = true;
  if (g_nLedOffCounter == 0)
  {
    bClick = false;
    ILI9163_LedOn();
  }

  g_nLedOffCounter = APP_LED_INTERVAL_MS;
  return bClick;
}

void App_RegulationLoop(app_measure_data_t* data)
{
  // provedeme regulace
  g_nCheckLastTime = Timer_GetTicks_ms();

  memset(data, 0, sizeof(wnd_main_data_t));

  dht_data_t dht_data;
  if (DHT_GetData(&dht_data) != DHT_OK)
  {
    // nastavit chybu mereni teploty
    data->nError = err_dht;
  }

  dht_data.Temp /= 10;

  data->nTemperature = dht_data.Temp;
  data->nHumidity = dht_data.Hum;

  uint8_t nReState = 0;

  // vystupy se spinaji proti napajeni -> takze relatka neguji
  if (data->nError != err_dht)
  {
    // temperature check
    if (dht_data.Temp < g_appData.temperature)
    {
      nReState |= (1 << app_re_pos_heat);
      GPIO_GetPort(app_re_heat)->BSRRH = GPIO_GetPin(app_re_heat); // reset (rele ON)
      data->bHeat = true;
    }
    else
    {
      GPIO_GetPort(app_re_heat)->BSRRL = GPIO_GetPin(app_re_heat); // set (rele OFF)
    }

    // max temperature check
    if (dht_data.Temp > g_appData.temp_max)
    {
      nReState |= (1 << app_re_pos_fan);
      GPIO_GetPort(app_re_fan)->BSRRH = GPIO_GetPin(app_re_fan); // reset (rele ON)
      data->bFan = true;
    }
    else
    {
      GPIO_GetPort(app_re_fan)->BSRRL = GPIO_GetPin(app_re_fan); // set (rele OFF)
    }
  }

  // light check
  rtc_record_time_t dt;
  RTCF4_Get(&dt);
  data->nHour = dt.hour;
  data->nMin = dt.min;
  uint8_t nHour = dt.hour;
  uint8_t nOn = g_appData.light_on;
  uint8_t nOff = g_appData.light_off;
  bool bInvers = false;

  // zjistime, jestli interval prechazi do druheho ne
  if (nOff < nOn)
  {
    nOn = g_appData.light_off;
    nOff = g_appData.light_on;
    bInvers = true;
  }

  if ((nHour >= nOn && nHour < nOff) ^ bInvers)
  {
    nReState |= (1 << app_re_pos_light);
    GPIO_GetPort(app_re_light)->BSRRH = GPIO_GetPin(app_re_light); // reset (rele ON)
    data->bLight = true;
  }
  else
  {
    GPIO_GetPort(app_re_light)->BSRRL = GPIO_GetPin(app_re_light); // set (rele OFF)
  }
}
