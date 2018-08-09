/*
 * wnd_main.c
 *
 *  Created on: 24. 3. 2018
 *      Author: vlada
 */

#include "windows/wnd_main.h"
#include "windows/wnd_edit.h"
#include "timer.h"
#include "App.h"
#include "app_data.h"
#include <stdio.h>

#ifdef DEBUG
  #define REGULATION_LOOP_MS    1000
#elif
  #define REGULATION_LOOP_MS   10000
#endif

#define WND_MAIN_ICON_SIZE      64

typedef enum
{
  main_tb_time = 0,
  main_tb_temp,
  main_tb_hum,
}wnd_main_tb_e;

const wnd_control arrMainControls[] =
{
  //  type       | left | top | right | bott |     id     |  alignment  |  tcolor  | font    |  text
    { wnd_textbox,  70,   30,    250,     85, main_tb_time, ALIGN_CENTER,   C_CYAN, &TEXT_BIG,   "" },
    { wnd_textbox,   5,  110,    154,    155, main_tb_temp, ALIGN_CENTER, C_YELLOW, &TEXT_BIG,   "" },
    { wnd_textbox, 155,  110,    309,    155,  main_tb_hum, ALIGN_CENTER, C_YELLOW, &TEXT_BIG,   "" },
    { wnd_none,      0,    0,      0,      0,            0,            0,        0,         0,   "" },
};

uint32_t g_nMeasureTimer;
bool     g_bRegulation;
app_measure_data_t g_lastData;

const wnd_window_t wndMain =
{
    WND_STYLE_2D | WND_STYLE_HIDE_TITLE,
    C_BLACK,
    (wnd_control*)&arrMainControls,
    "",
    WndMain_Init,
    WndMain_Callback,
    WndMain_Timer_1ms,
    WndMain_Exec,
    WndMain_ExitClickCallBack,
};

wnd_window_t* WndMain_GetTemplate()
{
  return (wnd_window_t*) &wndMain;
}

void WndMain_Init()
{
  Wm_SetEndClick();
  g_nMeasureTimer = 0;
}

void WndMain_Callback(UG_MESSAGE *msg)
{
  if (msg->event == OBJ_EVENT_PRERENDER && msg->id == OBJ_TYPE_TEXTBOX)
  {
    if (msg->sub_id == main_tb_time)
    {
//      UG_TextboxSetText(Wm_GetWnd(), msg->sub_id, g_strVer);
    }
  }
}

void WndMain_Timer_1ms()
{
  if (g_nMeasureTimer == 0)
  {
    g_nMeasureTimer = REGULATION_LOOP_MS;
    g_bRegulation = true;
  }

  g_nMeasureTimer--;
}

void WndMain_Exec()
{
  if (!g_bRegulation)
  {
    return;
  }

  app_measure_data_t data;
  App_RegulationLoop(&data);

  // pokud neni zkalibrovany display, vykreslit znacku
  if (!App_GetConfig()->lcd_calibrated)
  {
    UG_FillFrame(0, 0, 319, 10, C_RED);
  }
  else
  {
    UG_FillFrame(0, 0, 319, 10, C_BLACK);
  }

  // vypis casu
  char* text;
  text = UG_TextboxGetText(Wm_GetWnd(), main_tb_time);
  snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%02d:%02d", data.nHour, data.nMin);
  UG_TextboxSetText(Wm_GetWnd(), main_tb_time, text);

  // vypis teploty a vlhkosti
  if (data.nError != 1)
  {
    if (data.nTemperature != g_lastData.nTemperature)
    {
      text = UG_TextboxGetText(Wm_GetWnd(), main_tb_temp);
      snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d\370C", data.nTemperature);
      UG_TextboxSetText(Wm_GetWnd(), main_tb_temp, text);
      g_lastData.nTemperature = data.nTemperature;
    }

    if (data.nHumidity != g_lastData.nHumidity)
    {
      text = UG_TextboxGetText(Wm_GetWnd(), main_tb_hum);
      snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d %%", data.nHumidity);
      UG_TextboxSetText(Wm_GetWnd(), main_tb_hum, text);
      g_lastData.nHumidity = data.nHumidity;
    }
  }
  else if (data.nError == 1)   // chyba teploty
  {
    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_temp);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "--\370C");
    UG_TextboxSetText(Wm_GetWnd(), main_tb_temp, text);

    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_hum);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "-- %%");
    UG_TextboxSetText(Wm_GetWnd(), main_tb_hum, text);
  }

  // vykresleni symbolu LIGHT
  if (data.bLight != g_lastData.bLight)
  {
    if (data.bLight)
    {
      DrawSun(55, 195, yellow);
      g_lastData.bLight = true;
    }
    else
    {
      DrawSun(55, 195, C_BLACK);
      g_lastData.bLight = false;
    }
  }

  // vykresleni symbolu HEAT
  if (data.bHeat != g_lastData.bHeat)
  {
    if (data.bHeat)
    {
      DrawHeat(158, 195, red);
      g_lastData.bHeat = true;
    }
    else
    {
      DrawHeat(158, 195, C_BLACK);
      g_lastData.bHeat = false;
    }
  }

  // vykresleni symbolu FAN
  if (data.bFan != g_lastData.bFan)
  {
    if (data.bFan)
    {
      DrawFan(255, 195, blue);
      g_lastData.bFan = true;
    }
    else
    {
      DrawFan(255, 195, C_BLACK);
      g_lastData.bFan = false;
    }
  }

  g_bRegulation = false;
}

void WndMain_ExitClickCallBack()
{
  Wm_AddNewWindow(WndEdit_GetTemplate());
}
