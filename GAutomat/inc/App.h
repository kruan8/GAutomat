
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
