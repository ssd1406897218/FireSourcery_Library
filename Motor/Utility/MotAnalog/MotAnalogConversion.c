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
    @file 	.h
    @author FireSoucery
    @brief
    @version V0
*/
/******************************************************************************/
#include "MotAnalogConversion.h"
#include "MotAnalog.h"
#include "Peripheral/Analog/AnalogN/AnalogN.h"

#include <stdint.h>
#include <stdbool.h>



const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_HEAT_PCB 		= { .CHANNEL = MOT_ANALOG_CHANNEL_HEAT_PCB,			.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_HEAT_MOSFETS_TOP = { .CHANNEL = MOT_ANALOG_CHANNEL_HEAT_MOSFETS_TOP,	.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_HEAT_MOSFETS_BOT = { .CHANNEL = MOT_ANALOG_CHANNEL_HEAT_MOSFETS_BOT,	.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_VACC 			= { .CHANNEL = MOT_ANALOG_CHANNEL_VACC,				.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_VSENSE 			= { .CHANNEL = MOT_ANALOG_CHANNEL_VSENSE,			.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_THROTTLE 		= { .CHANNEL = MOT_ANALOG_CHANNEL_THROTTLE,			.ON_COMPLETE = 0U, };
const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_BRAKE 			= { .CHANNEL = MOT_ANALOG_CHANNEL_BRAKE,			.ON_COMPLETE = 0U, };

/******************************************************************************/
/*!
    @brief  Conversion
*/
/******************************************************************************/
//static const Analog_ConversionVirtualChannel_T CHANNELS_MONITOR[] =
//{
//	[0U] = {MOT_ANALOG_CHANNEL_HEAT_MOSFETS, 	0U},
//	[1U] = {MOT_ANALOG_CHANNEL_HEAT_PCB, 		0U},
//	[2U] = {MOT_ANALOG_CHANNEL_VACC,			0U},
//	[3U] = {MOT_ANALOG_CHANNEL_VSENSE,			0U},
//};
//
//const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_MONITOR =
//{
//	.P_CHANNELS 	= CHANNELS_MONITOR,
//	.CHANNEL_COUNT 	= sizeof(CHANNELS_MONITOR)/sizeof(Analog_ConversionVirtualChannel_T),
//	.ON_COMPLETE 	= 0U,
//};

/******************************************************************************/
/*!
    @brief  Conversion
*/
/******************************************************************************/
//static const Analog_ConversionVirtualChannel_T CHANNELS_USER[] =
//{
//	{MOT_ANALOG_CHANNEL_BRAKE,			0U},
//	{MOT_ANALOG_CHANNEL_THROTTLE,		0U},
//};
//
//const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_USER =
//{
//	.P_CHANNELS 	= CHANNELS_USER,
//	.CHANNEL_COUNT 	= sizeof(CHANNELS_USER)/sizeof(Analog_ConversionVirtualChannel_T),
//	.ON_COMPLETE 	= 0U,
//};
//
///******************************************************************************/
///*!
//    @brief  Conversion
//*/
///******************************************************************************/
//static const Analog_ConversionVirtualChannel_T CHANNELS_MOTORS_HEAT[] =
//{
////#ifdef MOTOR_COUNT > 1
//
////	{MOT_ANALOG_CHANNEL_MOTORS_HEAT_0,		0U},
//};
//
//const Analog_VirtualConversionChannel_T MOT_ANALOG_VIRTUAL_MOTORS_HEAT =
//{
//	.P_CHANNELS 	= CHANNELS_MOTORS_HEAT,
//	.CHANNEL_COUNT 	= sizeof(CHANNELS_MOTORS_HEAT)/sizeof(Analog_ConversionVirtualChannel_T),
//	.ON_COMPLETE 	= 0U,
//};
