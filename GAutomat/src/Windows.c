/*
 * Windows.c
 *
 *  Created on: 10. 8. 2017
 *      Author: priesolv
 */


#include "Windows.h"
#include "XPT2046.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_OBJECTS 10

#define WND_EDIT_OBJECTS  11

UG_GUI gui; // Global GUI structure

bool g_bExit = false;
bool g_bWndResult;
wnd_main_data_t g_lastData;

Coordinate  g_CrossPosition[3] = { {30, 45}, {290, 45}, {160, 210} };

char g_strVersion[] = { __DATE__ };

UG_WINDOW*       g_pActiveWindow;

void Wnd_Init()
{
  UG_Init(&gui, ILI9163_PixelSetRGB565, 320, 240);
  UG_FontSetHSpace(0);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*)ILI9163_FillFrame);

#ifdef DMA_ENABLE
  ILI9163_InitFillArea((TEXT_MAX).char_width * (TEXT_MAX).char_height);
  UG_DriverRegister(DRIVER_FILL_AREA, (void*)ILI9163_FillArea);
#endif

}

void WindowMain(wnd_main_data_t* data)
{
  memcpy(&g_lastData, data, sizeof (wnd_main_data_t));

  UG_FillScreen(C_BLACK);

  // print time
  UG_FontSelect(&TEXT_BIG);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_CYAN);

  char text[50];
  snprintf((char*)text, sizeof (text), "%02d:%02d", data->nHour, data->nMin);
  UG_PutString(100, 25, text);

  // print temperature and humidity
  UG_FontSelect(&TEXT_BIG);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_YELLOW);

  if (data->nError != 1)  // chyba teploty
  {
    snprintf((char*)text, sizeof (text), "%2d\370C", data->nTemperature);
    UG_PutString(32, 90, text);

    snprintf((char*)text, sizeof (text), "%2d %%", data->nHumidity);
    UG_PutString(204, 95, text);
  }
  else if (data->nError == 1)
  {
    snprintf((char*)text, sizeof (text), "--\370C");
    UG_PutString(32, 90, text);

    snprintf((char*)text, sizeof (text), "-- %%");
    UG_PutString(204, 95, text);
  }


  if (data->bLight)
  {
    DrawSun(55, 195, yellow);
  }

  if (data->bHeat)
  {
    DrawHeat(158, 195, red);
  }

  if (data->bFan)
  {
    DrawFan(255, 195, blue);
  }

}

void WindowMain_Callback(UG_MESSAGE *msg)
{
  if (msg->type == MSG_TYPE_OBJECT)
  {
    if (msg->id == OBJ_TYPE_BUTTON)
    {
      switch (msg->sub_id)
      {
      case BTN_ID_5:
//        g_bExit = true;
        break;

      }
    }
  }
}

bool Wnd_CreateWindow(wnd_window_t* pWindowTemplate, uint8_t nControls)
{
  UG_WINDOW winEdit;
  UG_WINDOW*  oldWin = g_pActiveWindow;
  g_pActiveWindow = &winEdit;
  g_bExit = false;

  // alokace pro vsechny ovladaci prvky
  UG_OBJECT objBuffWndEdit[nControls];

  uint8_t nTextboxCount = 0;
  uint8_t nButtonCount = 0;

  // vypocet prostoru pro controls dat
  for (uint8_t i = 0; i < nControls; ++i)
  {
    wnd_control* pCtrl = (wnd_control*)(pWindowTemplate->pControls + i);
    if (pCtrl->eType == wnd_textbox)
    {
      nTextboxCount++;
    }

    if (pCtrl->eType == wnd_button)
    {
      nButtonCount++;
    }
  }

  UG_TEXTBOX tb[nTextboxCount];
  UG_BUTTON bt[nButtonCount];
  char btnText[nButtonCount][WND_BUTTON_TEXT_MAX];  // prostor pro text buttonu (10 znaku/button)

  UG_WindowCreate(&winEdit, objBuffWndEdit, nControls, pWindowTemplate->CallBack);
  UG_WindowSetTitleText (&winEdit, pWindowTemplate->pCaption);
  UG_WindowSetTitleTextFont (&winEdit, &TEXT_NORMAL);
  UG_WindowSetTitleTextAlignment(&winEdit, ALIGN_CENTER);

  uint8_t nTbIndex = 0;
  uint8_t nBtIndex = 0;
  for (uint8_t i = 0; i < nControls; ++i)
  {
    wnd_control* pCtrl = (wnd_control*)(pWindowTemplate->pControls + i);
    if (pCtrl->eType == wnd_textbox)
    {
      Wnd_CreateTextBox(&winEdit, pCtrl, &tb[nTbIndex++]);
    }

    if (pCtrl->eType == wnd_button)
    {
      Wnd_CreateButton(&winEdit, pCtrl, &bt[nBtIndex]);
      if (pCtrl->pText)
      {
        strncpy(btnText[nBtIndex], pCtrl->pText, WND_BUTTON_TEXT_MAX);
      }

      UG_ButtonSetText(&winEdit, pCtrl->id, btnText[nBtIndex++]);
    }
  }

  UG_WindowShow(&winEdit);

  while(!g_bExit)
  {
    if (XPT2046_Press())
    {
      Coordinate Display;
      Coordinate* pCoo = Read_XPT2046();
      getDisplayPoint(&Display, pCoo);
      UG_TouchUpdate (Display.x, Display.y, TOUCH_STATE_PRESSED);
      while (XPT2046_Press());  // cekat na uvolneni
    }
    else
    {
      UG_TouchUpdate (-1, -1, TOUCH_STATE_RELEASED);
    }

    UG_Update();
  }



  g_bExit = false;
  g_pActiveWindow = oldWin;
  return g_bWndResult;
}

void Wnd_SetResult(bool bResult)
{
  g_bWndResult = bResult;
}

void Wnd_Exit()
{
  g_bExit = true;
}

UG_WINDOW* Wnd_GetWindow()
{
  return g_pActiveWindow;
}

void WndStart()
{
  // splash screen
  UG_FillScreen(C_BLACK);
  UG_FontSelect(&TEXT_BIG);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_LIME);
  UG_PutString(110, 50, "GROW");
  UG_PutString(75, 110, "automat");

  UG_FontSelect(&TEXT_NORMAL);
  UG_SetForecolor(C_YELLOW);
  UG_PutString(90, 215, g_strVersion);
}

bool Wnd_Calibrate(Coordinate* pScreenSamples, bool bEnableMessage)
{
  Coordinate * pCoo;

  UG_FontSelect(&TEXT_NORMAL);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_CYAN);

  for(uint8_t i = 0; i < 3; i++)
  {
    UG_FillScreen(C_BLACK);
    UG_PutString(100, 80, "KALIBRACE");

    if (bEnableMessage)
    {
      UG_SetForecolor(C_RED);
      UG_PutString(90, 110, "je neplatna");
      UG_SetForecolor(C_CYAN);
    }

    UG_PutString(60, 150, "Klikni na krizek");

    // vykreslit kriz
    DrawCross(g_CrossPosition[i].x,g_CrossPosition[i].y, white, yellow);
    pCoo = 0;
    while (!pCoo)
    {
      pCoo = Read_XPT2046();
    }

    // Todo: cekat na uvolneni - nemelo by tu spise byt 'press' ?
    while (Read_XPT2046());

    pScreenSamples[i].x = pCoo->x;
    pScreenSamples[i].y = pCoo->y;
  }

  XPT2046_SetCalibrationMatrix(&g_CrossPosition[0], pScreenSamples);
  UG_FillScreen(C_BLACK);
  return true;
}

void Wnd_SetCalibration(Coordinate* pScreenSamples)
{
  XPT2046_SetCalibrationMatrix(&g_CrossPosition[0], pScreenSamples);
}

void Wnd_CreateTextBox(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_TEXTBOX* pTextbox)
{
  UG_TextboxCreate(pWnd, pTextbox, pCtrl->id, pCtrl->left, pCtrl->top, pCtrl->right, pCtrl->bottom);
  UG_TextboxSetText(pWnd, pCtrl->id, (char*)pCtrl->pText);
  UG_TextboxSetFont(pWnd, pCtrl->id, pCtrl->font);
  UG_TextboxSetAlignment(pWnd, pCtrl->id, pCtrl->alignment);
  UG_TextboxSetForeColor(pWnd, pCtrl->id, pCtrl->text_color);
}

void Wnd_CreateButton(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_BUTTON* pButton)
{
  UG_ButtonCreate(pWnd, pButton, pCtrl->id, pCtrl->left, pCtrl->top, pCtrl->right, pCtrl->bottom);
  UG_ButtonSetFont(pWnd, pCtrl->id, pCtrl->font);
  UG_ButtonSetForeColor(pWnd, pCtrl->id, pCtrl->text_color);
}

void Wnd_SetButtonTextFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue)
{
  char* pText = UG_ButtonGetText(pWnd, nId);
  snprintf(pText, WND_BUTTON_TEXT_MAX, "%lu", nValue);
  UG_ButtonSetText(pWnd, nId, pText);
}

void Wnd_SetTextboxFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue)
{
  char * pText = UG_TextboxGetText(Wnd_GetWindow(), nId);
  snprintf(pText, WND_BUTTON_TEXT_MAX, "%lu", nValue);
  UG_TextboxSetText(Wnd_GetWindow(), nId, pText);
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

void DrawSun(uint16_t x, uint16_t y, uint16_t color)
{
  uint8_t nRadius = 15;
  uint8_t nRayLength = 10;
  uint8_t nSpace = 5;

  UG_FillCircle(x, y, nRadius, color);

  UG_DrawLine(x - nRadius - nSpace, y, x - nRadius - nSpace - nRayLength, y, color);
  UG_DrawLine(x + nRadius + nSpace, y, x + nRadius + nSpace + nRayLength, y, color);

  UG_DrawLine(x, y - nRadius - nSpace, x, y - nRadius - nSpace - nRayLength, color);
  UG_DrawLine(x, y + nRadius + nSpace, x, y + nRadius + nSpace + nRayLength, color);

}

void DrawHeat(uint16_t x, uint16_t y, uint16_t color)
{
  uint8_t nHalfSize = 30;
  UG_FillFrame(x - nHalfSize, y - nHalfSize, x + nHalfSize, y + nHalfSize, color);
}

void DrawFan(uint16_t x, uint16_t y, uint16_t color)
{
  uint8_t nHalfSize = 30;
  UG_FillFrame(x - nHalfSize, y - nHalfSize, x + nHalfSize, y + nHalfSize, color);
}

