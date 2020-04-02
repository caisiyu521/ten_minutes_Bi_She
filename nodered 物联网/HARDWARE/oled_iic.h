#ifndef __OLED_I2C_H
#define __OLED_I2C_H

#include "sys.h"

#define OLED_ADDRESS    0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

/* STM32 I2C 快速模式 */
#define I2C_Speed              400000

/*I2C接口*/
#define OLED_I2C                          I2C1
#define OLED_I2C_CLK                      RCC_APB1Periph_I2C1
#define OLED_I2C_CLK_INIT                 RCC_APB1PeriphClockCmd

#define OLED_I2C_SCL_PIN                  GPIO_Pin_6                 
#define OLED_I2C_SCL_GPIO_PORT            GPIOB                       
#define OLED_I2C_SCL_GPIO_CLK             RCC_APB2Periph_GPIOB
#define OLED_I2C_SCL_SOURCE               GPIO_PinSource6
#define OLED_I2C_SCL_AF                   GPIO_AF_I2C1

#define OLED_I2C_SDA_PIN                  GPIO_Pin_7                  
#define OLED_I2C_SDA_GPIO_PORT            GPIOB                       
#define OLED_I2C_SDA_GPIO_CLK             RCC_APB2Periph_GPIOB
#define OLED_I2C_SDA_SOURCE               GPIO_PinSource7
#define OLED_I2C_SDA_AF                   GPIO_AF_I2C1

//I2C初始化配置
void I2C_Configuration(void);
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);

void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);//设置光标位置
void OLED_Fill(unsigned char fill_Data);//填充整个屏幕
void OLED_CLS(void);//清屏
void OLED_ON(void);//将OLED从休眠中唤醒
void OLED_OFF(void);//让OLED休眠 -- 休眠模式下,OLED功耗不到10uA

 /**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
    *                   ch[] :- 要显示的字符串; 
    *                   TextSize : 字符大小(1:6*8 ; 2:8*16)
    * @retval 无
  */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);

 /**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7); 
    *                   N:汉字在codetab.h中的索引
    * @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);

 /**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
    *                   x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
    * @retval 无
  */
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);

#endif
