/*
 * wnd_calib.h
 *
 *  Created on: 15. 8. 2018
 *      Author: vlada
 */

#ifndef WINDOWS_WND_CALIB_H_
#define WINDOWS_WND_CALIB_H_

#include "stm32f4xx.h"
#include "common.h"
#include "Windows.h"
#include "wnd_manager.h"

wnd_window_t* WndCalib_GetTemplate();
void WndCalib_Init();

void WndCalib_Callback(UG_MESSAGE *msg);
void WndCalib_Exec();
void WndCalib_Timer_1ms();
void WndCalib_ClickCallBack();

void DrawSymbol();
void RemoveSymbol();
void DrawCross(uint16_t x, uint16_t y, uint16_t colorIn, uint16_t colorOut);

#endif /* WINDOWS_WND_CALIB_H_ */
