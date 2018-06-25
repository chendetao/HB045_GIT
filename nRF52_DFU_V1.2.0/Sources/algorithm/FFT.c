	#include "FFT.h"

	#include <stdlib.h>
	#include <math.h>
	
	/*********************************************************************
	快速傅里叶变换C程序包
	函数简介：此程序包是通用的快速傅里叶变换C语言函数，移植性强，以下部分不依
	赖硬件。此程序包采用联合体的形式表示一个复数，输入为自然顺序的复
	数（输入实数是可令复数虚部为0），输出为经过FFT变换的自然顺序的
	复数.此程序包可在初始化时调用create_sin_tab()函数创建正弦函数表，
	以后的可采用查表法计算耗时较多的sin和cos运算，加快可计算速度
	使用说明：使用此函数只需更改宏定义FFT_N的值即可实现点数的改变，FFT_N的
	应该为2的N次方，不满足此条件时应在后面补0。若使用查表法计算sin值和
	cos值，应在调用FFT函数前调用create_sin_tab()函数创建正弦表
	函数调用：FFT(s);
	时    间：2010-2-20
	版    本：Ver1.1
	参考文献：  
	**********************************************************************/

	float sin_table[FFT_N/2];
	struct compx s[FFT_N];
	short bb[FFT_N];
	float Fn[FFT_N];

	/*******************************************************************
	函数原型：struct compx EE(struct compx b1,struct compx b2)  
	函数功能：对两个复数进行乘法运算
	输入参数：两个以联合体定义的复数a,b
	输出参数：a和b的乘积，以联合体的形式输出
	*******************************************************************/
	#if 1
	struct compx EE(struct compx a, struct compx b)      
	{
		struct compx c;
		
		c.real = a.real*b.real - a.imag*b.imag;
		c.imag = a.real*b.imag + a.imag*b.real;
		
		return(c);
	}

	#else
	#define EE(a,b,c) do\
	{\
		c.real = a.real*b.real - a.imag*b.imag;\
		c.imag = a.real*b.imag + a.imag*b.real;\
	}while(0)	

	#endif
	
	/******************************************************************
	函数原型：void init_sin_table(float *sin_t)
	函数功能：创建一个正弦采样表，采样点数与傅里叶变换点数相同
	输入参数：*sin_t存放正弦表的数组指针
	输出参数：无
	******************************************************************/
	void init_sin_table(float sin_table[])                     
	{
		for ( int i = 0; i < FFT_N/2; i++ )
		{
			sin_table[i] =  (float)sin(2*PI*i/FFT_N);
		}
	}

	/******************************************************************
	函数原型：void sin_tab(float pi)
	函数功能：采用查表的方法计算一个数的正弦值
	输入参数：pi 所要计算正弦值弧度值，范围0--2*PI，不满足时需要转换
	输出参数：输入值pi的正弦值
	******************************************************************/
	float sin_tab(float pi)
	{
		int n;
		float a = 0;
	  
		n = (int)(pi*FFT_N/2/PI);
	   
		if ( (n >= 0) && (n < FFT_N/2) )
		{
			a = sin_table[n];
		}
		else if( (n >= FFT_N/2) && (n <FFT_N) )
		{
			a = -sin_table[n-FFT_N/2];
		}
		
		return a;
	}

	/******************************************************************
	函数原型：void cos_tab(float pi)
	函数功能：采用查表的方法计算一个数的余弦值
	输入参数：pi 所要计算余弦值弧度值，范围0--2*PI，不满足时需要转换
	输出参数：输入值pi的余弦值
	******************************************************************/
	float cos_tab(float pi)
	{
	   float a,pi2;
	   pi2 = pi + PI/2.0f;
	   if ( pi2 > 2.0f*PI )
	   {
		 pi2 -= 2.0f * PI;
	   }
	   a = sin_tab(pi2);
	   return a;
	}
	
	/*****************************************************************
	函数原型：void FFT(struct compx *xin,int N)
	函数功能：对输入的复数组进行快速傅里叶变换（FFT）
	输入参数：*xin复数结构体组的首地址指针，struct型
	输出参数：无
	*****************************************************************/
	void fft_( struct compx *xin)
	{
		int f,m,nv2,nm1,i,k,l,j=0;
	  	struct compx u, w, t;
	   
	  	nv2 = FFT_N/2;  //变址运算，即把自然顺序变成倒位序，采用雷德算法
	  	nm1 = FFT_N-1;  
	  	for ( i = 0; i < nm1; i++ )        
	  	{
	  		if ( i < j )                    //如果i<j,即进行变址
	  		{
	  			t = xin[j];           
	  			xin[j] = xin[i];
	  			xin[i] = t;
	  		}
	  		k = nv2;               //求j的下一个倒位序
	  		while( k <= j )        //如果k<=j,表示j的最高位为1     
	  		{           
				j=j-k;             //把最高位变成0
				k=k/2;             //k/2，比较次高位，依次类推，逐个比较，直到某个位为0
			}
			j = j+k;               //把0改为1
		}
	                         
		{
			int le,lei,ip;         //FFT运算核，使用蝶形运算完成FFT运算
			f = FFT_N;
			for ( l = 1;( f = f/2 ) != 1; l++ )       //计算l的值，即计算蝶形级数
	           ;
			for ( m = 1; m <= l; m++ )                //控制蝶形结级数
			{                                 //m表示第m级蝶形，l为蝶形级总数l=log（2）N
				le = 2<<(m-1);                //le蝶形结距离，即第m级蝶形的蝶形结相距le点
				lei = le/2;                   //同一蝶形结中参加运算的两点的距离
				u.real = 1.0f;                //u为蝶形结运算系数，初始值为1
				u.imag = 0.0f;
				w.real = cos_tab(PI/lei);     //w为系数商，即当前系数与前一个系数的商
				w.imag = -sin_tab(PI/lei);
				for ( j = 0; j <= lei-1; j++ )//控制计算不同种蝶形结，即计算系数不同的蝶形结
				{
					for ( i = j; i <= FFT_N-1; i = i+le )//控制同一蝶形结运算，即计算系数相同蝶形结
					{
						ip = i + lei;                    //i，ip分别表示参加蝶形运算的两个节点
						t = EE(xin[ip],u);               //蝶形运算，详见公式
						// EE(xin[ip],u,t);
						xin[ip].real = xin[i].real-t.real;
						xin[ip].imag = xin[i].imag-t.imag;
						xin[i].real  = xin[i].real+t.real;
						xin[i].imag  = xin[i].imag+t.imag;
					}
	      
					u = EE(u,w);                         //改变系数，进行下一个蝶形运算
					// EE(u,w,u);
				}
			}
		}  
	}


	/*****************************************************************
	函数原型：void fft_init(void)
	函数功能：傅里叶算法初始化函数,建立正选函数表、频点fn
	输入参数：无
	输出参数：无
	*****************************************************************/
	
	void fft_init(void)
	{
		init_sin_table(sin_table);
		
		fft_reset();

		for ( int i = 0; i < FFT_N; i++ )
		{
			Fn[i] = i * (Fs/FFT_N);
		}		
	}
	
	/*****************************************************************
	函数原型：void fft_reset(void)
	函数功能：傅里叶算法复位.
	输入参数：无
	输出参数：无
	*****************************************************************/
	void fft_reset(void)
	{	
		for ( int i = 0; i < FFT_N; i++ )          	//生成一组默认的数据.
		{
			s[i].imag = 0;
			s[i].real = 0; 
			bb[i] = (short)s[i].real;
		}
		
		fft_(s);                                	//进行快速福利叶变换FFT
		
		fft_m();                                    //求变换后的模值
	}

	void fft(void)
	{
		for ( int i = 0; i < FFT_N; i++ )
		{
			s[i].imag = 0;
			s[i].real = bb[i];
		}
		
	    fft_(s);                             		//进行快速傅里叶变换FFT
	}

	/*****************************************************************
	函数原型：void fft_m(void)
	函数功能：计算傅里叶变换后的模值.
	输入参数：无
	输出参数：无
	*****************************************************************/	
	
	void fft_m(void)
	{
	    for ( int i = 0; i < FFT_N; i++ )	//求变换后结果的模值，存入复数的实部部分
	    {
		    s[i].real = (float) (sqrt(s[i].real*s[i].real
		    	+ s[i].imag * s[i].imag));
	    }	
		
		// 搞一个伪陷波器
		
	    /** 60BMP */	   
	    s[18].real *= 0.125f;
	    s[19].real *= 0.105f;
	    s[20].real *= 0.095f;
	    s[21].real *= 0.075f;
	    s[22].real *= 0.095f;
	    s[23].real *= 0.105f;
	    s[24].real *= 0.125f;
	       
	    /** 120BMP */
	    s[38].real *= 0.125f;
	    s[39].real *= 0.105f;
	    s[40].real *= 0.095f;
	    s[41].real *= 0.075f;
	    s[42].real *= 0.095f;
	    s[43].real *= 0.105f;
	    s[44].real *= 0.125f;
	}

	/*****************************************************************
	函数原型：void fft_put(short real, short image)
	函数功能：添加一个数据到数据源中.
	输入参数：无
	输出参数：无
	*****************************************************************/
	
	void fft_put(short real, short image)
	{
		for ( int i = 0; i < FFT_N-1; i++ )
		{
			bb[i] = bb[i+1];
		}
			
		bb[FFT_N-1] = real;
	}

	/*****************************************************************
	函数原型：float get_max_frecency(float min, float max)
	函数功能：求取FFT变换后频率范围min-max中模值最大的频点.
	输入参数：无
	输出参数：无
	*****************************************************************/	
	
	float get_max_frecency(float min, float max)
	{
		float max_frecency;

		int sidx = 0, eidx = 0;
		int idx = 0;
		
		fft_m();

		for ( int i = 0; i < FFT_N; i++ )
		{
			if (Fn[i] > min) 
			{
				sidx = i;
				break;
			}
		}

		for ( int i = FFT_N-1; i >= 0; i-- )
		{
			if ( Fn[i] < max)
			{
				eidx = i;
				break;
			}
		}

		max_frecency = s[sidx].real;
		idx = sidx;
		for( int i = sidx; i < eidx; i++ )
		{
			if ( max_frecency < s[i].real )
			{
				max_frecency = s[i].real;
				idx = i;
			}
		}		
		
		return (float) Fn[idx];
	}

	/*****************************************************************
							THE END
	*****************************************************************/

