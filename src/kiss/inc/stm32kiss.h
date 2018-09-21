#ifndef __STM32KISS_H__
#define __STM32KISS_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef USE_STDPERIPH_DRIVER
#include "misc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
//#include "stm32f4xx_bkp.h"
//#include "stm32f4xx_can.h"
//#include "stm32f4xx_cec.h"
#include "stm32f4xx_crc.h"
#include "stm32f4xx_dac.h"
//#include "stm32f4xx_dbgmcu.h"
#include "stm32f4xx_dma.h"
//#include "stm32f4xx_exti.h"
#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"
#endif

typedef struct
{
	uint32_t min;
	uint32_t max;
} tLIMIT;

#ifdef STM32F4XX
#define DEVICE_ID_BLOCK_PTR ((uint8_t*)0x1FFF7A10)

#define FLASH_PAGE_SIZE      1024
#define FLASH_SIZE          (*((uint16_t *)0x1FFF7A22))

#define BOOTLOADER_SIZE     0x8000
#define BOOTLOADER_FROM    (FLASH_BASE)
#define BOOTLOADER_TO      (FLASH_BASE + BOOTLOADER_SIZE)
#define MAIN_START_FROM    (BOOTLOADER_TO)
#define MAIN_RUN_FROM      (FLASH_BASE + 0x10000)
#endif

#ifdef STM32F746xx
#define DEVICE_ID_BLOCK_PTR ((uint8_t*)0x1FF0F420)
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#define FLASH_PAGE_SIZE      1024
#define FLASH_SIZE          (*((uint16_t *) 0x1FF0F442))

#define BOOTLOADER_SIZE     0x8000
#define BOOTLOADER_FROM    (FLASH_BASE)
#define BOOTLOADER_TO      (FLASH_BASE + BOOTLOADER_SIZE)
#define MAIN_START_FROM    (BOOTLOADER_TO)
#define MAIN_RUN_FROM      (FLASH_BASE + 0x10000)

#define SRAM_BASE          SRAM1_BASE
#define CCMDATARAM_BASE    RAMDTCM_BASE
#endif

#ifdef STM32H743xx
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#define DEVICE_ID_BLOCK_PTR ((uint8_t*)0x1FF1E800)

//#define BOOTLOADER_SIZE     0x20000
//#define BOOTLOADER_FROM    (FLASH_BASE)
//#define BOOTLOADER_TO      (FLASH_BASE + BOOTLOADER_SIZE)
//#define MAIN_START_FROM    (BOOTLOADER_TO)
//#define MAIN_RUN_FROM      (FLASH_BASE + BOOTLOADER_SIZE)
//
//#define FLASH_SIZE_CORRECT   ((0x100000 - BOOTLOADER_SIZE) / 1024)

#define BOOTLOADER_SIZE     0x100000
#define BOOTLOADER_FROM    (FLASH_BASE)
#define BOOTLOADER_TO      (FLASH_BASE + BOOTLOADER_SIZE)
#define MAIN_START_FROM    (BOOTLOADER_TO)
#define MAIN_RUN_FROM      (FLASH_BASE + BOOTLOADER_SIZE)

#define FLASH_SIZE_CORRECT   ((FLASH_SIZE - BOOTLOADER_SIZE) / 1024)
#else
#define FLASH_SIZE_CORRECT   ((FLASH_SIZE*1024 - BOOTLOADER_SIZE) / 1024)
#endif

#define FLASH_SIZE_CORRECT_L (FLASH_SIZE_CORRECT & 0xFF)
#define FLASH_SIZE_CORRECT_H (FLASH_SIZE_CORRECT >> 8)

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define AVRG(a,b) (((a) + (b))/2)
#define DELTA(a,b, scale) ((MAX(a, b) - MIN(a, b))*scale) / AVRG(a, b)
#define IN_LIMIT(value, limit) ((limit.min <= value) && (value <= limit.max))

#define MILLION 1000000.0

#define STRUCT_CLEAR(v) memset((void *)&v, 0, sizeof(v))
#define ZERO_MEMORY(v) STRUCT_CLEAR(v)
#define LENGTH(v) (sizeof(v) / sizeof(v[0]))
#define OPT_BARRIER() asm volatile ("": : :"memory")

#define PI 3.1415926535897932384626433832795f

#define UNUSED_A __attribute__ ((unused))

#pragma GCC diagnostic ignored "-Wformat"

//#include "stm32kiss_adc.h"
//#include "stm32kiss_dac.h"
//#include "stm32kiss_gpio.h"
#include "stm32kiss_ticks.h"
//#include "stm32kiss_button.h"
//#include "stm32kiss_fifo.h"
//#include "stm32kiss_dma_usarts.h"

//void PrintChar(char c);
//signed int printf(const char *pFormat, ...);

extern const void *ptr_QSPI;
extern const void *ptr_SDRAM;
extern const void *ptr_SRAM_D1;
extern const void *ptr_SRAM_D2;
extern const void *ptr_SRAM_D3;


#include "usart_mini.h"
extern char __printf_buf[512];
//signed int snprintf_v2(char *pString, size_t length, const char *pFormat, ...);
#define printf(format, ...) {snprintf(__printf_buf, sizeof(__printf_buf)-1, format, ##__VA_ARGS__); send_str(__printf_buf);}

#if defined(STM32F746xx) || defined(STM32H743xx)

static inline uint32_t crc_block(const uint32_t *data, uint32_t cnt)
{
	CRC->CR |= CRC_CR_RESET;
	while (cnt--)
		CRC->DR = *(data++);
	return CRC->DR;
}

static inline uint32_t crc_calc(const void *data, uint32_t size)
{
	//printf("ERROR\tCRC\t%08lX\t%08lX\n", (uint32_t)data, (uint32_t)size);
	return crc_block(data, size / 4);
}
#endif

//#define BITBAND_SRAM(a,b) ((uint32_t *)(SRAM_BB_BASE + ((((uint32_t)(a)) - SRAM_BASE)*32 + (b*4))))  // Convert SRAM address
#endif //#ifndef __STM32KISS_H__
