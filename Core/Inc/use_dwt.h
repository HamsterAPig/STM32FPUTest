/**
 * @Description Observe CPU execution cycles with dwt counters
 * @Author jinming xi
 * @Date 2022/9/26
 * @Project F401CCEnableFPU-DSP
*/

#ifndef F401CCENABLEFPU_DSP_USE_DWT_H
#define F401CCENABLEFPU_DSP_USE_DWT_H
#include "stdint.h"

#define DWT_CR *(uint32_t*)0xE0001000
#define DWT_CYCCNT *(uint32_t *)0xE0001004
#define DEM_CR *(uint32_t *)0xE000EDFC
#define DEM_CR_TRCENA (1<<24)
#define DWT_CR_CYCCNTENA (1<<0)

void DWT_Init(void);
uint32_t DWT_TS_GET(void);
#endif //F401CCENABLEFPU_DSP_USE_DWT_H
