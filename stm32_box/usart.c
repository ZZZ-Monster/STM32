
#include "stm32f10x.h"
#include "usart.h"

uint8_t Temp;

void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 打开串口外设的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA , &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 115200;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART1, &USART_InitStructure);
	
	// 串口中断优先级配置
  
	  /* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	  
	  /* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	  /* 抢断优先级*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	  /* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	  /* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  /* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
	
	// 使能串口接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	
	
	// 使能串口
	USART_Cmd(USART1, ENABLE);	    
}


/*****************  发送一个字节（发送一个字符） **********************/
void Usart_SendByte(uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(USART1,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	
}



/*****************  发送字符串 **********************/
void Usart_SendString(char str[])
{
	unsigned int k=0;
	for(k=0;str[k]!='\0';k++)
	{
		Usart_SendByte(str[k]);
	}
  /* 等待发送完成 */
  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
}



/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord(uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(USART1,temp_h);	
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(USART1,temp_l);	
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	
}


// 串口中断服务函数
void USART1_IRQHandler(void) 
{
	
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{		
		Temp = USART_ReceiveData(USART1);
		USART_SendData(USART1,Temp);    
	}	 
}


///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(USART1, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(USART1);
}


