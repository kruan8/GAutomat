/*
 * rtc.c
 *
 *  Created on: 7. 11. 2016
 *      Author: priesolv
 */

// implementace pro STM32f410

#include <rtcf4.h>
#include <string.h>
#include <stdio.h>

#define RTC_CLOCK_SOURCE_LSE

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_DR_RESERVED_MASK    ((uint32_t)0x00FFFF3F)

/* Internal RTC defines */
#define RTC_LEAP_YEAR(year)             ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define RTC_DAYS_IN_YEAR(x)             RTC_LEAP_YEAR(x) ? 366 : 365
#define RTC_OFFSET_YEAR                 1970
#define RTC_SECONDS_PER_DAY             86400
#define RTC_SECONDS_PER_HOUR            3600
#define RTC_SECONDS_PER_MINUTE          60
#define RTC_BCD2BIN(x)                  ((((x) >> 4) & 0x0F) * 10 + ((x) & 0x0F))
#define RTC_CHAR2NUM(x)                 ((x) - '0')
#define RTC_CHARISNUM(x)                ((x) >= '0' && (x) <= '9')

/* Days in a month */
const uint8_t TM_RTC_Months[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, /* Not leap year */
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  /* Leap year */
};

static volatile uint32_t g_nTicks = 0;
static volatile uint32_t g_nUsartTimer;

void RTCF4_Init(void)
{
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* As the LSE is in the Backup domain and write access is denied to
     this domain after reset, you have to enable write access using
     PWR_BackupAccessCmd(ENABLE) function before to configure the LSE */
  PWR_BackupAccessCmd(ENABLE);

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  RTC_InitTypeDef RTC_InitStructure;

  /* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
  // 1024 Hz for DEBUG
#ifdef DEBUG
  uint32_t uwSynchPrediv = 8;
  uint32_t uwAsynchPrediv = 8;
#else
  uint32_t uwSynchPrediv = 0xFF;
  uint32_t uwAsynchPrediv = 0x7F;
#endif

  /* Configure the RTC data register and RTC prescaler */
  RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
  RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);

  /* Indicator for the RTC configuration */
  RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);

  PWR_BackupAccessCmd(DISABLE);
}

//void RTCF4_SetWakeUp(uint16_t nInterval)
//{
//  RTC->WPR = 0xCA; /* (7) */
//  RTC->WPR = 0x53; /* (7) */
//  RTC->CR &=~ RTC_CR_WUTE; /* (8) */
//  while((RTC->ISR & RTC_ISR_WUTWF) != RTC_ISR_WUTWF) /* (9) */
//  {
//    /* add time out here for a robust application */
//  }
//
//  RTC->WUTR = nInterval - 1;  // WUTR je delicka, takže 0 znamena 1 impulz
//  RTC->CR = RTC_CR_WUCKSEL_2 | RTC_CR_WUTE | RTC_CR_WUTIE; /* (11) */
//  RTC->WPR = 0xFE; /* (12) */
//  RTC->WPR = 0x64; /* (12) */
//
//  EXTI->IMR |= EXTI_IMR_IM20;       // unmask line 20
//  EXTI->RTSR |= EXTI_RTSR_TR20;     // Rising edge for line 20
//  NVIC_SetPriority(RTC_IRQn, 0);    // Set priority
//  NVIC_EnableIRQ(RTC_IRQn);         // Enable RTC_IRQn
//}

void RTCF4_Set(rtc_record_time_t *dt, bool bDate, bool bTime)
{
  PWR_BackupAccessCmd(ENABLE);
  RTC_WriteProtectionCmd(DISABLE);

  RTC_EnterInitMode();

  if (bDate)
  {
    RTC->DR = RTCF4_ByteToBcd2(dt->year) << 16 | 1 << 13 | RTCF4_ByteToBcd2(dt->month) << 8 | RTCF4_ByteToBcd2(dt->day);
  }

  if (bTime)
  {
    RTC->TR = RTCF4_ByteToBcd2(dt->hour) << 16 | RTCF4_ByteToBcd2(dt->min) << 8 | RTCF4_ByteToBcd2(dt->sec);
  }

  RTC_ExitInitMode();

  RTC_WriteProtectionCmd(ENABLE);

  RTC_WaitForSynchro();
  PWR_BackupAccessCmd(DISABLE);
}

void RTCF4_Get(rtc_record_time_t *dt)
{
  uint32_t value = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);

  dt->hour = (uint8_t)(RTC_BCD2BIN((value >> 16) & 0x3F));
  dt->min = (uint8_t)(RTC_BCD2BIN((value >> 8) & 0x7F));
  dt->sec = (uint8_t)(RTC_BCD2BIN(value & 0x7F));

  value = (uint32_t)(RTC->DR & RTC_DR_RESERVED_MASK);
  dt->year = (uint8_t)(RTC_BCD2BIN((value >> 16) & 0xFF));
  dt->month = (uint8_t)(RTC_BCD2BIN((value >> 8) & 0x1F));
  dt->day = (uint8_t)(RTC_BCD2BIN(value & 0x3F));
}

void RTCF4_PrintDT(uint8_t *pBuffer, uint8_t length)
{
  rtc_record_time_t dt;

  RTCF4_Get(&dt);
  snprintf((char*)pBuffer, length, "%d.%d.%d %02d:%02d:%02d",
        dt.day, dt.month, dt.year, dt.hour, dt.min, dt.sec);
}

uint32_t RTCF4_GetTicks()
{
  return g_nTicks;
}

uint32_t RTCF4_GetUsartTimer()
{
  return g_nUsartTimer;
}

void RTCF4_SetUsartTimer(uint32_t nInterval_ms)
{
  g_nUsartTimer = nInterval_ms;
}

int32_t RTCF4_GetUnixTimeStamp(rtc_record_time_t* data)
{
  uint32_t days = 0, seconds = 0;
  uint16_t i;
  uint16_t year = (uint16_t) (data->year + 2000);
  /* Year is below offset year */
  if (year < RTC_OFFSET_YEAR) {
    return 0;
  }
  /* Days in back years */
  for (i = RTC_OFFSET_YEAR; i < year; i++) {
    days += RTC_DAYS_IN_YEAR(i);
  }
  /* Days in current year */
  for (i = 1; i < data->month; i++) {
    days += TM_RTC_Months[RTC_LEAP_YEAR(year)][i - 1];
  }
  /* Day starts with 1 */
  days += data->day - 1;
  seconds = days * RTC_SECONDS_PER_DAY;
  seconds += data->hour * RTC_SECONDS_PER_HOUR;
  seconds += data->min * RTC_SECONDS_PER_MINUTE;
  seconds += data->sec;

  /* seconds = days * 86400; */
  return seconds;
}

void RTCF4_GetDateTimeFromUnix(rtc_record_time_t* data, uint32_t unix)
{
  uint16_t year;

  /* Get seconds from unix */
  data->sec = unix % 60;
  /* Go to minutes */
  unix /= 60;
  /* Get minutes */
  data->min = unix % 60;
  /* Go to hours */
  unix /= 60;
  /* Get hours */
  data->hour = unix % 24;
  /* Go to days */
  unix /= 24;

  /* Get week day */
  /* Monday is day one */
  data->day = (unix + 3) % 7 + 1;

  /* Get year */
  year = 1970;
  while (1) {
    if (RTC_LEAP_YEAR(year)) {
      if (unix >= 366) {
        unix -= 366;
      } else {
        break;
      }
    } else if (unix >= 365) {
      unix -= 365;
    } else {
      break;
    }
    year++;
  }
  /* Get year in xx format */
  data->year = (uint8_t) (year - 2000);
  /* Get month */
  for (data->month = 0; data->month < 12; data->month++) {
    if (RTC_LEAP_YEAR(year)) {
      if (unix >= (uint32_t)TM_RTC_Months[1][data->month]) {
        unix -= TM_RTC_Months[1][data->month];
      } else {
        break;
      }
    } else if (unix >= (uint32_t)TM_RTC_Months[0][data->month]) {
      unix -= TM_RTC_Months[0][data->month];
    } else {
      break;
    }
  }
  /* Get month */
  /* Month starts with 1 */
  data->month++;
  /* Get date */
  /* Date starts with 1 */
  data->day = unix + 1;
}

uint8_t RTCF4_ByteToBcd2(uint8_t Value)
{
  uint8_t bcdhigh = 0;

  while (Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }

  return  ((uint8_t)(bcdhigh << 4) | Value);
}

/**
  * @brief  Convert from 2 digit BCD to Binary.
  * @param  Value: BCD value to be converted.
  * @retval Converted word
  */
uint8_t RTCF4_Bcd2ToByte(uint8_t Value)
{
  uint8_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}

//void RTCF4_IRQHandler(void)
//{
//  // Check WUT flag
//  if(RTC->ISR & RTC_ISR_WUTF)
//  {
//    RTC->ISR =~ RTC_ISR_WUTF; /* Reset Wake up flag */
//    EXTI->PR = EXTI_PR_PR20; /* clear exti line 20 flag */
//  }
//}

//void SysTick_Handler(void)
//{
//  g_nTicks++;
//  if (g_nUsartTimer)
//  {
//    g_nUsartTimer--;
//  }
//}

void RTCF4_Test(void)
{
  // ---------- Test RTC ---------------
  rtc_record_time_t dt;

  dt.day = 15;
  dt.month = 11;
  dt.year = 16;
  dt.hour = 18;
  dt.min = 25;
  dt.sec = 0;
  RTCF4_Set(&dt, true, true);
  RTCF4_Get(&dt);
  uint32_t t = RTCF4_GetUnixTimeStamp(&dt);
  RTCF4_GetDateTimeFromUnix(&dt, t);
}
