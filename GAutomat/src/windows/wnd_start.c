/*
 * wnd_start.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "windows/wnd_start.h"
#include "windows/wnd_calib.h"
#include "windows/wnd_main.h"
#include "app_data.h"
#include "timer.h"

typedef enum
{
  start_tb_grow = 0,
  start_tb_automat,
  start_tb_date,
}wnd_start_tb_e;

const wnd_control arrStartControls[] =
{
  //  type       | left | top | right | bott |     id        |  alignment  |  tcolor  | font       |  text
    { wnd_textbox, 110,   50,    210,   100,    start_tb_grow, ALIGN_CENTER,   C_LIME,    &TEXT_BIG,    "GROW" },
    { wnd_textbox,  75,  110,    250,   155, start_tb_automat, ALIGN_CENTER,   C_LIME,    &TEXT_BIG, "automat" },
    { wnd_textbox,  70,  200,    250,   230,    start_tb_date, ALIGN_CENTER, C_YELLOW, &TEXT_NORMAL,  __DATE__ },
    { wnd_none,      0,    0,      0,     0,                0,            0,        0,            0,        "" },
};

const char g_strVer[] = { __DATE__ };

const wnd_window_t wndStart =
{
    WND_STYLE_2D | WND_STYLE_HIDE_TITLE,
    C_BLACK,
    (wnd_control*)&arrStartControls,
    "",
    WndStart_Init,
    WndStart_Callback,
    WndStart_Timer_1ms,
    NULL,
    WndStart_ClickCallBack,
};

#define WND_START_DELAY_MS         2000

uint32_t g_nStartTime;

wnd_window_t* WndStart_GetTemplate()
{
  return (wnd_window_t*) &wndStart;
}

void WndStart_Init()
{
  if (!AppData_LoadConfig())
  {
    AppData_SetConfigDefault();
  }

  g_nStartTime = Timer_GetTicks_ms();
}

void WndStart_Callback(UG_MESSAGE *msg)
{
  if (msg->event == OBJ_EVENT_PRERENDER && msg->id == OBJ_TYPE_TEXTBOX)
  {
    if (msg->sub_id == start_tb_date)
    {

    }
  }
}

void WndStart_Timer_1ms()
{
  if (Timer_GetTicks_ms() > (g_nStartTime + WND_START_DELAY_MS))
  {
    Wm_AddNewWindow(WndMain_GetTemplate());
    Wm_CloseWindow();
  }
}

void WndStart_ClickCallBack()
{
  AppData_SetCalibrateRequest(true);
  Wm_AddNewWindow(WndCalib_GetTemplate());
  Wm_CloseWindow();
}
