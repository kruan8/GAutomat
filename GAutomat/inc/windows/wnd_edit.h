/*
 * wnd_edit.h
 *
 *  Created on: 18. 9. 2017
 *      Author: vladicek
 */

#ifndef WINDOWS_WND_EDIT_H_
#define WINDOWS_WND_EDIT_H_

#include "stm32f4xx.h"
#include "common.h"
#include "Windows.h"

typedef struct
{

  uint8_t nHour;   // hodina RTC
  uint8_t nMin;    // minuta RTC
  uint8_t nLightOn;
  uint8_t nLightOff;
  uint8_t nTemperature;
  uint8_t nTemperatureMax;
}wnd_edit_data_t;

wnd_window_t* WndEdit_GetTemplate();
void WndEdit_Init();
bool Wnd_CreateWindowEdit(wnd_edit_data_t* pData);
void WindowEdit_Callback(UG_MESSAGE *msg);

uint8_t* Wnd_EditGetValuePointer(uint8_t nId);

#endif /* WINDOWS_WND_EDIT_H_ */
