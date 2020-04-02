#include "fy_mqtt.h"
#include "timer.h"

#include<stdlib.h>


//#define _DEBUG_MQTT

typedef enum
{
	//名字 	    值 			报文流动方向 	描述
	M_RESERVED1	=0	,	//	禁止	保留
	M_CONNECT		,	//	客户端到服务端	客户端请求连接服务端
	M_CONNACK		,	//	服务端到客户端	连接报文确认
	M_PUBLISH		,	//	两个方向都允许	发布消息
	M_PUBACK		,	//	两个方向都允许	QoS 1消息发布收到确认
	M_PUBREC		,	//	两个方向都允许	发布收到（保证交付第一步）
	M_PUBREL		,	//	两个方向都允许	发布释放（保证交付第二步）
	M_PUBCOMP		,	//	两个方向都允许	QoS 2消息发布完成（保证交互第三步）
	M_SUBSCRIBE		,	//	客户端到服务端	客户端订阅请求
	M_SUBACK		,	//	服务端到客户端	订阅请求报文确认
	M_UNSUBSCRIBE	,	//	客户端到服务端	客户端取消订阅请求
	M_UNSUBACK		,	//	服务端到客户端	取消订阅报文确认
	M_PINGREQ		,	//	客户端到服务端	心跳请求
	M_PINGRESP		,	//	服务端到客户端	心跳响应
	M_DISCONNECT	,	//	客户端到服务端	客户端断开连接
	M_RESERVED2		,	//	禁止	保留
}_typdef_mqtt_message;



//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00
const u8 parket_connetAck[] = {0x20,0x02,0x00,0x00};
const u8 parket_disconnet[] = {0xe0,0x00};
const u8 parket_heart[] = {0xc0,0x00};
const u8 parket_heart_reply[] = {0xc0,0x00};
const u8 parket_subAck[] = {0x90,0x03};

static void Mqtt_SendBuf(u8 *buf,u16 len);

static void Init(u8 *prx,u16 rxlen,u8 *ptx,u16 txlen);
static u8 Connect(char *ClientID,char *Username,char *Password);
static u8 SubscribeTopic(char *topic,u8 qos,u8 whether);
static u8 PublishData(char *topic, char *message, u8 qos);
static void SentHeart(void);
static void Disconnect(void);

_typdef_mqtt _mqtt = 
{
	0,0,
	0,0,
	Init,
	Connect,
	SubscribeTopic,
	PublishData,
	SentHeart,
	Disconnect,
};

static void Init(u8 *prx,u16 rxlen,u8 *ptx,u16 txlen)
{
	_mqtt.rxbuf = prx;_mqtt.rxlen = rxlen;
	_mqtt.txbuf = ptx;_mqtt.txlen = txlen;
	
	memset(_mqtt.rxbuf,0,_mqtt.rxlen);
	memset(_mqtt.txbuf,0,_mqtt.txlen);
	
	//无条件先主动断开
	_mqtt.Disconnect();Delay_ms(100);
	_mqtt.Disconnect();Delay_ms(100);
}


//连接服务器的打包函数
static u8 Connect(char *ClientID,char *Username,char *Password)
{
	u8 cnt=2;
	u8 wait;
    int ClientIDLen = strlen(ClientID);
    int UsernameLen = strlen(Username);
    int PasswordLen = strlen(Password);
    int DataLen;
	_mqtt.txlen=0;
	//可变报头+Payload  每个字段包含两个字节的长度标识
    DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);
	
	//固定报头
	//控制报文类型
    _mqtt.txbuf[_mqtt.txlen++] = 0x10;		//MQTT Message Type CONNECT
	//剩余长度(不包括固定头部)
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		_mqtt.txbuf[_mqtt.txlen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//可变报头
	//协议名
    _mqtt.txbuf[_mqtt.txlen++] = 0;        		// Protocol Name Length MSB    
    _mqtt.txbuf[_mqtt.txlen++] = 4;        		// Protocol Name Length LSB    
    _mqtt.txbuf[_mqtt.txlen++] = 'M';        	// ASCII Code for M    
    _mqtt.txbuf[_mqtt.txlen++] = 'Q';        	// ASCII Code for Q    
    _mqtt.txbuf[_mqtt.txlen++] = 'T';        	// ASCII Code for T    
    _mqtt.txbuf[_mqtt.txlen++] = 'T';        	// ASCII Code for T    
	//协议级别
    _mqtt.txbuf[_mqtt.txlen++] = 4;        		// MQTT Protocol version = 4    
	//连接标志
    _mqtt.txbuf[_mqtt.txlen++] = 0xc2;        	// conn flags 
    _mqtt.txbuf[_mqtt.txlen++] = 0;        		// Keep-alive Time Length MSB    
    _mqtt.txbuf[_mqtt.txlen++] = 60;        	// Keep-alive Time Length LSB  60S心跳包  
	
    _mqtt.txbuf[_mqtt.txlen++] = BYTE1(ClientIDLen);// Client ID length MSB    
    _mqtt.txbuf[_mqtt.txlen++] = BYTE0(ClientIDLen);// Client ID length LSB  	
	memcpy(&_mqtt.txbuf[_mqtt.txlen],ClientID,ClientIDLen);
    _mqtt.txlen += ClientIDLen;
    
    if(UsernameLen > 0)
    {   
        _mqtt.txbuf[_mqtt.txlen++] = BYTE1(UsernameLen);		//username length MSB    
        _mqtt.txbuf[_mqtt.txlen++] = BYTE0(UsernameLen);    	//username length LSB    
		memcpy(&_mqtt.txbuf[_mqtt.txlen],Username,UsernameLen);
        _mqtt.txlen += UsernameLen;
    }
    
    if(PasswordLen > 0)
    {    
        _mqtt.txbuf[_mqtt.txlen++] = BYTE1(PasswordLen);		//password length MSB    
        _mqtt.txbuf[_mqtt.txlen++] = BYTE0(PasswordLen);    	//password length LSB  
		memcpy(&_mqtt.txbuf[_mqtt.txlen],Password,PasswordLen);
        _mqtt.txlen += PasswordLen; 
    }  
	
	cnt=2;
	wait = 0;
	
	while(cnt--)
	{
		memset(_mqtt.rxbuf,0,_mqtt.rxlen);
		Mqtt_SendBuf(_mqtt.txbuf,_mqtt.txlen);
//		printf("%s\r\n",_mqtt.txbuf);
		wait=30;//等待3s时间
		while(wait--)
		{
//			printf("%s\r\n",_mqtt.rxbuf);
			//CONNECT
			if(_mqtt.rxbuf[0]==parket_connetAck[0] && _mqtt.rxbuf[1]==parket_connetAck[1]) //连接成功			   
			{
				printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
				return 1;//连接成功
			}
			Delay_ms(100);			
		}
	}
	return 0;
}


//MQTT订阅/取消订阅数据打包函数
//topic       主题 
//qos         消息等级 
//whether     订阅/取消订阅请求包
static u8 SubscribeTopic(char *topic,u8 qos,u8 whether)
{    
	u8 cnt=2;
	u8 wait;
    int topiclen = strlen(topic);
	
	int DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
	_mqtt.txlen=0;
	//固定报头
	//控制报文类型
    if(whether) _mqtt.txbuf[_mqtt.txlen++] = 0x82; //消息类型和标志订阅
    else	_mqtt.txbuf[_mqtt.txlen++] = 0xA2;    //取消订阅

	//剩余长度
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		_mqtt.txbuf[_mqtt.txlen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	//可变报头
    _mqtt.txbuf[_mqtt.txlen++] = 0;				//消息标识符 MSB
    _mqtt.txbuf[_mqtt.txlen++] = 0x01;           //消息标识符 LSB
	//有效载荷
    _mqtt.txbuf[_mqtt.txlen++] = BYTE1(topiclen);//主题长度 MSB
    _mqtt.txbuf[_mqtt.txlen++] = BYTE0(topiclen);//主题长度 LSB   
	memcpy(&_mqtt.txbuf[_mqtt.txlen],topic,topiclen);
    _mqtt.txlen += topiclen;
    
    if(whether)
    {
        _mqtt.txbuf[_mqtt.txlen++] = qos;//QoS级别
    }
	
	
	while(cnt--)
	{
		memset(_mqtt.rxbuf,0,_mqtt.rxlen);
		Mqtt_SendBuf(_mqtt.txbuf,_mqtt.txlen);
		wait=30;//等待3s时间
		while(wait--)
		{
			if(_mqtt.rxbuf[0]==parket_subAck[0] && _mqtt.rxbuf[1]==parket_subAck[1]) //订阅成功			   
			{
				return 1;//订阅成功
			}
			Delay_ms(100);			
		}
	}
	if(cnt) return 1;	//订阅成功
	return 0;
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
static u8 PublishData(char *topic, char *message, u8 qos)
{  
    int topicLength = strlen(topic);    
    int messageLength = strlen(message);     
    static u16 id=0;
	int DataLen;
	_mqtt.txlen=0;
	//有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
	//QOS为0时没有标识符
	//数据长度             主题名   报文标识符   有效载荷
    if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
    else	DataLen = (2+topicLength) + messageLength;   

    //固定报头
	//控制报文类型
    _mqtt.txbuf[_mqtt.txlen++] = 0x30;    // MQTT Message Type PUBLISH  

	//剩余长度
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		_mqtt.txbuf[_mqtt.txlen++] = encodedByte;
	}while ( DataLen > 0 );	
	
    _mqtt.txbuf[_mqtt.txlen++] = BYTE1(topicLength);//主题长度MSB
    _mqtt.txbuf[_mqtt.txlen++] = BYTE0(topicLength);//主题长度LSB 
	memcpy(&_mqtt.txbuf[_mqtt.txlen],topic,topicLength);//拷贝主题
    _mqtt.txlen += topicLength;
        
	//报文标识符
    if(qos)
    {
        _mqtt.txbuf[_mqtt.txlen++] = BYTE1(id);
        _mqtt.txbuf[_mqtt.txlen++] = BYTE0(id);
        id++;
    }
	memcpy(&_mqtt.txbuf[_mqtt.txlen],message,messageLength);
    _mqtt.txlen += messageLength;
        
	Mqtt_SendBuf(_mqtt.txbuf,_mqtt.txlen);
    return _mqtt.txlen;
}

static void SentHeart(void)
{
	Mqtt_SendBuf((u8 *)parket_heart,sizeof(parket_heart));
}

static void Disconnect(void)
{
	Mqtt_SendBuf((u8 *)parket_disconnet,sizeof(parket_disconnet));
}

static void Mqtt_SendBuf(u8 *buf,u16 len)
{
	#ifdef _DEBUG_MQTT
	Usart3_SendBuf(buf,len);
	#endif
	Usart3_SendBuf(buf,len);
	
}	

void Mqtt_Progress(u8 *buf,u16 len0){
	u8 buffer[buf_size] , len;
	len = find((char *)buf, '#', '#', (char *)buffer);
	if(len > 0){
        
		printf("buffer = %s , len %d\r\n", buffer, len);
//		
//		if(strstr((char *)buffer,"1") != NULL){
//			Led_Flag_shan = 0;
//		}
//		if(strstr((char *)buffer,"0")!= NULL){
//			Led_Flag_shan = 1;
//		}
		if(strstr((char *)buffer,"open")!=NULL){
			LED0 = 0;
//            printf("555555555555555555555555555555555555555\r\n");
		}
		else if(strstr((char *)buffer,"close")!=NULL){
			LED0 = 1;
//             printf("555555555555555555555555555555555555555\r\n");
		}
	}
	
	memset(buffer,0,sizeof(buffer));
//	free(buffer);
}

/*********************************************END OF FILE********************************************/


int find(char* buf, char q, char p, char* buffer) {
	int i = 0, j = 0, flag = 0;
	char yyy[buf_size];
	char buffer1[buf_size];
	for (i = 0; i < buf_size; i++) {
		if (buf[i] != '\0') {
			yyy[j++] = buf[i];
		}
	}
	yyy[j] = '\0';
	for (i = 0; i < (strlen(yyy)+1); i++) {
		*(buffer1 +i) = yyy[i];
	}
//	printf("筛除0 后的字符串 = %s ， 长度为 %d\r\n", buffer1, strlen(buffer1));
	j = 0;
	for (i = 0; i < strlen(buffer1); i++) {
		if (flag == 0) {
			if (buffer1[i] == q) {
				flag = 1;
			}
		}
		else if (flag == 1) {
			if (buffer1[i] == p) {
				flag = 2;
			}
		}
		if (flag == 1) {
			*(buffer + j) = buffer1[i];
			j++;
		}
		else if (flag == 2) {
			*(buffer + j) = buffer1[i];
			*(buffer + j + 1) = '\0';
			return j-1;
		}
	}
	return 0;
}






