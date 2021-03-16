/*******************************************************************************/
/*!
	@section LICENSE

	Copyright (C) 2021 FireSoucery / The Firebrand Forge Inc

	This file is part of FireSourcery_Library (https://github.com/FireSourcery/FireSourcery_Library).

	This program is free software: you can redistribute it and/or modify
	it under the terupdateInterval of the GNU General Public License as published by
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
    @file 	Linear.c
    @author FireSoucery
    @brief  Mathematical linear function.
     	 	e.g. dynamic look up table, Unit/ADC conversion using factor and divisor.
    @version V0
*/
/*******************************************************************************/
#ifndef LINEAR_H_
#define LINEAR_H_

#include "math_linear.h"

#include <stdint.h>

typedef struct Linear_Tag
{
	int32_t SlopeFactor; 	//m
	int32_t SlopeDivisor;
#ifdef CONFIG_CONFIG_LINEAR_SHIFT_DIVIDE
	uint8_t SlopeFactor_Shift; 	//m
	uint8_t SlopeDivisor_Shift;

	uint8_t ShiftFactor;
	uint8_t ShiftDivisor;
#endif
	int32_t Offset;			//b
} Linear_T;

int32_t Linear_Function(Linear_T * p_linear, int32_t x)
{
	return linear_f(p_linear->SlopeFactor, p_linear->SlopeDivisor, p_linear->Offset, x);
#ifdef CONFIG_LINEAR_SHIFT_DIVIDE
#endif
}

int32_t Linear_InvFunction(Linear_T * p_linear, int32_t y)
{
	return linear_invf(p_linear->SlopeFactor, p_linear->SlopeDivisor, p_linear->Offset, y);
#ifdef CONFIG_LINEAR_SHIFT_DIVIDE
#endif
}

int32_t Linear_Function_Rounded(Linear_T * p_linear, int32_t x)
{
	return linear_f(p_linear->SlopeFactor, p_linear->SlopeDivisor, p_linear->Offset, x);
}

int32_t Linear_InvFunction_Rounded(Linear_T * p_linear, int32_t y)
{
	return linear_invf(p_linear->SlopeFactor, p_linear->SlopeDivisor, p_linear->Offset, y);
}

#endif
