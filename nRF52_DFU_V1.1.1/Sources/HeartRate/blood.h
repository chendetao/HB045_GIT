#include <stdint.h>
//��ȡ��Ѫѹֵ
uint8_t GET_BP_MAX (void);

//��ȡ��Ѫѹֵ
uint8_t GET_BP_MIN (void);

//����Ѫѹ����
void Blood_Process (void);

//��ȡ����ֵ������Ѫѹ�ļ��㣩
void Get_Hr ( unsigned char data);

//�ر�Ѫѹ����
void Blood_Disable (void);

//50ms �ɼ�һ������
void Blood_50ms_process (void);

//�ɼ�����ͳ��
void Blood_500ms_process(void);
