#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "nvic.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "fy_mqtt.h"
#include "fy_network.h"
#include "rtc.h" 	
#include "dht11.h" 	
#include "oled_iic.h" 	

u8 txbuf[256];
u8 rxbuf[256];
u8 sta;
char mqtt_message[200];
uint8_t cnt_1_5_s = 0 , cnt_100_ms = 0;
uint8_t wendu,shidu;
uint8_t guang;
uint8_t flag;
uint8_t buf[100];
uint16_t weight;
uint8_t string[1000];

uint8_t flag = 0;

void ESP_INIT(void);

int main(void){	

	delay_init();	    	//延时函数初始化	  
	uart1_init(115200);	 	//串口初始化为115200
	uart2_init(115200);	 	//串口初始化为115200
	uart3_init(115200);
	LED_Init();				//初始化 LED
	Timer2_Init(1000,72);	//初始化 Timer2 1ms 中断一次
	Timer4_Init(200,7200);	
	RTC_Init();				
	nvic_init();			//配置 中断的 优先级 ， 放初始化 后面
	DHT11Configuration();
	I2C_Configuration();
	
	OLED_Init();
	OLED_CLS();
	OLED_ON();
	
	printf("CPU Start !\r\n");
	
	TIM_SetCompare3(TIM4,15);
	
	ESP_INIT();//ESP8266初始化
	
//	HX711_Maozhong();//测量一次毛重
	
	while(1){
		
		
		sprintf((char*)string,"shidu = %d",shidu);
		OLED_ShowStr(15,2,string,2);
		sprintf((char*)string,"wendu = %d",wendu);
		OLED_ShowStr(15,4,string,2);
		if(cnt_1_5_s){			//每个1.5s发送一次
			cnt_1_5_s = 0;
			DHT11_Read_Data(&wendu,&shidu);
//			sprintf(mqtt_message,"{\"shidu\":\"%d\",}",shidu);
//			if(flag == 0){
//				flag = 1;
			sprintf(mqtt_message,"{\"shidu\":\"%d\",\"wendu\":\"%d\"}",shidu,wendu);
			_mqtt.PublishData("/cai/environment",mqtt_message,0);
//			}
//			else {
//				flag = 0;
//				sprintf(mqtt_message,"{\"wendu\":\"%d\"}",wendu);
//				_mqtt.PublishData("cai/wendu",mqtt_message,0);
//			}
			printf("%d,%d\r\n",wendu,shidu);
			
			OLED_ShowStr(15,2,(unsigned char * )"wendu = %d",wendu);
			OLED_ShowStr(15,4,(unsigned char * )"shidu = %d",shidu);

//			sprintf(mqtt_message,"nian=%d,month=%d,day=%d,shi=%d,fen=%d,miao=%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
		}
		
		//	串口1 接受函数
		if(USART1_RX_STA&0x8000){	
			printf("%s \r\n" , buf);
			USART1_RX_STA=0;
			memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
		}
		if(_mqtt.rxlen){
			Mqtt_Progress(_mqtt.rxbuf,_mqtt.rxlen);	
			memset(_mqtt.rxbuf,0,_mqtt.rxlen);
			_mqtt.rxlen = 0;
		}
	}
}

void ESP_INIT(void){
	if(_net.Init(rxbuf,sizeof(rxbuf),txbuf,sizeof(txbuf))!=0){
		printf("Net Init OK!\r\n");
		sta++;
	}
	else{
		printf("Net Init Error!\r\n");		sta=0;
	}
	if(sta==1){
		if(_net.ConnectAP("ipone","123456321")!=0){
//		if(_net.ConnectAP("iPhone","88888888")!=0){
			printf("Conncet AP OK!\r\n");
			sta++;
		}
		else {
			printf("Conncet AP Error!\r\n");
			sta=0;
		}
	}
	if(sta==2){	
        if(_net.ConnectServer("TCP","ux5um0r.mqtt.iot.gz.baidubce.com",1883)!=0) {//连接TCP
//		if(_net.ConnectServer("TCP","n69ftrd.mqtt.iot.gz.baidubce.com",1883)!=0) {//连接TCP
			printf("Conncet Server OK!\r\n");
			sta++;
		}else{
			printf("Conncet Server Error!\r\n");	
			sta=0;
		}
	}
	if(sta==3){
		//登录MQTT
		_mqtt.Init(rxbuf,sizeof(rxbuf),txbuf,sizeof(txbuf));		
		if(_mqtt.Connect(
			"danpianji",
//			"n69ftrd/admin",//Username
//			"NKp9CWd2pzVmKdeO"//Password
            "ux5um0r/caisiyu",//Username
			"PIQIlH84IRk9dcE9"//Password
			) != 0){
			printf("Enter MQTT OK!\r\n");
			sta++;
		}else{		
			printf("Enter MQTT Error!\r\n");
			sta=0;
		}	
	}
	if(sta==4){
		if(_mqtt.SubscribeTopic("/cai/control",0,1) != 0){
			printf("SubscribeTopic OK!\r\n");
		}else{
			printf("SubscribeTopic Error!\r\n");
		}
	}
}

