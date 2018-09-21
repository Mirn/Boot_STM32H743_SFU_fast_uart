/*
 * hw_init.c
 *
 *  Created on: Sep 20, 2018
 *      Author: sj21d
 */

#include "stm32kiss.h"

CRC_HandleTypeDef hcrc;

__attribute__((weak)) void __libc_init_array() {};

void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc1)
{
	if(hcrc1->Instance !=CRC) return;
	__HAL_RCC_CRC_CLK_ENABLE();
}

void MX_CRC_Init(void)
{
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS;

	HAL_CRC_Init(&hcrc);
}

extern const int _itcm_sidata;
extern const int _itcm_sdata;
extern const int _itcm_edata;

extern const int _sidata;
extern const int _sdata;
extern const int _edata;
extern const int _isr_vector_addr;

void __BeforeInitCallback()
{
	SCB->VTOR = (uint32_t)(&_isr_vector_addr);
	SCB_EnableICache();
	SCB_EnableDCache();

	uint32_t *src = (uint32_t *)(&_itcm_sidata);
	uint32_t *dst = (uint32_t *)(&_itcm_sdata);
	while (((uint32_t)dst) < (uint32_t)(&_itcm_edata))
		*(dst++) = *(src++);
	//memcpy((void*)(&_itcm_sdata), (void *)(&_itcm_sidata), ((uint32_t)(&_itcm_edata) - (uint32_t)(&_itcm_sdata)));
}

void hw_printf_sections()
{
	printf("_isr_vector_addr\t%08lX\n", (uint32_t)(&_isr_vector_addr));
	printf("SCB->VTOR\t%08lX\r", SCB->VTOR);
	printf("\n");

	printf("\n");
	printf("_sidata\t%08lX\n", (uint32_t)(&_sidata));
	printf("_sdata \t%08lX\n", (uint32_t)(&_sdata));
	printf("_edata \t%08lX\n", (uint32_t)(&_edata));
	printf("\n");
	printf("_itcm_sidata\t%08lX\n", (uint32_t)(&_itcm_sidata));
	printf("_itcm_sdata \t%08lX\n", (uint32_t)(&_itcm_sdata));
	printf("_itcm_edata \t%08lX\n", (uint32_t)(&_itcm_edata));
	printf("\n");
}

void hw_init_all()
{
	SystemCoreClockUpdate();
	HAL_Init();

	MX_CRC_Init();

	SystemCoreClockUpdate();

	ticks_init();
	//delay_ms(1000);
	usart_init();
}
