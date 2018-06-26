#ifndef __OSAL_PWR_MGR_H__
#define __OSAL_PWR_MGR_H__

typedef struct
{
	unsigned long sleep_enable;
}GlobalConfiguration;

extern GlobalConfiguration GCongfiguration;

extern void osal_pwrmgr_powerconserve( void );

#endif
