/*
 * SPI1.c
 *
 *  Created on: 23. 1. 2017
 *      Author: priesolv
 */

#include "SPI1.h"
#include "common.h"

// implementace pro STM32F410

#define SPI1_SCK                         PB3
#define SPI1_MISO                        PB4
#define SPI1_MOSI                        PB5

void SPI1_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

  /* SPI SCK pin configuration */
  GPIO_ClockEnable(GPIO_GetPort(SPI1_SCK));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(SPI1_SCK);
  GPIO_Init(GPIO_GetPort(SPI1_SCK), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(SPI1_SCK), GPIO_GetPinSource(GPIO_GetPin(SPI1_SCK)), GPIO_AF_SPI1);

  /* SPI  MOSI pin configuration */
  GPIO_ClockEnable(GPIO_GetPort(SPI1_MOSI));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(SPI1_MOSI);
  GPIO_Init(GPIO_GetPort(SPI1_MOSI), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(SPI1_MOSI), GPIO_GetPinSource(GPIO_GetPin(SPI1_MOSI)), GPIO_AF_SPI1);

  /* SPI  MISO pin configuration */
  GPIO_ClockEnable(GPIO_GetPort(SPI1_MISO));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(SPI1_MISO);
  GPIO_Init(GPIO_GetPort(SPI1_MISO), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(SPI1_MISO), GPIO_GetPinSource(GPIO_GetPin(SPI1_MISO)), GPIO_AF_SPI1);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(SPI1);

  SPI_InitTypeDef  SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;  // je jedno co
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

void SPI1_SendData16(uint16_t nValue)
{
  SPI1->DR = nValue >> 8;
  while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  SPI1->DR = nValue;
  while (SPI1->SR & SPI_I2S_FLAG_BSY);
}

uint8_t SPI1_SendData8(uint8_t nValue)
{
  SPI1->DR = nValue;
  while (SPI1->SR & SPI_I2S_FLAG_BSY);

  return SPI1->DR;
}

void SPI1_SetPrescaler(spi_br_e ePrescaler)
{
  // stop SPI peripherial
  SPI1->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);

  SPI1->CR1 = (SPI1->CR1 & ~(SPI_CR1_BR)) | ePrescaler;

  // start SPI peripherial
  SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI1_SetMode(spi_mode_e eMode)
{
  // stop SPI peripherial
  SPI1->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);

  SPI1->CR1 = (SPI1->CR1 & ~(SPI_CR1_CPHA | SPI_CR1_CPOL)) | eMode;

  // start SPI peripherial
  SPI1->CR1 |= SPI_CR1_SPE;
}
