#include <rtthread.h>
#include <rtdevice.h>
#include <cJSON.h>
#include "data.h"
#include "lora.h"
#define ADC_DEV_NAME        "adc1"  /* ADC 设备名称 */
#define REFER_VOLTAGE       330         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

ship_date data;

rt_uint32_t read_value(rt_adc_device_t dev, rt_int8_t channel)
{
	/* 使能设备 */
	rt_adc_enable(dev, channel);
	/* 读取采样值 */
	return rt_adc_read(dev, channel);
}
static void adc_thread_entry(void * pa)
{
	rt_adc_device_t adc_dev;            /* ADC 设备句柄 */
	/* 查找设备 */
	adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
	while (1)
	{
		data.r_lv = read_value(adc_dev,5) / 4096.0 * 3.3;
		data.w_lv = read_value(adc_dev,7) / 4096.0 * 3.3;
		data.tur_lv = read_value(adc_dev,6) / 819;

		cJSON * root = cJSON_CreateObject();
		cJSON_AddNumberToObject(root,"r_lv",data.r_lv);
		cJSON_AddNumberToObject(root,"w_lv",data.w_lv);
		cJSON_AddNumberToObject(root,"tur_lv",data.tur_lv);
		char *str = cJSON_Print(root);
        lora_send(str);
        cJSON_Delete(root);
        rt_free(str);
		rt_thread_mdelay(1000);
	}
	
}
int adc_dev_init(void)
{
	rt_thread_t adc_dev = RT_NULL;
	adc_dev = rt_thread_create("adc_dev",adc_thread_entry,RT_NULL,1024,10,10);
	if(adc_dev != RT_NULL)
	{
		rt_thread_startup(adc_dev);
	}
	return RT_EOK;
}
INIT_APP_EXPORT(adc_dev_init);
