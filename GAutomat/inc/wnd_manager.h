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

typedef struct
{
  uint8_t nStyle;
  UG_COLOR nBackColor;
  const wnd_control* pControls;
  char* pCaption;
  void (*Init)(bool bFirst);             // inicializacni kod po vytvoreni okna
  void (*EventCallBack)( UG_MESSAGE* );  // reakce na udalosti
  void (*TimerCallBack)();               // casovac okna
  void (*ExecCallBack)();                // moznost neco vykonat pri behu okna
  void (*ClickCallBack)();           // moznost reagovat na kliknuti kamkoliv do okna
}wnd_window_t;

typedef void(*PtrExeCallback) (void);

void Wm_Init();
bool Wm_Exec();
bool Wm_AddNewWindow(wnd_window_t* pWndTemplate);
void Wm_CloseWindow();
UG_WINDOW* Wm_GetWnd();

#endif /* WND_MANAGER_H_ */
