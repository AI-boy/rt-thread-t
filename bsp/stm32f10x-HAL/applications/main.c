/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"


int main(void)
{
		/*user app*/
	
		rt_pin_mode(2, PIN_MODE_OUTPUT);
		
	
    return 0;
}
