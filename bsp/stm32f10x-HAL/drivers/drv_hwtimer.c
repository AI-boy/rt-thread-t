/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author           Notes
 * 2015-09-02     heyuanjie87      the first version
 * 2019-01-30     caiweikai        add
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_hwtimer.h"



#ifdef RT_USING_HWTIMER

TIM_HandleTypeDef htim2;

void NVIC_Configuration(void)
{
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);  
}

void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		TIM_MasterConfigTypeDef sMasterConfig = {0};
		
    TIM_TypeDef *tim;

    tim = (TIM_TypeDef *)(timer->parent.user_data);
		
		htim2.Instance = tim ;
		htim2.Init.Prescaler = 72;    //36M/36=1MHZ 
		htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim2.Init.Period = 1000;     //1000us=1ms 周期
		htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
		__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);

    if (state == 1)
    {			
				__HAL_RCC_TIM2_CLK_ENABLE(); //使能外设TIM2时钟---！！！！必须先使能时钟后方可操作外设寄存器
			
				NVIC_Configuration();
				HAL_TIM_Base_Init(&htim2);
			
				sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
				HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
			
				sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
				sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
				HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    }
}

rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{
    TIM_TypeDef *tim;
    rt_uint32_t mode;
	
    tim = (TIM_TypeDef *)(timer->parent.user_data);
	
    htim2.Instance = tim ;
	
		htim2.Init.Period = t;  //!!!!!
	
		__HAL_TIM_SET_AUTORELOAD(&htim2,t);
	
		mode = (opmode == HWTIMER_MODE_ONESHOT)? TIM_OPMODE_SINGLE : TIM_OPMODE_REPETITIVE;
	
		HAL_TIM_OnePulse_Init(&htim2,mode);		
	
		HAL_TIM_Base_Start(&htim2);     //开启定时器
    
    return RT_EOK;
}

void timer_stop(rt_hwtimer_t *timer)
{
    TIM_TypeDef *tim;

    tim = (TIM_TypeDef *)(timer->parent.user_data);
	
		htim2.Instance = tim ;
	
    HAL_TIM_Base_Stop(&htim2);
}

rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    TIM_TypeDef *tim;
    rt_err_t err = RT_EOK;

    tim = (TIM_TypeDef *)(timer->parent.user_data);
	
		htim2.Instance = tim ;
	
    switch (cmd)
    {
				case HWTIMER_CTRL_FREQ_SET:
						{
							  uint32_t clk;
								uint16_t val;
								rt_uint32_t freq;

								clk=HAL_RCC_GetPCLK1Freq();
								clk=clk*2;
							
								freq = *((rt_uint32_t*)arg);
	
								val = clk/freq; //分频数值  							
								htim2.Init.Prescaler=val;							
								HAL_TIM_Base_Stop_IT(&htim2);
								htim2.Instance->PSC = (uint32_t)val;
								__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
								HAL_TIM_Base_Start_IT(&htim2);   //使能定时器中断
						}
						break;
				default:
						{
								err = -RT_ENOSYS;
						}
						break;
    }

    return err;
}

rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
    TIM_TypeDef *tim;

    tim = (TIM_TypeDef *)(timer->parent.user_data);
	
		//htim = (TIM_HandleTypeDef *)tim;
		htim2.Instance = tim ;

    return ((rt_uint32_t)__HAL_TIM_GET_COUNTER(&htim2));
}

const struct rt_hwtimer_info _info =
{
    1000000,           /* the maximum count frequency can be set */
    1000,              /* the minimum count frequency can be set */
    0xFFFF,            /* the maximum counter value */
    HWTIMER_CNTMODE_UP,/* Increment or Decreasing count mode */
};

const struct rt_hwtimer_ops _ops =
{
    timer_init,
    timer_start,
    timer_stop,
    timer_counter_get,
    timer_ctrl,
};

static rt_hwtimer_t hw_timer0;

int stm32_hwtimer_init(void)
{
    hw_timer0.info = &_info;
    hw_timer0.ops  = &_ops;

    rt_device_hwtimer_register(&hw_timer0, "timer0", TIM2);

    return 0;
}

void TIM2_IRQHandler(void)
{
     /* TIM Update event */
	
  if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) !=RESET)
    {
      __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
			
			rt_device_hwtimer_isr(&hw_timer0);
    }
  }
}

INIT_BOARD_EXPORT(stm32_hwtimer_init);
#endif
