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
    @file
    @author FireSoucery
    @brief	Non Volatile Memory Write
    		Abstract Base Class. Template Pattern.
    @version V0
*/
/*******************************************************************************/
#include "NvMemory.h"
#include "Config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


/******************************************************************************/
/*!
	Private
*/
/******************************************************************************/
static inline const uint8_t* CalcOpCmdAddress(const NvMemory_T * p_this, const uint8_t * p_dest) CONFIG_NV_MEMORY_ATTRIBUTE_RAM_SECTION;
static inline const uint8_t* CalcOpCmdAddress(const NvMemory_T * p_this, const uint8_t * p_dest)
{
#ifdef CONFIG_NV_MEMORY_HW_OP_ADDRESS_RELATIVE
	return (uint8_t *)((uint32_t)p_dest + p_this->p_OpPartition->OP_ADDRESS_OFFSET);
#elif defined(CONFIG_NV_MEMORY_HW_OP_ADDRESS_ABSOLUTE)
	return p_dest;
#endif
}

static inline bool StartOpCmd(const NvMemory_T * p_this, size_t opIndex) CONFIG_NV_MEMORY_ATTRIBUTE_RAM_SECTION;
static inline bool StartOpCmd(const NvMemory_T * p_this, size_t opIndex)
{
	const uint8_t * p_cmdDest = CalcOpCmdAddress(p_this, &p_this->p_OpDest[opIndex]);
	const uint8_t * p_cmdData = &p_this->p_OpData[opIndex];
	size_t units = p_this->UnitsPerCmd;

//	if (p_this->StartCmd != 0U) /* start command is always set */
	p_this->StartCmd(p_this->CONFIG.P_HAL, p_cmdDest, p_cmdData, units);

	return (p_this->CONFIG.READ_ERROR_FLAGS(p_this->CONFIG.P_HAL) == false) ? true : false;
}


static inline uint32_t CalcAlignDown(uint32_t value, uint8_t align)
{
	return ((value) & -(align));
}

static inline uint32_t CalcAlignUp(uint32_t value, uint8_t align)
{
	return (-(-(value) & -(align)));
}

static inline bool CheckIsAligned(uint32_t value, uint8_t align)
{
	return ((align & (align - 1U)) == 0U); //align unit always power of 2
}

static inline bool CheckDestIsAligned(const uint8_t * p_dest, size_t size, uint8_t align)
{
	return (CheckIsAligned((uint32_t)p_dest, align) && CheckIsAligned(size, align) ? true : false);
}

static inline bool CheckIsBounded(uint32_t targetStart, size_t targetSize, uint32_t boundaryStart, size_t boundarySize)
{
	return ((targetStart >= boundaryStart) && ((targetStart + targetSize) <= (boundaryStart + boundarySize)));
}

static inline bool CheckDestIsBoundedPartition(const uint8_t * p_dest, size_t size, NvMemory_Partition_T * p_partition)
{
	return CheckIsBounded((uint32_t)p_dest, size, (uint32_t)p_partition->P_START, p_partition->SIZE);
}

static inline NvMemory_Partition_T * SearchParition(const NvMemory_Partition_T * p_partitionTable, uint8_t partitionCount, const uint8_t * p_dest, size_t size)
{
	NvMemory_Partition_T * p_partition = 0U;

	for (uint8_t iPartition = 0U; iPartition < partitionCount; iPartition++)
	{
		if (CheckDestIsBoundedPartition(p_dest, size, &p_partitionTable[iPartition]) == true)
		{
			p_partition = &p_partitionTable[iPartition];
		}
	}

	return p_partition;
}

static inline bool ProcSearchParition(NvMemory_T * p_this, const uint8_t * p_dest, size_t size)
{
	p_this->p_OpPartition = SearchParition(p_this->CONFIG.P_PARTITIONS, p_this->CONFIG.PARTITION_COUNT, p_dest, size);
	return ((p_this->p_OpPartition != 0U) ? true : false);
}

//static inline bool CheckOpParameters(NvMemory_T * p_this, const uint8_t * p_dest, size_t size, unitSize)
//{
//	return (ProcSearchParition(p_this, p_dest, size) == true) && (CheckDestIsAligned(p_dest, size, unitSize) == true);
//}

static inline uint32_t CalcChecksum(const uint8_t * p_data, size_t size)
{
	uint32_t sum = 0U;
	for (size_t iByte = 0U; iByte < size; iByte++)	{ sum += p_data[iByte]; }
	return sum;
}

static inline size_t CalcUnitsPerCmd(size_t opSize, uint8_t unitSize)
{
	return opSize/unitSize;
}

static inline void SetOpDataPtr(NvMemory_T * p_this, const uint8_t * p_source, size_t size)
{
	p_this->p_OpData	= p_source;
	p_this->OpSize 		= size;
}

static inline void SetOpDataBuffer(NvMemory_T * p_this, const uint8_t * p_source, size_t size)
{
	if (p_source != 0U)
	{
		memcpy(p_this->CONFIG.P_BUFFER, p_source, size);
	}

	SetOpDataPtr(p_this, p_this->CONFIG.P_BUFFER, size);
}

/******************************************************************************/
/*!
	Public Functions
*/
/******************************************************************************/
void NvMemory_Init(NvMemory_T * p_this)
{
	p_this->Status 			= NV_MEMORY_STATUS_SUCCESS;
	p_this->State 			= NV_MEMORY_STATE_IDLE;
	p_this->p_OpDest 		= 0U;
	p_this->OpIndex	  		= 0U;
	p_this->OpSize 			= 0U;
	p_this->IsVerifyEnable 	= true;
	p_this->IsOpBuffered 	= true;;
}

void NvMemory_SetYield(NvMemory_T * p_this, void (*yield)(void *), void * p_callbackData)
{
	p_this->Yield = yield;
	p_this->p_CallbackData = p_callbackData;
}


/******************************************************************************/
/*!
	Call from Subclass only
	Set - Common Blocking Non Blocking
*/
/******************************************************************************/
NvMemory_Status_T NvMemory_SetOpCommon(NvMemory_T * p_this, const uint8_t * p_dest, size_t opSize, size_t unitSize)
{
	NvMemory_Status_T status;

	if((ProcSearchParition(p_this, p_dest, opSize) == true) && (CheckDestIsAligned(p_dest, opSize, unitSize) == true))
	{
		p_this->p_OpDest = p_dest;
		status = NV_MEMORY_STATUS_SUCCESS;
	}
	else
	{
		status = NV_MEMORY_STATUS_ERROR_INPUT;
	}

	return status;
}

void NvMemory_SetOpData(NvMemory_T * p_this, const uint8_t * p_source, size_t size)
{
	(p_this->IsOpBuffered == true) ? SetOpDataBuffer(p_this, p_source, size) : SetOpDataPtr(p_this, p_source, size);
}

void NvMemory_SetOpCmdSize(NvMemory_T * p_this, size_t unitSize, uint8_t unitsPerCmd)
{
	p_this->UnitsPerCmd = unitsPerCmd;
	p_this->BytesPerCmd = unitsPerCmd * unitSize;
}

/* return true if correct */
bool NvMemory_ChecksumOp(const NvMemory_T * p_this)
{
	return (CalcChecksum(&p_this->p_OpDest[0U], p_this->OpSize) == CalcChecksum(p_this->CONFIG.P_BUFFER, p_this->OpSize)) ? true : false;
}

/******************************************************************************/
/*!
	Blocking Implementations
*/
/******************************************************************************/
NvMemory_Status_T NvMemory_ProcOpCommon_Blocking(NvMemory_T * p_this) CONFIG_NV_MEMORY_ATTRIBUTE_RAM_SECTION;
NvMemory_Status_T NvMemory_ProcOpCommon_Blocking(NvMemory_T * p_this)
{
	NvMemory_Status_T status = NV_MEMORY_STATUS_ERROR_CMD;

	if (p_this->CONFIG.READ_COMPLETE_FLAG(p_this->CONFIG.P_HAL) == true)
	{
		p_this->CONFIG.CLEAR_ERROR_FLAGS(p_this->CONFIG.P_HAL);

		for (size_t opIndex = 0U; opIndex < p_this->OpSize; opIndex += p_this->BytesPerCmd)
		{
			if (StartOpCmd(p_this, opIndex) == true)
			{
				while (p_this->CONFIG.READ_COMPLETE_FLAG(p_this->CONFIG.P_HAL) == false)
				{
					if (p_this->CONFIG.READ_ERROR_FLAGS(p_this->CONFIG.P_HAL) == true)
					{
						break;
					}
					if (p_this->Yield != 0U)
					{
						p_this->Yield(p_this->p_CallbackData);
					}
				}

//				p_this->FinalizeCmd(p_this->CONFIG.P_HAL); //if need to save read data

//				if(p_this->ReadData)
//				{
//					p_this->ReadData(p_this->CONFIG.P_HAL, &p_this->CONFIG.P_BUFFER[opIndex]);
//				}
//				if (p_this->OpType == NV_MEMORY_OPERATION_READ_ONCE)
//				{
//					HAL_NvMemory_ReadOnceData(p_this->CONFIG.P_HAL, &p_this->CONFIG.P_BUFFER[opIndex]);
//				}
			}
			else
			{
				break;
			}
		}

		//alternatively, use extra fp
//		if (p_this->CONFIG.READ_ERROR_FLAGS(p_this->CONFIG.P_HAL) == true)
//		{
//			status = p_this->ParseError(p_this);
//		}
//		else
//		{
			status = p_this->FinalizeOp(p_this);

			if (status == NV_MEMORY_STATUS_START_VERIFY)
			{
				status = NvMemory_ProcOpCommon_Blocking(p_this); // misra violation, single recursive call
			}
//		}

	}
	else
	{
		status = NV_MEMORY_STATUS_ERROR_BUSY;
	}

    return status;
}


/******************************************************************************/
/*!
	Non Blocking
*/
/******************************************************************************/
size_t NvMemory_GetOpBytesRemaining(NvMemory_T * p_this)
{
	return p_this->OpSize - p_this->OpIndex;
}

bool NvMemory_ReadIsOpComplete(NvMemory_T * p_this)
{
	return (p_this->State == NV_MEMORY_STATE_IDLE) && (p_this->CONFIG.READ_COMPLETE_FLAG(p_this->CONFIG.P_HAL) == true);
}

/*
 * returns true when complete
 */
bool NvMemory_ProcOp(NvMemory_T * p_this)
{
	bool isComplete = false;

	switch (p_this->State)
	{
		case NV_MEMORY_STATE_IDLE:
			break;

		case NV_MEMORY_STATE_ACTIVE:
			//multithread use mutex
			if (p_this->CONFIG.READ_ERROR_FLAGS(p_this->CONFIG.P_HAL) == false)
			{
				if (p_this->CONFIG.READ_COMPLETE_FLAG(p_this->CONFIG.P_HAL) == true)
				{
					//check first time
//					if (p_this->OpIndex > 0U)
//					{
	//					if (p_this->OpType == NV_MEMORY_OPERATION_READ_ONCE)
	//					{
	//						HAL_NvMemory_ReadOnceData(p_this->CONFIG.P_HAL, &p_this->Buffer[p_this->OpIndex]);
	//					}
//					}

					if (p_this->OpIndex < p_this->OpSize)
					{
						if (StartOpCmd(p_this, p_this->OpIndex) == true)
						{
							p_this->OpIndex += p_this->BytesPerCmd;
						}
					}
					else  //all pages complete
					{
						isComplete = true;
					}
				}
			}
			else //error occurred during command operation
			{
				isComplete = true;
			}

			if (isComplete == true)
			{
				p_this->Status = p_this->FinalizeOp(p_this);

				if (p_this->Status == NV_MEMORY_STATUS_START_VERIFY)
				{
					p_this->Status = NV_MEMORY_STATUS_PROCESSING;
					p_this->OpIndex = 0U;
					isComplete = false;
				}
				else
				{
					p_this->State = NV_MEMORY_STATE_IDLE;
					isComplete = true;
				}
			}
			break;

		default:
			break;
	}

	return isComplete;
}

NvMemory_Status_T NvMemory_StartOpCommon(NvMemory_T * p_this)
{
	if (NvMemory_ReadIsOpComplete(p_this) == true)
	{
		p_this->CONFIG.CLEAR_ERROR_FLAGS(p_this->CONFIG.P_HAL);
		p_this->State = NV_MEMORY_STATE_ACTIVE;
		p_this->Status = NV_MEMORY_STATUS_PROCESSING;
		p_this->OpIndex = 0U;
	}
	else
	{
		p_this->Status = NV_MEMORY_STATUS_ERROR_BUSY;
	}

    return p_this->Status;
}



/******************************************************************************/
/*!
	Virtual
*/
/******************************************************************************/
/*
 * creates copy of flash in buffer
 */
//void Flash_OpenVirtual(Flash_T * p_flash, const uint8_t * p_physical, size_t size)
//{
//	memcpy(&p_flash->Buffer[0U], p_physical, size);
//
//	p_flash->p_OpDest = CalcOpCmdAddress(p_flash, p_physical);
//	p_flash->OpIndex = 0U;
//	p_flash->OpSize = size;
//
//}
//
///*
// * dest of physical flash location
// */
//void Flash_WriteVirtual(Flash_T * p_flash, const uint8_t * p_physical, const uint8_t * p_src, size_t size)
//{
//	uint32_t offset;
//
//	if (p_physical >= p_flash->p_OpDest && p_physical + size < p_flash->p_OpDest + p_flash->OpSize)
//	{
//		offset = p_physical - p_flash->p_OpDest;
//		memcpy(&p_flash->Buffer[offset], p_src, size);
//	}
//}
//
//void Flash_ReadVirtual(Flash_T * p_flash, uint8_t * p_dest, const uint8_t * p_physical, size_t size)
//{
//	uint32_t offset;
//
//	if (p_physical >= p_flash->p_OpDest && p_physical <= p_flash->p_OpDest + p_flash->OpSize)
//	{
//		offset = p_physical - p_flash->p_OpDest;
//		memcpy(p_dest, &p_flash->Buffer[offset], size);
//	}
//}
//
//void Flash_CloseVirtual(Flash_T * p_flash)
//{
//	p_flash->State = FLASH_STATE_WRITE;
//}
//
//bool Flash_CloseVirtual_Blocking(Flash_T * p_flash)
//{
//	bool isSuccess = false;
//
//	if (Flash_Erase_Blocking(p_flash, p_flash->p_OpDest, p_flash->OpSize) == true)
//	{
//		if (Flash_Write_Blocking(p_flash, p_flash->p_OpDest, &p_flash->Buffer[0U], p_flash->OpSize) == true)
//		{
//			if (ChecksumOp(p_flash) == true)
//			{
//				if (p_flash->IsVerifyEnable == true)
//				{
//					if (Flash_VerifyWrite_Blocking(p_flash, p_flash->p_OpDest, &p_flash->Buffer[0U], p_flash->OpSize) == true)
//					{
//						isSuccess = true;
//					}
//				}
//				else
//				{
//					isSuccess = true;
//				}
//			}
//		}
//	}
//
//	return isSuccess;
//}


