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
    @version V0
*/
/*******************************************************************************/
#ifndef MATH_LINEAR_H
#define MATH_LINEAR_H

#include <stdint.h>

static inline int32_t linear_f(int32_t m_factor, int32_t m_divisor, int32_t b, int32_t x)
{
	return ( (m_factor*x + (m_divisor/2))/m_divisor + b ); // add (line->SlopeDivisor/2) to round up .5
}

/*
 * m in f(x) direction
 */
static inline int32_t linear_invf(int32_t m_factor, int32_t m_divisor, int32_t b, int32_t y)
{
	return ( ((y - b)*m_divisor + (m_factor/2))/m_factor ); // add (line->SlopeFactor/2) to round up .5
}

static inline int32_t linear_f_shift(int32_t m_shifted, uint8_t shift, int32_t b, int32_t x)
{
	return ( ((m_shifted*x) >> (shift)) + b );
}

static inline int32_t linear_invf_shift(int32_t invm_shifted, uint8_t shift, int32_t b, int32_t y)
{
	return ( ((y - b)*invm_shifted) >> shift );
}

#endif
