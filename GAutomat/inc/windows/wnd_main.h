/*
 * wnd_main.h
 *
 *  Created on: 24. 3. 2018
 *      Author: vlada
 */

#ifndef WINDOWS_WND_MAIN_H_
#define WINDOWS_WND_MAIN_H_

#include "stm32f4xx.h"
#include "common.h"
#include "Windows.h"
#include "wnd_manager.h"

wnd_window_t* WndMain_GetTemplate();
void WndMain_Init();
void WndMain_Callback(UG_MESSAGE *msg);
void WndMain_Timer_1ms();
void WndMain_Exec();
void WndMain_ClickCallBack();

#endif /* WINDOWS_WND_MAIN_H_ */
