#include "bsp_int.h"
#include "core_ca7.h"

static uint32_t irqNesting; //中断嵌套计数器
static  sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS]; //中断服务函数表

/**
* @brief  中断初始化函数
* @param  
* @retval 
*/
void int_init(void)
{
    GIC_Init();
    system_irqtable_init(); //初始化中断表
    __set_VBAR((uint32_t)0x87800000); //设置中断向量表偏移
}

/**
* @brief  初始化中断服务函数表
* @param  
* @retval 
*/
void system_irqtable_init(void)
{
    irqNesting = 0;
    for(uint32_t i = 0; i < NUMBER_OF_INT_VECTORS; ++i)
    {
        system_register_irq_handler((IRQn_Type)i, default_irq_handler, NULL);
    }
}

/**
* @brief  给指定的中断号注册中断服务函数
* @param  irq 注册的中断号
* @param  handler 注册的中断处理函数
* @param  userParam 中断服务处理函数
* @retval 
*/
void system_register_irq_handler(IRQn_Type irq,
                                system_irq_handler_t handler,
                                void *userParam)
{
    irqTable[irq].irqHandler = handler;
    irqTable[irq].userParam = userParam;
}

/**
* @brief  C语言中断服务函数
* @param  中断号
* @retval 
*/
void system_irq_handler(uint32_t giccIar)
{
    uint32_t intNum = giccIar & 0x3FFUL;

    //检查中断号是否符合要求
    if((intNum == 1020) || (intNum >= NUMBER_OF_INT_VECTORS))
    {
        return;
    }
    irqNesting++;//中断嵌套计数器加一
    irqTable[intNum].irqHandler(intNum, irqTable[intNum].userParam);
    irqNesting--;
}

/**
* @brief  默认中断服务函数
* @param  
* @retval 
*/
void default_irq_handler(uint32_t giccIar, void* userParam)
{
    while(1)
    {

    }
}