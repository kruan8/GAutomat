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

Coordinate* AppData_GetCoordinatePointer();

uint8_t AppData_GetLightOn();
bool AppData_SetLightOn(uint8_t nValue);
uint8_t* AppData_GetLightOnPointer();

uint8_t AppData_GetLightOff();
bool AppData_SetLightOff(uint8_t nValue);
uint8_t* AppData_GetLightOffPointer();

uint8_t AppData_GetTemperature();
bool AppData_SetTemperature(uint8_t nValue);
uint8_t* AppData_GetTemperaturePointer();

uint8_t AppData_GetTemperatureMax();
bool AppData_SetTemperatureMax(uint8_t nValue);
uint8_t* AppData_GetTemperatureMaxPointer();

void AppData_SetLcdCalibrated(bool bEnable);
bool AppData_GetLcdCalibrated();

void AppData_SetCalibrateRequest(bool bEnable);
bool AppData_GetCalibrateRequest();

void AppData_SetConfigDefault();
bool AppData_LoadConfig();
void AppData_SaveConfig();
uint32_t AppData_CountCRC32HW(uint8_t* buffer, uint16_t size);

#endif /* APP_DATA_H_ */
