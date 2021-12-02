/*
 * stimer.c
 *
 *  Created on: 2021年12月1日
 *      Author: hello
 */

#include <stdio.h>
#include "stimer.h"

volatile static uint32_t _stimer_ticks = 0;  ///< 定时器计数时基

static stimer_t* _stimer_list = NULL;        ///< 定时器列表

/**
 * 定时器句柄
 * @param  timer            定时器句柄
 * @param  name             定时器名称
 * @param  timeout_callback 定时器定时到达回调函数
 * @param  arg              传入定时器的参数
 * @param  ticks            定时器超时时间，ticks的单位取决于时基函数stimer_ticks的调用间隔时间，如果stimer_ticks每1ms调用一次，那么ticks单位就为1ms
 * @param  opt              定时器属性，=STIMER_OPT_SINGLE：单次定时器  =STIMER_OPT_REPEAT：周期定时器
 * @return                  =0：成功  =other：失败
 */
int stimer_init(stimer_t* timer, const char* name, void (*timeout_callback)(void* arg), void* arg, uint32_t ticks, int opt)
{
	timer->arg           = arg;
	timer->opt           = opt;
	timer->name          = name;
	timer->next          = NULL;
	timer->callback      = timeout_callback;
	timer->current_ticks = 0;
	timer->timeout_ticks = ticks;
	return 0;
}

/**
 * 启动定时器
 * @param  timer 定时器句柄
 * @return       =0：成功  <0：定时器不存在
 */
int stimer_start(stimer_t* timer)
{
	stimer_t* it = _stimer_list;
	while (it != NULL)
	{
		if (it == timer)    return -1;
		it = it->next;
	}
	timer->next = _stimer_list;
	_stimer_list = timer;
	timer->current_ticks = _stimer_ticks;
	return 0;
}

/**
 * 停止定时器
 * @param  timer 定时器句柄
 * @return       =0：成功  <0：定时器不存在
 */
int stimer_stop(stimer_t* timer)
{
	stimer_t** it = &_stimer_list;
	stimer_t* entry = NULL;
	while (*it != NULL)
	{
		entry = *it;
		if (*it == timer)
		{
			*it = entry->next;
			return 0;
		}
		it = &entry->next;
	}
	return -1;
}


/**
 * 重启定时器
 * @param  timer 定时器句柄
 * @return       =0：成功  <0：定时器不存在
 */
int stimer_restart(stimer_t* timer)
{
	stimer_t* entry = _stimer_list;
	while (entry != NULL)
	{
		if (entry == timer)
		{
			entry->current_ticks = _stimer_ticks;  // 重新开始计时
			return 0;
		}
		entry = entry->next;
	}
	return -1;
}

/**
 * 获取当前定时器已定时时间
 * @param  timer 定时器句柄
 * @return       当前定时时间
 */
uint32_t stimer_getticks(const stimer_t* timer)
{
	int tmp = _stimer_ticks - timer->current_ticks;
	if (tmp < 0) tmp += UINT32_MAX;
	return (uint32_t)tmp;
}

/// 定时器轮训函数，放置于main函数的while死循环中
void stimer_poll(void)
{
	stimer_t* entry = _stimer_list;
	while (entry != NULL)
	{
		if (((int)(_stimer_ticks - entry->current_ticks)) >= entry->timeout_ticks)
		{
			if (entry->opt == STIMER_OPT_SINGLE)
			{
				stimer_stop(entry);
			}
			entry->callback(entry->arg);
			entry->current_ticks = _stimer_ticks;
		}
		entry = entry->next;
	}
}

/// 定时器时基函数
void stimer_ticks(void)
{
	_stimer_ticks++;
}

////////////////////////////////////////////////////////////////////////////
///
///                  示例程序
///

#if 0

stimer_t UARTTimer;
stimer_t LED0Timer;

void LED0TimerCallback(void* arg)
{
	HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}

void UARTTimerCallback(void* arg)
{
	stimer_t* timer = (stimer_t*)arg;
	printf("!!! timer name:%s !!!\r\n", __FUNCTION__);
	stimer_start(timer);  // 再次启动定时器
}

int main()
{
	stimer_init(
	    &LED0Timer,                             // 定时器句柄
	    "LED0Timer",                            // 定时器名称
	    LED0TimerCallback,                      // 定时器溢出回调函数
	    &LED0Timer,                             // 参数
	    500,                                    // 定时周期
	    STIMER_OPT_REPEAT                       // 周期模式
	);
	stimer_start(&LED0Timer);                   // 启动定时器

	stimer_init(
	    &UARTTimer,                             // 定时器句柄
	    "UARTTimer",                            // 定时器名称
	    UARTTimerCallback,                      // 定时器溢出回调函数
	    &UARTTimer,                             // 参数
	    1000,                                   // 定时周期
	    STIMER_OPT_SINGLE                       // 单次模式
	);
	stimer_start(&UARTTimer);                   // 启动定时器

	for (;;)
	{
		stimer_poll();
	}
}

#endif

