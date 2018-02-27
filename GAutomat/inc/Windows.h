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

typedef struct
{
  uint8_t nError;
  uint8_t nHour;   // hodina RTC
  uint8_t nMin;    // minuta RTC
  uint8_t nTemperature;
  uint8_t nHumidity;
  bool bLight;
  bool bHeat;
  bool bFan;
}wnd_main_data_t;

typedef enum
{
  wnd_textbox = 0,
  wnd_button,
}wnd_ctrl_type_e;

typedef struct
{
  uint16_t left;
  uint16_t top;
  uint16_t right;
  uint16_t bottom;
  uint8_t id;
  uint8_t alignment;
  uint16_t text_color;
  const UG_FONT* font;
  wnd_ctrl_type_e eType;
  const char* pText;
}wnd_control;

typedef struct
{
  const wnd_control* pControls;
  char* pCaption;
  void (*CallBack)( UG_MESSAGE* );
}wnd_window_t;

#define TEXT_NORMAL   FONT_12X20
#define TEXT_BIG      FONT_24X40
#define TEXT_MAX      TEXT_BIG

#define WND_BUTTON_TEXT_MAX  (9 + 1)  // + ukoncovaci 0

void Wnd_Init();

void WindowMain(wnd_main_data_t* data);
void WindowMain_Callback(UG_MESSAGE *msg);
bool Wnd_CreateWindow(wnd_window_t* pWindowTemplate, uint8_t nControls);

UG_WINDOW* Wnd_GetWindow();
void Wnd_SetResult(bool bResult);
void Wnd_Exit();

void WndStart();

void Wnd_CreateTextBox(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_TEXTBOX* pTextbox);
void Wnd_CreateButton(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_BUTTON* pButton);
void Wnd_SetButtonTextFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);
void Wnd_SetTextboxFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue);

void DrawCross(uint16_t x, uint16_t y, uint16_t colorIn, uint16_t colorOut);
void DrawSun(uint16_t x, uint16_t y, uint16_t color);
void DrawHeat(uint16_t x, uint16_t y, uint16_t color);
void DrawFan(uint16_t x, uint16_t y, uint16_t color);

bool Wnd_Calibrate(Coordinate* pScreenSamples, bool bEnableMessage);
void Wnd_SetCalibration(Coordinate* pScreenSamples);

#endif /* WINDOWS_H_ */
