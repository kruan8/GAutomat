/*
 * wnd_start.h
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#ifndef WINDOWS_WND_START_H_
#define WINDOWS_WND_START_H_

#include "stm32f4xx.h"
#include "common.h"
#include "Windows.h"
#include "wnd_manager.h"

wnd_window_t* WndStart_Init();
void WndStart_Callback(UG_MESSAGE *msg);

#endif /* WINDOWS_WND_START_H_ */
