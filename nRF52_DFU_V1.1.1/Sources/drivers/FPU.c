
#include "FPU.h"
#include "nrf52.h"

void FPU_IRQHandler(void)
{
    unsigned long *fpscr = (unsigned long *)(FPU->FPCAR+0x40);
    (void)get_FPSCR();

    *fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}
