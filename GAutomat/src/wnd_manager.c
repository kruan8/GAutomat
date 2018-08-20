/*
 * wnd_manager.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "wnd_manager.h"
#include <string.h>
#include "timer.h"

#define WND_WINDOWS_MAX   10

#define APP_LED_INTERVAL_MS    60000

UG_GUI gui; // Global GUI structure

UG_WINDOW*        g_pActualWindow;

PtrExeCallback pExeCallback = 0;
PtrSysTickCallback pSysTickCb = 0;

wnd_window_t*    g_arrWindowStack[WND_WINDOWS_MAX];
uint8_t          g_nWindowPos = 0;

wnd_window_t*    g_pNewWindow = NULL;
bool             g_bClose;                  // zadost o zavreni okna
bool             g_bEndClick;
uint32_t         g_nLedOffCounter;


static bool Wm_CreateWindow(wnd_window_t* pWndTemplate, bool bFirstInit);
static void Wm_SysTickCallback();

void Wm_Init()
{
  UG_Init(&gui, ILI9163_PixelSetRGB565, 320, 240);
  UG_FontSetHSpace(0);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*)ILI9163_FillFrame);

#ifdef DMA_ENABLE
  ILI9163_InitFillArea((TEXT_MAX).char_width * (TEXT_MAX).char_height);
  UG_DriverRegister(DRIVER_FILL_AREA, (void*)ILI9163_FillArea);
#endif

  g_bClose = false;
  g_bEndClick = false;

  g_nLedOffCounter = APP_LED_INTERVAL_MS;

  Timer_SetSysTickCallback(Wm_SysTickCallback);
}

bool Wm_Exec()
{
  wnd_window_t* pWnd = NULL;

  while (true)
  {
    bool bFirstInit = false;
    if (g_pNewWindow)
    {
      g_nWindowPos++;
      if (g_nWindowPos == WND_WINDOWS_MAX)
      {
        g_nWindowPos--;
        return true;
      }

      g_arrWindowStack[g_nWindowPos] = g_pNewWindow;
      pWnd = g_pNewWindow;
      g_pNewWindow = NULL;
      bFirstInit = true;
    }
    else
    {
      if (g_nWindowPos == 0)
      {
        return false;
      }

      g_nWindowPos--;
      pWnd = g_arrWindowStack[g_nWindowPos];
    }

    if (pWnd)
    {
      Wm_CreateWindow(pWnd, bFirstInit);
    }
  }

  return true;
}

static bool Wm_CreateWindow(wnd_window_t* pWndTemplate, bool bFirstInit)
{
  UG_WINDOW window;
  g_bClose = false;

  // alokace pro vsechny ovladaci prvky
  uint8_t nControls = 0;          // pocet controls
  uint8_t nTextboxCount = 0;
  uint8_t nButtonCount = 0;
  while (true)
  {
    wnd_control* pCtrl = (wnd_control*)(pWndTemplate->pControls + nControls);
    if (pCtrl->eType == wnd_none)
    {
      break;
    }
    else if (pCtrl->eType == wnd_textbox)
    {
      nTextboxCount++;
    }
    else if (pCtrl->eType == wnd_button)
    {
      nButtonCount++;
    }

    nControls++;
  }

  UG_OBJECT objBuff[nControls];
  UG_TEXTBOX tb[nTextboxCount];
  UG_BUTTON bt[nButtonCount];

  char txtText[nTextboxCount][WND_TEXTBOX_TEXT_MAX + 1];  // prostor pro text textboxu (10 znaku/textbox)
  char btnText[nButtonCount][WND_BUTTON_TEXT_MAX + 1];  // prostor pro text buttonu (10 znaku/button)

  g_pActualWindow = &window;
  UG_WindowCreate(&window, objBuff, nControls, pWndTemplate->EventCallBack);
  UG_WindowSetStyle(&window, pWndTemplate->nStyle);
  UG_WindowSetBackColor(&window, pWndTemplate->nBackColor);

  if (pWndTemplate->nStyle & WND_STYLE_SHOW_TITLE)
  {
    UG_WindowSetTitleText (&window, pWndTemplate->pCaption);
    UG_WindowSetTitleTextFont (&window, &TEXT_NORMAL);
    UG_WindowSetTitleTextAlignment(&window, ALIGN_CENTER);
  }

  pExeCallback = pWndTemplate->ExecCallBack;

  pSysTickCb = pWndTemplate->TimerCallBack;

  uint8_t nTbIndex = 0;
  uint8_t nBtIndex = 0;
  for (uint8_t i = 0; i < nControls; ++i)
  {
    wnd_control* pCtrl = (wnd_control*)(pWndTemplate->pControls + i);
    if (pCtrl->eType == wnd_textbox)
    {
      Wnd_CreateTextBox(&window, pCtrl, &tb[nTbIndex]);
      if (pCtrl->pText)
      {
        strncpy(txtText[nTbIndex], pCtrl->pText, WND_TEXTBOX_TEXT_MAX);
      }

      UG_TextboxSetText(&window, pCtrl->id, txtText[nTbIndex++]);
    }

    if (pCtrl->eType == wnd_button)
    {
      Wnd_CreateButton(&window, pCtrl, &bt[nBtIndex]);
      if (pCtrl->pText)
      {
        strncpy(btnText[nBtIndex], pCtrl->pText, WND_BUTTON_TEXT_MAX);
      }

      UG_ButtonSetText(&window, pCtrl->id, btnText[nBtIndex++]);
    }
  }

  // call window INIT function
  if (pWndTemplate->Init)
  {
    pWndTemplate->Init(bFirstInit);
  }

  UG_WindowShow(&window);
  UG_Update();

  // message loop
  while(!g_bClose)
  {
    Coordinate* pCoo = Read_XPT2046();
    if (pCoo)
    {
      if (g_nLedOffCounter == 0)
      {
        // pri vypnutem displeji rozsvitit a nezpracovavat kliknuti
        ILI9163_LedOn();
        g_nLedOffCounter = APP_LED_INTERVAL_MS;
        continue;
      }

      // pri kliknuti nastavit casovac vypnuti LED podsviceni
      g_nLedOffCounter = APP_LED_INTERVAL_MS;

      if (pWndTemplate->ClickCallBack)
      {
        pWndTemplate->ClickCallBack();
      }

      Coordinate Display;
      getDisplayPoint(&Display, pCoo);
      UG_TouchUpdate (Display.x, Display.y, TOUCH_STATE_PRESSED);
    }
    else
    {
      UG_TouchUpdate (-1, -1, TOUCH_STATE_RELEASED);
    }

    // call window EXEC function
    if (pExeCallback)
    {
      pExeCallback();
    }

    UG_Update();
    IWDG_ReloadCounter();
  }

  return true;
}

UG_WINDOW* Wm_GetWnd()
{
  return g_pActualWindow;
}

bool Wm_AddNewWindow(wnd_window_t* pWndTemplate)
{
  g_pNewWindow = pWndTemplate;
  return true;
}

void Wm_CloseWindow()
{
  g_bClose = true;
}

static void Wm_SysTickCallback()
{
  if (pSysTickCb)
  {
    pSysTickCb();
  }

  if (g_nLedOffCounter)
  {
    g_nLedOffCounter--;
  }
  else
  {
    ILI9163_LedOff();
  }
}



