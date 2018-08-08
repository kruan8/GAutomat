
/*
 * App.h
 *
 *  Created on: 10. 8. 2017
 *      Author: priesolv
 */

#ifndef APP_H_
#define APP_H_

#include "stm32f4xx.h"
#include "common.h"
#include "XPT2046.h"

typedef struct
{
  Coordinate lcd[3];  // kalibracni udaje touch screenu
  bool    lcd_calibrated;
  uint8_t light_on;
  uint8_t light_off;
  uint8_t temperature;
  uint8_t temp_max;
  uint32_t crc;
  // zarovnat na 32bit !!!
} app_data_t;

typedef struct
{
  uint8_t nError;
  uint8_t nHour;   // hodina RTC
  uint8_t nMin;    // minuta RTC
  uint8_t nTemperature;
  uint8_t nHumidity;
  bool bLight;
  bool bHeat;
  bool bFan;
} app_measure_data_t;

typedef enum
{
  err_none = 0,
  err_dht,
}app_errors_t;

void App_Init();
void App_Timer1ms();
bool App_Click();
void App_Exec();
void App_RegulationLoop(app_measure_data_t* data);
void App_Calibrate();
app_data_t* App_GetConfig();
void App_SetConfigDefault();
void App_SaveConfig();
bool App_LoadConfig();

uint32_t App_CountCRC32HW(uint8_t* buffer, uint16_t size);

uint8_t App_GetLightOn();
bool App_SetLightOn(uint8_t nValue);
uint8_t App_GetLightOff();
bool App_SetLightOff(uint8_t nValue);
uint8_t App_GetTemperature();
bool App_SetTemperature(uint8_t nValue);
uint8_t App_GetTemperatureMax();
bool App_SetTemperatureMax(uint8_t nValue);

#endif /* APP_H_ */
