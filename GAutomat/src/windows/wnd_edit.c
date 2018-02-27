/*
 * wnd_edit.c

 *
 *  Created on: 18. 9. 2017
 *      Author: vladicek
 */

#include "windows/wnd_edit.h"
#include "windows/wnd_set_value.h"
#include <stddef.h>

typedef enum
{
  edit_tb_time = 0,
  edit_tb_light_on,
  edit_tb_light_off,
  edit_tb_temp,
  edit_tb_maxtemp,
}wnd_edit_tb_e;

typedef enum
{
  edit_bt_time_hour = 0,
  edit_bt_time_min,
  edit_bt_light_on,
  edit_bt_light_off,
  edit_bt_temp,
  edit_bt_tempmax,
  edit_bt_ok,
  edit_bt_cancel,
  edit_bt_p1,
  edit_bt_p2,
}wnd_edit_bt_e;

typedef struct
{
  uint8_t nOffset;   // offset clenu ve strukture 'wnd_edit_data_t', ktery se bude editovat
  char* pName;        // jmeno editovane hodnoty
}wnd_edit_names;

const wnd_edit_names arrEditNames[] =
{
    { offsetof(wnd_edit_data_t, nHour), "Cas (hodiny)" },
    { offsetof(wnd_edit_data_t, nMin),  "Cas (minuty)" },
    { offsetof(wnd_edit_data_t, nLightOn), "Svetlo zapnuto (hodiny)" },
    { offsetof(wnd_edit_data_t, nLightOff), "Svetlo vypnuto (hodiny)" },
    { offsetof(wnd_edit_data_t, nTemperature), "Teplota (\370C)" },
    { offsetof(wnd_edit_data_t, nTemperatureMax), "Teplota MAX (\370C)" },

};

const wnd_control arrEditControls[] =
{
  // left | top | right | bott |     id           |  alignment      | tcolor  | font       |  type      | text
    {  10,   10,    210,    40, edit_tb_time,      ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL, wnd_textbox, "Cas (h:m):"},
    { 180,   10,    235,    40, edit_bt_time_hour,                 0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },
    { 240,   10,    295,    40, edit_bt_time_min,                  0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },

    {  10,   40,    210,    70, edit_tb_light_on,  ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL, wnd_textbox, "Svetlo ZAP (h):"},
    { 210,   40,    265,    70, edit_bt_light_on,                  0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },

    {  10,   70,    210,   100, edit_tb_light_off, ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL, wnd_textbox, "Svetlo VYP (h):"},
    { 210,   70,    265,   100, edit_bt_light_off,                 0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },

    {  10,   100,    210,  130, edit_tb_temp,      ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL, wnd_textbox, "Teplota (\370C):"},
    { 210,   100,   265,   130, edit_bt_temp,                      0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },

    {  10,   130,   210,   160, edit_tb_maxtemp,   ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL, wnd_textbox, "Teplota MAX:"},
    { 210,   130,   265,   160, edit_bt_tempmax,                   0,   C_BLUE, &TEXT_NORMAL, wnd_button,  NULL },

    {  10,   170,   100,   200, edit_bt_cancel,                    0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "Storno" },
    { 120,   170,   170,   200, edit_bt_p1,                        0,  C_FOREST_GREEN, &TEXT_NORMAL, wnd_button,  "P1" },
    { 180,   170,   230,   200, edit_bt_p2,                        0,    C_RED, &TEXT_NORMAL, wnd_button,  "P2" },
    { 240,   170,   300,   200, edit_bt_ok,                        0,   C_BLUE, &TEXT_NORMAL, wnd_button,  "OK" },
};

const wnd_window_t wndEdit =
{
    (wnd_control*)&arrEditControls,
    "Nastaveni",
    WindowEdit_Callback
};

wnd_edit_data_t* g_pEditData;

bool Wnd_CreateWindowEdit(wnd_edit_data_t* pData)
{
  g_pEditData = (wnd_edit_data_t*)pData;
  uint8_t nControls = sizeof(arrEditControls) / sizeof(wnd_control);
  return Wnd_CreateWindow((wnd_window_t*)&wndEdit, nControls);
}

void WindowEdit_Callback(UG_MESSAGE *msg)
{
  if (msg->type == MSG_TYPE_OBJECT)
  {
    if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_PRESSED)
    {
      switch (msg->sub_id)
      {
      case edit_bt_ok:
        Wnd_SetResult(true);
        Wnd_Exit();
        break;
      case edit_bt_cancel:
        Wnd_SetResult(false);
        Wnd_Exit();
        break;
      case edit_bt_p1:
        g_pEditData->nLightOn = 18;
        g_pEditData->nLightOff = 6;
        g_pEditData->nTemperature = 19;
        UG_WindowShow(Wnd_GetWindow());
        break;
      case edit_bt_p2:
        g_pEditData->nLightOn = 18;
        g_pEditData->nLightOff = 8;
        g_pEditData->nTemperature = 21;
        UG_WindowShow(Wnd_GetWindow());
        break;
      }

      uint8_t* pnValue = Wnd_EditGetValuePointer(msg->sub_id);

      // pokud je nalezena hodnota, tak ji edituj
      if (pnValue)
      {
        char* pText = UG_TextboxGetText(Wnd_GetWindow(), msg->sub_id);
        if (Wnd_CreateWindowSetValue(*pnValue, pText))
        {
          *pnValue = Wnd_GetEditedValue();
        }

        UG_WindowShow(Wnd_GetWindow());
      }
    }
  }

  if (msg->event == OBJ_EVENT_PRERENDER && msg->id == OBJ_TYPE_BUTTON)
  {
    uint8_t* pnValue = Wnd_EditGetValuePointer(msg->sub_id);

    // pokud je nalezena hodnota, tak ji edituj
    if (pnValue)
    {
      Wnd_SetButtonTextFormInt(Wnd_GetWindow(), msg->sub_id, *pnValue);
    }
  }
}

uint8_t* Wnd_EditGetValuePointer(uint8_t nId)
{
  uint8_t* pnId;
  switch (nId)
  {
  case edit_bt_time_hour:
    pnId = &g_pEditData->nHour;
    break;
  case edit_bt_time_min:
    pnId = &g_pEditData->nMin;
    break;
  case edit_bt_light_on:
    pnId = &g_pEditData->nLightOn;
    break;
  case edit_bt_light_off:
    pnId = &g_pEditData->nLightOff;
    break;
  case edit_bt_temp:
    pnId = &g_pEditData->nTemperature;
    break;
  case edit_bt_tempmax:
    pnId = &g_pEditData->nTemperatureMax;
    break;
  default:
    pnId = NULL;
    break;
  }

  return pnId;
}
