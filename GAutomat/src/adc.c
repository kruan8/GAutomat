/*
 * adc.c
 *
 *  Created on: 25. 9. 2018
 *      Author: Priesol Vladimir
 */


#include "adc.h"

#define REF_VOLTAGE_MV     3300L

#define ADC_SAMPLES           10

void Adc_Init(void)
{
  // Enable clock for ADC1
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

   // Init ADC1
   ADC_InitTypeDef ADC_InitStruct;
   ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
   ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   ADC_InitStruct.ADC_NbrOfConversion = 1;
   ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStruct.ADC_ScanConvMode = DISABLE;
   ADC_Init(ADC1, &ADC_InitStruct);

   ADC_VBATCmd(ENABLE);

   ADC_Cmd(ADC1, ENABLE);

   // Select input channel for ADC1
   ADC_RegularChannelConfig(ADC1, ADC_Channel_Vbat, 1, ADC_SampleTime_480Cycles);
}

uint16_t Adc_ReadVBAT_mV(void)
{
  uint32_t nSumValue = 0;
  for (uint8_t i = 0; i < ADC_SAMPLES; i++)
  {
    ADC_SoftwareStartConv(ADC1);

    // Wait until conversion is finish
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    nSumValue += ADC_GetConversionValue(ADC1);
  }

  // battery voltage = VBAT/4
  nSumValue /= ADC_SAMPLES;
  return (nSumValue * (uint32_t)REF_VOLTAGE_MV * 1000 / 4096) / 1000 * 4;
}

void Adc_DeInit(void)
{
  ADC_Cmd(ADC1, DISABLE);
  ADC_VBATCmd(DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
}
