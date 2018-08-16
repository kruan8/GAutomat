/*
 * wnd_calib.c
 *
 *  Created on: 15. 8. 2018
 *      Author: vlada
 */

#include "windows/wnd_calib.h"
#include "windows/wnd_main.h"
#include "app_data.h"
#include "timer.h"

typedef enum
{
  calib_tb_text1 = 0,
  calib_tb_disable,
  calib_tb_text2,
}wnd_calib_e;

const wnd_control arrCalibControls[] =
{
  //  type       | left | top | right | bott |     id         |  alignment  | tcolor  | font       |  text
    { wnd_textbox, 100,   70,    240,   100,    calib_tb_text1, ALIGN_CENTER,  C_CYAN, &TEXT_NORMAL,   "KALIBRACE" },
    { wnd_textbox,  90,  110,    250,   140,  calib_tb_disable, ALIGN_CENTER,   C_RED, &TEXT_NORMAL, "je neplatna" },
    { wnd_textbox,  30,  150,    260,   180,    calib_tb_text2, ALIGN_CENTER,  C_CYAN, &TEXT_NORMAL, "Klikni na krizek" },
    { wnd_none,      0,    0,      0,     0,                 0,            0,       0,            0,        "" },
};

const wnd_window_t wndCalib =
{
    WND_STYLE_2D | WND_STYLE_HIDE_TITLE,
    C_BLACK,
    (wnd_control*)&arrCalibControls,
    "",
    WndCalib_Init,
    WndCalib_Callback,
    WndCalib_Timer_1ms,
    WndCalib_Exec,
    WndCalib_ClickCallBack,
};

const Coordinate  g_CrossPos[3] = { {32, 24}, {288, 120}, {160, 216} };

uint8_t g_nClickCounter;        // citac kliknuti kalibrace
Coordinate* g_pCoordinate;      // ulozeni kalibrace
bool        g_bFirstSymbol;     // flag pro vykresleni 1. symbolu


wnd_window_t* WndCalib_GetTemplate()
{
  return (wnd_window_t*) &wndCalib;
}

void WndCalib_Init()
{
  g_bFirstSymbol = 0;
  g_nClickCounter = 0;
  g_pCoordinate = AppData_GetCoordinatePointer();
}

void WndCalib_Exec()
{
  if (!g_bFirstSymbol)
  {
    g_bFirstSymbol = true;
    DrawSymbol();
  }
}

void WndCalib_Callback(UG_MESSAGE *msg)
{

}

void WndCalib_Timer_1ms()
{

}

void WndCalib_ClickCallBack(Coordinate* pScreenCoordinate)
{
  g_pCoordinate[g_nClickCounter].x = pScreenCoordinate->x;
  g_pCoordinate[g_nClickCounter].y = pScreenCoordinate->y;
  RemoveSymbol();
  g_nClickCounter++;
  if (g_nClickCounter == 3)
  {
    AppData_SaveConfig();
    AppData_SetLcdCalibrated(true);
    XPT2046_SetCalibrationMatrix((Coordinate*) &g_CrossPos[0], g_pCoordinate);
    Wm_AddNewWindow(WndMain_GetTemplate());
    Wm_CloseWindow();
  }

  DrawSymbol();
}

void DrawSymbol()
{
  // vykreslit kriz
  DrawCross(g_CrossPos[g_nClickCounter].x,g_CrossPos[g_nClickCounter].y, white, yellow);
}

void RemoveSymbol()
{
  uint16_t xs = g_CrossPos[g_nClickCounter].x - 15;
  uint16_t ys = g_CrossPos[g_nClickCounter].y - 15;
  uint16_t xe = g_CrossPos[g_nClickCounter].x + 16;
  uint16_t ye = g_CrossPos[g_nClickCounter].y + 16;

  UG_FillFrame(xs, ys, xe, ye, C_BLACK);
}

void DrawCross(uint16_t x, uint16_t y, uint16_t colorIn, uint16_t colorOut)
{
  UG_DrawLine(x - 15, y, x - 2, y, colorIn);
  UG_DrawLine(x + 2, y, x + 15, y, colorIn);
  UG_DrawLine(x, y - 15, x, y - 2, colorIn);
  UG_DrawLine(x, y + 2, x, y + 15, colorIn);

  UG_DrawLine(x - 15, y + 15, x - 7, y + 15, colorOut);
  UG_DrawLine(x - 15, y + 7, x - 15, y + 15, colorOut);

  UG_DrawLine(x - 15, y - 15, x - 7, y - 15, colorOut);
  UG_DrawLine(x - 15, y - 7, x - 15, y - 15, colorOut);

  UG_DrawLine(x + 7, y + 15, x + 15, y + 15, colorOut);
  UG_DrawLine(x + 15, y + 7, x + 15, y + 15, colorOut);

  UG_DrawLine(x + 7, y - 15, x + 15, y - 15, colorOut);
  UG_DrawLine(x + 15, y - 15, x + 15, y - 7, colorOut);
}
