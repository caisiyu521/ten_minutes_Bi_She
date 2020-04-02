#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include "delay.h"
#include "stdio.h"

#define wk_up GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define Key0 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)
#define Key1 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)

void Key_Init(void);
void key_scan(void);

#endif

