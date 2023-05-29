#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"

/*
 * @description	: mian函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/
    beep_init();
    key_init();

    int32_t key_value = 0;
    uint8_t beep_state = OFF;
    uint8_t led_state = OFF;
    uint8_t i = 0;

	while(1)			
	{	
		key_value = key_get_value();
        switch (key_value)
        {
        case KEY0_VALUE:
            beep_state = !beep_state;
            beep_switch(beep_state);
            break;
        default:
            break;
        }
        i++;
        if(50 == i)
        {
            i = 0;
            led_state = !led_state;
            led_switch(LED0, led_state);
        }
        delay(10);
	}

	return 0;
}
