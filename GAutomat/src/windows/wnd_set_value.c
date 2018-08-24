/*
 * wnd_set_value.c
 *
 *  Created on: 18. 9. 2017
 *      Author: vladicek
 */

#include "windows/wnd_set_value.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wnd_manager.h"

typedef enum
{
  set_tb_name = 0,
  set_tb_value,
}wnd_set_tb_e;

typedef enum
{
  set_bt_plus = 0,
  set_bt_minus,
  set_bt_ok,
  set_bt_cancel,
}wnd_set_bt_e;

const wnd_control arrSetControls[] =
{
  //   type      |left | top | right | bott |     id        |  alignment  | tcolor  |    font    | text
    { wnd_textbox,  50,   40,    250,    70, set_tb_name,    ALIGN_CENTER,  C_BLACK, &TEXT_NORMAL, NULL },
    { wnd_button,   95,  100,    135,   130, set_bt_minus,              0,   C_BLUE, &TEXT_NORMAL, "-" },
    { wnd_textbox, 145,  100,    195,   130, set_tb_value,   ALIGN_CENTER,    C_RED, &TEXT_NORMAL, NULL },
    { wnd_button,  200,  100,    240,   130, set_bt_plus,               0,   C_BLUE, &TEXT_NORMAL, "+" },

    { wnd_button,   40,   160,   150,   190, set_bt_cancel,             0,   C_BLUE, &TEXT_NORMAL, "Storno" },
    { wnd_button,  195,   160,   275,   190, set_bt_ok,                 0,   C_BLUE, &TEXT_NORMAL, "OK" },
    {   wnd_none,    0,     0,     0,     0,          0,                0,   C_BLUE, &TEXT_NORMAL, "" },
};

const wnd_window_t wndSetValue =
{
    WND_STYLE_2D | WND_STYLE_SHOW_TITLE,
    0xEF7D,
    (wnd_control*)&arrSetControls,
    "Zmena hodnoty",
    WndSetValue_Init,
    WndSetValue_Callback,
    NULL,
    NULL,
    NULL,
};

uint8_t* g_pnValue;
uint8_t  g_nValue;
uint8_t g_nMin;
uint8_t g_nMax;
char* g_strName;
char g_strValueText[WND_BUTTON_TEXT_MAX];

wnd_window_t* WndSetValue_GetTemplate()
{
  return (wnd_window_t*) &wndSetValue;
}

void WndSetValue_Init(bool bFirstInit)
{
  UG_TextboxSetText(Wm_GetWnd(), set_tb_name, g_strName);
  UG_TextboxSetText(Wm_GetWnd(), set_tb_value, g_strValueText);
  Wnd_SetTextboxFromInt(Wm_GetWnd(), set_tb_value, g_nValue);
}

void WndSetValue_SetValue(uint8_t* pnValue, char* strName, uint8_t nMin, uint8_t nMax)
{
  g_pnValue = pnValue;
  g_nValue = *pnValue;
  g_nMin = nMin;
  g_nMax = nMax;
  g_strName = strName;
}

void WndSetValue_Callback(UG_MESSAGE *msg)
{
  if (msg->type == MSG_TYPE_OBJECT)
  {
    if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_PRESSED)
    {
      switch (msg->sub_id)
      {
      case set_bt_plus:
        if (g_nValue < g_nMax)
        {
          g_nValue++;
          Wnd_SetTextboxFromInt(Wm_GetWnd(), set_tb_value, g_nValue);
        }
        break;
      case set_bt_minus:
        if (g_nValue > g_nMin)
        {
          g_nValue--;
          Wnd_SetTextboxFromInt(Wm_GetWnd(), set_tb_value, g_nValue);
        }

        break;
      case set_bt_ok:
        *g_pnValue = g_nValue;
        Wm_CloseWindow();
        break;
      case set_bt_cancel:
        Wm_CloseWindow();
        break;
      }
    }
  }

  if (msg->event == OBJ_EVENT_PRERENDER)
  {
    switch (msg->sub_id)
    {
    }
  }
}
