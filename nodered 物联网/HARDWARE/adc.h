#ifndef _ADC_H
#define _ADC_H

#include "stm32f10x.h"
#include "stm32f10x_adc.h"

void AD_Init(void);
float AD_Read(void);

#endif

