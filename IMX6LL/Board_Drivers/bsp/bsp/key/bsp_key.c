#include "bsp_key.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"

void key_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);

    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);

    gpio_pin_config_t key_config;
    key_config.direction = kGPIO_DigitalInput;
    gpio_init(GPIO1, 18, &key_config);
}


int32_t key_get_value(void)
{
    int32_t ret = 0;
    static uint8_t release = 1; //按键松开状态
    if((release == 1) && (gpio_pinread(GPIO1, 18) == 0)) //按键按下
    {
        delay(10);
        release = 0;
        while(gpio_pinread(GPIO1, 18) == 0);
            ret = KEY0_VALUE;
    }
    else if(gpio_pinread(GPIO1, 18) == 1)
    {
        ret = KEY_NONE;
        release = 1; //标记按键释放
    }
    
    return ret;
}