/*
 * wnd_start.c
 *
 *  Created on: 23. 3. 2018
 *      Author: vlada
 */

#include "windows/wnd_start.h"

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
    { wnd_textbox,  70,  200,    250,   230,    start_tb_date, ALIGN_CENTER, C_YELLOW, &TEXT_NORMAL,        "" },
    { wnd_none,      0,    0,      0,     0,                0,            0,        0,            0,        "" },
};

char g_strVer[] = { __DATE__ };

const wnd_window_t wndStart =
{
    WND_STYLE_2D | WND_STYLE_HIDE_TITLE,
    C_BLACK,
    (wnd_control*)&arrStartControls,
    "",
    WndStart_Callback,
    NULL,
    NULL,
};

wnd_window_t* WndStart_Init()
{
  return (wnd_window_t*) &wndStart;
}

void WndStart_Callback(UG_MESSAGE *msg)
{
  if (msg->event == OBJ_EVENT_PRERENDER && msg->id == OBJ_TYPE_TEXTBOX)
  {
    if (msg->sub_id == start_tb_date)
    {
      UG_TextboxSetText(Wm_GetWnd(), msg->sub_id, g_strVer);
    }
  }
}
