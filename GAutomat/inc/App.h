
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


typedef enum
{
  err_none = 0,
  err_dht,
} app_errors_t;

typedef struct
{
  app_errors_t nError;         // chyba regulacni smycky
  uint8_t nHour;               // hodina RTC
  uint8_t nMin;                // minuta RTC
  uint8_t nTemperature;        // namerena teplota
  uint8_t nHumidity;           // namerena vlhkost
  bool bLight;                 // stav svetla
  bool bHeat;                  // stav vyhrivani
  bool bFan;                   // stav vetraku
} app_measure_data_t;

void App_Init();
bool App_RegulationLoop(app_measure_data_t* data);

#endif /* APP_H_ */
