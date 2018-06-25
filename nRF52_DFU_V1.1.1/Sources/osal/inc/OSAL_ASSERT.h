
#ifndef __OSAL_ASSERT_H__
#define __OSAL_ASSERT_H__

#define OSAL_PANIC(x)

#define OSAL_ASSERT(exprt) do\
{\
    if ( exprt ) { OSAL_PANIC("OSAL Falut\r\n"); while(1); }\
}while(0)

#endif
