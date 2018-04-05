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
  edit_tb_name = 0,
  edit_tb_value,
}wnd_edit_tb_e;

typedef enum
{
  edit_bt_plus = 0,
  edit_bt_minus,
  edit_bt_ok,
  edit_bt_cancel,
}wnd_edit_bt_e;

const wnd_control arrSetControls[] =
{
  //   type      |left | top | right | bott |     id        |  alignment  | tcolor  |    font     | text
    { wnd_textbox,  50,   40,    250,    70, edit_tb_name,    ALIGN_CENTER,  C_BLACK, &TEXT_NORMAL, NULL },
    { wnd_button,   95,  100,    135,   130, edit_bt_minus,              0,   C_BLUE, &TEXT_NORMAL, "-" },
    { wnd_textbox, 145,  100,    195,   130, edit_tb_value,   ALIGN_CENTER,    C_RED, &TEXT_NORMAL, NULL },
    { wnd_button,  200,  100,    240,   130, edit_bt_plus,               0,   C_BLUE, &TEXT_NORMAL, "+" },

    { wnd_button,   40,   160,   150,   190, edit_bt_cancel,             0,   C_BLUE, &TEXT_NORMAL, "Storno" },
    { wnd_button,  195,   160,   275,   190, edit_bt_ok,                 0,   C_BLUE, &TEXT_NORMAL, "OK" },
    {   wnd_none,    0,     0,     0,     0,          0,                 0,   C_BLUE, &TEXT_NORMAL, "" },
};

const wnd_window_t wndSetValue =
{
    WND_STYLE_2D | WND_STYLE_SHOW_TITLE,
    0xEF7D,
    (wnd_control*)&arrSetControls,
    "Zmena hodnoty",
    WndSetValue_Init,
    WindowSetValue_Callback,
    NULL,
    NULL,
    NULL,
};

uint8_t* g_pnValue;
uint8_t  g_nValue;
char* g_strName;
char g_strValueText[WND_BUTTON_TEXT_MAX];

wnd_window_t* WndSetValue_GetTemplate()
{
  return (wnd_window_t*) &wndSetValue;
}

void WndSetValue_Init()
{

}

void WndSetValue_SetValue(uint8_t* pnValue, char* strName)
{
  g_pnValue = pnValue;
  g_nValue = *pnValue;
  g_strName = strName;
}

//uint16_t WndSetValue_CreateWindowSetValue(uint16_t nValue, char* pName)
//{
////  g_nValue = nValue;
////  g_pName = pName;
////  uint8_t nControls = sizeof(arrSetControls) / sizeof(wnd_control);
////  return Wnd_CreateWindow((wnd_window_t*)&wndSetValue, nControls);
//}

//uint16_t Wnd_GetEditedValue()
//{
//  return g_nValue;
//}

void WindowSetValue_Callback(UG_MESSAGE *msg)
{
  if (msg->type == MSG_TYPE_OBJECT)
  {
    if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_PRESSED)
    {
      switch (msg->sub_id)
      {
      case edit_bt_plus:
        g_nValue++;
        Wnd_SetTextboxFormInt(Wnd_GetWindow(), edit_tb_value, g_nValue);
        break;
      case edit_bt_minus:
        if (g_pnValue)
        {
          g_nValue--;
          Wnd_SetTextboxFormInt(Wnd_GetWindow(), edit_tb_value, g_nValue);
        }

        break;
      case edit_bt_ok:
        Wm_CloseWindow();
        break;
      case edit_bt_cancel:
        Wm_CloseWindow();
        break;
      }
    }
  }

  if (msg->event == OBJ_EVENT_PRERENDER)
  {
    switch (msg->sub_id)
    {
    case edit_tb_name:
      UG_TextboxSetText(Wm_GetWnd(), edit_tb_name, g_strName);
      break;
    case edit_tb_value:
      {
        UG_TextboxSetText(Wm_GetWnd(), edit_tb_value, g_strValueText);
        Wnd_SetTextboxFormInt(Wm_GetWnd(), edit_tb_value, *g_pnValue);
      }
      break;
    default:
      break;
    }
  }
}
