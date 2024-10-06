/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */
 
#include "board.h"
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


#ifdef BSP_USING_DS18B20
#include <stdlib.h>
#include "drivers/sensor.h"
#include "sensor_dallas_ds18b20.h"



static void read_temp_entry(void *parameter)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t res;

    dev = rt_device_find(parameter);
    if (dev == RT_NULL)
    {
        rt_kprintf("Can't find device:%s\n", parameter);
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed!\n");
        return;
    }
    rt_device_control(dev, RT_SENSOR_CTRL_SET_ODR, (void *)100);

    while (1)
    {
        res = rt_device_read(dev, 0, &sensor_data, 1);
        if (res != 1)
        {
            rt_kprintf("read data failed!size is %d\n", res);
            rt_device_close(dev);
            return;
        }
        else
        {
            if (sensor_data.data.temp >= 0)
            {
                rt_kprintf("temp:%3d.%dC, timestamp:%5d\n",
                           sensor_data.data.temp / 10,
                           sensor_data.data.temp % 10,
                           sensor_data.timestamp);
            }
            else
            {
                rt_kprintf("temp:-%2d.%dC, timestamp:%5d\n",
                           abs(sensor_data.data.temp / 10),
                           abs(sensor_data.data.temp % 10),
                           sensor_data.timestamp);
            }
        }
        rt_thread_mdelay(100);
    }
}

static int ds18b20_read_temp_sample(void)
{
    rt_thread_t ds18b20_thread;

    ds18b20_thread = rt_thread_create("18b20tem",
                                      read_temp_entry,
                                      "temp_ds18b20",
                                      1024,
                                      RT_THREAD_PRIORITY_MAX / 2,
                                      20);
    if (ds18b20_thread != RT_NULL)
    {
        rt_thread_startup(ds18b20_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(ds18b20_read_temp_sample);

static int rt_hw_ds18b20_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.user_data = (void *)BSP_DS18B20_PIN;
    rt_hw_ds18b20_init("ds18b20", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_ds18b20_port);
#endif