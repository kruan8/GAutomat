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
#include "Windows.h"

uint16_t Wnd_CreateWindowSetValue(uint16_t nValue, char* pName);
uint16_t Wnd_GetEditedValue();
void WindowSetValue_Callback(UG_MESSAGE *msg);

#endif /* WINDOWS_WND_SET_VALUE_H_ */
