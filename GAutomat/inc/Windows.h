/*
 * Windows.h
 *
 *  Created on: 10. 8. 2017
 *      Author: priesolv
 */

#ifndef WINDOWS_H_
#define WINDOWS_H_

#include "stm32f4xx.h"
#include <stdbool.h>
#include "ILI9163.h"
#include "ugui.h"
#include "common.h"
#include "XPT2046.h"

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


#define TEXT_NORMAL   FONT_12X20
#define TEXT_BIG      FONT_24X40
#define TEXT_MAX      TEXT_BIG

#define WND_BUTTON_TEXT_MAX  (10)  // + ukoncovaci 0
#define WND_TEXTBOX_TEXT_MAX  (20)  // + ukoncovaci 0


void Wnd_CreateTextBox(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_TEXTBOX* pTextbox);
void Wnd_CreateButton(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_BUTTON* pButton);
void Wnd_SetButtonTextFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);
void Wnd_SetTextboxFromInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);

#endif /* WINDOWS_H_ */
