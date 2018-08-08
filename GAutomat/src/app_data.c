/*
 * app_data.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "app_data.h"

// Todo: !!! existuji dve stejne struktury (App.h) !!!
typedef struct
{
  Coordinate lcd[3];  // kalibracni udaje touch screenu
  bool       lcd_calibrated;
  uint8_t    light_on;
  uint8_t    light_off;
  uint8_t    temperature;
  uint8_t    temp_max;
  uint32_t   crc;
  // zarovnat na 32bit !!!
} app_data_t;


app_data_t g_appData;

uint8_t AppData_GetLightOn()
{
  return g_appData.light_on;
}

bool AppData_SetLightOn(uint8_t nValue)
{
  g_appData.light_on = nValue;
  return true;
}

uint8_t AppData_GetLightOff()
{
  return g_appData.light_off;
}

bool AppData_SetLightOff(uint8_t nValue)
{
  g_appData.light_off = nValue;
  return true;
}

uint8_t AppData_GetTemperature()
{
  return g_appData.temperature;
}

bool AppData_SetTemperature(uint8_t nValue)
{
  g_appData.temperature = nValue;
  return true;
}

uint8_t AppData_GetTemperatureMax()
{
  return g_appData.temp_max;
}

bool AppData_SetTemperatureMax(uint8_t nValue)
{
  g_appData.temp_max = nValue;
  return true;
}


