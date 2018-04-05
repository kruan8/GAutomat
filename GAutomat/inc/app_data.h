/*
 * app_data.h
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#ifndef APP_DATA_H_
#define APP_DATA_H_

#include "stm32f4xx.h"
#include "common.h"
#include "XPT2046.h"


uint8_t AppData_GetLightOn();
bool AppData_SetLightOn(uint8_t nValue);
uint8_t AppData_GetLightOff();
bool AppData_SetLightOff(uint8_t nValue);
uint8_t AppData_GetTemperature();
bool AppData_SetTemperature(uint8_t nValue);
uint8_t AppData_GetTemperatureMax();
bool AppData_SetTemperatureMax(uint8_t nValue);

#endif /* APP_DATA_H_ */
