/*
 * ILI9163.c
 * code for STM32F0
 */

/*
 *		Zapojeni vyvodu pro displej:
 *
 *    SCK:    SPI1-SCK:  PB3  - (SPI clk)
 *    SDA:    SPI1-MOSI: PB5  - (SPI data)
 *
 *		A0:	    PB6 - (command/data)
 *		CS:     PB9 - (CS display)
 *		RESET:  PB7 - (reset)
 *		LED:    PC12 - (backlight LED)
 */

// vytvoreno podle:
// https://github.com/sumotoy/TFT_ILI9163C

#include "ILI9163.h"
#include "font.h"
#include "SPI1.h"
#include "timer.h"
#include "common.h"
#include <stdlib.h>

#define ILI9163_A0                         PB6
#define ILI9163_A0_PIN                     GPIO_Pin_6
#define ILI9163_A0_GPIO_PORT               GPIOB

#define ILI9163_CS                         PB9
#define ILI9163_CS_PIN                     GPIO_Pin_9
#define ILI9163_CS_GPIO_PORT               GPIOB

#define ILI9163_RST                        PB7
#define ILI9163_RST_PIN                    GPIO_Pin_7
#define ILI9163_RST_GPIO_PORT              GPIOB

#define ILI9163_LED                        PC12
#define ILI9163_LED_PIN                    GPIO_Pin_12
#define ILI9163_LED_GPIO_PORT              GPIOC


#define ILI9163_RST_HIGH					(ILI9163_RST_GPIO_PORT->BSRRL = ILI9163_RST_PIN)
#define ILI9163_RST_LOW			 			(ILI9163_RST_GPIO_PORT->BSRRH = ILI9163_RST_PIN)

#define ILI9163_CS_ENABLE					(ILI9163_CS_GPIO_PORT->BSRRH = ILI9163_CS_PIN)  // pin reset
#define ILI9163_CS_DISABLE		 		(ILI9163_CS_GPIO_PORT->BSRRL = ILI9163_CS_PIN)  // pin set

#define ILI9163_LED_ON						(ILI9163_LED_GPIO_PORT->BSRRL = ILI9163_LED_PIN)
#define ILI9163_LED_OFF			 			(ILI9163_LED_GPIO_PORT->BSRRH = ILI9163_LED_PIN)

#define ILI9163_A0_DATA						(ILI9163_A0_GPIO_PORT->BSRRL = ILI9163_A0_PIN)
#define ILI9163_A0_REG			 			(ILI9163_A0_GPIO_PORT->BSRRH = ILI9163_A0_PIN)

#define ILI9163_SPI_PRESCALER     spi_br_4 // SPI1 bezi na APB2 (tzn. 100MHz), tzn. :4 = 25Mhz

#define DMA_STREAM                DMA2_Stream5

#define BUFFER_LEN                10

// chyba spatneho zadratovani displeje (128x160)
// je treba posunout zobrazovanou pamet o 32 radku nize
// s timto je potreba pocitat i pri aplikaci vertikalniho rolovani

TextParam params;

// akcelerace uGUI - FILL AREA DRIVER
// void(*g_push_pixel)(uint16_t);
uint16_t *g_pMemFillArea;
uint16_t g_nPixelCount;
uint16_t g_nPixelCounter;
uint16_t g_nCharPosX;
uint16_t g_nCharPosY;
uint16_t g_nCharEndX;
uint16_t g_nCharEndY;

uint16_t g_arrBuffer[BUFFER_LEN];

void ILI9163_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

  GPIO_ClockEnable(GPIO_GetPort(ILI9163_A0));
	GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(ILI9163_A0);
	GPIO_Init(GPIO_GetPort(ILI9163_A0), &GPIO_InitStructure);

	GPIO_ClockEnable(GPIO_GetPort(ILI9163_CS));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(ILI9163_CS);
  GPIO_Init(GPIO_GetPort(ILI9163_CS), &GPIO_InitStructure);

  GPIO_ClockEnable(GPIO_GetPort(ILI9163_RST));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(ILI9163_RST);
  GPIO_Init(GPIO_GetPort(ILI9163_RST), &GPIO_InitStructure);

  GPIO_ClockEnable(GPIO_GetPort(ILI9163_LED));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(ILI9163_LED);
  GPIO_Init(GPIO_GetPort(ILI9163_LED), &GPIO_InitStructure);

	ILI9163_CS_DISABLE;

	// display reset
  ILI9163_RST_HIGH;
  Timer_Delay_ms(20);
	ILI9163_RST_LOW;
	Timer_Delay_ms(20);
	ILI9163_RST_HIGH;
	Timer_Delay_ms(20);

	SPI1_SetPrescaler(ILI9163_SPI_PRESCALER);

	// driver configuration
	ILI9163_WriteToReg(0x01); //Software reset
	ILI9163_WriteToReg(0x11); //Exit Sleep
	Timer_Delay_ms(20);

	ILI9163_WriteToReg(0x26); //Set default gamma
	ILI9163_WriteData8(0x04);

	ILI9163_WriteToReg(0xC0); //Set Power Control 1
	ILI9163_WriteData16(0x1F00);

	ILI9163_WriteToReg(0xC1); //Set Power Control 2
	ILI9163_WriteData8(0x00);

	ILI9163_WriteToReg(0xC2); //Set Power Control 3
	ILI9163_WriteData16(0x0007);

	ILI9163_WriteToReg(0xC3); //Set Power Control 4 (Idle mode)
	ILI9163_WriteData16(0x0007);

	ILI9163_WriteToReg(0xC5); //Set VCom Control 1
	ILI9163_WriteData8(0x24); // VComH = 3v
	ILI9163_WriteData8(0xC8); // VComL = 0v

	ILI9163_WriteToReg(0x13); // normal mode

	ILI9163_WriteToReg(0x38); //Idle mode off
	//SB(0x39, Reg); //Enable idle mode

	ILI9163_WriteToReg(0x3A);	//Set pixel mode
	ILI9163_WriteData8(0x05);	// (16bit/pixel)

	ILI9163_WriteToReg(0x36);  //Set Memory access mode
	ILI9163_WriteData8(ILI9163_SET_MEM);

//  ILI9163_WriteToReg(0x33); // set scroll offset
//	ILI9163_WriteData16(32);
//	ILI9163_WriteData16(128);
//	ILI9163_WriteData16(0);

	ILI9163_WriteToReg(0x29);  // Display on

	ILI9163_WriteToReg(0x20);  // off Inv mode

#ifdef DMA_ENABLE
	ILI9163_ConfigDMA();
#endif

	ILI9163_LED_ON;

	ILI9163_FillScreen(black);

}

void ILI9163_WritePixel(uint16_t x, uint16_t y, uint16_t color)
{
	ILI9163_SetAddress(x, y, x, y);
	ILI9163_WriteData16(color);
}

void ILI9163_PrintChar(uint16_t x, uint16_t y, char c)
{
	if(x > ILI9163_RES_X - 5 || y > ILI9163_RES_Y - 8 || c < 0x20)
	{
		return;
	}

	c -= 0x20;  // znakova sada bude zacinat mezerou (ASCII 32)
	c *= 5;     // 5 bytu/znak

	ILI9163_SetAddress(x, y, x + 4 + 1, y + 7);
	for(uint8_t YCnt = 0; YCnt < 8; YCnt++)
	{
		for(uint8_t XCnt = 0; XCnt < 5 + 1; XCnt++)
		{
		  if(XCnt < 5 && (font[c + XCnt] & (1 << YCnt)))
		  {
			ILI9163_WriteData16(params.txtColor);
		  }
		  else
		  {
			ILI9163_WriteData16(params.bkgColor);
		  }
		}
	}
}

void ILI9163_PrintText(uint16_t x, uint16_t y, char* text)
{
  while (*text)
  {
    ILI9163_PrintChar(x, y, *text++);
    x += 5 + 1;
  }
}

void ILI9163_SetTextParams(	uint16_t txtColor,	uint16_t bkgColor)
{
  params.txtColor = txtColor;
  params.bkgColor = bkgColor;
}

void ILI9163_SetAddress(uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop)
{
  yStart += ILI9163_OFFSET;
  yStop += ILI9163_OFFSET;

  SPI1_SetPrescaler(ILI9163_SPI_PRESCALER);

	ILI9163_WriteToReg(0x2A);			// Column Address Set
	ILI9163_WriteData16(xStart);
	ILI9163_WriteData16(xStop);

	ILI9163_WriteToReg(0x2B);			// page address set
	ILI9163_WriteData16(yStart);
	ILI9163_WriteData16(yStop);

	ILI9163_WriteToReg(0x2C);			// memory write
}

void ILI9163_WriteToReg(uint8_t value)
{
	ILI9163_CS_ENABLE;
	ILI9163_A0_REG;

	SPI1->DR = value;
	while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
	while (SPI1->SR & SPI_I2S_FLAG_BSY);
	ILI9163_CS_DISABLE;
}

void ILI9163_WriteData8(uint8_t value)
{
	ILI9163_CS_ENABLE;
	ILI9163_A0_DATA;

	SPI1->DR = value;
	while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
	while (SPI1->SR & SPI_I2S_FLAG_BSY);
	ILI9163_CS_DISABLE;
}

void ILI9163_WriteData16(uint16_t value)
{
	ILI9163_CS_ENABLE;
	ILI9163_A0_DATA;

	SPI1->DR = (uint8_t)(value >> 8);
	while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
	SPI1->DR = (uint8_t)(value & 0xFF);
	while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
	while (SPI1->SR & SPI_I2S_FLAG_BSY);
	ILI9163_CS_DISABLE;
}

void ILI9163_WriteData16Block(uint16_t* arrBuffer, uint16_t nLength)
{
  ILI9163_CS_ENABLE;
  ILI9163_A0_DATA;

  while (nLength--)
  {
    uint16_t nValue = *arrBuffer++;
    SPI1->DR = (uint8_t)(nValue >> 8);
    while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
    SPI1->DR = (uint8_t)(nValue & 0xFF);
    while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  }

  while (SPI1->SR & SPI_I2S_FLAG_BSY);

  ILI9163_CS_DISABLE;
}

void ILI9163_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  if (x >= ILI9163_RES_X || y >= ILI9163_RES_Y)
  {
    return;
  }

  if (x + w > ILI9163_RES_X)
  {
    w = ILI9163_RES_X - x;
  }

  if (y + h > ILI9163_RES_Y)
  {
    h = ILI9163_RES_Y - y;
  }

  ILI9163_SetAddress(x, y, x + w - 1, y + h - 1);

#ifdef DMA_ENABLE
	SPI_Cmd(SPI1, DISABLE);
	SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
	SPI_Cmd(SPI1, ENABLE);
	ILI9163_CS_ENABLE;
	ILI9163_A0_DATA;
	uint32_t nSize = w * h;
	while (nSize)
	{
		uint16_t nLength = 0xFFFF;
		if (nSize < 0xFFFF)
		{
			nLength = nSize;
		}

		ILI9163_TransferDMA((uint32_t)&color, nLength, false);
		while ((DMA_GetFlagStatus(DMA_STREAM, DMA_FLAG_TCIF5) == RESET));
		nSize -= nLength;
	}

	ILI9163_CS_DISABLE;
	SPI_Cmd(SPI1, DISABLE);
	SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
	SPI_Cmd(SPI1, ENABLE);
#else
	for (uint32_t i = w * h; i > 0; i--)
	{
		ILI9163_WriteData16(color);
	}
#endif
}

void ILI9163_FillScreen(uint16_t color)
{
	ILI9163_FillRect(0, 0, ILI9163_RES_X, ILI9163_RES_Y, color);
}

void ILI9163_ConfigDMA()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)(1);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) 0;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;

	DMA_Init(DMA_STREAM, &DMA_InitStructure);

	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
}

void ILI9163_TransferDMA(uint32_t nMemAddr, uint16_t nLength, bool bMemInc)
{
	DMA_Cmd(DMA_STREAM, DISABLE);
	while ( DMA_STREAM->CR & DMA_SxCR_EN);

	// Todo: po prenosu je nastaven i flag chyby FEIF - proc?
	DMA_ClearFlag(DMA_STREAM, DMA_FLAG_FEIF5|DMA_FLAG_DMEIF5|DMA_FLAG_TEIF5|DMA_FLAG_HTIF5|DMA_FLAG_TCIF5);
	DMA_STREAM->M0AR = nMemAddr;
	DMA_STREAM->NDTR = nLength;
	DMA_STREAM->CR &= ~DMA_MemoryInc_Enable;
	if (bMemInc)
	{
	  DMA_STREAM->CR |= DMA_MemoryInc_Enable;
	}

	DMA_Cmd(DMA_STREAM, ENABLE);
}

void ILI9163_LedOn()
{
  ILI9163_LED_ON;
}

void ILI9163_LedOff()
{
  ILI9163_LED_OFF;
}

void ILI9163_PixelSetRGB565(int16_t x, int16_t y, uint16_t color)
{
  ILI9163_WritePixel((uint16_t) x, (uint16_t)y, color);
}

uint8_t ILI9163_FillFrame(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  ILI9163_FillRect(x1, y1, x2 - x1, y2 - y1, color);
  return 0;
}

void ILI9163_InitFillArea(uint16_t nMemSize)
{
  g_pMemFillArea = malloc(nMemSize * sizeof(uint16_t));
}

void* ILI9163_FillArea(int16_t posX, int16_t posY, int16_t nEndX, int16_t nEndY)
{
  g_nCharPosX = posX;
  g_nCharPosY = posY;
  g_nCharEndX = nEndX;
  g_nCharEndY = nEndY;
  g_nPixelCount = (nEndX - posX + 1) * (nEndY - posY + 1);
  g_nPixelCounter = 0;
  return ILI9163_FillAreaSetPixel;
}

void ILI9163_FillAreaSetPixel(uint16_t nColor)
{
  *(g_pMemFillArea + g_nPixelCounter) = nColor;
  g_nPixelCounter++;
  if (g_nPixelCounter == g_nPixelCount)
  {
    // vsechny pixely znaku jsou zadany -> spustit DMA prenos
    ILI9163_SetAddress(g_nCharPosX, g_nCharPosY, g_nCharEndX, g_nCharEndY);

    SPI_Cmd(SPI1, DISABLE);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
    SPI_Cmd(SPI1, ENABLE);
    ILI9163_CS_ENABLE;
    ILI9163_A0_DATA;
    uint32_t nSize = g_nPixelCount;
    while (nSize)
    {
      uint16_t nLength = 0xFFFF;
      if (nSize < 0xFFFF)
      {
        nLength = nSize;
      }

      ILI9163_TransferDMA((uint32_t)g_pMemFillArea, nLength, true);
      while ((DMA_GetFlagStatus(DMA_STREAM, DMA_FLAG_TCIF5) == RESET));
      nSize -= nLength;
    }

    ILI9163_CS_DISABLE;
    SPI_Cmd(SPI1, DISABLE);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
    SPI_Cmd(SPI1, ENABLE);
  }
}

// vykresleni jednobitove bitmapy
// bitmapa vygenerovana pomoci programu 'Image2Code' (volba 3, smer up->down, left->right)
void ILI9163_DrawMonochromeBitmap(uint16_t xPos, uint16_t yPos, BMPbpp1* bmp)
{
  uint8_t* pData;
  uint8_t data;
  uint16_t c;

  if (bmp->pData == NULL)
  {
    return;
  }

  c = bmp->nColor;
  pData = (uint8_t*)bmp->pData;

  ILI9163_SetAddress(xPos, yPos, xPos + bmp->nWidth - 1, yPos + bmp->nHeight - 1);
  uint8_t nBufPos = 0;
  for (uint16_t y = 0; y < bmp->nHeight; y++)
  {
     for (uint16_t x = 0; x < bmp->nWidth; x += 8)
     {
        data = *pData++;
        for (uint8_t i = 0; i < 8; i++)
        {

          if (nBufPos == BUFFER_LEN)
          {
            // poslat obsah bufferu do radice
            ILI9163_WriteData16Block(g_arrBuffer, nBufPos);

#ifdef DMA_ENABLE
            // DMA transfer
#else
            // block transfer
#endif
            nBufPos = 0;
          }

          if (data & 0x80)
          {
            g_arrBuffer[nBufPos] = c;
          }
          else
          {
            g_arrBuffer[nBufPos] = 0;
          }

          nBufPos++;
          data <<= 1;
        }
     }
  }
}
