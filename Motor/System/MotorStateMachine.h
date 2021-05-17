/*******************************************************************************/
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
/*******************************************************************************/
/*******************************************************************************/
/*!
    @file 	MotorStateMachine.h
    @author FireSoucery
    @brief  MotorStateMachine
    @version V0
*/
/*******************************************************************************/
#ifndef MOTOR_STATE_MACHINE_H
#define MOTOR_STATE_MACHINE_H

#include "../Motor.h"
//#include "../Motor_FOC.h"

typedef enum MotorStateMachine_TransitionInput_Tag
{
	MOTOR_TRANSITION_FAULT_CLEAR,
	MOTOR_TRANSITION_INIT_COMPLETE,
	MOTOR_TRANSITION_CALIBRATION_COMPLETE,
	MOTOR_TRANSITION_ALIGN_COMPLETE,
	MOTOR_TRANSITION_FAULT,

	MOTOR_TRANSITION_STOP,

	MOTOR_TRANSITION_ALIGN,





} MotorStateMachine_TransitionInput_T;

typedef enum MotorStateMachine_OutputInput_Tag
{
	MOTOR_STATE_BUTTON_NEXT,
	MOTOR_OUTPUT_BUTTON_PREV,

//	MOTOR_STATUS_NO_OP = 0xFFu,
//	STATE_INPUT_RESERVED_NO_OP = 0xFFu,
} MotorStateMachine_OutputInput_T;

extern void MotorStateMachine_Init(Motor_T * p_motor);

#endif
