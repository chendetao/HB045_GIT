#include "step_process.h"
#include "bma2x2.h"
#include "bma253.h"

struct AccelDevice accelDevice;
unsigned int mTicks = 0;

////////////////////////////////////////////////////////////////////////////////

#define FILTER_NR    9

static int filter_arr[3][FILTER_NR];

/** 
 * func void accel_filter_init(void);
 * To init the filter array.
 */
void accel_filter_init(void)
{
	for ( register unsigned int i = 0; i < 2; i++ )
	{
		for ( register unsigned int j = 0; j < FILTER_NR; j++ )
		{
			filter_arr[i][j] = 0;
		}
	}
}

int accel_filter( unsigned int idx, unsigned int value )
{
	register int sum = 0;

	for ( register unsigned int i = 0; i < FILTER_NR-1; i++ )
	{
		filter_arr[idx][i] = filter_arr[idx][i+1];
        sum += filter_arr[idx][i+1]; 
	}

	filter_arr[idx][FILTER_NR-1] = value;
    sum += value;
	
	return sum / FILTER_NR;
}

int Find_Max_int(int * pdata, int len)
{
  int max = pdata[0];
  
  for(int i=1;i<len;i++)
  {
    if(pdata[i]>max)
    {
      max = pdata[i];
    }
  }
  
  return max;
}

int Find_Min_int(int * pdata, int len)
{
  int min = pdata[0];
  
  for(int i=1;i<len;i++)
  {
    if(pdata[i]<min)
    {
      min = pdata[i];
    }
  }
  
  return min;
}

///////////////////////////////////////////////////////////

struct AccelDevice accelDevice;

static  int X[5];

unsigned int calcuX( unsigned int value )
{
	//static int dirflag = 0/*, toMidleLines = 0*/;
	static int /*mTicks = 0, *//*hTicks = 0,*/ lTicks = 0;
	int D4MaxMin;// = 0/*, done = 0*/;

	X[0] = X[1];
	X[1] = X[2];
	X[2] = X[3];
	X[3] = X[4];
	X[4] = value;

	/* Find the wave crest */
	if ( (X[0]<X[1] && X[1]<X[2] && X[2]>X[3] && X[3]>X[4]) ||
		((X[0]==X[1]) && (X[1]>X[2] && X[2]>X[3] && X[3]>X[4])) ||
		( X[3] > X[2] && X[3] > X[4] ) ) 
	{
		// accelDevice.hightCnt[0]++;
		// accelDevice.MaxThreshold[0] = X[1];
		accelDevice.MaxThreshold[0] = Find_Max_int(X, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[0] - accelDevice.MinThreshold[0];

		//accelDevice.MidleThreshold[0] =  (int)(accelDevice.MaxThreshold[0] - D4MaxMin * 0.2);

		if ( ((mTicks-lTicks)>=200) && ((mTicks-lTicks)<=2000) )
		{
			
		}

		//hTicks = mTicks;
	}

	/* Find the wave trough */
	if (  (X[0]>X[1] && X[1]>X[2] && X[2]<X[3] && X[3]<=X[4])
		||  ( (X[0]==X[1]) && (X[1]<X[2] && X[2]<X[3] && X[3]<=X[4]) ) ||
		( X[3] < X[2] && X[3] < X[4] ) ) 
	{
		// accelDevice.lowCnt[0]++;
		// accelDevice.MinThreshold[0] = X[1];
		accelDevice.MinThreshold[0] = Find_Min_int(X, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[0] - accelDevice.MinThreshold[0];

		//accelDevice.MidleThreshold[0] =  (int)(accelDevice.MinThreshold[0] + D4MaxMin * 0.2);
			
		if ( ( ((mTicks - lTicks) >= 200) &&((mTicks - lTicks) <= 2000))
			 && (D4MaxMin > 35))
            //      && ( D4MaxMin > 50) )
		{			
			int temp = accelDevice.iCounter[0] + 1;
			
			if ( D4MaxMin > 42 )
			{				
				accelDevice.Counter[0]++;
				accelDevice.iCounter[0]++;
			}

			int max = (accelDevice.iCounter[1] > accelDevice.iCounter[2])
                ?(accelDevice.iCounter[1]):(accelDevice.iCounter[2]);

			if ( D4MaxMin > 42 )
			{			
				if ( accelDevice.iCounter[0] < max )
				{
					accelDevice.iCounter[0] = max;
					accelDevice.allCounter  = max;
				}else{
					accelDevice.allCounter = accelDevice.iCounter[0];			
				}
			}
			
			if ( max <= temp )
			{
				accelDevice.WaveCounter++;
				accelDevice.WaveAmplitudeSum += D4MaxMin;				
			}
		}

		lTicks = mTicks;
	}

	return 0;
}

static int Y[5];

unsigned int calcuY( unsigned int value )
{
	// static int dirflag = 0/*, toMidleLines = 0*/;
    static int /*mTicks = 0*//*, hTicks = 0, */ lTicks = 0;
	int D4MaxMin;// = 0/*, done = 0*/;

	Y[0] = Y[1];
	Y[1] = Y[2];
	Y[2] = Y[3];
	Y[3] = Y[4];
	Y[4] = value;

    /* Find the wave crest */
	if ( (Y[0]<Y[1] && Y[1]<Y[2] && Y[2] > Y[3] && Y[3] > Y[4]) ||
		((Y[0]==Y[1]) && (Y[1]>Y[2] && Y[2]>Y[3] && Y[3]>Y[4]))	||
		( Y[3] > Y[2] && Y[3] > Y[4] ) ) 
	{
		// accelDevice.hightCnt[1]++;
		// accelDevice.MaxThreshold[1] = Y[1];
		accelDevice.MaxThreshold[1] = Find_Max_int(Y, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[1] - accelDevice.MinThreshold[1];

		//accelDevice.MidleThreshold[1] =  (int)(accelDevice.MaxThreshold[1] - D4MaxMin * 0.2);

		if ( ((mTicks-lTicks)>=200) && ((mTicks-lTicks)<=2000) )
		{
			
		}

		// hTicks = mTicks;
	}

	/* Find the wave trough */
	if (  (Y[0]>Y[1] && Y[1]>Y[2] && Y[2]<Y[3] && Y[3]<=Y[4])
		||  ( (Y[0]==Y[1]) && (Y[1]<Y[2] && Y[2]<Y[3] && Y[3]<=Y[4]) ) ||
		( Y[3] < Y[2] && Y[3] < Y[4] ) )
	{
		// accelDevice.lowCnt[1]++;
		// accelDevice.MinThreshold[1] = Y[1];
		accelDevice.MinThreshold[1] = Find_Min_int(Y, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[1] - accelDevice.MinThreshold[1];

		//accelDevice.MidleThreshold[1] =  (int)(accelDevice.MinThreshold[1] + D4MaxMin * 0.2);
			
		if ( ( ((mTicks - lTicks) >= 200) &&((mTicks - lTicks) <= 2000))
			 && ( D4MaxMin > 35) )
            // && ( D4MaxMin > 50) )
		{
			int temp = accelDevice.iCounter[1] + 1;
			
			if ( D4MaxMin > 42 )
			{
				accelDevice.Counter[1]++;
				accelDevice.iCounter[1]++;
			}
			
			int max = (accelDevice.iCounter[0] > accelDevice.iCounter[2])
                ?(accelDevice.iCounter[0]):(accelDevice.iCounter[2]);

			if ( D4MaxMin > 42 )
			{
				if ( accelDevice.iCounter[1] < max )
				{
					accelDevice.iCounter[1] = max;
					accelDevice.allCounter  = max;
				}else{
					accelDevice.allCounter = accelDevice.iCounter[1];				
				}
			}
			
			if ( max <= temp )
			{
				accelDevice.WaveCounter++;
				accelDevice.WaveAmplitudeSum += D4MaxMin;				
			}			
		}

		lTicks = mTicks;
	}

	return 0;
}

static int Z[5];

unsigned int calcuZ( unsigned int value )
{
	// static int dirflag = 0/*, toMidleLines = 0*/;
    static int /*mTicks = 0*//*, hTicks = 0,*/ lTicks = 0;
	int D4MaxMin;// = 0/*, done = 0*/;

	Z[0] = Z[1];
	Z[1] = Z[2];
	Z[2] = Z[3];
	Z[3] = Z[4];
	Z[4] = value;

	/* Find the wave crest */
	if ( (Z[0]<Y[1] && Z[1]<Z[2] && Z[2] > Z[3] && Z[3] > Z[4]) ||
		((Z[0]==Z[1]) && (Z[1]>Z[2] && Z[2]>Z[3] && Z[3]>Z[4]))	||
		( Z[3] > Z[2] && Z[3] > Z[4] ) )
	{
		// accelDevice.hightCnt[2]++;
		// accelDevice.MaxThreshold[2] = Z[1];
		accelDevice.MaxThreshold[2] = Find_Max_int(Z, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[2] - accelDevice.MinThreshold[2];

		//accelDevice.MidleThreshold[2] =  (int)(accelDevice.MaxThreshold[2] - D4MaxMin * 0.2);

		if ( ((mTicks-lTicks)>=200) && ((mTicks-lTicks)<=2000) )
		{
			
		}

		// hTicks = mTicks;
	}

	/* Find the wave trough */
	if (  (Z[0]>Z[1] && Z[1]>Z[2] && Z[2]<Z[3] && Z[3]<=Z[4])
		||  ( (Z[0]==Z[1]) && (Z[1]<Z[2] && Z[2]<Z[3] && Z[3]<=Z[4]) ) ||
		( Z[3] < Z[2] && Z[3] < Z[4] ) )
	{
		// accelDevice.lowCnt[2]++;
		// accelDevice.MinThreshold[2] = Z[1];
		accelDevice.MinThreshold[2] = Find_Min_int(Z, 5);
		
		D4MaxMin = accelDevice.MaxThreshold[2] - accelDevice.MinThreshold[2];

		//accelDevice.MidleThreshold[2] = (int)(accelDevice.MinThreshold[2] + D4MaxMin * 0.2);
			
		if ( ( ((mTicks - lTicks) >= 200) &&((mTicks - lTicks) <= 2000))
			 && ( D4MaxMin > 35) )
            //        && ( D4MaxMin > 50) )
		{
			int temp = accelDevice.iCounter[2] + 1;
			
			if ( D4MaxMin > 42 )
			{				
				accelDevice.Counter[2]++;
				accelDevice.iCounter[2]++;
			}
                                             
			unsigned int max = (accelDevice.iCounter[1] > accelDevice.iCounter[0])
                ?(accelDevice.iCounter[1]):(accelDevice.iCounter[0]);

			if ( D4MaxMin > 42 )
			{
				if ( accelDevice.iCounter[2] < max )
				{
					accelDevice.iCounter[2] = max;
					accelDevice.allCounter  = max;
				}else{
					accelDevice.allCounter = accelDevice.iCounter[2];				
				}
			}
			
			if ( max <= temp )
			{
				accelDevice.WaveCounter++;
				accelDevice.WaveAmplitudeSum += D4MaxMin;				
			}			
		}

		lTicks = mTicks;
	}

	return 0;
}

unsigned int Step_main_thread(struct bma2x2_accel_data_fifo *accel)
{   
    struct bma2x2_accel_data_fifo *pa;
    register int val;
    
    pa = accel;
    
    val = pa->x + 512;      
    val = accel_filter(0, val);
    calcuX(val);
    
    val = pa->y + 512;      
    val = accel_filter(1, val);
    calcuY(val);
    
    val = pa->z + 512;      
    val = accel_filter(2, val);
    calcuZ(val);   
    
    mTicks += 25;
  
    return 0;
}
////////////////////////////////////////////////////////////////////////////////

/**
 * 0.00016656 sec = 166.56us
 * 0.00016566 sec = 165.66us
 * 0.00016550 sec = 165.50us
 * compiler with optimize for time 0.00013581 sec = 135.81us
 */

extern struct bma2x2_accel_data_fifo accel_fifo[];

void AccelHandle(void)
{    
	#if (FIFO_DEPTH == 16)
    for( register int i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH/2 )
    {
        Step_main_thread(&accel_fifo[i]);
        Step_main_thread(&accel_fifo[i+1]);
        Step_main_thread(&accel_fifo[i+2]);
        Step_main_thread(&accel_fifo[i+3]);
        Step_main_thread(&accel_fifo[i+4]);
        Step_main_thread(&accel_fifo[i+5]);
        Step_main_thread(&accel_fifo[i+6]);
        Step_main_thread(&accel_fifo[i+7]); 
	}
	#elif (FIFO_DEPTH == 20)
    for( register int i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH/2 )
    {
        Step_main_thread(&accel_fifo[i]);
        Step_main_thread(&accel_fifo[i+1]);
        Step_main_thread(&accel_fifo[i+2]);
        Step_main_thread(&accel_fifo[i+3]);
        Step_main_thread(&accel_fifo[i+4]);
        Step_main_thread(&accel_fifo[i+5]);
        Step_main_thread(&accel_fifo[i+6]);
        Step_main_thread(&accel_fifo[i+7]); 
        Step_main_thread(&accel_fifo[i+8]);
        Step_main_thread(&accel_fifo[i+9]);		
	}
	#elif (FIFO_DEPTH == 31)
    for( register int i = 0; i < FIFO_DEPTH; i += FIFO_DEPTH/2 )
    {
        Step_main_thread(&accel_fifo[i]);
        Step_main_thread(&accel_fifo[i+1]);
        Step_main_thread(&accel_fifo[i+2]);
        Step_main_thread(&accel_fifo[i+3]);
        Step_main_thread(&accel_fifo[i+4]);
        Step_main_thread(&accel_fifo[i+5]);
        Step_main_thread(&accel_fifo[i+6]);
        Step_main_thread(&accel_fifo[i+7]); 
        Step_main_thread(&accel_fifo[i+8]);
        Step_main_thread(&accel_fifo[i+9]);		
	}	
		Step_main_thread(&accel_fifo[30]);
	#else
	#error "FIFO DEPTH Error"
	#endif
}
