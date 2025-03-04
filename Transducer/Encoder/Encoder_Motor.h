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
	@file 	Encoder_Motor.h
	@author FireSourcery
	@brief 	Encoder + pole pairs, electrical angle calculation
			Capture DeltaD, fixed DeltaT
			Angle defined to be  16 bits
	@version V0
 */
/******************************************************************************/
#ifndef ENCODER_MOTOR_H
#define ENCODER_MOTOR_H

#include "Encoder_DeltaT.h"
#include "Encoder_DeltaD.h"
#include "Encoder.h"

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
/*!
	Capture DeltaD Mode
	CounterD Angle Functions
	DeltaD measures mechanical angle
 */
/******************************************************************************/
static inline uint32_t Encoder_Motor_GetMechanicalDelta(Encoder_T * p_encoder)
{
	return Encoder_DeltaD_GetDeltaAngle(p_encoder);
}

/*!
	Electrical Delta Angle, change in Angle [Degree16s] per Control Period.
	MechanicalDelta * PolePairs
	Overflow threshold > 1 electrical cycle
 */
static inline uint32_t Encoder_Motor_GetElectricalDelta(Encoder_T * p_encoder)
{
	return Encoder_ConvertCounterDToAngle(p_encoder, p_encoder->DeltaD * p_encoder->Params.MotorPolePairs) ;
}

//static inline uint32_t Encoder_Motor_ConvertMechanicalRpmToDeltaD(Encoder_T * p_encoder, uint16_t mechRpm)
//{
//	return Encoder_ConvertRotationalSpeedToDeltaD_RPM(p_encoder, mechRpm);
//}
//
//static inline uint32_t Encoder_Motor_ConvertDeltaDToMechanicalRpm(Encoder_T * p_encoder, uint16_t deltaD_Ticks)
//{
//	return Encoder_ConvertDeltaDToRotationalSpeed_RPM(p_encoder, deltaD_Ticks);
//}

/*
 * 10k rpm, 20kHz => 546 ~= .83 percent of revolution
 */
static inline uint32_t Encoder_Motor_ConvertMechanicalRpmToMechanicalDelta(Encoder_T * p_encoder, uint16_t mechRpm)
{
	return Encoder_DeltaD_ConvertRotationalSpeedToDeltaAngle_RPM(p_encoder, mechRpm);
}

/*!
	Convert Mechanical Angular Speed [Revolutions per Minute] to Electrical Delta [Degree16s Per Control Period]

	Skips conversion through DeltaD
 */
static inline uint32_t Encoder_Motor_ConvertMechanicalRpmToElectricalDelta(Encoder_T * p_encoder, uint16_t mechRpm)
{
//	return (mechRpm << CONFIG_ENCODER_ANGLE_DEGREES_BITS) / 60U * p_encoder->Params.MotorPolePairs / p_encoder->UnitT_Freq;
	return Encoder_DeltaD_ConvertRotationalSpeedToDeltaAngle_RPM(p_encoder, mechRpm * p_encoder->Params.MotorPolePairs);
}

/*!
	Integrates speed to position, use ticks
 */
//handle inside module or outside?
//static inline void Encoder_Motor_IntegrateSpeed_RPM(Encoder_T * p_encoder, int32_t * p_theta, uint16_t speed_Rpm)
//{
//	uint32_t electricalDelta = Encoder_Motor_ConvertMechanicalRpmToElectricalDelta(p_encoder, speed_Rpm);
//	*p_theta += electricalDelta;
//}

/******************************************************************************/
/*!
	Capture DeltaT Mode
	Control Periods / Encoder Pulse > 1
	Interpolate Delta Angle
 */
/******************************************************************************/
/*!
	Hall => Use CountPerRevolution = polePairs*6. Capture on Commutation Step
 */
static inline uint32_t Encoder_Motor_InterpolateMechanicalDelta(Encoder_T * p_encoder, uint32_t pollingIndex)
{
	return Encoder_DeltaT_InterpolateAngle(p_encoder, pollingIndex);
}

static inline uint32_t Encoder_Motor_InterpolateElectricalDelta(Encoder_T * p_encoder, uint32_t pollingIndex)
{
	//todo check overflow boundaries
//	if (pollingIndex > UINT32_MAX / (p_encoder->UnitInterpolateAngle * p_encoder->Params.MotorPolePairs))
//	{
//		return Encoder_InterpolateAngle(p_encoder, pollingIndex) * p_encoder->Params.MotorPolePairs;
//	}
//	else
	{
		return Encoder_DeltaT_InterpolateAngle(p_encoder, pollingIndex * p_encoder->Params.MotorPolePairs);
	}
}

/*
 	 Control periods per encoder pulse, hall phase
 */
static inline uint32_t Encoder_Motor_ConvertMechanicalRpmToInterpolationFreq(Encoder_T * p_encoder, uint16_t mechRpm)
{
	return Encoder_DeltaT_ConvertRotationalSpeedToInterpolationFreq_RPM(p_encoder, mechRpm);
}

static inline uint32_t Encoder_Motor_ConvertInterpolationFreqToMechanicalRpm(Encoder_T * p_encoder, uint16_t controlPeriods)
{
	return Encoder_DeltaT_ConvertInterpolationFreqToRotationalSpeed_RPM(p_encoder, controlPeriods);
}

static inline uint32_t Encoder_Motor_GetInterpolationFreq(Encoder_T *p_encoder)
{
	return Encoder_DeltaT_GetInterpolationFreq(p_encoder);
}


/******************************************************************************/
/*!
	Both Capture Modes
 */
/******************************************************************************/
static inline uint32_t Encoder_Motor_GetMechanicalTheta(Encoder_T * p_encoder)
{
	return Encoder_GetAngle(p_encoder);
}

/*!
	Electrical Theta Angle, position Angle [Degree16s]
 */
static inline uint32_t Encoder_Motor_GetElectricalTheta(Encoder_T * p_encoder)
{
	return Encoder_ConvertCounterDToAngle(p_encoder, p_encoder->AngularD * p_encoder->Params.MotorPolePairs);
}


/******************************************************************************/
/*!
	Speed
 */
/******************************************************************************/
/*!
	Get Mechanical Angular Speed [Degree16s Per Second]
 */
static inline uint32_t Encoder_Motor_GetMechanicalSpeed(Encoder_T * p_encoder)
{
	return Encoder_GetAngularSpeed(p_encoder);
}

static inline uint32_t Encoder_Motor_GetMechanicalRpm(Encoder_T * p_encoder)
{
	/* Max DeltaD = 447, for UnitSpeed = 160000, PolerPairs = 12 */
	return Encoder_GetRotationalSpeed_RPM(p_encoder);
}

/*!
	Get Electrical Angular Speed [Degree16s Per Second]
 */
static inline uint32_t Encoder_Motor_GetElectricalSpeed(Encoder_T * p_encoder)
{
	return Encoder_Motor_GetMechanicalSpeed(p_encoder) * p_encoder->Params.MotorPolePairs;

	/* Max DeltaD = 2,236, for UnitSpeed = 160000, PolerPairs = 12 */
	//	return p_encoder->DeltaD * p_encoder->UnitAngularSpeed * p_encoder->Params.MotorPolePairs / p_encoder->DeltaT;

	//	return p_encoder->DeltaD * (p_encoder->UnitAngularSpeed  / p_encoder->DeltaT) * p_encoder->Params.MotorPolePairs
}
static inline uint32_t Encoder_Motor_GetElectricalRpm(Encoder_T * p_encoder)
{
	return Encoder_Motor_GetMechanicalRpm(p_encoder) * p_encoder->Params.MotorPolePairs;
//	return ((p_encoder->DeltaD * p_encoder->UnitAngularSpeed * p_encoder->Params.MotorPolePairs >> (32 - p_encoder->UnitAngularD_DivisorShift) - 6U) * 60U >> 6U)/ p_encoder->DeltaT;
}

/*

 */
static inline int32_t Encoder_Motor_GetGroundSpeed(Encoder_T * p_encoder)
{
	return Encoder_GetLinearSpeed(p_encoder);
}



#endif
