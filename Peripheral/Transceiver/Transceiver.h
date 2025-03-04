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
	@file 	HAL_Transceiver.h
	@author FireSoucery
	@brief
	@version V0
*/
/******************************************************************************/
#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

#include "Config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef const struct
{

}
Transceiver_Config_T;



/*
 *
 */
typedef const struct
{
	void * p_Context;
	void 		(* Init)		(void * p_context);
	bool 		(* SendChar)	(void * p_context, uint8_t txChar);
	bool 		(* RecvChar)	(void * p_context, uint8_t * p_rxChar);
	uint32_t 	(* SendBytes)	(void * p_context, const uint8_t * p_srcBuffer, 	size_t bufferSize);
	uint32_t 	(* RecvBytes)	(void * p_context, uint8_t * p_destBuffer, 			size_t bufferSize);
	bool 		(* SendString)	(void * p_context, const uint8_t * p_srcBuffer, 	size_t length);
	bool 		(* RecvString)	(void * p_context, uint8_t * p_destBuffer, 			size_t length);
}
Transceiver_T;

#define TRANSCEIVER_CONFIG(p_context, Init, SendChar, RecvChar, SendBytes, RecvBytes, SendString, RecvString)		\
{													\
	.p_Context	= p_context											\
}

void Transceiver_Init(const Transceiver_T * p_transceiver)	{p_transceiver->Init(p_transceiver->p_Context);}
bool Transceiver_SendChar(const Transceiver_T * p_transceiver, uint8_t txChar) 		{return p_transceiver->SendChar(p_transceiver->p_Context, txChar);}
bool Transceiver_RecvChar(const Transceiver_T * p_transceiver, uint8_t * p_rxChar)	{return p_transceiver->RecvChar(p_transceiver->p_Context, p_rxChar);}
uint32_t Transceiver_SendBytes(const Transceiver_T * p_transceiver, const uint8_t * p_srcBuffer, size_t srcSize)	{p_transceiver->SendBytes(p_transceiver->p_Context, p_srcBuffer, srcSize);}
uint32_t Transceiver_RecvBytes(const Transceiver_T * p_transceiver, uint8_t * p_destBuffer, size_t destSize)		{p_transceiver->RecvBytes(p_transceiver->p_Context, p_destBuffer, destSize);}
bool Transceiver_SendString(const Transceiver_T * p_transceiver, const uint8_t * p_src, size_t length)				{p_transceiver->SendString(p_transceiver->p_Context, p_src, length);}
bool Transceiver_RecvString(const Transceiver_T * p_transceiver, uint8_t * p_dest, size_t length)					{p_transceiver->RecvString(p_transceiver->p_Context, p_dest, length);}
bool Transceiver_Send(const Transceiver_T * p_transceiver, const uint8_t * p_srcBuffer, size_t length)		{return Transceiver_SendString(p_transceiver, p_srcBuffer, length);}
uint32_t Transceiver_Recv(const Transceiver_T * p_transceiver, uint8_t * p_destBuffer, size_t length)		{return Transceiver_RecvBytes(p_transceiver, p_destBuffer, length);}

//void Transceiver_ConfigBaudRate(const Transceiver_T * p_transceiver, uint32_t baudRate)
//{
//	HAL_Transceiver_ConfigBaudRate(p_transceiver->CONFIG.P_HAL_SERIAL, baudRate);
//}

static inline void Transceiver_Poll(const Transceiver_T * p_transceiver)
{
//	Transceiver_PollRestartRxIsr(p_Serial)

}

static inline uint32_t Transceiver_GetRxFullCount(const Transceiver_T * p_transceiver)
{

}

static inline uint32_t Transceiver_GetTxEmptyCount(const Transceiver_T * p_transceiver)
{

}

static inline void Transceiver_EnableTx(const Transceiver_T * p_transceiver){}
static inline void Transceiver_DisableTx(const Transceiver_T * p_transceiver){}
static inline void Transceiver_EnableRx(const Transceiver_T * p_transceiver){}
static inline void Transceiver_DisableRx(const Transceiver_T * p_transceiver){}

#endif

