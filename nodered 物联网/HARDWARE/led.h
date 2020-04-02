#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED0 PAout(8)
#define LED1 PBout(12)

void LED_Init(void);

#endif
