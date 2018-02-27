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
  uint8_t light_on;
  uint8_t light_off;
  uint8_t temperature;
  uint8_t temp_max;
  uint32_t crc;
  // zarovnat na 32bit !!!
} app_config_t;

typedef enum
{
  err_none = 0,
  err_dht,
}app_errors_t;

void App_Init();
void App_Timer1ms();
bool App_Click();
void App_Exec();
void App_Calibrate();
app_config_t* App_GetConfig();
void App_SetConfigDefault();
void App_SaveConfig();
bool App_LoadConfig();

uint32_t App_CountCRC32HW(uint8_t* buffer, uint16_t size);

#endif /* APP_H_ */
