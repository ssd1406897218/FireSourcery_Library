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
    @file 	Linear_Voltage.c
    @author FireSoucery
    @brief  Linear_Voltage module conventional function definitions
    @version V0
*/
/*******************************************************************************/
#include "Linear_Voltage.h"
#include "Linear.h"
#include <stdint.h>

/******************************************************************************/
/*!
	@brief Initialize Linear struct using Voltage Divider parameters.
	Struct contains numerator and denominator of ADC to Vin conversion factor.

	Init as Linear_Function(adcu) = voltage
	Supports
	adcu -> voltage
	adcu -> frac16
	voltage -> adcu
	frac16 	-> adcu

	VPerADCFactor = vRef * (r1 + r2);
	VPerADCDivisor = (2^adcBits - 1) * r2;

	VDIV = VIN*(R2/(R1+R2))
	DIV = (R2/(R1+R2))
	VADC_RES = VREF/ADC_MAX

	ADC = VIN*DIV/VADC_RES = VIN*(R2*ADC_MAX)/((R1+R2)*VREF)
	VIN = ADC*VADC_RES/DIV = ADC*(VREF*(R1+R2))/(ADC_MAX*R2)
	VIN/ADC = VADC_RES/DIV = VREF*(R1 + R2)/(ADC_MAX*R2)

	@param[in] line - Struct containing calculated intermediate values
	@param[in] r1 - R1 value expressed as a whole number
	@param[in] r2 - R2 value expressed as a whole number
	@param[in] vRef - reference voltage
	@param[in] adcBit - Number of ADC bits
 */
/******************************************************************************/
void Linear_Voltage_Init(Linear_T * p_linear, uint16_t r1, uint16_t r2, uint8_t adcVRef10, uint8_t adcBits, uint16_t vInMax)
{
#ifdef CONFIG_LINEAR_DIVIDE_SHIFT
	p_linear->SlopeFactor 			= ((adcVRef10 * (r1 + r2)) << (16U - adcBits)) / r2 / 10U; 		// (VREF*(R1 + R2) << 16)/(ADC_MAX*R2)
	p_linear->SlopeDivisor_Shift 	= 16U;
	p_linear->SlopeDivisor 			= ((r2 << 16U) * 10U / (adcVRef10 * (r1 + r2)));				// ((R2) << 16)/(VREF*(R1 + R2))
	p_linear->SlopeFactor_Shift 	= 16U - adcBits;
#elif defined (CONFIG_LINEAR_DIVIDE_NUMERICAL)

	//overflow caution
	//(adcu_max*VREF*(R1+R2) < uint32_max
	p_linear->SlopeFactor 	= adcVRef10 * (r1 + r2) / 10U;					// (VREF*(R1+R2))
	p_linear->SlopeDivisor 	= (((uint32_t)1UL << adcBits) - 1U) * r2; 		// (ADC_MAX*R2)
#endif

	p_linear->Intercept = 0U;
 	p_linear->RangeReference = vInMax - 0U;
}
