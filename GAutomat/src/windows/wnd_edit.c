/*
 * wnd_edit.c

 *
 *  Created on: 18. 9. 2017
 *      Author: vladicek
 */

#include "wnd_manager.h"
#include "windows/wnd_edit.h"
#include "windows/wnd_set_value.h"
#include <stddef.h>
#include "rtcf4.h"
#include "app_data.h"
#include "app.h"

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

rtc_record_time_t g_dt;

const wnd_control arrEditControls[] =
{
  // type       | left | top | right | bott |     id           |  alignment      | tcolor  | font               |    text
    {wnd_textbox,  10,   10,    210,    40, edit_tb_time,      ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL,         "Cas (h:m):"},
    {wnd_button,  180,   10,    235,    40, edit_bt_time_hour,                 0,   C_BLUE, &TEXT_NORMAL,         NULL },
    {wnd_button,  240,   10,    295,    40, edit_bt_time_min,                  0,   C_BLUE, &TEXT_NORMAL,         NULL },

    {wnd_textbox,  10,   40,    210,    70, edit_tb_light_on,  ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL,         "Svetlo ZAP (h):"},
    {wnd_button,  210,   40,    265,    70, edit_bt_light_on,                  0,   C_BLUE, &TEXT_NORMAL,         NULL },

    {wnd_textbox,  10,   70,    210,   100, edit_tb_light_off, ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL,         "Svetlo VYP (h):"},
    {wnd_button,  210,   70,    265,   100, edit_bt_light_off,                 0,   C_BLUE, &TEXT_NORMAL,         NULL },

    {wnd_textbox,  10,   100,   210,   130, edit_tb_temp,      ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL,         "Teplota (\370C):"},
    {wnd_button,  210,   100,   265,   130, edit_bt_temp,                      0,   C_BLUE, &TEXT_NORMAL,         NULL },

    {wnd_textbox,  10,   130,   210,   160, edit_tb_maxtemp,   ALIGN_CENTER_LEFT,  C_BLACK, &TEXT_NORMAL,         "Teplota MAX:"},
    {wnd_button,  210,   130,   265,   160, edit_bt_tempmax,                   0,   C_BLUE, &TEXT_NORMAL,         NULL },

    {wnd_button,   10,   170,   100,   200, edit_bt_cancel,                    0,   C_BLUE, &TEXT_NORMAL,         "Storno" },
    {wnd_button,  120,   170,   170,   200, edit_bt_p1,                        0,  C_FOREST_GREEN, &TEXT_NORMAL,  "P1" },
    {wnd_button,  180,   170,   230,   200, edit_bt_p2,                        0,    C_RED, &TEXT_NORMAL,         "P2" },
    {wnd_button,  240,   170,   300,   200, edit_bt_ok,                        0,   C_BLUE, &TEXT_NORMAL,         "OK" },
    {wnd_none,      0,     0,     0,     0,          0,                        0,        0,            0,           "" },
};

const wnd_window_t wndEdit =
{
    WND_STYLE_2D | WND_STYLE_SHOW_TITLE,
    0xEF7D,
    (wnd_control*)&arrEditControls,
    "Nastaveni",
    WndEdit_Init,
    WindowEdit_Callback,
    NULL,
    NULL,
    NULL,
};

wnd_edit_data_t* g_pEditData;

wnd_window_t* WndEdit_GetTemplate()
{
  return (wnd_window_t*) &wndEdit;
}

void WndEdit_Init()
{
  RTCF4_Get(&g_dt);
}

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
        App_SaveConfig();
        Wm_CloseWindow();
        break;
      case edit_bt_cancel:
        Wm_CloseWindow();
        break;
      case edit_bt_p1:
        App_SetLightOn(18);
        App_SetLightOff(6);
        App_SetTemperature(19);
        AppData_SetTemperatureMax(28);
        UG_WindowShow(Wm_GetWnd());
        break;
      case edit_bt_p2:
        App_SetLightOn(18);
        App_SetLightOff(8);
        App_SetTemperature(21);
        AppData_SetTemperatureMax(28);
        UG_WindowShow(Wm_GetWnd());
        break;
      case edit_bt_light_on:
        Wm_AddNewWindow(WndSetValue_GetTemplate());
        WndSetValue_SetValue(&App_GetConfig()->light_on, "Svetlo zap.");
        Wm_CloseWindow();
        break;
      case edit_bt_light_off:
        Wm_AddNewWindow(WndSetValue_GetTemplate());
        WndSetValue_SetValue(&App_GetConfig()->light_off, "Svetlo vyp.");
        Wm_CloseWindow();
        break;
      case edit_bt_temp:
        Wm_AddNewWindow(WndSetValue_GetTemplate());
        WndSetValue_SetValue(&App_GetConfig()->temperature, "Teplota");
        Wm_CloseWindow();
        break;
      case edit_bt_tempmax:
        Wm_AddNewWindow(WndSetValue_GetTemplate());
        WndSetValue_SetValue(&App_GetConfig()->temp_max, "Teplota max");
        Wm_CloseWindow();
        break;
      }

//      uint8_t* pnValue = Wnd_EditGetValuePointer(msg->sub_id);
//
//      // pokud je nalezena hodnota, tak ji edituj
//      if (pnValue)
//      {
//        char* pText = UG_TextboxGetText(Wnd_GetWindow(), msg->sub_id);
//        if (Wnd_CreateWindowSetValue(*pnValue, pText))
//        {
//          *pnValue = Wnd_GetEditedValue();
//        }
//
//        UG_WindowShow(Wnd_GetWindow());
//      }
    }
  }

  if (msg->event == OBJ_EVENT_PRERENDER)
  {
    if (msg->id == OBJ_TYPE_TEXTBOX)
    {

    }

    if (msg->id == OBJ_TYPE_BUTTON)
    {
      uint32_t nValue = 0;
      bool bUpdate = true;
      switch (msg->sub_id)
      {
      case edit_bt_time_hour:
        nValue = g_dt.hour;
        break;
      case edit_bt_time_min:
        nValue = g_dt.min;
        break;
      case edit_bt_light_on:
        nValue = AppData_GetLightOn();
        break;
      case edit_bt_light_off:
        nValue = AppData_GetLightOff();
        break;
      case edit_bt_temp:
        nValue = AppData_GetTemperature();
        break;
      case edit_bt_tempmax:
        nValue = AppData_GetTemperatureMax();
        break;
      default:
        bUpdate = false;
      }

      if (bUpdate)
      {
        Wnd_SetButtonTextFormInt(Wm_GetWnd(), msg->sub_id, nValue);
      }
    }
  }
}

//uint8_t* Wnd_EditGetValuePointer(uint8_t nId)
//{
//  uint8_t* pnId;
//  switch (nId)
//  {
//  case edit_bt_time_hour:
//    pnId = &g_pEditData->nHour;
//    break;
//  case edit_bt_time_min:
//    pnId = &g_pEditData->nMin;
//    break;
//  case edit_bt_light_on:
//    pnId = &g_pEditData->nLightOn;
//    break;
//  case edit_bt_light_off:
//    pnId = &g_pEditData->nLightOff;
//    break;
//  case edit_bt_temp:
//    pnId = &g_pEditData->nTemperature;
//    break;
//  case edit_bt_tempmax:
//    pnId = &g_pEditData->nTemperatureMax;
//    break;
//  default:
//    pnId = NULL;
//    break;
//  }
//
//  return pnId;
//}
