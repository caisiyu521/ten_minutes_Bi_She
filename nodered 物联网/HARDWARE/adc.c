#include "adc.h"

float AD_Read(void){
	float temp;
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//使能指定的 ADC 的软件转换启动功能
	temp = ADC_GetConversionValue(ADC1)*3.3/0xfff;// stm32 的ADC是12位的 ，满量程是 0xfff，3.3/0xfff得到分辨率
	return temp;
}

void AD_Init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;
	
	//使能 ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//GPIOB.0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//模拟输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // 工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//规定了模数转换工作还是单次（单通道）模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//模数转换工作在连续模式。
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;// ADC 数据向右边对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;//规定了顺序进行规则转换的 ADC 通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);
	//设置指定 ADC 的规则组通道，设置它们的转化顺序和采样时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_13Cycles5);//ADC1,ADC 通道8,规则组采样顺序,采样时间为 13.5 周期
	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);//AWDOG 中断屏蔽
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);//重置指定的 ADC 的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));//获取 ADC 重置校准寄存器的状态
	ADC_StartCalibration(ADC1);//开始指定 ADC 的校准状态
	while(ADC_GetCalibrationStatus(ADC1));//获取指定 ADC 的校准程序
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//使能指定的 ADC 的软件转换启动功能
}




