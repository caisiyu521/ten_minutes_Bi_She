#include "dht11.h"
      
//复位DHT11
void DHT11_Reset(void)	   
{                 
	DHT_Direction_O; 	
    O_DHT_DAT = 0; 	
    delay_ms(20);    	
    O_DHT_DAT = 1; 	
	delay_us(30);     
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在

uint8_t DHT11_IsOnline(void) 	   
{   
	uint8_t retry = 0;
	DHT_Direction_I;
    while (I_DHT_DAT && retry < 100)//DHT11会拉低40~80us
	{
		retry ++;
		delay_us(1);
	}

	if(retry >= 100)
	{
		return 1;
	}
	else
	{
		retry = 0;
	}
    while (!I_DHT_DAT && retry < 100)//DHT11拉低后会再次拉高40~80us
	{
		retry ++;
		delay_us(1);
	}

	if(retry >= 100)
	{
		return 1;
	}	    
	return 0;
}



uint8_t DHT11_ReadBit(void) 			 
{
 	uint8_t retry = 0;
	while(I_DHT_DAT && retry < 100)
	{
		retry ++;
		delay_us(1);
	}
	retry = 0;
	while(!I_DHT_DAT && retry < 100)
	{
		retry ++;
		delay_us(1);
	}
	delay_us(40);//等待40us
	if(I_DHT_DAT)
	{
		return 1;
	}
	else 
	{
		return 0;	
	}		
}





uint8_t DHT11_ReadByte(void)    
{        
    uint8_t i,dat;
    dat = 0;
	for (i = 0; i < 8; i ++) 
	{
   		dat <<= 1; 
	    dat |= DHT11_ReadBit();
    }						    
    return dat;
}



//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败

uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)    
{        
 	uint8_t buf[5];
	uint8_t i;
	DHT11_Reset();
	if(DHT11_IsOnline() == 0)
	{
		for(i = 0; i < 5; i ++)//读取40位数据
		{
			buf[i] = DHT11_ReadByte();
		}
		if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
		{
			*humi = buf[0];
			*temp = buf[2];
		}
	}
	else 
	{
		return 1;
	}
	return 0;	    
}

 
//PB13
void DHT11Configuration(void)
{	 
 	GPIO_InitTypeDef  MyGPIO;
 	RCC_APB2PeriphClockCmd(DHTCLKLINE, ENABLE);	 
 	MyGPIO.GPIO_Pin 	= DHT_DAT_Pin;				 
 	MyGPIO.GPIO_Mode 	= GPIO_Mode_Out_PP; 		 
 	MyGPIO.GPIO_Speed 	= GPIO_Speed_50MHz;
 	GPIO_Init(DHTPORT, &MyGPIO);				 
	O_DHT_DAT = 1;		
	DHT11_Reset();  //复位DHT11
} 
