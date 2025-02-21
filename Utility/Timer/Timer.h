/******************************************************************************/
/*!
	@section LICENSE

	Copyright (C) 2021 FireSoucery / The Firebrand Forge Inc

	This file is part of FireSourcery_Library (https://github.com/FireSourcery/FireSourcery_Library).

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/******************************************************************************/
/******************************************************************************/
/*!
	@file  	Timer.h
	@author FireSourcery
	@brief 	Elapsed time

	@version V0
 */
/******************************************************************************/
#ifndef TIMER_H
#define TIMER_H

#include "Config.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	const volatile uint32_t * const P_BASE; /* Base Timer */
	const uint32_t BASE_FREQ;
} Timer_Config_T;

typedef struct
{
	const Timer_Config_T CONFIG;
	uint32_t Period;			/* In Base Freq Ticks, 0 is Disable */
	bool IsOneShot;

	uint32_t TimeRef;
//	bool IsEnabled;
//	 uint32_t OneShotCount;
//	 uint32_t PeriodN;			/* In Base Freq Ticks, 0 is Loop infinite */
//	 uint32_t PeriodNCounter;	/* In Base Freq Ticks, 0 is Disable */
} Timer_T;

#define TIMER_CONFIG(p_Base, BaseFreq)						\
{															\
	.CONFIG = {.P_BASE = p_Base, .BASE_FREQ = BaseFreq, },	\
}

/******************************************************************************/
/*!
    @brief	Timer
*/
/******************************************************************************/
static inline void Timer_Restart(Timer_T * p_timer) 	{p_timer->TimeRef = *p_timer->CONFIG.P_BASE;}
//static inline void Timer_ZeroBase(Timer_T * p_timer) 	{p_timer->TimeRef = *p_timer->CONFIG.P_BASE = 0U;}

static inline uint32_t Timer_GetElapsed(Timer_T * p_timer)
{
	uint32_t ticks;

#ifdef CONFIG_TIMER_OVERFLOW_WRAP
	//Not necessarily needed if overflow time is in days. e.g using millis
	if (*p_timer->CONFIG.P_BASE < p_timer->TimeRef)
	{
		ticks = UINT32_MAX - p_timer->TimeRef + *p_timer->CONFIG.P_BASE
	}
	else
#else
	{
		ticks = *p_timer->CONFIG.P_BASE - p_timer->TimeRef;
	}
#endif

	return ticks;
}

static inline uint32_t Timer_GetElapsed_Ticks(Timer_T * p_timer) 	{return Timer_GetElapsed(p_timer);}
static inline uint32_t Timer_GetElapsed_Seconds(Timer_T * p_timer) 	{return Timer_GetElapsed_Ticks(p_timer) / p_timer->CONFIG.BASE_FREQ;}
static inline uint32_t Timer_GetElapsed_Millis(Timer_T * p_timer) 	{return Timer_GetElapsed_Ticks(p_timer) * 1000U / p_timer->CONFIG.BASE_FREQ;} //1 hour for overflow if timer is millis, 3 min for 20khz

static inline uint32_t Timer_GetElapsed_Micros(Timer_T * p_timer)
{
	uint32_t ticks = Timer_GetElapsed_Ticks(p_timer);
	uint32_t micros;

	if(ticks > UINT32_MAX / 1000000U)
	{
		micros = ticks / p_timer->CONFIG.BASE_FREQ * 1000000U;
	}
	else
	{
		micros = ticks * 1000000U / p_timer->CONFIG.BASE_FREQ;
	}

	return  micros;
}

static inline bool Timer_GetIsComplete(Timer_T * p_timer) 		{return (Timer_GetElapsed_Ticks(p_timer) >= p_timer->Period) ? true : false;}
static inline bool Timer_CheckComplete(Timer_T * p_timer) 		{return Timer_GetIsComplete(p_timer);}

//static inline uint32_t Timer_ConvertMillisToTicks(Timer_T * p_timer, uint32_t ms) {return (p_timer->CONFIG.BASE_FREQ * ms / 1000U);}
//static inline uint32_t Timer_GetFreq(Timer_T * p_timer) {return p_timer->CONFIG.BASE_FREQ / p_timer->Period;}

/*
 * freq != 0U
 * freq < Base Freq
 *
 * periodic mode only, unless support Nshot timer
 */
static inline void Timer_SetFreq			(Timer_T * p_timer, uint16_t freq)		{p_timer->Period = p_timer->CONFIG.BASE_FREQ / freq;}

static inline void Timer_SetPeriod			(Timer_T * p_timer, uint32_t ticks) 	{p_timer->Period = ticks;}
static inline void Timer_SetPeriod_Ticks	(Timer_T * p_timer, uint32_t ticks)		{Timer_SetPeriod(p_timer, ticks);}
static inline void Timer_SetPeriod_Millis	(Timer_T * p_timer, uint32_t millis)	{p_timer->Period = p_timer->CONFIG.BASE_FREQ * millis / 1000U;}

static inline void Timer_StartPeriod		(Timer_T * p_timer, uint32_t ticks) 	{Timer_SetPeriod(p_timer, ticks); Timer_Restart(p_timer);}
static inline void Timer_StartPeriod_Ticks	(Timer_T * p_timer, uint32_t ticks) 	{Timer_StartPeriod(p_timer, ticks);}
static inline void Timer_StartPeriod_Millis	(Timer_T * p_timer, uint32_t millis) 	{Timer_SetPeriod_Millis(p_timer, millis); Timer_Restart(p_timer);}

/*
 * Periodic Timer
 */
static inline bool Timer_PollPeriodic(Timer_T * p_timer)
{
	bool isComplete = Timer_CheckComplete(p_timer);

	if (isComplete == true)
	{
		Timer_Restart(p_timer);
	}

	return isComplete;
}

static inline void Timer_InitPeriodic		(Timer_T * p_timer, uint32_t ticks) 	{p_timer->IsOneShot = false; p_timer->Period = ticks;}

/*
 * OneShot Timer
 */
static inline bool Timer_PollOneShot(Timer_T * p_timer)
{
	bool isComplete = (p_timer->Period > 0U) && (Timer_CheckComplete(p_timer) == true);

	if (isComplete == true)
	{
		p_timer->Period = 0U;
	}

	return isComplete;
}

static inline void Timer_InitOneShot		(Timer_T * p_timer) {p_timer->IsOneShot = true; p_timer->Period = 0U; }
static inline void Timer_StartOneShot		(Timer_T * p_timer, uint32_t ticks) 	{Timer_StartPeriod(p_timer, ticks); }


/*
 * Flex Timer
 */
static inline bool Timer_Poll(Timer_T * p_timer)
{
	bool isComplete = (p_timer->Period > 0U) && (Timer_CheckComplete(p_timer) == true);

	if (isComplete == true)
	{
		(p_timer->IsOneShot == true) ? p_timer->Period = 0U : Timer_Restart(p_timer);
	}

	return isComplete;
}

static inline void Timer_Init(Timer_T * p_timer) { p_timer->IsOneShot = false;	p_timer->Period = 0U;}
static inline void Timer_SetPeriodic		(Timer_T * p_timer) {p_timer->IsOneShot = false;}
static inline void Timer_SetOneShot			(Timer_T * p_timer) {p_timer->IsOneShot = true; }
static inline bool Timer_GetIsOneShot		(Timer_T * p_timer) {return p_timer->IsOneShot;}

#endif /* TIMER_H */
