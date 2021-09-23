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
	@file 	HAL_Serial.h
	@author FireSoucery
	@brief
	@version V0
*/
/******************************************************************************/
#include "Serial.h"

#include <stdint.h>
#include <stdbool.h>


/*
 * Single threaded buffer read/write only need disable channel ISR
 */
static inline void EnterCriticalTx(Serial_T * p_serial)
{
#if defined(CONFIG_SERIAL_MULTITHREADED_LIBRARY_DEFINED) || defined(CONFIG_SERIAL_MULTITHREADED_USER_DEFINED)
		Critical_Enter(&p_serial->TxSemaphore);
#else
		HAL_Serial_DisableTxInterrupt(p_serial->p_HAL_Serial);
#endif
}

static inline void ExitCriticalTx(Serial_T * p_serial)
{
#if defined(CONFIG_SERIAL_MULTITHREADED_LIBRARY_DEFINED) || defined(CONFIG_SERIAL_MULTITHREADED_USER_DEFINED)
		Critical_Exit(&p_serial->TxSemaphore);
#else
		HAL_Serial_EnableTxInterrupt(p_serial->p_HAL_Serial);
#endif
}

static inline void EnterCriticalRx(Serial_T * p_serial)
{
#if defined(CONFIG_SERIAL_MULTITHREADED_LIBRARY_DEFINED) || defined(CONFIG_SERIAL_MULTITHREADED_USER_DEFINED)
		Critical_Enter(&p_serial->RxSemaphore);
#else
		HAL_Serial_DisableRxInterrupt(p_serial->p_HAL_Serial);
#endif
}

static inline void ExitCriticalRx(Serial_T * p_serial)
{
#if defined(CONFIG_SERIAL_MULTITHREADED_LIBRARY_DEFINED) || defined(CONFIG_SERIAL_MULTITHREADED_USER_DEFINED)
		Critical_Exit(&p_serial->RxSemaphore);
#else
		HAL_Serial_EnableRxInterrupt(p_serial->p_HAL_Serial);
#endif
}


uint8_t Serial_RecvChar(Serial_T * p_serial) //Dequeue data
{
	uint8_t rxChar;

	if(p_serial->RxBufferHead == p_serial->RxBufferTail)
	{
		rxChar = -1; //Return empty char
	}
	else
	{
		EnterCriticalRx(p_serial);
		rxChar = p_serial->p_RxBuffer[p_serial->RxBufferTail];
		p_serial->RxBufferTail = (p_serial->RxBufferTail + 1U) % p_serial->RxBufferSize;
		ExitCriticalRx(p_serial);
	}

	return rxChar;
}


void Serial_SendChar(Serial_T * p_serial, uint8_t txChar) //enqueue
{
	uint32_t txBufferHeadNext;

	if((p_serial->TxBufferHead == p_serial->TxBufferTail) && HAL_Serial_ReadTxRegEmpty(p_serial->p_HAL_Serial))
	{
		HAL_Serial_WriteTxChar(p_serial->p_HAL_Serial, txChar);
	}
	else
	{
		EnterCriticalTx(p_serial);
		txBufferHeadNext = (p_serial->TxBufferHead + 1U) % p_serial->TxBufferSize;

		if (txBufferHeadNext == p_serial->TxBufferTail)
		{

		}
		else
		{
			p_serial->p_TxBuffer[p_serial->TxBufferHead] = txChar;
			p_serial->TxBufferHead = txBufferHeadNext;
			HAL_Serial_EnableTxInterrupt(p_serial->p_HAL_Serial);
		}
		ExitCriticalTx(p_serial);
	}
}

// buffer length

uint32_t Serial_RecvBuffer(Serial_T * p_serial, uint8_t * p_destBuffer, uint32_t bufferSize)
{
	uint32_t charCount;

	if(p_serial->RxBufferHead != p_serial->RxBufferTail)
	{
		EnterCriticalRx(p_serial);
		for (charCount = 0U; charCount < bufferSize; charCount++)
		{
			if (p_serial->RxBufferHead == p_serial->RxBufferTail)
			{
				break;
			}
			else
			{
				p_destBuffer[charCount] = p_serial->p_RxBuffer[p_serial->RxBufferTail];
				p_serial->RxBufferTail = (p_serial->RxBufferTail + 1U) % p_serial->RxBufferSize;
			}
		}
		ExitCriticalRx(p_serial);
	}

	return charCount;
}

uint32_t Serial_SendBuffer(Serial_T * p_serial, const uint8_t * p_srcBuffer, uint32_t bufferSize)
{
	uint32_t charCount;
//	EnterCriticalTx(p_serial); if multi threaded move
	uint32_t txBufferHeadNext = (p_serial->TxBufferHead + 1U) % p_serial->TxBufferSize;

	if (txBufferHeadNext != p_serial->TxBufferTail)
	{
		EnterCriticalTx(p_serial);
		for (charCount = 0; charCount < bufferSize; charCount++)
		{
			txBufferHeadNext = (p_serial->TxBufferHead + 1U) % p_serial->TxBufferSize;
			if (txBufferHeadNext == p_serial->TxBufferTail)
			{
				break;
			}
			else
			{
				p_serial->p_TxBuffer[p_serial->TxBufferHead] = p_srcBuffer[charCount];
				p_serial->TxBufferHead = txBufferHeadNext;
			}
		}
		HAL_Serial_EnableTxInterrupt(p_serial->p_HAL_Serial);
		ExitCriticalTx(p_serial);
	}

	return charCount;
}

bool Serial_RecvString(Serial_T * p_serial, uint8_t * p_destBuffer, uint32_t length)
{
	bool status;

	if (Serial_GetAvailableRx(p_serial) < length)
	{
		status = false;
	}
	else
	{
		EnterCriticalRx(p_serial);
		for (uint32_t destBufferIndex = 0U; destBufferIndex < length; destBufferIndex++)
		{
			p_destBuffer[destBufferIndex] = p_serial->p_RxBuffer[p_serial->RxBufferTail];
			p_serial->RxBufferTail = (p_serial->RxBufferTail + 1U) % p_serial->RxBufferSize;
		}
		ExitCriticalRx(p_serial);

		status = true;
	}

	return status;
}

bool Serial_SendString(Serial_T * p_serial, const uint8_t * p_srcBuffer, uint32_t length)
{
	bool status;

	if (Serial_GetAvailableTx(p_serial) < length)
	{
		status = false;
	}
	else
	{
		EnterCriticalTx(p_serial);
		for (uint32_t srcBufferIndex = 0U; srcBufferIndex < length; srcBufferIndex++)
		{
			p_serial->p_TxBuffer[p_serial->TxBufferHead] = p_srcBuffer[srcBufferIndex];
			p_serial->TxBufferHead = (p_serial->TxBufferHead + 1U) % p_serial->TxBufferSize;
		}
		HAL_Serial_EnableTxInterrupt(p_serial->p_HAL_Serial);
		ExitCriticalTx(p_serial);

		status = true;
	}

	return status;
}

void Serial_SetBaudRate(Serial_T * p_serial, uint32_t baudRate)
{
	HAL_Serial_ConfigBaudRate(p_serial->p_HAL_Serial, baudRate);
}

void Serial_Init
(
	Serial_T * p_serial,
	Serial_Init_T * p_serialInit
)
{

	//todo change to 1 pointer
	p_serial->p_HAL_Serial 			= p_serialInit->P_HAL_SERIAL;
	p_serial->p_TxBuffer 			= p_serialInit->P_TX_BUFFER;
	p_serial->TxBufferSize 			= p_serialInit->TX_BUFFER_SIZE;
	p_serial->p_RxBuffer 			= p_serialInit->P_RX_BUFFER;
	p_serial->RxBufferSize 			= p_serialInit->RX_BUFFER_SIZE;

	p_serial->TxBufferHead 			= 0;
	p_serial->TxBufferTail 			= 0;
	p_serial->RxBufferHead 			= 0;
	p_serial->RxBufferTail 			= 0;


	HAL_Serial_Init(p_serial->p_HAL_Serial);
	Serial_EnableRx(p_serial);
}

//bool Serial_RecvChar_Buffer(Serial_T * p_serial, uint8_t * p_rxChar)
//{
//	if(p_serial->RxBufferHead == p_serial->RxBufferTail) return false;
//
//	Critical_Enter(p_serial);
//	*p_rxChar = p_serial->p_RxBuffer[p_serial->RxBufferTail];
//	p_serial->RxBufferTail = (p_serial->RxBufferTail + 1) % p_serial->RxBufferSize;
//	Critical_Exit(p_serial);
//
//	return true;
//}
