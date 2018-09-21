/*
 * sysFastMemCopy.h
 *
 *  Created on: 2018/07/20
 *      Author: e.sitnikov
 */

#ifndef SYSFASTMEMCOPY_H_
#define SYSFASTMEMCOPY_H_

#include "stdint.h"

void sysFastMemCopy( uint8_t *pDest, const uint8_t *pSrc, uint32_t len );
void sysFastMemCopyAlignedDwords( void *pDest, const void *pSrc, uint32_t len );

#endif /* SYSFASTMEMCOPY_H_ */
