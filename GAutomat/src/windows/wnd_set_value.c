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
  // left | top | right | bott |     id           |  alignment  | tcolor  |    font     |  type      | text
    {  50,   40,    250,    70, edit_tb_name,       ALIGN_CENTER,  C_BLACK, &TEXT_NORMAL, wnd_textbox, NULL },
    {  95,  100,    135,   130, edit_bt_minus,                 0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "-" },
    { 145,  100,    195,   130, edit_tb_value,      ALIGN_CENTER,    C_RED, &TEXT_NORMAL, wnd_textbox, NULL },
    { 200,  100,    240,   130, edit_bt_plus,                  0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "+" },

    {  40,   160,   150,   190, edit_bt_cancel,                0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "Storno" },
    { 195,   160,   275,   190, edit_bt_ok,                    0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "OK" },
};

const wnd_window_t wndSet =
{
    (wnd_control*)&arrSetControls,
    "Zmena hodnoty",
    WindowSetValue_Callback,
};

uint16_t g_nValue;
char* g_pName;
char g_strValueText[WND_BUTTON_TEXT_MAX];

uint16_t Wnd_CreateWindowSetValue(uint16_t nValue, char* pName)
{
  g_nValue = nValue;
  g_pName = pName;
  uint8_t nControls = sizeof(arrSetControls) / sizeof(wnd_control);
  return Wnd_CreateWindow((wnd_window_t*)&wndSet, nControls);
}

uint16_t Wnd_GetEditedValue()
{
  return g_nValue;
}

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
        if (g_nValue)
        {
          g_nValue--;
          Wnd_SetTextboxFormInt(Wnd_GetWindow(), edit_tb_value, g_nValue);
        }

        break;
      case edit_bt_ok:
        Wnd_SetResult(true);
        Wnd_Exit();
        break;
      case edit_bt_cancel:
        Wnd_SetResult(false);
        Wnd_Exit();
        break;
      }
    }
  }

  if (msg->event == OBJ_EVENT_PRERENDER)
  {
    switch (msg->sub_id)
    {
    case edit_tb_name:
      UG_TextboxSetText(Wnd_GetWindow(), edit_tb_name, g_pName);
      break;
    case edit_tb_value:
      {
        UG_TextboxSetText(Wnd_GetWindow(), edit_tb_value, g_strValueText);
        Wnd_SetTextboxFormInt(Wnd_GetWindow(), edit_tb_value, g_nValue);
      }
      break;
    default:
      break;
    }
  }
}
