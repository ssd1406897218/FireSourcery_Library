/**************************************************************************/
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
/**************************************************************************/
/**************************************************************************/
/*!
    @file 	BEMF.h
    @author FireSoucery
    @brief 	Bemf monitor and reporting algorithms

	Implement sensorless 6-step, 2-phase active, algorithm.

    @version V0
*/
/**************************************************************************/
#ifndef BEMF_H
#define BEMF_H

#include "HAL.h"
#include "Config.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef CONFIG_BEMF_ADC_8
	typedef uint8_t bemf_t;
#elif defined(CONFIG_BEMF_ADC_16)
	typedef uint16_t bemf_t;
#endif

typedef enum
{
	BEMF_SAMPLE_MODE_PWM_ON,
	BEMF_SAMPLE_MODE_PWM_OFF,
	BEMF_SAMPLE_MODE_PWM_MIXED,
	BEMF_SAMPLE_MODE_PWM_BIPOLAR,
	BEMF_SAMPLE_MODE_INTEGRAL,
}
BEMF_SampleMode_T;

typedef enum
{
	BEMF_MODE_PASSIVE, // disable ZCD calc
	BEMF_MODE_OBSERVE,
	BEMF_MODE_COMMUTATION,
}
BEMF_Mode_T;

typedef enum
{
	BEMF_ZCD_STATUS_FALSE,
	BEMF_ZCD_STATUS_TRUE,
	BEMF_ZCD_STATUS_MISSED,
}
BEMF_ZcdStatus_T;

typedef enum
{
	BEMF_PHASE_A,
	BEMF_PHASE_B,
	BEMF_PHASE_C,
}
BEMF_PhaseId_T;


typedef struct
{
	const HAL_BEMF_T * p_HAL_Bemf;
//	uin32_t TimerFreq;

	volatile const uint32_t * p_Timer;

	volatile const bemf_t * p_VPhaseA_ADCU;		/// User app provides pointer to ADC results. only valid during its off phase.
	volatile const bemf_t * p_VPhaseB_ADCU;
	volatile const bemf_t * p_VPhaseC_ADCU;
	volatile const bemf_t * p_VBus_ADCU;

	volatile const bemf_t * volatile p_VBemf_ADCU;	///observe phase.  Variable pointer to ADC results. Phase voltage PWM On: VPhase == (3/2)VBEMF + VBus/2

	volatile BEMF_PhaseId_T BemfPhase;

	volatile int16_t VBemf_SignedADCU;				/// Phase-to-ground voltage, 3/2 * phase to-neutral
	volatile int16_t VBemfPrev_SignedADCU;			/// better to save 2 samples, delta is only used for zc

	volatile bool IsBemfRising;
	volatile bool IsZeroCrossingDetectionComplete;
	volatile bool IsCommutation;

	volatile BEMF_SampleMode_T SampleMode;
	volatile BEMF_Mode_T Mode; 					/// UseSensorlessCommutation


	//Time in Timer Ticks
	volatile uint32_t TimeBemf;						/// sample time stamp, from commutation
	volatile uint32_t TimeBemfPrev;					/// sample time stamp, from commutation
	volatile uint32_t TimeZeroCrossing; 			/// Time between prev commutation and zero crossing, 30 degrees.
	volatile uint32_t TimeZeroCrossingPeriod; 		/// Time between previous 2 zero crossings, 60 degrees.
	volatile uint16_t TimeBlank;					/// Blank time after commutation. aka "demagnetization time".



	volatile uint16_t PhaseAdvanceTime;			/// Commutation delay reduction. phase advance for high freq when voltage leads current. proportional to inductance.
	uint16_t ZeroCrossingThreshold_ADCU;		/// set value other than zero
	//uint16_t BlankTimeScalar;
	//uint16_t UsePWMOnThreshold;			/// for mixed sample mode, use PWM On ZCD when PWM On cycle is above this time.
	//uin16t_t DiodeDrop; //if no complementry pwm vd/2

	volatile uint32_t TimeCommutation; 			/// use for polling method, commutation time stamp, not needed if isr generated and counter resets to 0,
}
BEMF_T;


/*
	Set from outside method
 */
static inline void BEMF_MapPhaseA(BEMF_T * p_bemf) {p_bemf->p_VBemf_ADCU = p_bemf->p_VPhaseA_ADCU; p_bemf->BemfPhase = BEMF_PHASE_A;}
static inline void BEMF_MapPhaseB(BEMF_T * p_bemf) {p_bemf->p_VBemf_ADCU = p_bemf->p_VPhaseB_ADCU; p_bemf->BemfPhase = BEMF_PHASE_B;}
static inline void BEMF_MapPhaseC(BEMF_T * p_bemf) {p_bemf->p_VBemf_ADCU = p_bemf->p_VPhaseC_ADCU; p_bemf->BemfPhase = BEMF_PHASE_C;}

static inline void BEMF_MapCwPhaseAC_IO(BEMF_T * p_bemf){BEMF_MapPhaseB(p_bemf); p_bemf->IsBemfRising = true;} //todo
static inline void BEMF_MapCwPhaseBC_IO(BEMF_T * p_bemf){BEMF_MapPhaseA(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCwPhaseBA_IO(BEMF_T * p_bemf){BEMF_MapPhaseC(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCwPhaseCA_IO(BEMF_T * p_bemf){BEMF_MapPhaseB(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCwPhaseCB_IO(BEMF_T * p_bemf){BEMF_MapPhaseA(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCwPhaseAB_IO(BEMF_T * p_bemf){BEMF_MapPhaseC(p_bemf); p_bemf->IsBemfRising = true;}

static inline void BEMF_MapCcwPhaseAC_IO(BEMF_T * p_bemf){BEMF_MapPhaseB(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCcwPhaseBC_IO(BEMF_T * p_bemf){BEMF_MapPhaseA(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCcwPhaseBA_IO(BEMF_T * p_bemf){BEMF_MapPhaseC(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCcwPhaseCA_IO(BEMF_T * p_bemf){BEMF_MapPhaseB(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCcwPhaseCB_IO(BEMF_T * p_bemf){BEMF_MapPhaseA(p_bemf); p_bemf->IsBemfRising = true;}
static inline void BEMF_MapCcwPhaseAB_IO(BEMF_T * p_bemf){BEMF_MapPhaseC(p_bemf); p_bemf->IsBemfRising = true;}

static inline BEMF_PhaseId_T BEMF_GetPhaseId(BEMF_T * p_bemf) {return p_bemf->BemfPhase;}

/*
 * Module determines sector Id via zcd
 */
//static inline void BEMF_CaptureSector_IO(BEMF_T * p_bemf)
//{

//	switch(bemf->SectorId)
//	{
//	case SECTOR_1: //AC
//		BEMF_MapPhaseAC_IO();
//	}
//

//}


/*
	Filters are responsible of caller
	Phase-to-ground voltage, 3/2 * phase to-neutral

	BEMF_SAMPLE_MODE_PWM_ON -> need sufficient duty cycle to determine
	BEMF_SAMPLE_MODE_PWM_OFF -> Must subtract noise or use threshold. no 3/2 factor in this case

 */
static inline void BEMF_CaptureBemf(BEMF_T * p_bemf)
{
	p_bemf->VBemfPrev_SignedADCU = p_bemf->VBemf_SignedADCU;

	switch(p_bemf->SampleMode)
	{
	case BEMF_SAMPLE_MODE_PWM_BIPOLAR: break;
	case BEMF_SAMPLE_MODE_PWM_ON: 	p_bemf->VBemf_SignedADCU = (int32_t)(*p_bemf->p_VBemf_ADCU) - ((int32_t)(*p_bemf->p_VBus_ADCU) / 2); break;
	case BEMF_SAMPLE_MODE_PWM_OFF: 	p_bemf->VBemf_SignedADCU = (int32_t)(*p_bemf->p_VBemf_ADCU) - (int32_t)p_bemf->ZeroCrossingThreshold_ADCU; break;
	default:	break;
	}
}

/*
	Implicit Input:  IsBemfRising, Bemf phase
	software compare, todo hw comparator
 */
static inline bool BEMF_PollZeroCrossingDetection(BEMF_T * p_bemf)
{
	int16_t vBemf;
	int16_t vBemfPrev;
	uint32_t timeZeroCrossing; /* new zc time */
	bool isZcd = false;

	if (!p_bemf->IsZeroCrossingDetectionComplete)
	{
		if(!p_bemf->IsBemfRising) // invert falling bemf to use same rising zcd routine
		{
			vBemf 	= -p_bemf->VBemf_SignedADCU;
			vBemfPrev = -p_bemf->VBemfPrev_SignedADCU;
		}
		else
		{
			vBemf 	= p_bemf->VBemf_SignedADCU;
			vBemfPrev = p_bemf->VBemfPrev_SignedADCU;
		}

		if(vBemf >= p_bemf->ZeroCrossingThreshold_ADCU) // is zero crossing, calc time of zc
		{
			// optionally account for error of -(p_bemf->TimeBemf - p_bemf->TimeZeroCrossing) past zc time,
			if ((vBemf - vBemfPrev) > vBemf) // zcd falls within 2 samples
			{
				timeZeroCrossing = p_bemf->TimeBemf - (((p_bemf->TimeBemf - p_bemf->TimeBemfPrev) * vBemf / (vBemf - vBemfPrev))); //estimate using slope
			}
			else //missed zcd
			{
				timeZeroCrossing = p_bemf->TimeBemf - ((p_bemf->TimeBemf - p_bemf->TimeBemfPrev) / 2); //assume average
			}

			//			p_bemf->TimeZeroCrossingPeriod = timeZeroCrossing + p_bemf->TimeZeroCrossing;
			//			p_bemf->TimeZeroCrossing = timeZeroCrossing;
			// average zc with last zc
			p_bemf->TimeZeroCrossingPeriod = (p_bemf->TimeZeroCrossingPeriod + (p_bemf->TimeZeroCrossing + timeZeroCrossing)) / 2;
			p_bemf->TimeZeroCrossing = (p_bemf->TimeZeroCrossing + timeZeroCrossing) / 2;

			p_bemf->IsZeroCrossingDetectionComplete = true;
			isZcd = true;
		}
	}

	return isZcd;
}

static inline bool BEMF_PollTimeZeroCrossing_IO(BEMF_T * p_bemf)
{
//	uint32_t time = HAL_BEMF_ReadTimer(p_bemf->p_HAL_Bemf);
	bool newData = false;
	uint32_t time = *p_bemf->p_Timer;

	p_bemf->TimeBemfPrev 			= p_bemf->TimeBemf;
	p_bemf->TimeBemf 				= time;

	if (time > p_bemf->TimeBlank) // || passive mode
	{
		BEMF_CaptureBemf(p_bemf);
		if (BEMF_PollZeroCrossingDetection(p_bemf))
		{
			//HAL_BEMF_StartTimerInterrupt(p_bemf->p_HAL_Bemf, p_bemf->TimeZeroCrossing - p_bemf->PhaseAdvanceTime);
			p_bemf->TimeCommutation = p_bemf->TimeZeroCrossing - p_bemf->PhaseAdvanceTime;
			newData =  true;
		}
	}

	return newData;
}


/*
	On timer ISR
 */
static inline void BEMF_CaptureCommutation_IO(BEMF_T * p_bemf)
{
	p_bemf->IsZeroCrossingDetectionComplete = false;
	p_bemf->TimeBlank = p_bemf->TimeZeroCrossing / 2; //default case 25% of commutation time, 50% of zct. set 0 for passive mode?

	HAL_BEMF_StartTimerInterrupt(p_bemf->p_HAL_Bemf, p_bemf->TimeZeroCrossingPeriod); //back up time, if zcd miss

	//BEMF_ProcSector_IO(); determine next phase from inside, set from outside // select measure observe phase

#ifdef CONFIG_BEMF_COMMUTATION //report status otherwise
	p_bemf->Commutation(p_bemf->p_Data);
#endif

	p_bemf->IsCommutation = true;
}

/*
	Can be off by upto 1 PWM Period
 */
static inline bool BEMF_PollCommutation_IO(BEMF_T * p_bemf)
{
//	p_bemf->PhaseAdvanceTime = PwmPeriod_Ticks/2?
	if (HAL_BEMF_ReadTimer(p_bemf->p_HAL_Bemf) > p_bemf->TimeZeroCrossing)
	{
		BEMF_CaptureCommutation_IO(p_bemf);
		return true;
	}
}

static inline bool BEMF_GetCommutationPulse(BEMF_T * p_bemf)
{
	bool status;

	if (p_bemf->IsCommutation == true)
	{
		p_bemf->IsCommutation = false;
		status = true;
	}
	else
	{
		status = false;
	}

	return status;
}


#endif
