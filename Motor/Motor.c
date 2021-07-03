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
    @file 	Motor.c
    @author FireSoucery
    @brief  Motor module conventional function definitions.
    @version V0
*/
/**************************************************************************/
#include "Motor.h"

#include "Config.h"
//#include "Default.h"

#include "MotorStateMachine.h"

#include "System/StateMachine/StateMachine.h"
#include "System/Thread/Thread.h"

#include "Transducer/Encoder/Encoder_Motor.h"
#include "Transducer/Encoder/Encoder.h"
#include "Transducer/Phase/Phase.h"

#include "Math/Linear/Linear_ADC.h"
#include "Math/Linear/Linear.h"



/*
 * General Init
 */
void Motor_Init(Motor_T * p_motor, const Motor_Init_T * p_motorInit)
{
	p_motor->p_Init = p_motorInit;
	MotorStateMachine_Init(p_motor);
}

//void Motor_InitConst(Motor_T * p_motor, const Motor_Init_T * p_motorInit)
//{
//	p_motor->p_Init = p_motorInit;
//}
//
//void Motor_InitParam(Motor_T * p_motor, const Motor_Parameters_T * p_parameters)
//{
//
//}

//state machine init-state run
void Motor_InitReboot(Motor_T * p_motor)
{
	const Motor_Init_T * p_motorInit = p_motor->p_Init; //Load Compile time consts

//	MotorFlash_LoadParameterAll(p_motor);   //todo circular depenency

	/*
	 * HW Wrappers Init
	 */
	Phase_Init
	(
		&p_motor->Phase,
		&p_motorInit->HAL_PHASE,
		p_motorInit->PHASE_PWM_PERIOD
	);

	if (p_motor->Parameters.CommutationMode == MOTOR_COMMUTATION_MODE_SIX_STEP || p_motor->Parameters.SensorMode == MOTOR_SENSOR_MODE_HALL)
	{
		//all sixstep modes and hall foc mode use CaptureTime
		Encoder_Motor_InitCaptureTime
		(
			&p_motor->Encoder,
			&p_motorInit->HAL_ENCODER,
			p_motorInit->HALL_ENCODER_TIMER_COUNTER_MAX,
			p_motorInit->HALL_ENCODER_TIMER_COUNTER_FREQ,
			p_motorInit->MOTOR_PWM_FREQ,
			p_motor->Parameters.PolePairs,
			p_motor->Parameters.PolePairs*6U,  // use PolePairs * 6 for count per commutation or PolePairs for per erotation
			p_motor->Parameters.EncoderDistancePerCount
		);

		Encoder_DeltaT_InitExtendedTimer
		(
			&p_motor->Encoder,
			&p_motor->MillisTimerBase,
			1000U,
			1000U
		);

		Hall_Init
		(
			&p_motor->Hall,
			&p_motorInit->HAL_HALL,
			MOTOR_SECTOR_ID_1,
			MOTOR_SECTOR_ID_2,
			MOTOR_SECTOR_ID_3,
			MOTOR_SECTOR_ID_4,
			MOTOR_SECTOR_ID_5,
			MOTOR_SECTOR_ID_6,
			p_motor->Parameters.HallVirtualSensorInvBMap,
			p_motor->Parameters.HallVirtualSensorAMap,
			p_motor->Parameters.HallVirtualSensorInvCMap,
			p_motor->Parameters.HallVirtualSensorBMap,
			p_motor->Parameters.HallVirtualSensorInvAMap,
			p_motor->Parameters.HallVirtualSensorCMap,
			MOTOR_SECTOR_ERROR_000,
			MOTOR_SECTOR_ERROR_111
		);
	}
	else if (p_motor->Parameters.SensorMode == MOTOR_SENSOR_MODE_ENCODER)
	{
		Encoder_Motor_InitCaptureCount
		(
			&p_motor->Encoder,
			&p_motorInit->HAL_ENCODER,
			p_motorInit->MOTOR_PWM_FREQ,
			p_motor->Parameters.PolePairs,
			p_motor->Parameters.EncoderCountsPerRevolution,
			p_motor->Parameters.EncoderDistancePerCount
		);

		Encoder_SetQuadratureMode(&p_motor->Encoder, p_motor->Parameters.EncoderIsQuadratureModeEnabled);
		Encoder_SetQuadratureDirectionCalibration(&p_motor->Encoder, p_motor->Parameters.EncoderIsALeadBPositive);
	}
	else //FOC open loop
	{
		//unused for now
		Encoder_Motor_InitCaptureCount
		(
			&p_motor->Encoder,
			&p_motorInit->HAL_ENCODER,
			p_motorInit->MOTOR_PWM_FREQ,
			p_motor->Parameters.PolePairs,
			p_motor->Parameters.PolePairs, //p_motor->Parameters.EncoderCountsPerRevolution,
			p_motor->Parameters.EncoderDistancePerCount
		 );
	}

	BEMF_Init
	(
		&p_motor->Bemf,
		&p_motor->ControlTimerBase,
		&p_motor->p_Init->P_VA_ADCU,
		&p_motor->p_Init->P_VB_ADCU,
		&p_motor->p_Init->P_VC_ADCU,
		&p_motor->p_Init->P_VBUS_ADCU,
		p_motor->Parameters.BemfSampleMode
	);

	FOC_Init(&p_motor->Foc);

	Thread_InitThreadPeriodic_Period //Timer only mode
	(
		&p_motor->ControlTimerThread,
		&p_motor->ControlTimerBase,
		p_motorInit->MOTOR_PWM_FREQ,
		1U,
		0U,
		0U
	);

	Thread_InitThreadPeriodic_Period //Timer only mode
	(
		&p_motor->MillisTimerThread,
		&p_motor->MillisTimerBase,
		1000U,
		1U,
		0U,
		0U
	);

	Thread_InitThreadPeriodic_Period //Timer only mode
	(
		&p_motor->SecondsTimerThread,
		&p_motor->MillisTimerBase,
		1000U,
		1000U,
		0U,
		0U
	);


	/*
	 * initial runtime config settings
	 */

	Phase_Polar_ActivateMode(&p_motor->Phase, p_motor->Parameters.PhasePwmMode);


	/*
	 * Run calibration later, default zero to middle adc
	 */
	Linear_ADC_Init(&p_motor->UnitIa, 2048U, 4095U, 120U); //temp 120amp
	Linear_ADC_Init(&p_motor->UnitIb, 2048U, 4095U, 120U);
	Linear_ADC_Init(&p_motor->UnitIc, 2048U, 4095U, 120U);


	Linear_Voltage_Init(&p_motor->UnitVBus, p_motorInit->LINEAR_V_BUS_R1, p_motorInit->LINEAR_V_BUS_R2, p_motorInit->LINEAR_V_ADC_VREF, p_motorInit->LINEAR_V_ADC_BITS, p_motor->Parameters.VBusRef);
	Linear_Voltage_Init(&p_motor->UnitVabc, p_motorInit->LINEAR_V_ABC_R1, p_motorInit->LINEAR_V_ABC_R2, p_motorInit->LINEAR_V_ADC_VREF, p_motorInit->LINEAR_V_ADC_BITS, p_motor->Parameters.VBusRef);

	//Linear_Init(&(p_Motor->VFMap), vPerRPM, 1, vOffset); //f(freq) = voltage

	p_motor->Direction 		= MOTOR_DIRECTION_CCW;
	p_motor->DirectionInput = MOTOR_DIRECTION_CCW;
	p_motor->SpeedFeedback_RPM = 0U;
	p_motor->VPwm = 0U;

}

void Motor_SetZero(Motor_T * p_motor)
{
	p_motor->SpeedFeedback_RPM = 0U;
	p_motor->VPwm = 0;
}

void Motor_SetDirection(Motor_T * p_motor, Motor_Direction_T direction)
{
	p_motor->Direction = direction;

	if (direction = MOTOR_DIRECTION_CW)
	{
		Hall_SetDirection(&p_motor->Hall, HALL_DIRECTION_CW); //only hall sensor tracks commutation direction
//		BEMF_SetDirection(&p_motor->Bemf, direction); //if bemf module parses rising/falling edge
	}
	else
	{
		Hall_SetDirection(&p_motor->Hall, HALL_DIRECTION_CCW);
//		BEMF_SetDirection(&p_motor->Bemf, direction);
	}
}

////set buffered direction, check on state machine run
void Motor_SetDirectionInput(Motor_T * p_motor, Motor_Direction_T direction)
{
	p_motor->DirectionInput = direction;
}

//static inline bool Motor_SetDirection_ (Motor_T * p_motor)
//{
//	//proc direction
//	//		 if (p_motor->Direction != p_motor->InputDirection)//direction reversed
//	//		{
//	//			Blinky_SetOnTime(&p_Motor->Alarm, 1000)
//	//		}
//}
//
//static inline bool Motor_SetDirection_ (Motor_T * p_motor)
//{
//	if (p_motor->Direction != p_motor->DirectionInput)
//	{
//		Motor_SetDirection(p_motor->DirectionInput);
//	}
//}

void Motor_PollStop(Motor_T * p_motor)
{
	if (p_motor->Parameters.CommutationMode == MOTOR_COMMUTATION_MODE_SIX_STEP || p_motor->Parameters.SensorMode == MOTOR_SENSOR_MODE_HALL)
	{
		if (Encoder_DeltaT_PollWatchStop(&p_motor->Encoder))
		{
			p_motor->SpeedFeedback_RPM = 0; //todo stop flag
		}
	}
	else if (p_motor->Parameters.SensorMode == MOTOR_SENSOR_MODE_OPEN_LOOP)
	{
		if ((p_motor->UserCmd == 0U)) //no braking/freewheel in openloop
		{
//			if(p_motor->UserCmdPrev > 0U)
//			{
				p_motor->SpeedFeedback_RPM = 0;
//			}
		}
	}
//	else //other modes covered by always capture DeltaD
//	{
//
//	}
}


/*
 * Control Variable Functions
 */
//per motor
void Motor_SetUserCmd(Motor_T * p_motor, uint16_t userCommand)
{
	p_motor->UserCmdPrev = p_motor->UserCmd;
	p_motor->UserCmd = userCommand;
}

//int32_t Motor_CalcRampIncIndex(Motor_T * p_motor, uint32_t * p_index)
//{
//	return Linear_Ramp_CalcTarget_IncIndex(&p_motor->Ramp, p_index, 1U);
//}


//proc ramp update ~millis
void Motor_SetRampAccelerate(Motor_T * p_motor, uint16_t acceration)
{

	if (p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_SPEED_VOLTAGE || p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_SPEED_CURRENT)
	{

	}
	else if (p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_CURRENT)
	{

	}
	else //p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_VOLTAGE
	{
		//Ramp to throttle over 1s
//		if (p_motor->UserCmd > p_motor->UserCmdPrev)
		{
			//(int32_t)p_motor->UserCmd - (int32_t)p_motor->UserCmdPrev
			Linear_Ramp_InitAcceleration(&p_motor->Ramp, 20000U, p_motor->VPwm, p_motor->UserCmd, p_motor->UserCmd);
		}

		//todo match  bemf
	}
	p_motor->RampIndex = 0;
}

void Motor_SetRampDecelerate(Motor_T * p_motor, uint16_t deceleration)
{
	if (p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_SPEED_VOLTAGE || p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_SPEED_CURRENT)
	{

	}
	else if (p_motor->Parameters.ControlMode == MOTOR_CONTROL_MODE_CONSTANT_CURRENT)
	{

	}
	else
	{
		//Decel by Brake per 1s
		//todo match  bemf/current
		Linear_Ramp_InitAcceleration(&p_motor->Ramp, 20000U, p_motor->VPwm, 0, -(int32_t)p_motor->UserCmd);

	}
	p_motor->RampIndex = 0;
}


/*
 * Speed PID Feedback Loop
 */
void Motor_ProcSpeedFeedback(Motor_T * p_motor)
{
	p_motor->SpeedFeedback_RPM = Encoder_Motor_GetMechanicalRpm(&p_motor->Encoder);
	//	//p_motor->SpeedFeedback_RPM = Filter_MovAvg(p_motor->Speed_RPM, coef, coef);
	//	PID_Proc(&p_motor->PidSpeed, p_motor->SpeedFeedback_RPM, setpoint);
}



//static inline void Motor_SetBrakeRegenOptimal(Motor_T * p_motor)
//{
//	p_motor->VPwm = Linear_Voltage_CalcUnsignedFraction16(&p_motor->UnitVabc, GetVBemf(&p_motor)) / 2U;
//}
//
//static inline void Motor_SetBrakeRegenProportional(Motor_T * p_motor, uint16_t intensity)
//{
//	p_motor->VPwm = Linear_Voltage_CalcUnsignedFraction16(&p_motor->UnitVabc, GetVBemf(&p_motor)) * (65536U - intensity) >> 16U;
//
//	//braking 0% -> pwm 100% of back emf;
//	//braking 10% -> pwm 90% of back emf;
//	//braking 50% -> pwm 50% of back emf;
//	//braking 90% -> pwm 10% of back emf;
//}
//
//static inline void Motor_SetBrakeRegenScalar(Motor_T * p_motor, uint16_t intensity)
//{
//	p_motor->VPwm = Linear_Voltage_CalcUnsignedFraction16(&p_motor->UnitVabc, GetVBemf(&p_motor)) * (32768U - ((intensity + 32768U) / p_motor->Parameters.BrakeCoeffcient)) >> 16U;
//
//	// e.g
//	// RegenCoeffcientInput = 4, RegenCoeffcient -> fract16(1,4)
//
//	//braking 0% -> pwm 62.5% of back emf;
//	//braking 10% -> pwm 60% of back emf;
//	//braking 50% -> pwm 50% of back emf;
//	//braking 90% -> pwm 40% of back emf;
//	//braking 100% -> pwm 37.5% of back emf;
//}


//static inline void Motor_ProcControlVarBrake(Motor_T * p_motor)
//{
//	switch (p_motor->Parameters.BrakeMode)
//	{
//	case MOTOR_BRAKE_MODE_SCALAR:
//		Motor_ProcRamp(p_motor);
//		break;
//	case MOTOR_BRAKE_MODE_REGEN_OPTIMAL:
//		Motor_SixStep_SetBrakeRegenOptimal(p_motor);
//		break;
//	case MOTOR_BRAKE_MODE_REGEN_PROPRTIONAL:
//		Motor_SixStep_SetBrakeRegenProportional(p_motor, p_motor->UserCmd);
//		break;
//	case MOTOR_BRAKE_MODE_REGEN_SCALAR:
//		Motor_SixStep_SetBrakeRegenScalar(p_motor, p_motor->UserCmd);
//		break;
//	default:
//		break;
//	}
//
//}

//split motor state version and controvar verions
void Motor_ProcControlVariable(Motor_T * p_motor)
{
	uint32_t debug;
	switch (p_motor->Parameters.ControlMode)
	{
//	case MOTOR_CONTROL_MODE_OPEN_LOOP:
//		p_motor->VPwm = p_motor->UserCmd / 4U;
//		break;

	case MOTOR_CONTROL_MODE_CONSTANT_VOLTAGE:
		if (p_motor->IBus_Frac16 > (58982U/2U))
		{
			debug = p_motor->IBus_Frac16;
			p_motor->VPwm = p_motor->RampCmd;


//			p_motor->VPwm = p_motor->VPwm - ((p_motor->IBus_Frac16 - 58982U) * p_motor->VPwm >> 16U);
//			p_motor->VPwm = p_motor->VPwm * 58982U / p_motor->PhaseCurrentFiltered_Frac16;

			//proc constant current pid, set integral term to vpwm.
		}
		else
		{
			p_motor->VPwm = p_motor->RampCmd;
		}
		break;

	case MOTOR_CONTROL_MODE_SCALAR_VOLTAGE_FREQ:
		//p_motor->VPwm = p_motor->RampCmd * p_motor->Speed_RPM * p_motor->VRpmGain;
		break;

	case MOTOR_CONTROL_MODE_CONSTANT_SPEED_VOLTAGE:
		Motor_PollSpeedFeedback(p_motor);
		break;

	case MOTOR_CONTROL_MODE_CONSTANT_CURRENT:

		break;

	default:
		break;
	}

}

/*
 * Align State
 */
void Motor_StartAlign(Motor_T * p_motor)
{
	p_motor->ControlTimerBase = 0U;
	Thread_SetTimerPeriod(&p_motor->ControlTimerThread, 20000U); //Parameter.AlignTime

	//	set to usercmd  ? p_motor->UserCmd
	//Phase_ActuatePhaseA
	Phase_ActuateDutyCycle(&p_motor->Phase, 6553U/4U, 0, 0); /* Align Phase A 10% pwm */
	Phase_ActuateState(&p_motor->Phase, 1, 1, 1);
}

bool Motor_ProcAlign(Motor_T * p_motor)
{
	bool status;

	if (Thread_PollTimerComplete(&p_motor->ControlTimerThread) == true)
	{
		p_motor->ElectricalAngle= 0U;
		Encoder_Reset(&p_motor->Encoder);
//		Motor_Float(&p_motor->Foc);
		status = true;
	}
	else
	{
		status = false;
	}

	return status;
}


/*
 * Calibration State Functions
 */
void Motor_StartCalibrateEncoder(Motor_T * p_motor)
{
	p_motor->ControlTimerBase = 0U;
	Thread_SetTimerPeriod(&p_motor->ControlTimerThread, 20000U);

	Phase_ActuateDutyCycle(&p_motor->Phase, 6553U/4U, 0, 0); /* Align Phase A 10% pwm */
	Phase_ActuateState(&p_motor->Phase, true, true, true);
}

void Motor_CalibrateEncoder(Motor_T * p_motor)
{
	static uint8_t state = 0; //limits calibration to 1 at a time;

	const uint16_t duty = 65536/10/4;

	bool isComplete = false;

	if (Thread_PollTimerCompletePeriodic(&p_motor->ControlTimerThread) == true)
	{
		switch (state)
		{
		case 0U:
			Encoder_DeltaD_CalibrateQuadratureReference(&p_motor->Encoder);

			Phase_ActuateState(&p_motor->Phase, true, true, true);
			Phase_ActuateDutyCycle(&p_motor->Phase, 0U, duty, 0U);
			state++;
			break;

		case 1U:
			Encoder_DeltaD_CalibrateQuadratureDirectionPositive(&p_motor->Encoder);
			Phase_ActuateState(&p_motor->Phase, false, false, false);
			state = 0;
			isComplete = true;
			break;

		default:
			break;

		}
	}

	return isComplete;
}

Motor_AlignMode_T Motor_GetAlignMode(Motor_T *p_motor)
{
	Motor_AlignMode_T alignMode;

	if (p_motor->Parameters.SensorMode == MOTOR_SENSOR_MODE_HALL)
	{
		alignMode = MOTOR_ALIGN_MODE_DISABLE;
	}
	else
	{
		//		if useHFI alignMode= MOTOR_ALIGN_MODE_HFI;
		//		else
		alignMode = MOTOR_ALIGN_MODE_ALIGN;
	}


	return alignMode;
}

void Motor_StartCalibrateHall(Motor_T * p_motor)
{
	p_motor->ControlTimerBase = 0U;
	Thread_SetTimerPeriod(&p_motor->ControlTimerThread, 20000U); //Parameter.HallCalibrationTime
}

//120 degree hall aligned with phase
bool Motor_CalibrateHall(Motor_T * p_motor)
{
	static uint8_t state = 0; //limits calibration to 1 at a time;

	const uint16_t duty = 65536 / 10/4;

	bool isComplete = false;

	if (Thread_PollTimerCompletePeriodic(&p_motor->ControlTimerThread) == true)
	{
		switch (state)
		{
		case 0U:
			Phase_ActuateState(&p_motor->Phase, true, true, true);

			Phase_ActuateDutyCycle(&p_motor->Phase, duty, 0U, 0U);
			state++;
			break;

		case 1U:
			Hall_CalibrateSensorAPhaseBC(&p_motor->Hall, MOTOR_SECTOR_ID_2);

			Phase_ActuateDutyCycle(&p_motor->Phase, duty, duty, 0U);
			state++;
			break;

		case 2U:
			Hall_CalibrateSensorInvCPhaseBA(&p_motor->Hall, MOTOR_SECTOR_ID_3);

			Phase_ActuateDutyCycle(&p_motor->Phase, 0U, duty, 0);
			state++;
			break;

		case 3U:
			Hall_CalibrateSensorBPhaseCA(&p_motor->Hall, MOTOR_SECTOR_ID_4);

			Phase_ActuateDutyCycle(&p_motor->Phase, 0U, duty, duty);
			state++;
			break;

		case 4U:
			Hall_CalibrateSensorInvAPhaseCB(&p_motor->Hall, MOTOR_SECTOR_ID_5);

			Phase_ActuateDutyCycle(&p_motor->Phase, 0U, 0U, duty);
			state++;
			break;

		case 5U:
			Hall_CalibrateSensorCPhaseAB(&p_motor->Hall, MOTOR_SECTOR_ID_6);

			Phase_ActuateDutyCycle(&p_motor->Phase, duty, 0U, duty);
			state++;
			break;

		case 6U:
			Hall_CalibrateSensorInvBPhaseAC(&p_motor->Hall, MOTOR_SECTOR_ID_1);

			Phase_ActuateState(&p_motor->Phase, false, false, false);
			state = 0;
			isComplete = true;
			break;

		default:
			break;

		}
	}

	return isComplete;
}



//void Motor_OnBlock(Motor_T * p_motor)
//{
//
//}
