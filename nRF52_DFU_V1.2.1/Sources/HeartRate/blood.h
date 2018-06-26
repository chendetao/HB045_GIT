#include <stdint.h>
//获取高血压值
uint8_t GET_BP_MAX (void);

//获取低血压值
uint8_t GET_BP_MIN (void);

//计算血压函数
void Blood_Process (void);

//获取心率值（用于血压的计算）
void Get_Hr ( unsigned char data);

//关闭血压功能
void Blood_Disable (void);

//50ms 采集一次数据
void Blood_50ms_process (void);

//采集数据统计
void Blood_500ms_process(void);
