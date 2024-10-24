#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_gpio.h>
#define A_IN_1 GET_PIN(C,5)
#define A_IN_2 GET_PIN(B,0)
int contrl_font(void)
{
	rt_pin_mode(A_IN_1,PIN_MODE_OUTPUT);	
    rt_pin_mode(A_IN_2,PIN_MODE_OUTPUT);	
	rt_pin_write(A_IN_1,0);
    rt_pin_write(A_IN_2,1);

    return RT_EOK;
}

INIT_APP_EXPORT(contrl_font);


/*
 * 程序清单：这是一个 PWM 设备使用例程
 * 例程导出了 pwm_led_sample 命令到控制终端
 * 命令调用格式：pwm_led_sample
 * 程序功能：通过 PWM 设备控制 LED 灯的亮度，可以看到LED不停的由暗变到亮，然后又从亮变到暗。
*/

#include <rtthread.h>
#include <rtdevice.h>

#define PWM_DEV_NAME        "pwm3"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL     3       /* PWM通道 */

struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */

int pwm_led_sample(void)
{
    rt_uint32_t period, pulse, dir;

    period = 50000;    /* 周期为0.5ms，单位为纳秒ns */
    dir = 1;            /* PWM脉冲宽度值的增减方向 */
    pulse = 25000;          /* PWM脉冲宽度值，单位为纳秒ns */

    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }

    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
    /* 使能设备 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
    
    
}
