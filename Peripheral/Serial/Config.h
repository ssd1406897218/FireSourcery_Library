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
	@file 	Config.h
	@author FireSoucery
	@brief
	@version V0
*/
/******************************************************************************/
#ifndef CONFIG_SERIAL_H
#define CONFIG_SERIAL_H

#if defined(CONFIG_SERIAL_CRITICAL_ENABLE)
#elif defined(CONFIG_SERIAL_CRITICAL_PATH)
#endif

#if  defined(CONFIG_SERIAL_MULTITHREADED_USE_MUTEX)
	#if defined(CONFIG_SERIAL_CRITICAL_ENABLE) || defined(CONFIG_SERIAL_CRITICAL_PATH

	#else
		#error "HAL_Serial "
	#endif
#elif defined(CONFIG_SERIAL_MULTITHREADED_USE_CRITICAL)

#else
 	#define CONFIG_SERIAL_SINGLE_THREADED
#endif

#if defined(CONFIG_SERIAL_HW_FIFO_DISABLE)
#else
#endif

//#if defined(CONFIG_HAL_SERIAL_PATH)
//#elif defined(CONFIG_HAL_PLATFORM)
//#else
//	#error "HAL_Serial - UNDEFINED"
//#endif

#endif
