/*
 * adc.h
 *
 *  Created on: 25. 9. 2018
 *      Author: vlada
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32f4xx.h"
#include <stdbool.h>

void Adc_Init(void);
uint16_t Adc_ReadVBAT_mV(void);
void Adc_DeInit(void);

#endif /* ADC_H_ */
