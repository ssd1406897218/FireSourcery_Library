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
#include "MotAnalogMonitor.h"
#include "Motor/Utility/MotAnalog/MotAnalog.h"

#include <stdint.h>

void MotAnalogMonitor_Init(MotAnalogMonitor_T * p_motorUser)
{
	memcpy(&p_motorUser->Params, p_motorUser->CONFIG.P_PARAMS, sizeof(MotAnalogMonitor_Params_T));
}

void MotAnalogMonitor_SetParams(MotAnalogMonitor_T * p_motorUser, const MotAnalogMonitor_Params_T * p_param)
{
	memcpy(&p_motorUser->Params, p_param, sizeof(MotAnalogMonitor_Params_T));
}
