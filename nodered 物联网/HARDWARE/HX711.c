#include "HX711.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
uint16_t count;
uint16_t Weight_Maopi ;

void HX711_Init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

unsigned long ReadCount(void){
	unsigned long Count;
	unsigned char i;
	
	HX711_SCK = 0;
	while(HX711_DATA_Read); //AD转换未结束则等待，否则开始读取
	
	for (i=0;i<24;i++){
		HX711_SCK = 1;
		Count=Count<<1; //下降沿来时变量Count左移一位，右侧补零
		HX711_SCK = 0;
		if(HX711_DATA_Read) Count++;
	}
	HX711_SCK = 1;
	delay_us(1);
	Count=Count^0x800000;//第25个脉冲下降沿来时，转换数据
	HX711_SCK = 0;
	return(Count);
}



#define N 50
uint16_t Weight_Shiwu ;
uint16_t Weight_Exange ;
u8 flag_max = 0;
long unsigned int Weight_shang = 0;


void HX711_Maozhong(void){
	u8 i = 0 ;u8 flag = 0;
	long unsigned int Weight = 0;
	long unsigned int Weight_all = 0 ;
	delay_ms(200);
	if(flag == 0)
		Weight_shang = ReadCount();
	delay_ms(200);
	for (i=0;i<N;i++){
		Weight = ReadCount();
		if(((Weight-Weight_shang)>10)||((Weight_shang-Weight)>10))
			Weight = Weight_shang;		
		Weight_all += Weight;
		delay_ms(10);
	}
	Weight_Maopi = Weight_all/N;
	
	
	Weight_Exange = ReadCount()- Weight_Maopi;
	Weight_Exange = (long)((float)Weight_Exange/430);
	Weight_shang = Weight_Exange;
	
	flag = 0;
//	if(flag_max)
}

long Get_Weight(void){
	static u8 flag = 0;
	Weight_Shiwu = ReadCount()- Weight_Maopi;
	Weight_Shiwu = (long)((float)Weight_Shiwu/430);
	
	if(flag == 0)
		Weight_shang = Weight_Shiwu;
	
	if((Weight_Shiwu >= 1) && (Weight_shang <= 150)){
		flag_max += 1;
		printf("flag = %d\r\n",flag_max);
	}
	else if((Weight_Shiwu <=150) && (Weight_shang >= 1)){
		flag_max -= 1;
		printf("flag = %d\r\n",flag_max);
	}
//	
//	if(flag_max == 0){
//		Weight_shang = Weight_Shiwu-Weight_Exange;
//		return (Weight_Shiwu-Weight_Exange);
//	}
//	if(flag_max == 1){
//		Weight_Shiwu = Weight_Shiwu + (150-Weight_Exange);
//		Weight_shang = Weight_Shiwu;
//		return Weight_Shiwu;
//	}
//	if(flag_max == 2){
//		Weight_Shiwu = Weight_Shiwu + (150-Weight_Exange) + 150;
//		Weight_shang = Weight_Shiwu;
		return Weight_Shiwu;
//	}
//	return 0;
}
