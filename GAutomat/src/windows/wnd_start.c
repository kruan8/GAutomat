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

#define WND_START_DELAY_MS         2000

typedef enum
{
  start_tb_grow = 0,
  start_tb_automat,
  start_tb_date,
}wnd_start_tb_e;

static const wnd_control arrStartControls[] =
{
  //  type       | left | top | right | bott |     id        |  alignment  |  tcolor  | font       |  text
    { wnd_textbox, 110,   50,    210,   100,    start_tb_grow, ALIGN_CENTER,   C_LIME,    &TEXT_BIG,    "GROW" },
    { wnd_textbox,  75,  110,    250,   155, start_tb_automat, ALIGN_CENTER,   C_LIME,    &TEXT_BIG, "automat" },
    { wnd_textbox,  70,  200,    250,   230,    start_tb_date, ALIGN_CENTER, C_YELLOW, &TEXT_NORMAL,  __DATE__ },
    { wnd_none,      0,    0,      0,     0,                0,            0,        0,            0,        "" },
};

static const wnd_window_t wndStart =
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


static uint32_t g_nStartTime;

wnd_window_t* WndStart_GetTemplate()
{
  return (wnd_window_t*) &wndStart;
}

void WndStart_Init(bool bFirstInit)
{
  if (!AppData_LoadConfig())
  {
    AppData_SetConfigDefault();
  }

  g_nStartTime = Timer_GetTicks_ms();
}

void WndStart_Callback(UG_MESSAGE *msg)
{

}

void WndStart_Timer_1ms()
{
  if (Timer_GetTicks_ms() > (g_nStartTime + WND_START_DELAY_MS))
  {
    if (AppData_GetLcdCalibrated())
    {
      XPT2046_SetCalibrationMatrix(WndCalib_GetCoordinate(), AppData_GetCoordinatePointer());
    }

    WM_AddNewWindow(WndMain_GetTemplate());
    WM_CloseWindow();
  }
}

void WndStart_ClickCallBack()
{
  WM_AddNewWindow(WndCalib_GetTemplate());
  WM_CloseWindow();
}
