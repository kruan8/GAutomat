
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
void App_RegulationLoop(app_measure_data_t* data);

#endif /* APP_H_ */
