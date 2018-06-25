#ifndef __FPU_H__
#define __FPU_H__

#define CONFIG_USING_FPU 			1

#define FPU_EXCEPTION_MASK 0x0000009F
#define FPU_IRQ_PRIORITY_LOW 6

inline unsigned long get_FPSCR(void)
{
  register unsigned long __regfpscr __asm("fpscr");
  return(__regfpscr);	
}


inline void set_FPSCR(unsigned long fpscr)
{
	register unsigned long __regfpscr __asm("fpscr");
  __regfpscr = (fpscr);
}

#endif
