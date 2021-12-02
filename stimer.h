/*
 * stimer.h
 *
 *  Created on: 2021Äê12ÔÂ1ÈÕ
 *      Author: hello
 */

#ifndef STIMER_H_
#define STIMER_H_

#include <stdint.h>

#define STIMER_OPT_SINGLE  1
#define STIMER_OPT_REPEAT  2

typedef struct _stimer
{
	const char* name;
	uint32_t timeout_ticks;
	uint32_t current_ticks;
	void (*callback)(void* arg);
	int opt;
	void* arg;
	struct _stimer* next;
}stimer_t;

typedef struct _stimer stimer_t;

int stimer_init(stimer_t* timer, const char* name, void (*timeout_callback)(void* arg), void* arg, uint32_t ticks, int opt);
int stimer_start(stimer_t* timer);
int stimer_stop(stimer_t* timer);
int stimer_restart(stimer_t* timer);
uint32_t stimer_getticks(const stimer_t* timer);

void stimer_poll(void);

void stimer_ticks(void);

#endif /* STIMER_H_ */
