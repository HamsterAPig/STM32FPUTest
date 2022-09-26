/**
 * @Description Observe CPU execution cycles with dwt counters
 * @Author jinming xi
 * @Date 2022/9/26
 * @Project F401CCEnableFPU-DSP
*/
#include "use_dwt.h"

/**
 * DWT Init
 */
void DWT_Init(void)
{
    DEM_CR |= (uint32_t) DEM_CR_TRCENA;
    DWT_CYCCNT = (uint32_t)0U;
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

/**
 * Get CPU execution cycle
 * @return DWT Counter for CPU execution
 */
uint32_t DWT_TS_GET(void)
{
    return ((uint32_t)DWT_CYCCNT);
}