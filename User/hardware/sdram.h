/**
 * @brief   : SDRAM驱动(W9825G6KH 16位宽 32M字节)
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#ifndef SDRAM_H
#define SDRAM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "memory.h"


void SDRAM_Init(void);


#ifdef __cplusplus
}
#endif
#endif /* SDRAM_H */


