/*
 * sfu_commands.c
 *
 *  Created on: 08 ���� 2016 �.
 *      Author: Easy
 */

#include "stm32kiss.h"
#include "usart_mini.h"
#include "packet_receiver.h"

#ifdef USE_STDPERIPH_DRIVER
#include "stm32f4xx_flash.h"
#include "stm32f4xx_crc_inline.h"
#include "stm32f4xx_flash_inline.h"
#else
#include "hw_init.h"
#endif

#define SFU_VER 0x0100

#define SFU_CMD_ERASE_PART   0xB3
#define SFU_CMD_INFO    0x97
#define SFU_CMD_ERASE   0xC5
#define SFU_CMD_WRITE   0x38
#define SFU_CMD_START   0x26
#define SFU_CMD_TIMEOUT 0xAA
#define SFU_CMD_WRERROR 0x55
#define SFU_CMD_HWRESET 0x11

static void sfu_command_info(uint8_t code, uint8_t *body, uint32_t size);
static void sfu_command_erase(uint8_t code, uint8_t *body, uint32_t size);
static void sfu_command_write(uint8_t code, uint8_t *body, uint32_t size);
static void sfu_command_start(uint8_t code, uint8_t *body, uint32_t size);

static uint32_t write_addr = 0;

void sfu_command_init()
{
	uint32_t temp;
	packet_send(SFU_CMD_HWRESET, (void*)&temp, 0);
}

void sfu_command_timeout()
{
	if (write_addr == 0) return;
	write_addr = 0;
	packet_send(SFU_CMD_TIMEOUT, (uint8_t*)&write_addr, sizeof(write_addr));
	HAL_FLASH_Lock();
}

void sfu_command_parser(uint8_t code, uint8_t *body, uint32_t size)
{
	if (code == SFU_CMD_INFO)  sfu_command_info(code, body, size);
	if (code == SFU_CMD_ERASE) sfu_command_erase(code, body, size);
	if (code == SFU_CMD_WRITE) sfu_command_write(code, body, size);
	if (code == SFU_CMD_START) sfu_command_start(code, body, size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void serialize_uint32(uint8_t *body, uint32_t value)
{
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	*((uint32_t *)body) = value;
#else
	body[0] = (value >>  0) & 0xFF;
	body[1] = (value >>  8) & 0xFF;
	body[2] = (value >> 16) & 0xFF;
	body[3] = (value >> 24) & 0xFF;
#endif
}

static inline void serialize_uint16(uint8_t *body, uint16_t value)
{
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	*((uint16_t *)body) = value;
#else
	body[0] = (value >>  0) & 0xFF;
	body[1] = (value >>  8) & 0xFF;
#endif
}

static inline uint32_t deserialize_uint32(uint8_t *body)
{
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	return *((uint32_t *)body);
#else
	return  ((uint32_t)body[0] <<  0) |
			((uint32_t)body[1] <<  8) |
			((uint32_t)body[2] << 16) |
			((uint32_t)body[3] << 24);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sfu_command_info(uint8_t code, UNUSED_A uint8_t *body, UNUSED_A uint32_t size)
{
	const uint32_t CPU_TYPE = DBGMCU->IDCODE & 0xFFFF0FFF; //7..4 bits reserved

	for (uint32_t index = 0; index < 12; index++)
		body[index] = DEVICE_ID_BLOCK_PTR[index];

	serialize_uint32(body + 12, CPU_TYPE);
	serialize_uint16(body + 16, FLASH_SIZE_CORRECT);
	serialize_uint16(body + 18, SFU_VER);
	serialize_uint32(body + 20, recive_size());
	serialize_uint32(body + 24, MAIN_START_FROM);
	serialize_uint32(body + 28, MAIN_RUN_FROM);

	packet_send(code, body, 32);
}

typedef struct {
	uint8_t sector_id;
	uint8_t total_size;
} tFLASH_sectors;

#ifdef USE_STDPERIPH_DRIVER
#define ADDR_COMPRESS 0x00004000
const tFLASH_sectors sectors[] = {
		{FLASH_Sector_2, (0x00004000 / ADDR_COMPRESS)},
		{FLASH_Sector_3, (0x00008000 / ADDR_COMPRESS)},
		{FLASH_Sector_4, (0x00018000 / ADDR_COMPRESS)},
		{FLASH_Sector_5, (0x00038000 / ADDR_COMPRESS)},
		{FLASH_Sector_6, (0x00058000 / ADDR_COMPRESS)},
		{FLASH_Sector_7, (0x00078000 / ADDR_COMPRESS)},
		{FLASH_Sector_8, (0x00098000 / ADDR_COMPRESS)},
		{FLASH_Sector_9, (0x000B8000 / ADDR_COMPRESS)},
		{FLASH_Sector_10,(0x000D8000 / ADDR_COMPRESS)},
		{FLASH_Sector_11,(0x000F8000 / ADDR_COMPRESS)},
};
#endif

#ifdef STM32F745xx
#define ADDR_COMPRESS 0x00004000
#define FLASH_COMPLETE HAL_OK
const tFLASH_sectors sectors[] = {
		{FLASH_SECTOR_1, (0x00008000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_2, (0x00010000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_3, (0x00018000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_4, (0x00038000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_5, (0x00058000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_6, (0x00078000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_7, (0x00098000 / ADDR_COMPRESS)},
};
#endif

#ifdef STM32H743xx
#define ADDR_COMPRESS 0x00010000
#define FLASH_COMPLETE HAL_OK
const tFLASH_sectors sectors[] = {
		{FLASH_SECTOR_0, (0x00020000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_1, (0x00040000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_2, (0x00060000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_3, (0x00080000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_4, (0x000A0000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_5, (0x000C0000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_6, (0x000E0000 / ADDR_COMPRESS)},
		{FLASH_SECTOR_7, (0x00100000 / ADDR_COMPRESS)},
};
#endif

HAL_StatusTypeDef flash_block_write(uint32_t wr_addr, const uint32_t * const dword_ptr, const uint32_t dword_count);

static void sfu_command_erase(uint8_t code, uint8_t *body, uint32_t size)
{
	if (size != 4) return;

	uint32_t firmware_size = deserialize_uint32(body);

	if (firmware_size > 0)
	{
#ifdef USE_STDPERIPH_DRIVER
		FLASH_Status status = FLASH_BUSY;
		FLASH_Unlock_inline();
		FLASH_ClearFlag_inline(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
#endif

#if defined(STM32F746xx) || defined(STM32H743xx)
		HAL_StatusTypeDef status = HAL_ERROR;
		HAL_FLASH_Unlock();
#endif

		for (uint32_t pos = 0; pos < LENGTH(sectors); pos++)
		{
#ifdef USE_STDPERIPH_DRIVER
			if ((FLASH_SIZE == 512) && (sectors[pos].sector_id == FLASH_Sector_7)) break;

			status = FLASH_EraseSector_inline(sectors[pos].sector_id, VoltageRange_3);
			if (status != FLASH_COMPLETE)
				break;
#endif

#ifdef STM32F745xx
			FLASH_EraseInitTypeDef erase_info = {
					.TypeErase = FLASH_TYPEERASE_SECTORS,
					.Sector = sectors[pos].sector_id,
					.NbSectors = 1,
					.VoltageRange = FLASH_VOLTAGE_RANGE_3,
			};
			uint32_t error = 0;

			status = HAL_FLASHEx_Erase(&erase_info, &error);
			if (status != HAL_OK)
				break;
#endif

#ifdef STM32H743xx
			FLASH_EraseInitTypeDef erase_info = {
					.TypeErase		= FLASH_TYPEERASE_SECTORS,
					.VoltageRange	= FLASH_VOLTAGE_RANGE_3,
					.Banks			= FLASH_BANK_2,
					.Sector			= sectors[pos].sector_id,
					.NbSectors		= 1,
			};
			uint32_t error = 0;

			status = HAL_FLASHEx_Erase(&erase_info, &error);
			if (status != HAL_OK)
				break;
			status = FLASH_WaitForLastOperation((uint32_t)5000, FLASH_BANK_1);
			if (status != HAL_OK)
				break;
			status = FLASH_WaitForLastOperation((uint32_t)5000, FLASH_BANK_2);
			if (status != HAL_OK)
				break;
#endif

			packet_send(SFU_CMD_ERASE_PART, (uint8_t *)&pos, sizeof(pos));

			uint32_t erased_size = ((uint32_t)sectors[pos].total_size) * ADDR_COMPRESS;
			if (erased_size >= firmware_size)
				break;
		}

		uint32_t t = DWT_CYCCNT;
		uint32_t dummy[256];
		memset(dummy, 0xFF, sizeof(dummy));
		flash_block_write(MAIN_START_FROM, (const uint32_t * )dummy, sizeof(dummy) / sizeof(uint32_t));
		t = DWT_CYCCNT - t;
		//printf("dummy 256 FF write time\t%lu\tuSec\r", t / (SystemCoreClock / 1000000));
		send_str("dummy 256 FF write time\t%lu\tuSec\r");

#ifdef USE_STDPERIPH_DRIVER
		FLASH_Lock_inline();
#endif

#if defined(STM32F746xx) || defined(STM32H743xx)
		HAL_FLASH_Lock();
#endif

		if (status == FLASH_COMPLETE)
		{
			write_addr = MAIN_START_FROM;
			packet_send(code, body, size);
			return;
		}
	}

	packet_send(code, body, 0);
}

#ifdef USE_STDPERIPH_DRIVER
__attribute__ ((long_call, section(".data")))
FLASH_Status flash_block_write(uint32_t wr_addr, uint32_t *data, uint32_t count)
{
	FLASH_Status status = FLASH_BUSY;
	FLASH_Unlock_inline();

	FLASH_ClearFlag_inline(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	while (count--)
	{
		status = FLASH_ProgramWord_inline(wr_addr, *data);
		if (status != FLASH_COMPLETE) break;

		wr_addr += 4;
		data++;
	}
	FLASH_Lock_inline();
	return status;
}
#endif

#ifdef STM32F745xx
HAL_StatusTypeDef flash_block_write(uint32_t wr_addr, uint32_t *data, uint32_t count)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_FLASH_Unlock();


	while (count--)
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, wr_addr, *data);
		if (status != FLASH_COMPLETE) break;

		wr_addr += 4;
		data++;
	}
	HAL_FLASH_Lock();
	return status;
}
#endif

#ifdef STM32H743xx
HAL_StatusTypeDef flash_block_write(uint32_t wr_addr, const uint32_t * const dword_ptr, const uint32_t dword_count)
{
	const uint32_t BLOCK_SIZE = 256 / 8; //Flash minimal write size 256 bit
	HAL_StatusTypeDef status = HAL_ERROR;
	int32_t count = dword_count * 4;
	uint32_t data = (uint32_t)dword_ptr;

	HAL_FLASH_Unlock();
	while (count > 0)
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, wr_addr, data);
		if (status != FLASH_COMPLETE) break;

		wr_addr += BLOCK_SIZE;
		data    += BLOCK_SIZE;
		count   -= BLOCK_SIZE;
	}
	HAL_FLASH_Lock();
	return status;
}
#endif

static void sfu_command_write(uint8_t code, uint8_t *body, uint32_t size)
{
	if (size > 4)
	{
		uint32_t body_addr = deserialize_uint32(body);

		if ((write_addr & 0xFF000000) != FLASH_BASE)
		{
			packet_send(SFU_CMD_WRERROR, body, 0);
			return;
		}

		uint32_t *word_data = (uint32_t*)&(body[4]);
		uint32_t word_count = (size - 4) / 4;

		//printf("WR:\t%08X\t%08X\t%u\r", body_addr, write_addr, word_count);		//send_str("WR\r");

		if ((body_addr == write_addr) && (word_count > 0))
		{
			if (flash_block_write(write_addr, word_data, word_count) != FLASH_COMPLETE)
			{
				write_addr = 0;
				packet_send(SFU_CMD_WRERROR, body, 0);
				return;
			};

			write_addr += (word_count * 4);
		}
	}

	serialize_uint32(body + 0, write_addr);
	serialize_uint32(body + 4, recive_count());

	packet_send(code, body, 8);
}

__attribute__( ( naked ) )
void jump_main(uint32_t stack, uint32_t func)
{
	__set_MSP(stack);
	(*(void (*)())(func))();
}

void main_start()
{
	uint32_t *boot_from = (uint32_t*)MAIN_RUN_FROM;

#ifndef STM32H743xx
	if (((boot_from[0] >> 24) != (SRAM_BASE >> 24)) &&
		((boot_from[0] >> 24) != (CCMDATARAM_BASE >> 24)))
		return send_str("SRAM ERROR\r");
#else
	if (((boot_from[0] >> 24) != (D1_DTCMRAM_BASE >> 24)) &&
		((boot_from[0] >> 24) != (D1_AXISRAM_BASE >> 24)) &&
		((boot_from[0] >> 24) != (D2_AHBSRAM_BASE >> 24)) &&
		((boot_from[0] >> 24) != (D3_SRAM_BASE >> 24)))
		return send_str("SRAM ERROR\r");
#endif

	if (((boot_from[1] >> 24) != (FLASH_BASE >> 24)) && (boot_from[1] > MAIN_RUN_FROM))
		return send_str("FLASH ERROR\r");

	send_str("CONTEXT OK\r\r");

#ifdef USE_STDPERIPH_DRIVER
	usart_deinit();
	RCC_DeInit();
#endif

#if defined(STM32F746xx) || defined(STM32H743xx)
	HAL_UART_DeInit(&huart1);
	HAL_CRC_DeInit(&hcrc);
	HAL_RCC_DeInit();
#ifndef STM32H743xx
	HAL_DeInit();
#else
	__HAL_RCC_AHB1_FORCE_RESET();
	__HAL_RCC_AHB1_RELEASE_RESET();

	__HAL_RCC_AHB2_FORCE_RESET();
	__HAL_RCC_AHB2_RELEASE_RESET();

	__HAL_RCC_AHB4_FORCE_RESET();
	__HAL_RCC_AHB4_RELEASE_RESET();

	__HAL_RCC_APB3_FORCE_RESET();
	__HAL_RCC_APB3_RELEASE_RESET();

	__HAL_RCC_APB1L_FORCE_RESET();
	__HAL_RCC_APB1L_RELEASE_RESET();

	__HAL_RCC_APB1H_FORCE_RESET();
	__HAL_RCC_APB1H_RELEASE_RESET();

	__HAL_RCC_APB2_FORCE_RESET();
	__HAL_RCC_APB2_RELEASE_RESET();

	__HAL_RCC_APB4_FORCE_RESET();
	__HAL_RCC_APB4_RELEASE_RESET();
#endif
	NVIC->ICER[ 0 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 1 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 2 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 3 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 4 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 5 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 6 ] = 0xFFFFFFFF ;
	NVIC->ICER[ 7 ] = 0xFFFFFFFF ;

	NVIC->ICPR[ 0 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 1 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 2 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 3 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 4 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 5 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 6 ] = 0xFFFFFFFF ;
	NVIC->ICPR[ 7 ] = 0xFFFFFFFF ;

	SysTick->CTRL = 0 ;
	SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk ;

	if( CONTROL_SPSEL_Msk & __get_CONTROL( ) )
	{  /* MSP is not active */
	  __set_CONTROL( __get_CONTROL( ) & ~CONTROL_SPSEL_Msk ) ;
	}

	SCB->VTOR = ( uint32_t )MAIN_RUN_FROM;
#endif

	jump_main(boot_from[0], boot_from[1]);
}

static void sfu_command_start(uint8_t code, uint8_t *body, uint32_t size)
{
	if (size != 4) return;

	uint32_t *from = (uint32_t*)MAIN_START_FROM;
	uint32_t count = (write_addr - MAIN_START_FROM);

#if defined(STM32F746xx) || defined(STM32H743xx)
	uint32_t crc = crc_block(from, count / 4);
	uint32_t need = deserialize_uint32(body);
#else
	CRC_ResetDR_inline();
	uint32_t crc = CRC_CalcBlockCRC_inline(from, count / 4);
	uint32_t need = deserialize_uint32(body);
#endif

	serialize_uint32(body + 0, (uint32_t)from);
	serialize_uint32(body + 4, count);
	serialize_uint32(body + 8, crc);

	packet_send(code, body, 12);

	if (crc == need)
	{
		write_addr = 0;

		send('\r');
		send_str("CRC OK\r");

		HAL_FLASH_Lock();
		main_start();
	}
	else
	{
		send_str("CRC != NEED\r");
//		delay_ms(10000);
//		send_file("sfu_crc_base.bin", from, count);
	}
}
