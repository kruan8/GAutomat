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

  char* text;
  text = UG_TextboxGetText(Wm_GetWnd(), main_tb_time);
  snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%02d:%02d", data.nHour, data.nMin);
  UG_TextboxSetText(Wm_GetWnd(), main_tb_time, text);

  if (data.nError != 1)  // chyba teploty
  {
    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_temp);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d\370C", data.nTemperature);
    UG_TextboxSetText(Wm_GetWnd(), main_tb_temp, text);

    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_hum);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d %%", data.nHumidity);
    UG_TextboxSetText(Wm_GetWnd(), main_tb_hum, text);
  }
  else if (data.nError == 1)
  {
    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_temp);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "--\370C");
    UG_TextboxSetText(Wm_GetWnd(), main_tb_temp, text);

    text = UG_TextboxGetText(Wm_GetWnd(), main_tb_hum);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "-- %%");
    UG_TextboxSetText(Wm_GetWnd(), main_tb_hum, text);
  }

  g_bRegulation = false;
}

void WndMain_ExitClickCallBack()
{
  Wm_AddNewWindow(WndEdit_GetTemplate());
}
