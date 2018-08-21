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
#include "ILI9163.h"
#include "ugui.h"

#define TEXT_NORMAL   FONT_12X20
#define TEXT_BIG      FONT_24X40
#define TEXT_MAX      TEXT_BIG

#define WND_BUTTON_TEXT_MAX  (10)  // + ukoncovaci 0
#define WND_TEXTBOX_TEXT_MAX  (20)  // + ukoncovaci 0

typedef enum
{
  wnd_none = 0,
  wnd_textbox,
  wnd_button,
}wnd_ctrl_type_e;

typedef struct
{
  wnd_ctrl_type_e eType;
  uint16_t left;
  uint16_t top;
  uint16_t right;
  uint16_t bottom;
  uint8_t id;
  uint8_t alignment;
  uint16_t text_color;
  const UG_FONT* font;
  const char* pText;
}wnd_control;

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

void Wnd_SetButtonTextFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);
void Wnd_SetTextboxFromInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);

#endif /* WND_MANAGER_H_ */
