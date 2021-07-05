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
    @file 	Motor_Step.h
    @author FireSoucery
    @brief  Motor Six Step submodule.
    		Six Step commutation strategy. Polar PWM
    @version V0
*/
/**************************************************************************/
#ifndef MOTOR_SIXSTEP_H
#define MOTOR_SIXSTEP_H

#include "Motor.h"
#include "Config.h"

#include "HAL_Motor.h"

#include "Transducer/Hall/Hall.h"
#include "Transducer/BEMF/BEMF.h"
#include "Transducer/Phase/Phase.h"

#include "Transducer/Encoder/Encoder_DeltaT.h"
//#include "Transducer/Encoder/Encoder_DeltaD.h"
#include "Transducer/Encoder/Encoder_Motor.h"
#include "Transducer/Encoder/Encoder.h"

//#include "Math/Linear/Linear_Voltage.h"
//#include "Math/Linear/Linear_ADC.h"
#include "Math/Linear/Linear_Ramp.h"
#include "Math/Linear/Linear.h"

#include <stdint.h>
#include <stdbool.h>


/*
 * SixStep Capture includes time stamp
 * on adc isr
 * must be inside adc isr for capture time
 */

// capture phase mapped to bemf module
static inline void Motor_SixStep_CaptureBemf_IO(Motor_T * p_motor)
{
	BEMF_PollTimeCaptureVPhaseObserve_IO(&p_motor->Bemf);
}

//capture set by adc
static inline void Motor_SixStep_CaptureBemfA_IO(Motor_T * p_motor)
{
	BEMF_PollTimeCaptureVPhaseA_IO(&p_motor->Bemf);
}

static inline void Motor_SixStep_CaptureBemfB_IO(Motor_T * p_motor)
{
	BEMF_PollTimeCaptureVPhaseB_IO(&p_motor->Bemf);
}

static inline void Motor_SixStep_CaptureBemfC_IO(Motor_T * p_motor)
{
	BEMF_PollTimeCaptureVPhaseC_IO(&p_motor->Bemf);
}


static inline void Motor_SixStep_ActivateSector(Motor_T * p_motor)
{
	if(p_motor->IsActiveControl) {Phase_Polar_SetDutyCyle(&p_motor->Phase, p_motor->VPwm);}

	switch (p_motor->CommutationSector)
	{
	case MOTOR_SECTOR_ID_0:
		//set error
		break;

	case MOTOR_SECTOR_ID_1: //Phase AC
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateAC(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_2; BEMF_MapCcwPhaseAC_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_6; BEMF_MapCwPhaseAC_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfB(p_motor); //sets as conversion set for commutation cycle
		break;

	case MOTOR_SECTOR_ID_2:
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateBC(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_3; BEMF_MapCcwPhaseBC_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_1; BEMF_MapCwPhaseBC_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfA(p_motor);
		break;

	case MOTOR_SECTOR_ID_3:
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateBA(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_4; BEMF_MapCcwPhaseBA_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_2; BEMF_MapCwPhaseBA_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfC(p_motor);
		break;

	case MOTOR_SECTOR_ID_4:
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateCA(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_5; BEMF_MapCcwPhaseCA_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_3; BEMF_MapCwPhaseCA_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfB(p_motor);
		break;

	case MOTOR_SECTOR_ID_5:
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateCB(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_6; BEMF_MapCcwPhaseCA_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_4; BEMF_MapCwPhaseCA_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfA(p_motor);
		break;

	case MOTOR_SECTOR_ID_6:
		if(p_motor->IsActiveControl) {Phase_Polar_ActivateAB(&p_motor->Phase);}
		if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_1; BEMF_MapCcwPhaseAB_IO(&p_motor->Bemf);}
		else											{p_motor->NextSector = MOTOR_SECTOR_ID_5; BEMF_MapCwPhaseAB_IO(&p_motor->Bemf);}

		HAL_Motor_LoadConversionBemfC(p_motor);
		break;

	case MOTOR_SECTOR_ID_7:
		//set error
		break;

	default:
		break;
	}
}

static inline bool Motor_SixStep_PollOpenLoop(Motor_T * p_motor)
{
	bool commutation;

	if (Thread_PollTimerCompletePeriodic(&p_motor->ControlTimerThread) == true)
	{
		p_motor->OpenLoopSpeed_RPM = Linear_Ramp_CalcTarget_IncIndex(&p_motor->OpenLoopRamp, &p_motor->OpenLoopRampIndex, p_motor->OpenLoopCommutationPeriod);
		p_motor->OpenLoopCommutationPeriod = Encoder_Motor_ConvertMechanicalRpmToControlResolution(&p_motor->Encoder, p_motor->OpenLoopSpeed_RPM);
		Thread_SetTimerPeriod(&p_motor->ControlTimerThread, p_motor->OpenLoopCommutationPeriod);
		commutation = true;
	}
	else
	{
		commutation = false;
	}

	return commutation;
}




/*
 * State Threads
 */

//state loop in 20khz pwm thread
//spin and freewheel state
static inline void Motor_SixStep_ProcPhaseControl(Motor_T * p_motor, Motor_SensorMode_T liveSensorMode)
{
	bool commutation = false;
	bool procControlVar = false;

	switch (liveSensorMode) //active sensor feedback
	{
	case MOTOR_SENSOR_MODE_OPEN_LOOP:
		if(p_motor->IsActiveControl)
		{
			if(Motor_SixStep_PollOpenLoop(p_motor))
			{
				p_motor->CommutationSector = p_motor->NextSector;
				commutation = true;
			}

			p_motor->VPwm = p_motor->UserCmd / 4U;
		}
		// open loop does not have resume from freewheel function, must wait till stop
		break;

	case MOTOR_SENSOR_MODE_BEMF:
		if (BEMF_PollCycleTimer(&p_motor->Bemf))
		{
			p_motor->CommutationSector = p_motor->NextSector;
			commutation = true;
		}

		if(p_motor->IsActiveControl)
		{
			procControlVar = true;
		}

		break;

	case MOTOR_SENSOR_MODE_HALL:
		if(Hall_PollSensorsEdge_IO(&p_motor->Hall))
		{
			p_motor->CommutationSector = Hall_GetCommutation(&p_motor->Hall);
			commutation = true;
		}

		if(p_motor->IsActiveControl)
		{
			procControlVar = true;
		}

		break;

	case MOTOR_SENSOR_MODE_ENCODER:
		//encoder use FOC
		break;

	default:
		break;
	}

	BEMF_PollZeroCrossingDetection(&p_motor->Bemf); //always poll zcd with last capture

	if(commutation)
	{
		Encoder_DeltaT_Capture_IO(&p_motor->Encoder);
		Encoder_DeltaT_CaptureExtension_IO(&p_motor->Encoder);
		BEMF_StartCycle_IO(&p_motor->Bemf);
	}

	if (procControlVar)
	{
		Motor_ProcRamp(p_motor);
		Motor_ProcControlVariable(p_motor);
	}

	Motor_SixStep_ActivateSector(p_motor);
}

static inline bool Motor_SixStep_GetBemfReliable(Motor_T * p_motor)
{
	bool isComplete;
	if (BEMF_GetZeroCrossingCounter(p_motor) > 10U)
	{
		isComplete = true;
	}
	else
	{
		isComplete = false;
	}

	return isComplete;
}

static inline void Motor_SixStep_StartOpenLoop(Motor_T * p_motor) //start active control
{
//	Motor_SixStep_SetPhaseControlActive(p_motor, true);

//	p_motor->IsActiveControl = true;
	BEMF_SetObserveMode(&p_motor->Bemf, BEMF_MODE_COMMUTATION);

	#define OPEN_LOOP_START_SPEED 20U
	#define OPEN_LOOP_SPEED 100U

	//openloop speed ramp	// must start at sufficient speed for timer driven fixed angle displacements
	Linear_Ramp_InitMillis(&p_motor->OpenLoopRamp, 20000U, OPEN_LOOP_START_SPEED, OPEN_LOOP_SPEED, 1000U);
	p_motor->OpenLoopRampIndex = 0U;

	p_motor->OpenLoopCommutationPeriod = 0U;
	Thread_SetTimerPeriod(&p_motor->ControlTimerThread, 0U); //next pwm will commutate

	//			if aligned 		//motor aligned to phase A
	//			if (p_motor->Direction == MOTOR_DIRECTION_CCW) 	{p_motor->NextSector = MOTOR_SECTOR_ID_2;}
	//			else											{p_motor->NextSector = MOTOR_SECTOR_ID_6;}

	p_motor->NextSector = MOTOR_SECTOR_ID_1;
}

//Start Spin from Stop
static inline void Motor_SixStep_StartSpin(Motor_T * p_motor) //start active control
{
//	p_motor->IsActiveControl = true;
//	if (p_motor->IsActiveControl == true)
//	{
//
//	}
//	else
//	{
//		error
//	}

	BEMF_SetObserveMode(&p_motor->Bemf, BEMF_MODE_COMMUTATION);

	p_motor->ControlTimerBase = 0U; //overflow precaution
//		Encoder_DeltaT_SetInitial(&p_motor->Encoder, 5U);

	switch (p_motor->Parameters.SensorMode)
	{

	case MOTOR_SENSOR_MODE_OPEN_LOOP:
		break;

	case MOTOR_SENSOR_MODE_BEMF:
//			p_motor->IsStartUp = true;
		break;

	case MOTOR_SENSOR_MODE_HALL:
		Hall_CaptureSensors_IO(&p_motor->Hall);
		p_motor->CommutationSector = Hall_GetCommutation(&p_motor->Hall);

		break;

//	case MOTOR_SENSOR_MODE_ENCODER: //encoder use foc
//		break;

	default:
		break;
	}

}

static inline void Motor_SixStep_StartFreewheel(Motor_T * p_motor) //start observe
{
	Motor_Float(p_motor);
//	p_motor->IsActiveControl = false;
	BEMF_SetObserveMode(&p_motor->Bemf, BEMF_MODE_PASSIVE); //no blank time
}



//void Motor_SixStep_Jog(Motor_T *p_motor)
//{
//	if (p_motor->JogSteps)
//	{
//		p_motor->JogSteps--;
//	Motor_SixStep_ActivateSector(p_motor, p_motor->NextSector);
//	}
//}

#endif




