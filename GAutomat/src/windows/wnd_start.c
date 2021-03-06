/*
 * wnd_start.c
 *
 *  Created on: 23. 3. 2018
 *      Author: Priesol Vladimir
 *
 *      START okno:
 *      - vypise nazev programu
 *      - zmeri zobrazi napeti baterie,
 *      - ceka 3 sekundy na click, jesli ma prejit do kalibrace LCD,
 *      jinak prejde do MAIN window
 *      - animuje prouzek pri odecitani casu
 *
 */

#include "windows/wnd_start.h"
#include "windows/wnd_calib.h"
#include "windows/wnd_main.h"
#include "app_data.h"
#include "timer.h"
#include "adc.h"
#include <stdio.h>

#define WND_START_DELAY_MS         3000
#define WND_START_X                  10
#define WND_START_Y                 175

typedef enum
{
  start_tb_grow = 0,
  start_tb_automat,
  start_tb_date,
  start_tb_vbat,
}wnd_start_tb_e;

static const wnd_control arrStartControls[] =
{
  //  type       | left | top | right | bott |     id        |  alignment  |  tcolor  | font       |  text
    { wnd_textbox, 110,   50,    210,   100,    start_tb_grow, ALIGN_CENTER,   C_LIME,    &TEXT_BIG,    "GROW" },
    { wnd_textbox,  75,  110,    250,   155, start_tb_automat, ALIGN_CENTER,   C_LIME,    &TEXT_BIG, "automat" },
    { wnd_textbox,  70,  200,    250,   230,    start_tb_date, ALIGN_CENTER, C_YELLOW, &TEXT_NORMAL,  __DATE__ },
    { wnd_textbox, 160,    5,    310,    35,    start_tb_vbat, ALIGN_H_RIGHT, C_CYAN, &TEXT_NORMAL,     NULL },
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
    WndStart_Exec,
    WndStart_ClickCallBack,
};


static uint32_t g_nStartTime;     // ticks vytvoreni okna
char            g_strVBAT[12];    // pamet pro ulozeni stringu napeti baterie

bool            g_bDrawPixel;     // flag povoleni vykresleni dalsi sloupce prouzku
uint16_t        g_nCounterColumn; // citac casu pro vykresleni dalsi sloupce prouzku
uint16_t        g_nPixelX;        // index sloupce prouzku

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

  // measure and display VBAT
  Adc_Init();
  uint16_t mVBAT = Adc_ReadVBAT_mV();
  mVBAT = (mVBAT + 50) / 100;         // zaokrouhleni na desetiny
  snprintf(g_strVBAT, sizeof (g_strVBAT), "BAT: %d,%.1dV", mVBAT / 10, mVBAT % 10);
  UG_TextboxSetText(WM_GetWnd(), start_tb_vbat, g_strVBAT);

  g_nCounterColumn = 0;
  g_nPixelX = 0;
  g_bDrawPixel = false;

  g_nStartTime = Timer_GetTicks_ms();
}

void WndStart_Callback(UG_MESSAGE *msg)
{
}

void WndStart_Exec()
{
  // vykresli dalsi pixel v animovanem prouzku
  if (g_bDrawPixel)
  {
    g_bDrawPixel = false;
    UG_DrawPixel(WND_START_X + g_nPixelX, WND_START_Y, C_CYAN);
    UG_DrawPixel(WND_START_X + g_nPixelX, WND_START_Y + 1, C_CYAN);
    UG_DrawPixel(WND_START_X + g_nPixelX, WND_START_Y + 2, C_CYAN);
    g_nPixelX++;
  }
}

void WndStart_Timer_1ms()
{
  // je cas vykreslit dalsi pixel?
  g_nCounterColumn++;
  if (g_nCounterColumn == 10)
  {
    g_nCounterColumn = 0;
    g_bDrawPixel = true;
  }

  if (Timer_GetTicks_ms() > (g_nStartTime + WND_START_DELAY_MS))
  {
    if (AppData_GetLcdCalibrated())
    {
      XPT2046_SetCalibrationMatrix(WndCalib_GetCoordinate(), AppData_GetCoordinatePointer());
    }

    Adc_DeInit();
    WM_AddNewWindow(WndMain_GetTemplate());
    WM_CloseWindow();
  }
}

void WndStart_ClickCallBack()
{
  Adc_DeInit();
  WM_AddNewWindow(WndCalib_GetTemplate());
  WM_CloseWindow();
}
