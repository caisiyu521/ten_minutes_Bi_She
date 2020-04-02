#ifndef __HX711_H
#define __HX711_H

#include "sys.h"

#define HX711_SCK PBout(13) 
#define HX711_DATA_Read GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)


unsigned long ReadCount(void);
void HX711_Init(void);
void HX711_Maozhong(void);
long Get_Weight(void);
#endif


