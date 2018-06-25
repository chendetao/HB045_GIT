#ifndef __OSAL_H__
#define __OSAL_H__

extern unsigned long osal_clear_event( unsigned long , unsigned long);
extern unsigned long osal_set_event( unsigned long , unsigned long);
extern void osal_start_system( void );
extern void osal_run_system ( void );

extern unsigned long sleepEnable;

static inline unsigned int __Myget_PRIMASK(void)
{
  register unsigned int __regPriMask    __asm("primask");
  return(__regPriMask);
}

static inline void __Myset_PRIMASK(unsigned int priMask)
{
  register unsigned int __regPriMask         __asm("primask");
  __regPriMask = (priMask);
}

#define entry_critical(x)  do{ (x = __Myget_PRIMASK()); __disable_irq(); }while(0)
#define leave_critical(x)  do{ (__Myset_PRIMASK(x)); }while(0)

#define SIZEOF(x) (sizeof(x)/sizeof(x[0]))

#endif
