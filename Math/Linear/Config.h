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
    @file 	Config.c
    @author FireSoucery
    @brief
    @version V0
*/
/******************************************************************************/
#ifndef CONFIG_LINEAR_H
#define CONFIG_LINEAR_H

#ifdef CONFIG_LINEAR_SHIFT_DIVIDE_SHIFT

#elif defined(CONFIG_LINEAR_DIVIDE_NUMIRICAL)

#else
	#define CONFIG_LINEAR_DIVIDE_SHIFT
#endif

// compile time const adc bits for all linear adc instances
//#ifdef CONFIG_LINEAR_ADC_BITS
//
//#else
//	#define CONFIG_LINEAR_ADC_BITS 12
//#endif


#endif
