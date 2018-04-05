/*
 * wnd_manager.h
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#ifndef WND_MANAGER_H_
#define WND_MANAGER_H_

#include "stm32f4xx.h"
#include "common.h"
#include "Windows.h"

typedef void(*PtrExeCallback) (void);

void Wm_Init();
bool Wm_Exec();
bool Wm_AddNewWindow(wnd_window_t* pWndTemplate);
void Wm_CloseWindow();
void Wm_SetEndClick();
UG_WINDOW* Wm_GetWnd();

#endif /* WND_MANAGER_H_ */
