#include "stm32f10x_it.h" 
#include "sys.h"
#include "led.h"

extern uint8_t cnt_1_5_s , cnt_100_ms;
extern uint8_t Led_Flag_shan;

//TIM2中断服务函数
void TIM2_IRQHandler(void){	
	static int count1  , count2 , count3;
	static _Bool flag1 = 0 ;
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET){//判断是否进入TIM2更新中断
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除TIM2更新中断
		if(Led_Flag_shan == 0){
			if(++count1 == 100){
				count1 = 0;
				if(flag1 == 0)	{flag1 = 1 ; LED1 = 0;}
				else if(flag1 == 1)	{flag1 = 0 ; LED1 = 1;}
			}
		}
		else {
			LED1 = 1;
		}
		
		if(++count2 == 1000){
			count2 = 0;
			cnt_1_5_s = 1;
		}
		if(++count3 == 100){
			count3 = 0;
			cnt_100_ms = 1;
		}
	}
}











