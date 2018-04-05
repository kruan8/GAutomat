/*
 * DHT.c
 *
 *  Created on: 23. 3. 2017
 *      Author: priesolv
 */

/*
 * driver pro ctreni tpeloty a vlhkosti z cidla DHT11 a DHT22 (AM2302)
 * driver pouziva casovac DHT_TIM pro presne odmerovani delky impulsu
 */

#include "DHT.h"

#define DHT_PIN                GPIO_Pin_12
#define DHT_PORT               GPIOB
#define DHT_GPIO_CLK           RCC_AHB1Periph_GPIOB

#define DHT_PIN_LOW            (DHT_PORT->BSRRH = DHT_PIN)
#define DHT_PIN_HIGH           (DHT_PORT->BSRRL = DHT_PIN)

#define DHT_TIM                TIM9
#define DHT_TIM_CLOCK          RCC_APB2Periph_TIM9

dht_error_e DHT_Read(dht_data_t* data);
void DHT_Delay_us(uint16_t nDelay_us);
void DHT_ResetTimer();
uint16_t DHT_GetTimerCounter();

void DHT_Init(uint32_t nPCLK_Frequency)
{
  RCC_AHB1PeriphClockCmd(DHT_GPIO_CLK, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = DHT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_Init(DHT_PORT, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(DHT_TIM_CLOCK, ENABLE);
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_Prescaler = nPCLK_Frequency / 1000000; // zde potrebujeme 1MHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(DHT_TIM, &TIM_TimeBaseStructure);

//  DBGMCU->APB2FZ |= DBGMCU_TIM9_STOP;
  TIM_Cmd(DHT_TIM, ENABLE);

  // prvni cteni - zresetovani teplomeru)
  dht_data_t data;
  DHT_Read(&data);
}

dht_error_e DHT_GetData(dht_data_t* data)
{
  return DHT_Read(data);
}

dht_error_e DHT_Read(dht_data_t* data)
{
  uint8_t buff[5];

  /* Set pin low for 18ms */
  DHT_PIN_LOW;
  DHT_Delay_us(18000);

  /* Set pin high to ~30 us */
  DHT_PIN_HIGH;
  DHT_Delay_us(30);

  /* Wait 20us for acknowledge, low signal */
  DHT_ResetTimer();
  while (DHT_PORT->IDR & DHT_PIN)
  {
    if (DHT_GetTimerCounter() > 20)
    {
      return DHT_CONNECTION_ERROR;
    }
  }

  /* Wait high signal, about 80-85us long (measured with logic analyzer) */
  DHT_ResetTimer();
  while ((DHT_PORT->IDR & DHT_PIN) == 0)
  {
    if (DHT_GetTimerCounter() > 85)
    {
      return DHT_WAITHIGH_ERROR;
    }
  }

  /* Wait low signal, about 80-85us long (measured with logic analyzer) */
  DHT_ResetTimer();
  while (DHT_PORT->IDR & DHT_PIN)
  {
    if (DHT_GetTimerCounter() > 100)  // 85
    {
      return DHT_WAITLOW_ERROR;
    }
  }

  /* Read 5 bytes */
  for (uint8_t j = 0; j < 5; j++)
  {
    buff[j] = 0;
    for (uint8_t i = 8; i > 0; i--)
    {
      /* We are in low signal now, wait for high signal and measure time */
      /* Wait high signal, about 57-63us long (measured with logic analyzer) */
      DHT_ResetTimer();
      while ((DHT_PORT->IDR & DHT_PIN) == 0)
      {
        if (DHT_GetTimerCounter() > 75)
        {
          return DHT_WAITHIGH_LOOP_ERROR;
        }
      }

      // High signal detected, start measure high signal, it can be 26us for 0 or 70us for 1
      DHT_ResetTimer();
      while (DHT_PORT->IDR & DHT_PIN)
      {
        if (DHT_GetTimerCounter() > 100)  //90
        {
          return DHT_WAITLOW_LOOP_ERROR;
        }
      }

      if (DHT_GetTimerCounter() > 35)
      {
        buff[j] |= 1 << (i - 1);  // We read 1
      }
    }
  }

  /* Check for parity */
  if (((buff[0] + buff[1] + buff[2] + buff[3]) & 0xFF) != buff[4])
  {
    return DHT_PARITY_ERROR;
  }

#if (DHT22_AM2302 == 1)

  /* Set humidity */
  data->Hum = ((buff[0] << 8) + buff[1]) / 10;

  /* Negative temperature */
  if (buff[2] & 0x80)
  {
    data->Temp = -((buff[2] & 0x7F) << 8) + buff[3];
  }
  else
  {
    data->Temp = (buff[2] << 8) + buff[3];
  }
#else

  /* Set humidity */
  data->Hum = buff[0]; // << 8 | d[1];

  /* Negative temperature */
  if (buff[2] & 0x80)
  {
    data->Temp = -((buff[2] & 0x7F) * 10 + buff[3]);
  }
  else
  {
    data->Temp = (buff[2]) * 10 + buff[3];
  }
#endif
  return DHT_OK;
}

void DHT_ResetTimer()
{
  DHT_TIM->EGR |= TIM_EGR_UG;  // counter reset
}

uint16_t DHT_GetTimerCounter()
{
  return DHT_TIM->CNT;
}

void DHT_Delay_us(uint16_t nDelay_us)
{
  DHT_ResetTimer();
  while (DHT_GetTimerCounter() < nDelay_us);
}


