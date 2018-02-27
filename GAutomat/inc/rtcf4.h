/*
 * rtc.h
 *
 *  Created on: 7. 11. 2016
 *      Author: priesolv
 */

#ifndef RTCF4_H_
#define RTCF4_H_

#include "stm32f4xx.h"
#include <stdbool.h>

typedef struct
{
  uint8_t second: 4;
  uint8_t second10: 4;
  uint8_t minute: 4;
  uint8_t minute10: 4;
  uint8_t hour: 4;
  uint8_t hour10: 4;
} rtc_time_t;

typedef struct
{
  uint8_t day: 4;
  uint8_t day10: 4;
  uint8_t month: 4;
  uint8_t month10: 1;
  uint8_t week_day: 3;
  uint8_t year: 4;
  uint8_t year10: 4;
} rtc_date_t;

typedef struct
{
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
}rtc_record_time_t;

void RTCF4_Init(void);
void RTCF4_Set(rtc_record_time_t *dt, bool bDate, bool bTime);
void RTCF4_Get(rtc_record_time_t *dt);
void RTCF4_SetWakeUp(uint16_t nInterval);
void RTCF4_PrintDT(uint8_t *pBuffer, uint8_t length);

uint32_t RTCF4_GetTicks();
uint32_t RTCF4_GetUsartTimer();
void RTCF4_SetUsartTimer(uint32_t nInterval_ms);

int32_t RTCF4_GetUnixTimeStamp(rtc_record_time_t* data);
void RTCF4_GetDateTimeFromUnix(rtc_record_time_t* data, uint32_t unix);

uint8_t RTCF4_ByteToBcd2(uint8_t Value);
uint8_t RTCF4_Bcd2ToByte(uint8_t Value);

void RTCF4_Test(void);

#endif /* RTCF4_H_ */
