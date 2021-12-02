/*
 * stimer.c
 *
 *  Created on: 2021��12��1��
 *      Author: hello
 */

#include <stdio.h>
#include "stimer.h"

volatile static uint32_t _stimer_ticks = 0;  ///< ��ʱ������ʱ��

static stimer_t* _stimer_list = NULL;        ///< ��ʱ���б�

/**
 * ��ʱ�����
 * @param  timer            ��ʱ�����
 * @param  name             ��ʱ������
 * @param  timeout_callback ��ʱ����ʱ����ص�����
 * @param  arg              ���붨ʱ���Ĳ���
 * @param  ticks            ��ʱ����ʱʱ�䣬ticks�ĵ�λȡ����ʱ������stimer_ticks�ĵ��ü��ʱ�䣬���stimer_ticksÿ1ms����һ�Σ���ôticks��λ��Ϊ1ms
 * @param  opt              ��ʱ�����ԣ�=STIMER_OPT_SINGLE�����ζ�ʱ��  =STIMER_OPT_REPEAT�����ڶ�ʱ��
 * @return                  =0���ɹ�  =other��ʧ��
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
 * ������ʱ��
 * @param  timer ��ʱ�����
 * @return       =0���ɹ�  <0����ʱ��������
 */
int stimer_start(stimer_t* timer)
{
	stimer_t* it = _stimer_list;

	while (it != NULL)
	{
		if (it == timer)
		{
			return -1;
		}
		it = it->next;
	}

	timer->next = _stimer_list;
	_stimer_list = timer;

	timer->current_ticks = _stimer_ticks;

	return 0;
}

/**
 * ֹͣ��ʱ��
 * @param  timer ��ʱ�����
 * @return       =0���ɹ�  <0����ʱ��������
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
 * ������ʱ��
 * @param  timer ��ʱ�����
 * @return       =0���ɹ�  <0����ʱ��������
 */
int stimer_restart(stimer_t* timer)
{
	stimer_t* entry = _stimer_list;
	while (entry != NULL)
	{
		if (entry == timer)
		{
			entry->current_ticks = _stimer_ticks;  // ���¿�ʼ��ʱ
			return 0;
		}
		entry = entry->next;
	}
	return -1;
}

/**
 * ��ȡ��ǰ��ʱ���Ѷ�ʱʱ��
 * @param  timer ��ʱ�����
 * @return       ��ǰ��ʱʱ��
 */
uint32_t stimer_getticks(const stimer_t* timer)
{
	int tmp = _stimer_ticks - timer->current_ticks;
	if (tmp < 0) tmp += UINT32_MAX;
	return (uint32_t)tmp;
}

/// ��ʱ����ѵ������������main������while��ѭ����
void stimer_poll(void)
{
	int tmp;
	stimer_t* entry = _stimer_list;
	while (entry != NULL)
	{
		tmp = _stimer_ticks - entry->current_ticks;
		if (tmp < 0) // С�����ʱ����Ƿ�����uint32_t��ֵ�������
		{
			tmp += UINT32_MAX;
		}
		if (tmp >= entry->timeout_ticks)
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

/// ��ʱ��ʱ������
void stimer_ticks(void)
{
	_stimer_ticks++;
}

////////////////////////////////////////////////////////////////////////////
///
///                  ʾ������
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
	stimer_start(timer);  // �ٴ�������ʱ��
}

int main()
{
	stimer_init(
	    &LED0Timer,                             // ��ʱ�����
	    "LED0Timer",                            // ��ʱ������
	    LED0TimerCallback,                      // ��ʱ������ص�����
	    &LED0Timer,                             // ����
	    500,                                    // ��ʱ����
	    STIMER_OPT_REPEAT                       // ����ģʽ
	);
	stimer_start(&LED0Timer);                   // ������ʱ��

	stimer_init(
	    &UARTTimer,                             // ��ʱ�����
	    "UARTTimer",                            // ��ʱ������
	    UARTTimerCallback,                      // ��ʱ������ص�����
	    &UARTTimer,                             // ����
	    1000,                                   // ��ʱ����
	    STIMER_OPT_SINGLE                       // ����ģʽ
	);
	stimer_start(&UARTTimer);                   // ������ʱ��

	for (;;)
	{
		stimer_poll();
	}
}

#endif

