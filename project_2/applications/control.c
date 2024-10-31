#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_gpio.h>


#include <rtthread.h>
#include <rtdevice.h>

#define PWM_DEV_NAME        "pwm3"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL     1       /* PWM通道 */
#define CONTROL_PIN_2 GET_PIN(C,7)
typedef struct
{
    struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */
    struct rt_pwm_configuration cfg;    /* PWM配置 */
}control_pwm;

control_pwm control_dev_1;

int pwm_init(void)
{

    /* 查找设备 */
    control_dev_1.pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (control_dev_1.pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
        return RT_ERROR;
    }
    control_dev_1.cfg.channel = PWM_DEV_CHANNEL;
    control_dev_1.cfg.period = 500000;
    control_dev_1.cfg.pulse = 0;
    
    /* 设置PWM周期和脉冲宽度默认值 */
    rt_pwm_set(control_dev_1.pwm_dev, control_dev_1.cfg.channel, control_dev_1.cfg.period, control_dev_1.cfg.pulse);
    /* 使能设备 */
    rt_pwm_enable(control_dev_1.pwm_dev, control_dev_1.cfg.channel);

    rt_pin_mode(CONTROL_PIN_2,PIN_MODE_OUTPUT);
    rt_pin_write(CONTROL_PIN_2, PIN_LOW);
    return RT_EOK;
}
int pwm_set_duty(control_pwm * pwm_dev,float duty)
{
    pwm_dev->cfg.pulse = (rt_uint32_t)(pwm_dev->cfg.period * duty);
    
    return rt_pwm_set(pwm_dev->pwm_dev, PWM_DEV_CHANNEL, pwm_dev->cfg.period, pwm_dev->cfg.pulse);
}


static void control_front(void)
{
    pwm_set_duty(&control_dev_1,0.5);
}

static void control_back(void)
{
    rt_pwm_set(control_dev_1.pwm_dev, PWM_DEV_CHANNEL,control_dev_1.cfg.period, 0);
}

static void order_command(int value)
{
    switch (value)
    {
        case 87:
        case 68:
            rt_kprintf("front\n");
            control_front();
            break;
        case 65:
        case 51:
            rt_kprintf("back\n");
            control_back();
        default:
            control_back();
            break;
    }
}
void control_thread_entry(void *parameter)
{
    rt_mailbox_t control_mb = RT_NULL;
    while (control_mb == RT_NULL)
    {
        control_mb = (rt_mailbox_t)rt_object_find("control",RT_Object_Class_MailBox);
        rt_thread_mdelay(10);
    }
    int value = 0;
    while (1)
    {
       if(rt_mb_recv(control_mb,(rt_ubase_t*)&value,RT_WAITING_FOREVER) == RT_EOK)
       {
            order_command(value);
       }
    }
}

int control_pwm_init(void)
{
    rt_thread_t control_thread;
    pwm_init();
    control_thread = rt_thread_create("control",
                                      control_thread_entry,
                                      RT_NULL,
                                      1024,
                                      10,
                                      20);
    if (control_thread != RT_NULL)
    {
        rt_thread_startup(control_thread);
    }
    return RT_EOK;
}
INIT_APP_EXPORT(control_pwm_init);
