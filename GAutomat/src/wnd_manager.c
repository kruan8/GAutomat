/*
 * wnd_manager.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "wnd_manager.h"
#include "timer.h"
#include "XPT2046.h"
#include <string.h>
#include <stdio.h>


#define WND_WINDOWS_MAX       10
#define APP_LED_INTERVAL_MS   60000

#define X_LCD_SIZE            320
#define Y_LCD_SIZE            240

#define BLINK_INTERVAL        100;

static UG_GUI gui; // Global GUI structure

static UG_WINDOW*        g_pActualWindow;

static PtrExeCallback pExeCallback = 0;
static PtrSysTickCallback pSysTickCb = 0;

static wnd_window_t*    g_arrWindowStack[WND_WINDOWS_MAX];
static uint8_t          g_nWindowPos = 0;

static wnd_window_t*    g_pNewWindow = NULL;
static bool             g_bClose;                  // zadost o zavreni okna
static bool             g_bEndClick;
static uint32_t         g_nLedOffTimer;
static uint32_t         g_nBlinkCounter;
static uint32_t         g_nBlinkTimer;
static bool             g_bBlinkLight;


static bool WM_CreateWindow(wnd_window_t* pWndTemplate, bool bFirstInit);
static void WM_SysTickCallback();

void WM_CreateTextBox(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_TEXTBOX* pTextbox);
void WM_CreateButton(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_BUTTON* pButton);


void WM_Init()
{
  UG_Init(&gui, ILI9163_PixelSetRGB565, ILI9163_GetResolutionX(), ILI9163_GetResolutionY());
  UG_FontSetHSpace(0);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*)ILI9163_FillFrame);

#ifdef DMA_ENABLE
  ILI9163_InitFillArea((TEXT_MAX).char_width * (TEXT_MAX).char_height);
  UG_DriverRegister(DRIVER_FILL_AREA, (void*)ILI9163_FillArea);
#endif

  g_bClose = false;
  g_bEndClick = false;
  g_bBlinkLight = true;         // blikani musi skoncit ve stavu ON

  g_nLedOffTimer = APP_LED_INTERVAL_MS;

  Timer_SetSysTickCallback(WM_SysTickCallback);
}

bool WM_Exec()
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
      WM_CreateWindow(pWnd, bFirstInit);
    }
  }

  return true;
}

static bool WM_CreateWindow(wnd_window_t* pWndTemplate, bool bFirstInit)
{
  UG_WINDOW window;
  g_bClose = false;

  // smazani artefaktu okolo okna
  UG_FillFrame(0, 0, X_LCD_SIZE, Y_LCD_SIZE, C_BLACK);

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
      WM_CreateTextBox(&window, pCtrl, &tb[nTbIndex]);
      if (pCtrl->pText)
      {
        strncpy(txtText[nTbIndex], pCtrl->pText, WND_TEXTBOX_TEXT_MAX);
      }

      UG_TextboxSetText(&window, pCtrl->id, txtText[nTbIndex++]);
    }

    if (pCtrl->eType == wnd_button)
    {
      WM_CreateButton(&window, pCtrl, &bt[nBtIndex]);
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
      if (g_nLedOffTimer == 0)
      {
        // pri vypnutem displeji rozsvitit a nezpracovavat kliknuti
        ILI9163_LedOn(true);
        g_nLedOffTimer = APP_LED_INTERVAL_MS;
        continue;
      }

      // pri kliknuti nastavit casovac vypnuti LED podsviceni
      g_nLedOffTimer = APP_LED_INTERVAL_MS;

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

UG_WINDOW* WM_GetWnd()
{
  return g_pActualWindow;
}

bool WM_AddNewWindow(wnd_window_t* pWndTemplate)
{
  g_pNewWindow = pWndTemplate;
  return true;
}

void WM_CloseWindow()
{
  g_bClose = true;
}

static void WM_SysTickCallback()
{
  if (pSysTickCb)
  {
    pSysTickCb();
  }

  if (g_nLedOffTimer)
  {
    g_nLedOffTimer--;
  }
  else
  {
    ILI9163_LedOn(false);
  }

  // blikani podsvicenim displeje pri neplatne konfiguraci
  if (g_nBlinkCounter)
  {
    if (g_nBlinkTimer)
    {
      g_nBlinkTimer--;
    }
    else
    {
      g_bBlinkLight = !g_bBlinkLight;
      g_nBlinkTimer = BLINK_INTERVAL;
      ILI9163_LedOn(g_bBlinkLight);
      if (g_bBlinkLight)
      {
        g_nBlinkCounter--;
      }
    }
  }

}

void WM_ResetLedOffTimer()
{
  g_nLedOffTimer = APP_LED_INTERVAL_MS;
}

void WM_SetBlink(uint8_t nBlinkCount)
{
  g_nBlinkCounter = nBlinkCount;
  g_nBlinkTimer = BLINK_INTERVAL;
  g_bBlinkLight = true;
  ILI9163_LedOn(g_bBlinkLight);
}

void WM_CreateTextBox(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_TEXTBOX* pTextbox)
{
  UG_TextboxCreate(pWnd, pTextbox, pCtrl->id, pCtrl->left, pCtrl->top, pCtrl->right, pCtrl->bottom);
  UG_TextboxSetText(pWnd, pCtrl->id, (char*)pCtrl->pText);
  UG_TextboxSetFont(pWnd, pCtrl->id, pCtrl->font);
  UG_TextboxSetAlignment(pWnd, pCtrl->id, pCtrl->alignment);
  UG_TextboxSetForeColor(pWnd, pCtrl->id, pCtrl->text_color);
}

void WM_CreateButton(UG_WINDOW* pWnd, wnd_control* pCtrl, UG_BUTTON* pButton)
{
  UG_ButtonCreate(pWnd, pButton, pCtrl->id, pCtrl->left, pCtrl->top, pCtrl->right, pCtrl->bottom);
  UG_ButtonSetFont(pWnd, pCtrl->id, pCtrl->font);
  UG_ButtonSetForeColor(pWnd, pCtrl->id, pCtrl->text_color);
}

void WM_SetButtonTextFormInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue)
{
  char* pText = UG_ButtonGetText(pWnd, nId);
  snprintf(pText, WND_BUTTON_TEXT_MAX, "%lu", nValue);
  UG_ButtonSetText(pWnd, nId, pText);
}

void WM_SetTextboxFromInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue)
{
  char * pText = UG_TextboxGetText(pWnd, nId);
  snprintf(pText, WND_BUTTON_TEXT_MAX, "%lu", nValue);
  UG_TextboxSetText(pWnd, nId, pText);
}


