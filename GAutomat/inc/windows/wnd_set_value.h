/*
 * wnd_set_value.h
 *
 *  Created on: 18. 9. 2017
 *      Author: vladicek
 */

#ifndef WINDOWS_WND_SET_VALUE_H_
#define WINDOWS_WND_SET_VALUE_H_

#include "stm32f4xx.h"
#include "common.h"
#include "wnd_manager.h"

wnd_window_t* WndSetValue_GetTemplate();
void WndSetValue_Init(bool bFirstInit);
uint16_t Wnd_CreateWindowSetValue(uint16_t nValue, char* pName);
uint16_t Wnd_GetEditedValue();
void WndSetValue_Callback(UG_MESSAGE *msg);

void WndSetValue_SetValue(uint8_t* pnValue, char* strName, uint8_t nMin, uint8_t nMax);

#endif /* WINDOWS_WND_SET_VALUE_H_ */
