
#include "bsp_delay.h"

void delay_init(void)
{
    GPT1->CR = 0; /* 计数清零 */
    GPT1->CR = 1 << 15; /* 软件复位 */
    while((GPT1->CR) & (1 << 15)); /* 等待复位完成 */
    
    /*
   	 * GPT的CR寄存器,GPT通用设置
   	 * bit22:20	000 输出比较1的输出功能关闭，也就是对应的引脚没反应
     * bit9:    0   Restart模式,当CNT等于OCR1的时候就产生中断
     * bit8:6   001 GPT时钟源选择ipg_clk=66Mhz
     * bit
  	 */
	GPT1->CR = (1<<6);

    /*
     * GPT的PR寄存器，GPT的分频设置
     * bit11:0  设置分频值，设置为0表示1分频，
     *          以此类推，最大可以设置为0XFFF，也就是最大4096分频
	 */
	GPT1->PR = 65;	/* 设置为65，即66分频，因此GPT1时钟为66M/(65+1)=1MHz */

    /*
      * GPT的OCR1寄存器，GPT的输出比较1比较计数值，
      *	GPT的时钟为1Mz，那么计数器每计一个值就是就是1us。
      * 为了实现较大的计数，我们将比较值设置为最大的0XFFFFFFFF,
      * 这样一次计满就是：0XFFFFFFFFus = 4294967296us = 4295s = 71.5min
      * 也就是说一次计满最多71.5分钟，存在溢出
	  */
	GPT1->OCR[0] = 0XFFFFFFFF;

	GPT1->CR |= 1<<0;			//使能GPT1

#if 0
	/*
     * GPT的PR寄存器，GPT的分频设置
     * bit11:0  设置分频值，设置为0表示1分频，
     *          以此类推，最大可以设置为0XFFF，也就是最大4096分频
	 */
	GPT1->PR = 65;	//设置为1，即65+1=66分频，因此GPT1时钟为66M/66=1MHz


	 /*
      * GPT的OCR1寄存器，GPT的输出比较1比较计数值，
      * 当GPT的计数值等于OCR1里面值时候，输出比较1就会发生中断
      * 这里定时500ms产生中断，因此就应该为1000000/2=500000;
	  */
	GPT1->OCR[0] = 500000;

	/*
     * GPT的IR寄存器，使能通道1的比较中断
     * bit0： 0 使能输出比较中断
	 */
	GPT1->IR |= 1 << 0;

	/*
     * 使能GIC里面相应的中断，并且注册中断处理函数
	 */
	GIC_EnableIRQ(GPT1_IRQn);	//使能GIC中对应的中断
	system_register_irqhandler(GPT1_IRQn, (system_irq_handler_t)gpt1_irqhandler, NULL);	//注册中断服务函数	
#endif
}

void delayus(volatile uint32_t us)
{
    uint64_t oldcnt = 0;
    uint64_t newcnt = 0;
    uint64_t tcntvalue = 0;

    oldcnt = GPT1->CNT;
    while(1)
    {
        newcnt = GPT1->CNT;
        if(newcnt != oldcnt)
        {
            if(newcnt > oldcnt) /* GPT是向上计数器，并且没有溢出 */
            {
                tcntvalue += newcnt - oldcnt;
            }
            else /* 发生溢出 */
            {
                tcntvalue += 0xffffffff - oldcnt + newcnt;
            }
            oldcnt = newcnt;
            if(tcntvalue >= us)
            {
                break;
            }
        }
    }
}

void delayms(volatile uint32_t ms)
{
    uint32_t i = 0;
    for(i = 0; i < ms; ++i)
    {
        delayus(1000);
    }
}

#if 0
/* 中断处理函数 */
void gpt1_irqhandler(void)
{ 
	static unsigned char state = 0;

	state = !state;

	/*
     * GPT的SR寄存器，状态寄存器
     * bit2： 1 输出比较1发生中断
	 */
	if(GPT1->SR & (1<<0)) 
	{
		led_switch(LED2, state);
	}
	
	GPT1->SR |= 1<<0; /* 清除中断标志位 */
}
#endif

/*
 * @description	: 短时间延时函数
 * @param - n	: 要延时循环次数(空操作循环次数，模式延时)
 * @return 		: 无
 */
void delay_short(volatile uint32_t n)
{
	while(n--){}
}

/*
 * @description	: 延时函数,在396Mhz的主频下
 * 			  	  延时时间大约为1ms
 * @param - n	: 要延时的ms数
 * @return 		: 无
 */
void delay(volatile uint32_t n)
{
	while(n--)
	{
		delay_short(0x7ff);
	}
}
