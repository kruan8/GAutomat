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

void Wnd_SetTextboxFromInt(UG_WINDOW* pWnd, uint8_t nId, uint32_t nValue)
{
  char * pText = UG_TextboxGetText(pWnd, nId);
  snprintf(pText, WND_BUTTON_TEXT_MAX, "%lu", nValue);
  UG_TextboxSetText(pWnd, nId, pText);
}
