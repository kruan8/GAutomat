/*
 * DHT.c
 *
 *  Created on: 23. 3. 2017
 *      Author: priesolv
 */

/*
 * driver pro ctreni tpeloty a vlhkosti z cidla DHT11 a DHT22 (AM2302, nastaveno symbolem DHT22_AM2302 == 1)
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

#define MAX_TICS 10000
#define DHT11_OK 0
#define DHT11_NO_CONN 1
#define DHT11_CS_ERROR 2

#define DHT_LEVEL_COUNT        10         // minimalni pocet stejnych urovni (filtrace zakmitu)

#define LOW                    false
#define HIGH                   true

/* Function declaration */
static dht_error_e DHT_Read(dht_data_t* data);
static bool DHT_WaitForLevel(bool bLevel, uint8_t nMaxTime_us);
static void DHT_Delay_us(uint16_t nDelay_us);
static void DHT_ResetTimer();
static uint16_t DHT_GetTimerCounter_us();


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

  DHT_PIN_HIGH;  // init line state

  RCC_APB2PeriphClockCmd(DHT_TIM_CLOCK, ENABLE);
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = nPCLK_Frequency / 1000000; // we need 1MHz here
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;

  TIM_TimeBaseInit(DHT_TIM, &TIM_TimeBaseStructure);

  DBGMCU->APB2FZ |= DBGMCU_TIM9_STOP;
  TIM_Cmd(DHT_TIM, ENABLE);
}

dht_error_e DHT_Read(dht_data_t* data)
{
  uint8_t buff[5];

  /* RESET - set pin low for min 18ms */
  DHT_PIN_LOW;
#if (DHT22_AM2302 == 1)
  DHT_Delay_us(5000 + 2000);
#else
  DHT_Delay_us(18000 + 2000);
#endif

  DHT_PIN_HIGH;
  DHT_Delay_us(5);

  /* Wait for LOW 20-40us acknowledge */
  if (!DHT_WaitForLevel(LOW, 80))
  {
    return DHT_CONNECTION_ERROR;
  }

  /* Wait for HIGH, about 80-85us */
  if (!DHT_WaitForLevel(HIGH, 160))
  {
    return DHT_WAITHIGH_ERROR;
  }

  /* Wait for LOW, about 80-85us */
  if (!DHT_WaitForLevel(LOW, 160))
  {
    return DHT_WAITLOW_ERROR;
  }

  /* Read 5 bytes */
  for (uint8_t j = 0; j < 5; j++)
  {
    buff[j] = 0;
    for (uint8_t i = 8; i > 0; i--)
    {
      /* We are in low signal now, wait for high signal and measure time */
      /* Wait HIGH about 57-63us */
      if (!DHT_WaitForLevel(HIGH, 100))
      {
        return DHT_WAITHIGH_LOOP_ERROR;
      }

      // wait for LOW -> 26us for 0 or 70us for 1
      if (!DHT_WaitForLevel(LOW, 140))
      {
        return DHT_WAITLOW_LOOP_ERROR;
      }

      if (DHT_GetTimerCounter_us() > 40)
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

dht_error_e DHT_GetData(dht_data_t* data)
{
  return DHT_Read(data);
}

bool DHT_WaitForLevel(bool bLevel, uint8_t nMaxTime_us)
{
  uint8_t nLevelCount = 0;

  DHT_ResetTimer();
  while (nLevelCount < DHT_LEVEL_COUNT)
  {
    if ((bool)(DHT_PORT->IDR & DHT_PIN) == bLevel)
    {
      nLevelCount++;
    }
    else
    {
      if (nLevelCount)
      {
        nLevelCount--;
      }
    }

    if (DHT_GetTimerCounter_us() > nMaxTime_us)
    {
      return false;
    }
  }

  return true;
}

void DHT_ResetTimer()
{
  DHT_TIM->EGR |= TIM_EGR_UG;  // counter reset
}

uint16_t DHT_GetTimerCounter_us()
{
  return DHT_TIM->CNT;
}

void DHT_Delay_us(uint16_t nDelay_us)
{
  DHT_ResetTimer();
  while (DHT_GetTimerCounter_us() < nDelay_us);
}


