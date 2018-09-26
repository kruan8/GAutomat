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
#include <string.h>

#include "../Icons/light.c"
#include "../Icons/fan.c"
#include "../Icons/heat.c"

// interval mereni
#ifdef DEBUG
  #define REGULATION_LOOP_MS         1000
#else
  #define REGULATION_LOOP_MS        10000
#endif

#define WND_MAIN_ICON_SIZE             64       // velikost ikon pro zobrazeni stavu automatu

#define WND_MAIN_BLINK_COUNT            5       // pocet bliknuti pri neplatne kalibraci
#define WND_MAIN_BLINK_INTERVAL_MS   3000       // interval mezi zablikanim

typedef enum
{
  main_tb_time = 0,
  main_tb_temp,
  main_tb_hum,
}wnd_main_tb_e;

static const wnd_control arrMainControls[] =
{
  //  type       | left | top | right | bott |     id     |  alignment  |  tcolor  | font    |  text
    { wnd_textbox,  70,   30,    250,     85, main_tb_time, ALIGN_CENTER,   C_CYAN, &TEXT_BIG,   "" },
    { wnd_textbox,   5,  110,    154,    155, main_tb_temp, ALIGN_CENTER, C_YELLOW, &TEXT_BIG,   "" },
    { wnd_textbox, 155,  110,    309,    155,  main_tb_hum, ALIGN_CENTER, C_YELLOW, &TEXT_BIG,   "" },
    { wnd_none,      0,    0,      0,      0,            0,            0,        0,         0,   "" },
};

static const wnd_window_t wndMain =
{
    WND_STYLE_2D | WND_STYLE_HIDE_TITLE,
    C_BLACK,
    (wnd_control*)&arrMainControls,
    "",
    WndMain_Init,
    WndMain_Callback,
    WndMain_Timer_1ms,
    WndMain_Exec,
    WndMain_ClickCallBack,
};

const BMPbpp1 BmpLight = { ImgLight, 64, 64, C_YELLOW };
const BMPbpp1 BmpFan = { ImgFan, 64, 64, C_BLUE };
const BMPbpp1 BmpHeat = { ImgHeat, 64, 64, C_RED };

static bool               g_bRegulationResult;           // vysledek regulace (kontrola chyby)
static uint32_t           g_nMeasureTimer;               // odcitani intervalu merici smycky
static bool               g_bRegulation;                 // flag vyprseni intervalu pro mereni
static app_measure_data_t g_lastData;                    // posledni namerena data (pro zamezeni zbytecneho prekreslovani)
static uint32_t           g_nBlinkTimer;                 // timer pro odmerovani intervali mezi zablikanim

static uint32_t           g_nIndicationTimer;            // timer pro indikaci behu zarizeni

wnd_window_t* WndMain_GetTemplate()
{
  return (wnd_window_t*) &wndMain;
}

void WndMain_Init(bool bFirstInit)
{
  g_nMeasureTimer = 0;
  g_nBlinkTimer = 0;
  memset (&g_lastData, 0, sizeof (g_lastData));
  g_bRegulationResult = true;
}

void WndMain_Callback(UG_MESSAGE *msg)
{
  if (msg->event == OBJ_EVENT_PRERENDER && msg->id == OBJ_TYPE_TEXTBOX)
  {
    if (msg->sub_id == main_tb_time)
    {

    }
  }
}

void WndMain_Timer_1ms()
{
  // odmerovani indikace behu
  g_nIndicationTimer++;


  // odmerovani intervalu regulacni smycky
  if (g_nMeasureTimer == 0)
  {
    g_nMeasureTimer = REGULATION_LOOP_MS;
    g_bRegulation = true;
  }

  g_nMeasureTimer--;

  // pokud neni zakalibrovano nebo je chyba mereni, blikej podsvicenim
  if (!AppData_GetLcdCalibrated() | !g_bRegulationResult)
  {
    WM_ResetLedOffTimer();      // zakaz usinani displeje
    if (g_nBlinkTimer == 0)
    {
      g_nBlinkTimer = WND_MAIN_BLINK_INTERVAL_MS;
      WM_SetBlink(WND_MAIN_BLINK_COUNT);
    }
    else
    {
      g_nBlinkTimer--;
    }
  }

}

void WndMain_Exec()
{
  if (g_nIndicationTimer & 0x400)
  {
    g_nIndicationTimer = 0;
    UG_FontSelect(&TEXT_BIG);
    UG_PutChar(' ', 148, 38, C_CYAN, C_BLACK);
  }

  if (g_nIndicationTimer & 0x100)
  {
    UG_FontSelect(&TEXT_BIG);
    UG_PutChar(':', 148, 38, C_CYAN, C_BLACK);
  }

  if (!g_bRegulation)
  {
    return;
  }

  app_measure_data_t data;
  memcpy(&data, &g_lastData, sizeof (app_measure_data_t));
  g_bRegulationResult = App_RegulationLoop(&data);

  // pokud neni zkalibrovany display, vykreslit znacku
  if (!AppData_GetLcdCalibrated())
  {
    UG_FillFrame(0, 0, 319, 20, C_RED);
  }
  else
  {
    UG_FillFrame(0, 0, 319, 20, C_BLACK);
  }

  // vypis casu
  char* text;
  text = UG_TextboxGetText(WM_GetWnd(), main_tb_time);
  snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%02d:%02d", data.nHour, data.nMin);
  UG_TextboxSetText(WM_GetWnd(), main_tb_time, text);

  // vypis teploty a vlhkosti
  if (data.nError != 1)
  {
    if (data.nTemperature != g_lastData.nTemperature)
    {
      text = UG_TextboxGetText(WM_GetWnd(), main_tb_temp);
      snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d\370C", data.nTemperature);
      UG_TextboxSetText(WM_GetWnd(), main_tb_temp, text);
      g_lastData.nTemperature = data.nTemperature;
    }

    if (data.nHumidity != g_lastData.nHumidity)
    {
      text = UG_TextboxGetText(WM_GetWnd(), main_tb_hum);
      snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "%2d %%", data.nHumidity);
      UG_TextboxSetText(WM_GetWnd(), main_tb_hum, text);
      g_lastData.nHumidity = data.nHumidity;
    }
  }
  else if (data.nError == 1)   // chyba teploty
  {
    text = UG_TextboxGetText(WM_GetWnd(), main_tb_temp);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "--\370C");
    UG_TextboxSetText(WM_GetWnd(), main_tb_temp, text);

    text = UG_TextboxGetText(WM_GetWnd(), main_tb_hum);
    snprintf((char*)text, WND_TEXTBOX_TEXT_MAX, "-- %%");
    UG_TextboxSetText(WM_GetWnd(), main_tb_hum, text);
  }

  // vykresleni symbolu LIGHT
  if (data.bLight != g_lastData.bLight)
  {
    if (data.bLight)
    {
      ILI9163_DrawMonochromeBitmap(23, 163, (BMPbpp1*) &BmpLight);
      g_lastData.bLight = true;
    }
    else
    {
      UG_FillFrame(23, 163, 23 + 64, 163 + 64, C_BLACK);
      g_lastData.bLight = false;
    }
  }

  // vykresleni symbolu HEAT
  if (data.bHeat != g_lastData.bHeat)
  {
    if (data.bHeat)
    {
      ILI9163_DrawMonochromeBitmap(126, 163, (BMPbpp1*) &BmpHeat);
      g_lastData.bHeat = true;
    }
    else
    {
      UG_FillFrame(126, 163, 126 + 64, 163 + 64, C_BLACK);
      g_lastData.bHeat = false;
    }
  }

  // vykresleni symbolu FAN
  if (data.bFan != g_lastData.bFan)
  {
    if (data.bFan)
    {
      ILI9163_DrawMonochromeBitmap(223, 163, (BMPbpp1*) &BmpFan);
      g_lastData.bFan = true;
    }
    else
    {
      UG_FillFrame(223, 163, 223 + 64, 163 + 64, C_BLACK);
      g_lastData.bFan = false;
    }
  }

  g_bRegulation = false;
}

void WndMain_ClickCallBack()
{
  if (AppData_GetLcdCalibrated())
  {
    WM_AddNewWindow(WndEdit_GetTemplate());
    WM_CloseWindow();
  }
}
