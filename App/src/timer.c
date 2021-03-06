#include "timer.h"
#include "usart.h"
#include "usart3.h"
#include "string.h"  
#include "stdlib.h"  
#include <stdio.h>
//extern void Reset_Device_Status(u8 status);

//定时器6中断服务程序		    
void TIM6_IRQHandler(void)
{
	u8 index;
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)					  //是更新中断
	{	
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  					//清除TIM6更新中断标志

		BSP_Printf("TIM6 1S interrupt\r\n");
		//BSP_Printf("TIM6_S HB: %d, HB TIMER: %d, Msg TIMEOUT: %d\r\n", dev.hb_count, dev.hb_timer, dev.msg_timeout);

#if 0	
		//再次发送心跳包的定时计数
		if(dev.hb_timer >= HB_1_MIN)
		{
			//for(index=DEVICE_01; index<DEVICEn; index++)
			{
				//BSP_Printf("TIM6 Dev[%d].total: %d, passed: %d\n", index, g_device_status[index].total, g_device_status[index].passed);
			}
			
			if(dev.status == CMD_IDLE)
			{
				printf("TIM6: HB Ready\r\n");
				dev.msg_recv = 0;	
				Reset_Device_Status(CMD_HB);
			}
			dev.hb_timer = 0;
		}
		else
			dev.hb_timer++;
#endif

		for(index=DEVICE_01; index<DEVICEn; index++)
		{
			switch(g_device_status[index].power)
			{
				case ON:
				{
					if(g_device_status[index].total==0)
					{
						BSP_Printf("Error: Dev[%d] %d %d %d\n", index, g_device_status[index].power, g_device_status[index].total, g_device_status[index].passed);
						g_device_status[index].power = UNKNOWN;
					}
					else
					{
						if(g_device_status[index].passed >= g_device_status[index].total)
						{
							BSP_Printf("Dev[%d]: %d %d %d\n", index, g_device_status[index].power, g_device_status[index].total, g_device_status[index].passed);					
							g_device_status[index].passed=g_device_status[index].total=0;
							g_device_status[index].power=OFF;
							Device_OFF(index);
						}
						else
							g_device_status[index].passed++;
					}
				}	
				break;
					
				case OFF:
					if(g_device_status[index].total!=0)
						g_device_status[index].power = UNKNOWN;
				break;
					
				case UNKNOWN:
				default:
				break;
			}
		}

#if 0	
		switch(dev.status)
		{
			case CMD_LOGIN:
				if(dev.msg_expect & MSG_DEV_LOGIN)
				{
					if(dev.reply_timeout >= HB_1_MIN)
					{
						dev.msg_expect &= ~MSG_DEV_LOGIN;
						dev.reply_timeout = 0;
						dev.msg_timeout++;
						BSP_Printf("[%d]: Ready to send Msg MSG_DEV_LOGIN again\n", dev.msg_timeout);
					}
					dev.reply_timeout++;
				}
			break;
			case CMD_HB:
				if(dev.msg_expect & MSG_DEV_HB)
				{
					if(dev.reply_timeout >= HB_1_MIN)
					{
						dev.msg_expect &= ~MSG_DEV_HB;
						dev.reply_timeout = 0;
						dev.msg_timeout++;
						BSP_Printf("[%d]: Ready to send Msg MSG_DEV_HB again\n", dev.msg_timeout);
					}
					dev.reply_timeout++;
				}				
			break;
			case CMD_CLOSE_DEVICE:
				if(dev.msg_expect & MSG_DEV_CLOSE)
				{
					if(dev.reply_timeout >= HB_1_MIN)
					{
						dev.msg_expect &= ~MSG_DEV_CLOSE;
						dev.reply_timeout = 0;
						dev.msg_timeout++;
						BSP_Printf("[%d]: Ready to send Msg MSG_DEV_CLOSE again\n", dev.msg_timeout);
					}
					dev.reply_timeout++;
				}				
			break;
			default:
			break;
		}

		if(dev.msg_timeout >= NUMBER_MSG_MAX_RETRY)
		{	
			BSP_Printf("Retry sending too many times, need reset\n");	
			dev.msg_timeout = 0;
			dev.need_reset = ERR_RETRY_TOO_MANY_TIMES;
		}	
		//BSP_Printf("TIM6_E Dev Status: %d, Msg expect: %d, Msg recv: %d\r\n", dev.status, dev.msg_expect, dev.msg_recv);
		//BSP_Printf("TIM6_E HB: %d, HB TIMER: %d, Msg TIMEOUT: %d\r\n", dev.hb_count, dev.hb_timer, dev.msg_timeout);
#endif		
		TIM_SetCounter(TIM6,0); 
	}
}

//通用定时器6中断初始化
//这里选择为APB1的1倍，而APB1为24M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//arr：自动重装值。
//psc：时钟预分频数	
void TIM6_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);				//TIM6时钟使能    
	
	//定时器TIM6初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE );                   //使能指定的TIM6中断,允许更新中断
	
	//TIM_Cmd(TIM6,ENABLE);//开启定时器6
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

//定时器7中断服务程序		    
void TIM7_IRQHandler(void)
{ 	

	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{	 		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //清除TIM7更新中断标志    
		USART3_RX_STA|=1<<15;	//标记接收完成
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7
		
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//添加结束符 

		BSP_Printf("USART BUF:%s\r\n",USART3_RX_BUF);
	}
}

//TIMER7的初始化 用在USART3（对接SIM800）的中断接收程序/////////
//通用定时器7中断初始化
//这里选择为APB1的1倍，而APB1为24M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
	
	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE );                   //使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM7,ENABLE);//开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		    	//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      	//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}
