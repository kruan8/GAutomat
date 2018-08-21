/*
 * app_data.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "app_data.h"

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

#define APPDATA_BACKUP_RAM_ADDR          1        // prvni backup registr pro zapis (??? adresa 0 se mi pri resetu prepisovala ???)

app_data_t g_appData;

Coordinate* AppData_GetCoordinatePointer()
{
  return &(g_appData.lcd[0]);
}

uint8_t AppData_GetLightOn()
{
  return g_appData.light_on;
}

uint8_t* AppData_GetLightOnPointer()
{
  return &g_appData.light_on;
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

uint8_t* AppData_GetLightOffPointer()
{
  return &g_appData.light_off;
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

uint8_t* AppData_GetTemperaturePointer()
{
  return &g_appData.temperature;
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

uint8_t* AppData_GetTemperatureMaxPointer()
{
  return &g_appData.temp_max;
}

bool AppData_SetTemperatureMax(uint8_t nValue)
{
  g_appData.temp_max = nValue;
  return true;
}

void AppData_SetLcdCalibrated(bool bEnable)
{
  g_appData.lcd_calibrated = bEnable;
}

bool AppData_GetLcdCalibrated()
{
  return g_appData.lcd_calibrated;
}

void AppData_SetConfigDefault()
{
  g_appData.light_on = 18;
  g_appData.light_off = 6;
  g_appData.temperature = 18;
  g_appData.temp_max = 28;

  g_appData.lcd_calibrated = false;
  AppData_SaveConfig();
}

bool AppData_LoadConfig()
{
  uint8_t nS = sizeof (app_data_t);

  uint8_t nSize = sizeof(app_data_t) / sizeof(uint32_t);
  uint32_t *pData = (uint32_t*)&g_appData;
  uint32_t nIndex = APPDATA_BACKUP_RAM_ADDR;

  while (nSize--)
  {
    *pData = RTC_ReadBackupRegister(nIndex++);
    pData++;
  }

  nSize = sizeof (g_appData) - sizeof (g_appData.crc);
  uint32_t crc = AppData_CountCRC32HW((uint8_t*)&g_appData, nSize);
  if (crc != g_appData.crc)
  {
    return false;
  }

  return true;
}

void AppData_SaveConfig()
{
  uint8_t nSize = sizeof (g_appData) - sizeof (g_appData.crc);
  g_appData.crc = AppData_CountCRC32HW((uint8_t*)&g_appData, nSize);

  PWR_BackupAccessCmd(ENABLE);

  nSize = sizeof(app_data_t) / sizeof(uint32_t);
  uint32_t *pData = (uint32_t*)&g_appData;
  uint32_t nIndex = APPDATA_BACKUP_RAM_ADDR;
  while (nSize--)
  {
    RTC_WriteBackupRegister(nIndex++, *pData);
    pData++;
  }

  PWR_BackupAccessCmd(DISABLE);
}

uint32_t AppData_CountCRC32HW(uint8_t* buffer, uint16_t size)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

  CRC->CR = CRC_CR_RESET;
  for (uint16_t a = 0; a < size; a++)
  {
    CRC->DR = buffer[a];
  }

  uint32_t crc = (uint32_t) (CRC->DR);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, DISABLE);
  return crc;
}
