#include "string.h"
#include "time.h"
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_Flash.h"

#include "ClockTick.h"
#include "btprotocol.h"
#include "UserSetting.h"
#include "Log.h"

struct Queue q;
unsigned char qbuf[4096/QUEUE_ITEM_SIZE][QUEUE_ITEM_SIZE];

/*******************************************************************************
 *                                                                             *
 *  从Flash加载历史数据到内存队列中,以便快速发送,避免频繁操作Flash             *
 *                                                                             *
 ******************************************************************************/

int store_queue( int identifier, struct Queue *q, int length)
{
	if ( q->lock == 1 )  /* 队列中包含有数据,无法继续加载，稍后重试  */
	{
		return (QUEUE_LOCKED);
	}
	
	if ( fmc.lock == 1 ) /* Flash正在被lock,无法操作，稍后重试 */
	{
		return (QUEUE_LOCKED);
	}
	
	if ( fm.erea[identifier].items == 0 )
	{
		return (QUEUE_EMPTY);
	}
	
	q->lock  = 1;
	q->front = q->tail = 0;
	q->items = 0;
	
	/**
	 * 将所有数据加载到队列中.
	 */
	while( (fm.erea[identifier].items != 0) && (q->items < 256) )
	{
		flash_memory_get( identifier, qbuf[q->tail], length, false);
		q->tail++;
		q->items++;
	}
	
	/**
     * 数据加载完成之后，更新信息控制块.	
	 */
	osal_set_event( taskFlashTaskId, TASK_FLASH_UPDATE_EVT );
	
	return 0;
}

/*******************************************************************************
 *                                                                             *
 *  从RAM队列中取一条数据,数据取完成之后unlock                                 *
 *                                                                             *
 ******************************************************************************/

int load_queue( int identifier, struct Queue *q, unsigned char *buf, int length )
{	
	if ( q->items > 0 )
	{
		memcpy( buf, qbuf[q->front], length );
		
		q->front++;
		q->items--;
	}else{
		return (QUEUE_EMPTY);
	}
	
	/** 取完成最后一条数据之后,队列空，则解锁写入操作 */
	if ( q->items == 0 )
	{
		q->lock  = 0;
	}
	
	return 0;
}

/*******************************************************************************
 *                                                                             *
 *  蓝牙断开后将Queue重置,避免死锁                                             *
 *                                                                             *
 ******************************************************************************/

int reset_queue( void )
{
	q.front = q.tail = 0;
	q.items = 0;
	q.lock  = 0;
	
	return 0;
}
	
unsigned long taskUploadTaskId;

unsigned long taskUpload( unsigned long task_id, unsigned long events )
{
	static unsigned char pedo_all 	= 0;	
    static unsigned char pedo_done 	= 0;
	static unsigned char sleep_all 	= 0;	
	static unsigned char sleep_done = 0;
    static unsigned char hr_all     = 0;
	static unsigned char hr_done    = 0;
	static unsigned char bp_done    = 0;
	static unsigned char bp_all     = 0;
	static unsigned char spo2_done  = 0;
	static unsigned char spo2_all   = 0;
	static unsigned char seg_pedo_done_all = 0;
	static unsigned char seg_pedo_done   = 0;	
	static unsigned char running_all = 0;
	static unsigned char running_done = 0;
	
    static unsigned char txbuf[20]  = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	
    unsigned char buf[16];
	int fret;
		
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the pedometer data saved in NV Flash to the app                 //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////	

    if( events & TASK_UPLOAD_PEDO_1_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
		
        if( ble.isConnected != 1){
            return (events ^ TASK_UPLOAD_PEDO_1_EVT );
        }
		
		memset( txbuf, 0, sizeof(txbuf));
 
        txbuf[0] = 0x03;
        txbuf[1] = 0xC0;
		txbuf[2] = pedo_all = q.items;
        txbuf[3] = pedo_done = 0;
        txbuf[19]= 0x0;
        
        if ( pedo_all != 0 )
        {
         	fret = load_queue(FM_ID_PEDOMETER, &q, buf, 16);
			if ( fret == QUEUE_EMPTY )			
			{
				return ( events ^ TASK_UPLOAD_PEDO_1_EVT ); 
			}			
			memcpy( txbuf+4, buf, 16 );
        }else
        {
            memset( txbuf+4, 0, 8);
        }
        
        bt_protocol_tx( txbuf, sizeof(txbuf) ); 
        
        if ( ++pedo_done < pedo_all ){  /* Done ? */
            osal_start_timerEx( task_id, TASK_UPLOAD_PEDO_2_EVT, 75);
        }
        
        return (events ^ TASK_UPLOAD_PEDO_1_EVT);
    }
    
    if ( events & TASK_UPLOAD_PEDO_2_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
         
        if ( ble.isConnected != 1 )
        {
            pedo_done = 0;
            return ( events ^ TASK_UPLOAD_PEDO_2_EVT );
        }        
        
		memset( txbuf, 0, sizeof(txbuf));
		
        txbuf[0] = 0x3; txbuf[1] = 0xC0; 
        txbuf[2] = pedo_all; txbuf[3] = pedo_done;

		fret = load_queue(FM_ID_PEDOMETER, &q, buf, 16);
		if ( fret == QUEUE_EMPTY )		
		{
			return ( events ^ TASK_UPLOAD_PEDO_2_EVT );
		}		

        if ( pedo_done++ < pedo_all )
        {
            txbuf[19]= 0x0;      /* CRC */
            
			memcpy( txbuf+4, buf, 16);
			
            bt_protocol_tx( txbuf, sizeof(txbuf) ); 
            osal_start_timerEx(task_id, TASK_UPLOAD_PEDO_2_EVT,75);
        }else{
            pedo_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_PEDO_2_EVT );
    }
    
	
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the sleep data saved in NV Flash to the app                     //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////	
    
    if( events & TASK_UPLOAD_SL_1_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
        if( ble.isConnected != 1){
            return ( events ^ TASK_UPLOAD_SL_1_EVT );
        }
		
		memset( txbuf, 0, sizeof(txbuf));
 
        txbuf[0] = 0x0C; 
		txbuf[1] = 0x01;
        txbuf[2] = sleep_all = q.items;
        txbuf[3] = sleep_done = 0;
        txbuf[19] = 0x0;
        
        if ( sleep_done < sleep_all )
        {
        	fret = load_queue( FM_ID_SLEEP, &q, buf, 16 );
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_SL_1_EVT ); 
			}
			memcpy( txbuf+4, buf, 16);
        }else
        {
            memset( txbuf+4, 0, 16);
        }
        
        bt_protocol_tx ( txbuf, sizeof(txbuf) ); 
        
        if ( ++sleep_done < sleep_all ){  /* Done ? */
            osal_start_timerEx( task_id, TASK_UPLOAD_SL_2_EVT, 75);
        }
        
        return (events ^ TASK_UPLOAD_SL_1_EVT);
    }
    
    if ( events & TASK_UPLOAD_SL_2_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
         
        if( ble.isConnected != 1)
        {
            sleep_done = 0;
            return ( events ^ TASK_UPLOAD_SL_2_EVT );
        }        
        
		memset( txbuf, 0, sizeof(txbuf));
		
        txbuf[0] = 0x0C; 
		txbuf[1] = 0x01; 
        txbuf[2] = sleep_all; txbuf[3] = sleep_done;
	
		fret = load_queue(FM_ID_PEDOMETER, &q, buf, 16);
		if ( fret == QUEUE_EMPTY ) 
		{
			return ( events ^ TASK_UPLOAD_SL_2_EVT );
		}		
		
        if ( sleep_done++ < sleep_all )
        {
            txbuf[19]= 0x0;      /* CRC */
			
			memcpy( txbuf+4, buf, 16);
            
            bt_protocol_tx ( txbuf, sizeof(txbuf) ); 
            osal_start_timerEx(task_id, TASK_UPLOAD_SL_2_EVT, 75);
        }else{
            sleep_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_SL_2_EVT );
    }  
    
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the heart rate saved in NV Flash to the app                     //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////	
	
    if( events & TASK_UPLOAD_HR_1_EVT )
    {        
        /** Dont' send if bluetooth is disconnected */
        if( ble.isConnected != 1){
            return (events ^ TASK_UPLOAD_HR_1_EVT);
        }
		
		memset( txbuf, 0, sizeof(txbuf));
 
        hr_all = q.items;
        hr_done  = 0;
        
		// 心率
		// 0x0A 01 00 00 00 00 YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
        txbuf[0] = 0x0A;
        txbuf[1] = 0x01;
        txbuf[2] = hr_all / 256;
        txbuf[3] = hr_all % 256;
        txbuf[4] = hr_done/256;
        txbuf[5] = hr_done % 256;
        txbuf[19]= 0x0;
        
        if ( hr_all != 0 )
        {
        	fret = load_queue(FM_ID_HR, &q, buf, 16);
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_HR_1_EVT ); 
			}
        
			// Date & Time Hr
			memcpy( txbuf+6, buf, 7);			
        }else
        {
            memset( buf, 0, 16);
        }
        
        bt_protocol_tx ( txbuf, sizeof(txbuf) ); 
			
        if ( ++hr_done < hr_all ){  /* Done */
            
            osal_start_timerEx( task_id, TASK_UPLOAD_HR_2_EVT, 75);
        }     

		return ( events ^ TASK_UPLOAD_HR_1_EVT );
	}
	
	if ( events & TASK_UPLOAD_HR_2_EVT )
	{
        /** Dont' send if bluetooth is disconnected */
		
        if ( ble.isConnected != 1 )
        {
            hr_done = 0;
            return ( events ^ TASK_UPLOAD_HR_2_EVT );
        }       

		memset( txbuf, 0, sizeof(txbuf));		
        
        txbuf[0] = 0x0A;
        txbuf[1] = 0x01;
        txbuf[2] = hr_all / 256;
        txbuf[3] = hr_all % 256;
        txbuf[4] = hr_done/256;
        txbuf[5] = hr_done % 256;
		
		fret = load_queue( FM_ID_HR, &q, buf, 16 );
		if ( fret == QUEUE_EMPTY )
		{
			return ( events ^ TASK_UPLOAD_HR_2_EVT );
		}			
           
        if ( hr_done++ < hr_all  )
        {   
            txbuf[19]= 0x0;      /* CRC */
           
            // 0x0A 01 Ah Al Ch Cl YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
            memcpy( txbuf+6, buf, 7);
        
            bt_protocol_tx ( txbuf, sizeof(txbuf) );  
            
            osal_start_timerEx(task_id, TASK_UPLOAD_HR_2_EVT, 75);
        }else{
            hr_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_HR_2_EVT );
    }
	
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the blood pressure saved in NV Flash to the app                 //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////
	
	if ( events & TASK_UPLOAD_B_PRESSURE_1_EVT )
	{
        /** Dont' send if bluetooth is disconnected */
        if ( ble.isConnected != 1 )
        {
            return ( events ^ TASK_UPLOAD_B_PRESSURE_1_EVT );
        } 
		
		memset( txbuf, 0, sizeof(txbuf));
		
		bp_all = q.items;
        bp_done  = 0;

		// 血压 0x11 01 Ah Al Ch Cl YY MM DD hh mm ss Hp Lp Hr 00 00 00 00 00
        txbuf[0] = 0x11;
        txbuf[1] = 0x01;
        txbuf[2] = bp_all / 256;
        txbuf[3] = bp_all % 256;
        txbuf[4] = bp_done/256;
        txbuf[5] = bp_done % 256;
        txbuf[19] = 0x0;

        if ( bp_all != 0 )
        {
        	fret = load_queue( FM_ID_B_PRESSURE, &q, buf, 16 );
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_B_PRESSURE_1_EVT ); 
			}
			memcpy(txbuf+6, buf, 9);
        }		
        
        bt_protocol_tx ( txbuf, sizeof(txbuf) ); 
        
        if ( ++bp_done < bp_all ){  /* Done */

            osal_start_timerEx( task_id, TASK_UPLOAD_B_PRESSURE_2_EVT, 75);
        }
        
        return (events ^ TASK_UPLOAD_B_PRESSURE_1_EVT);
    }
	
	if ( events & TASK_UPLOAD_B_PRESSURE_2_EVT )
	{
        /** Dont' send if bluetooth is disconnected */
         
        if ( ble.isConnected != 1 )
        {
            bp_done = 0;
            return ( events ^ TASK_UPLOAD_B_PRESSURE_2_EVT );
        }        
        
		memset( txbuf, 0, sizeof(txbuf));
		
        txbuf[0] = 0x11;
        txbuf[1] = 0x01;
        txbuf[2] = bp_all / 256;
        txbuf[3] = bp_all % 256;
        txbuf[4] = bp_done/256;
        txbuf[5] = bp_done % 256;

		fret = load_queue( FM_ID_B_PRESSURE, &q, buf, 16 );
		if ( fret == QUEUE_EMPTY )		
		{
			return ( events ^ TASK_UPLOAD_B_PRESSURE_2_EVT );
		}
				
        if ( bp_done++ < bp_all )
        {   
            txbuf[19]= 0x0;      /* CRC */
            
            // 0x11 01 Ah Al Ch Cl YY MM DD hh mm ss Hp Lp Hr 00 00 00 00 00
            memcpy( txbuf+6, buf, 9);
        
            bt_protocol_tx ( txbuf, sizeof(txbuf) );  
            
            osal_start_timerEx(task_id, TASK_UPLOAD_B_PRESSURE_2_EVT, 75);
        }else{
            bp_done = 0;
        }	
		
		return ( events ^ TASK_UPLOAD_B_PRESSURE_2_EVT );
	}	
    
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the SP02 saved in NV Flash to the app                           //
    //                                                                        //
    //////////////////////////////////////////////////////////////////////////// 	
	
    if( events & TASK_UPLOAD_SP02_1_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
        if( ble.isConnected != 1){
            return (events ^ TASK_UPLOAD_SP02_1_EVT );
        }
		
		memset( txbuf, 0, sizeof(txbuf));
		
        spo2_all = q.items;
        spo2_done  = 0;
        
		// 血氧
        txbuf[0] = 0x12;
        txbuf[1] = 0x01;
        txbuf[2] = spo2_all / 256;
        txbuf[3] = spo2_all % 256;
        txbuf[4] = spo2_done/256;
        txbuf[5] = spo2_done % 256;
        txbuf[19] = 0x0;
		
        if ( spo2_all != 0 )
        {
        	fret = load_queue( FM_ID_SPO2, &q, buf, 16 );
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_SP02_1_EVT ); 
			}
			
			memcpy( txbuf + 6, buf, 8);
        }else
        {
            memset( txbuf + 6, 0, 8);
        }
        		
        bt_protocol_tx( txbuf, sizeof(txbuf) ); 
		        
        if ( ++spo2_done < spo2_all ){  /* Done ? */

            osal_start_timerEx( task_id, TASK_UPLOAD_SP02_2_EVT, 75);
        }
        
        return (events ^ TASK_UPLOAD_SP02_1_EVT);
    }
    
    if ( events & TASK_UPLOAD_SP02_2_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
         
        if ( ble.isConnected != 1 )
        {
            spo2_done = 0;
            return ( events ^ TASK_UPLOAD_SP02_2_EVT );
        }        
        
		memset( txbuf, 0, sizeof(txbuf));
		
		// 血氧 0x12 01 AH AL CH CL YY MM DD hh mm ss SH SL 00 00 00 00 00 00
        txbuf[0] = 0x12;
        txbuf[1] = 0x01;
        txbuf[2] = spo2_all / 256;
        txbuf[3] = spo2_all % 256;
        txbuf[4] = spo2_done/256;
        txbuf[5] = spo2_done % 256;
        txbuf[19] = 0x0;
		
		fret = load_queue( FM_ID_SPO2, &q, buf, 16 );
		if ( fret == QUEUE_EMPTY )
		{
			return ( events ^ TASK_UPLOAD_SP02_2_EVT );
		}		
        
        if ( spo2_done++ < spo2_all )
        {   
			txbuf[19]= 0x0;      /* CRC */
            memcpy( txbuf + 6, buf, 8);
			
            bt_protocol_tx( txbuf, sizeof(txbuf) ); 
            osal_start_timerEx(task_id, TASK_UPLOAD_SP02_2_EVT, 75);
        }else{
            spo2_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_SP02_2_EVT );
    }	
	
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the segment pedometer data to app                               //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////	
    if( events & TASK_UPLOAD_SEG_PEDO_1_EVT )
    {
		unsigned long temp;
        /** Dont' send if bluetooth is disconnected */
        if( ble.isConnected != 1){
            return (events ^ TASK_UPLOAD_SEG_PEDO_1_EVT );
        }
		
		memset( txbuf, 0, sizeof(txbuf));
		
		seg_pedo_done_all = q.items;
        seg_pedo_done  = 0;
        
		// Segment pedometer
		// 0x1A EN A3 A2 A1 00 00 00  00 00 00 00 00 00 t1  t2  t3  t4  CN   00
        txbuf[0] = 0x1A;
        txbuf[1] = 0x04;
		
		temp = (seg_pedo_done_all << 12) | (seg_pedo_done & 0xFFF);
		txbuf[2] = (temp>>16) & 0xFF;
		txbuf[3] = (temp>>8) & 0xFF;
		txbuf[4] = (temp>>0) & 0xFF;
		
        if ( seg_pedo_done_all != 0 )
        {
			fret = load_queue( FM_ID_SEG_PEDO, &q, buf, 16 );
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_SEG_PEDO_1_EVT ); 
			}
			memcpy( txbuf + 5, buf, 14);
        }else
        {
            memset( txbuf + 5, 0, 16);
        }
		
        bt_protocol_tx( txbuf, sizeof(txbuf) ); 
		
        if ( ++seg_pedo_done < seg_pedo_done_all ){  /* Done ? */      
            osal_start_timerEx( task_id, TASK_UPLOAD_SEG_PEDO_2_EVT, 75);
        }
        
        return (events ^ TASK_UPLOAD_SEG_PEDO_1_EVT);
    }
    
    if ( events & TASK_UPLOAD_SEG_PEDO_2_EVT )
    {
        /** Dont' send if bluetooth is disconnected */
        unsigned long temp;
		
        if ( ble.isConnected != 1 )
        {
            seg_pedo_done = 0;
            return ( events ^ TASK_UPLOAD_SEG_PEDO_2_EVT );
        }        
        
		memset( txbuf, 0, sizeof(txbuf));
		
		// Segment pedometer
		// 0x1A EN A3 A2 A1 00 00 00 00 00 00 00 00 00 t1 t2 t3 t4 CN 00
		
        txbuf[0] = 0x1A;
        txbuf[1] = 0x04;
		
		temp = (seg_pedo_done_all << 12) | (seg_pedo_done & 0xFFF);
		txbuf[2] = (temp>>16) & 0xFF;
		txbuf[3] = (temp>>8) & 0xFF;
		txbuf[4] = (temp>>0) & 0xFF;

		fret = load_queue( FM_ID_SEG_PEDO, &q, buf, 16 );
		if ( fret == QUEUE_EMPTY )
		{
			return ( events ^ TASK_UPLOAD_SEG_PEDO_2_EVT );
		}
        
	    if ( seg_pedo_done++ < seg_pedo_done_all )
        {
			txbuf[19]= 0x0;      /* CRC */
            memcpy( txbuf + 5, buf, 14);
			
            bt_protocol_tx( txbuf, sizeof(txbuf) ); 
            osal_start_timerEx(task_id, TASK_UPLOAD_SEG_PEDO_2_EVT, 75);
        }else{
            seg_pedo_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_SEG_PEDO_2_EVT );
    }
	
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Upload the running data to app                                         //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////	
    if ( events & TASK_UPLOAD_STORE_RUNNING_1_EVT )
    {
        // 运动数据
        // 格式：1B EN SS C2 C1 C0 D6 D5 D4 D3 D2 D1 D0 U3 U2 U1 U0 TH TL CRC
        
        /** 蓝牙断开,不发送 */
        if ( ble.isConnected != 1 ){
            return (events ^ TASK_UPLOAD_STORE_RUNNING_1_EVT);
        }
		
		memset( txbuf, 0, sizeof(txbuf));
 
		running_all = q.items;
        running_done  = 0;
        
        txbuf[0] = 0x1B;
        txbuf[1] = 0x01;    // EN
        txbuf[2] = 0x00;    // SS
        
        txbuf[3] = running_all>>4;
        txbuf[4] = ((running_all & 0Xf)<<4) | ((running_done>>8)&0xF);
        txbuf[5] = running_done & 0xFF;
       		
        if ( running_all != 0 )
        {
			fret = load_queue( FMC_ID_RUNNING, &q, buf, 16 );
			if ( fret == QUEUE_EMPTY )
			{
				return ( events ^ TASK_UPLOAD_STORE_RUNNING_1_EVT ); 
			}
        }
		
		txbuf[1] |= (buf[0] & 0xF0); // EN
		txbuf[2] = buf[1]; // SS
		memcpy( txbuf+6, buf+2, 7+4+2);                 
        
        bt_protocol_tx(txbuf, 20); 
        
        if ( ++running_done < running_all ){  /* Done ? */
            // running_done++; // bug 2018.02.28
            osal_start_timerEx( task_id, TASK_UPLOAD_STORE_RUNNING_2_EVT, 75);
        }        
        
        return (events ^ TASK_UPLOAD_STORE_RUNNING_1_EVT);
    }
    
    if ( events & TASK_UPLOAD_STORE_RUNNING_2_EVT )
    {
        // 运动数据
        // 格式：1B EN SS C2 C1 C0 D6 D5 D4 D3 D2 D1 D0 U3 U2 U1 U0 TH TL CRC
        
        if ( ble.isConnected != 1 ){
            running_done = 0;
            return ( events ^ TASK_UPLOAD_STORE_RUNNING_2_EVT );
        }        
        
        txbuf[0] = 0x1B;
        txbuf[1] = 0x01;        // EN
        txbuf[2] = 0x00;        // SS
        
        txbuf[3] = running_all>>4;
        txbuf[4] = ((running_all & 0Xf)<<4) | ((running_done>>8)&0xF);
        txbuf[5] = running_done & 0xFF;
 
		fret = load_queue( FMC_ID_RUNNING, &q, buf, 16 );
		if ( fret == QUEUE_EMPTY )
		{
			return ( events ^ TASK_UPLOAD_STORE_RUNNING_2_EVT );
		}
        
	    if ( running_done++ < running_all )
        {
            txbuf[1] |= (buf[0] & 0xF0); // EN
            txbuf[2] = buf[1]; // SS
            memcpy( txbuf+6, buf+2, 7+4+2);
            
            // running_done++; // bug 2018.02.28
        
            bt_protocol_tx( txbuf,20 );             
            osal_start_timerEx(task_id, TASK_UPLOAD_STORE_RUNNING_2_EVT, 75);
        }else{
            running_done = 0;
        }
       
        return ( events ^ TASK_UPLOAD_STORE_RUNNING_2_EVT );
    }	
	
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    //  上传本次(最近一次)的结果                                              //
    //                                                                        //
    //////////////////////////////////////////////////////////////////////////// 
	
	/* Upload the current pedometer data to app */

	if ( events & TASK_UPLOAD_PEDO_EVT )
	{
		if ( ble.isConnected != 1 )
		{
			return ( events ^ TASK_UPLOAD_PEDO_EVT );
		}
		
		memset( txbuf, 0, sizeof(txbuf));
		
		txbuf[0] = 0x03;
		txbuf[1] = 0x07;

		txbuf[2] = pedometer.counter>>16;
		txbuf[3] = pedometer.counter>>8;
		txbuf[4] = pedometer.counter>>0;
		
		txbuf[5] = pedometer.distance>>16;
		txbuf[6] = pedometer.distance>> 8;
		txbuf[7] = pedometer.distance>> 0;

		txbuf[8] = pedometer.calorie>>16;
		txbuf[9] = pedometer.calorie>> 8;
		txbuf[10]= pedometer.calorie>> 0;
			
		bt_protocol_tx( txbuf, 20 );
		
		return ( events ^ TASK_UPLOAD_PEDO_EVT );
	}
	
	#if 1
	/** 上传心率值 */
    if ( events & TASK_UPLOAD_HR_EVT )
    {
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0]  = 0x0A;
		txbuf[1]  = 0x00;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());
        
        // 心率 0x0A 00 00 00 00 00 YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
        
        txbuf[12] = hr.hr;
        
        bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_HR_EVT );
	}
	#endif
	        
	#if 1
	/* 上传血压数据 */
	if ( events & TASK_UPLOAD_B_PRESSURE_EVT )
    {
        memset( txbuf, 0, sizeof(txbuf) );

        txbuf[0]  = 0x11;
		txbuf[1]  = 0x00;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());

        // 血压 0x11 00 00 00 00 00 YY MM DD hh mm ss HP LP Hr 00 00 00 00 00
        
        txbuf[12] = hr.hp;
        txbuf[13] = hr.lp;
        txbuf[14] = hr.hr;
        
        bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_B_PRESSURE_EVT );
	}
	#endif

	#if 1
	/** 上传血氧数据 */
	if ( events & TASK_UPLOAD_SPO2_EVT )
    {
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0]  = 0x12;
		txbuf[1]  = 0x00;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());
		
        // 血氧 0x12 00 00 00 00 00 YY MM DD hh mm ss SH SL 00 00 00 00 00 00
        
        txbuf[12] = hr.sp_h;
        txbuf[13] = hr.sp_l;
            
        bt_protocol_tx( txbuf, sizeof(txbuf) );
        
        return ( events ^ TASK_UPLOAD_SPO2_EVT );
    }
	#endif
	
	/* 上传睡眠数据 */
	if ( events & TASK_UPLOAD_SL_EVT )
	{
		// 睡眠 0x0C Ty AL CU YY MM DD hh mm YY MM DD hh mm Dh Dl Lh Ll 00 00
		
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0] = 0x0C;
        txbuf[1] = 0x00;
        txbuf[2] = 0x00;
        txbuf[3] = 0x00;
        
		bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_SL_EVT );
	}
	
	/** 新增iBand支持，测量完成时,通知APP读取 */
    if ( events & TASK_UPLOAD_HR_DONE_EVT )
    {
		// 0xFC 09 00 00 00 00 YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0]  = 0xFC;
		txbuf[1]  = 0x09;
        txbuf[2]  = 0X00;		
       
        bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_HR_DONE_EVT );
	}	
	
	/** 新增iBand支持，测量中,上报心率数据 */
    if ( events & TASK_UPLOAD_iBand_HR_DECT_EVT )
    {
		// 0x0A 03 00 00 00 00 YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0]  = 0x0A;
		txbuf[1]  = 0x03;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());

        txbuf[12] = hr.hr;
       
        bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_iBand_HR_DECT_EVT );
	}	
	
	/** 新增iBand支持，测量中,上报血压数据 */
	if ( events & TASK_UPLOAD_iBand_B_PRUESSURE_EVT )
	{
		// 0x11 03 00 00 00 00 YY MM DD hh mm ss HP LP Hr 00 00 00 00 00
		
        memset( txbuf, 0, sizeof(txbuf) );

        txbuf[0]  = 0x11;
		txbuf[1]  = 0x03;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());
        
        txbuf[12] = hr.hp;
        txbuf[13] = hr.lp;
        txbuf[14] = hr.hr;
        
        bt_protocol_tx( txbuf, sizeof(txbuf) );
		
		return ( events ^ TASK_UPLOAD_iBand_B_PRUESSURE_EVT ); 
	}
	
	/** 新增iBand支持，测量中,上报血氧数据 */
	if ( events & TASK_UPLOAD_iBand_SPO2_EVT )
    {
        memset( txbuf, 0, sizeof(txbuf) );
        
        txbuf[0]  = 0x12;
		txbuf[1]  = 0x03;
		
        txbuf[6]  = DECToBCD(TimerYY());
        txbuf[7]  = DECToBCD(TimerMM());
        txbuf[8]  = DECToBCD(TimerDD());
        txbuf[9]  = DECToBCD(TimerHH());
        txbuf[10] = DECToBCD(TimerMI());
        txbuf[11] = DECToBCD(TimerSE());
		
        // 血氧 0x12 03 00 00 00 00 YY MM DD hh mm ss SH SL 00 00 00 00 00 00
        
        txbuf[12] = hr.sp_h;
        txbuf[13] = hr.sp_l;
            
        bt_protocol_tx( txbuf, sizeof(txbuf) );
        
        return ( events ^ TASK_UPLOAD_iBand_SPO2_EVT );
    }	
	
	/** 新增iBand支持，上报分段计步数据 */
	if ( events & TASK_UPLOAD_iBand_SEG_PEDO_EVT )
    {
		// 0x1A 01 00 00 00 00 00 00 00 00 00 00 00 00 t1 t2 t3 t4 CN 00
		if ( ble.isConnected != 1 )
		{
			return ( events ^ TASK_UPLOAD_iBand_SEG_PEDO_EVT );
		}
		
		unsigned long utc;
		struct tm time;
		
		time.tm_hour = TimerHH();
		time.tm_min = TimerMI();
		time.tm_sec = TimerSE();
		time.tm_mday = TimerDD();
		time.tm_mon = TimerMM()-1;
		time.tm_year = TimerYY()+2000 - 1900;
		utc = mktime(&time) - 3600; /** 前一个小时 */		
		
		memset( txbuf, 0, sizeof(txbuf));
		
		txbuf[0] = 0x1A;
		txbuf[1] = 0x01;
		txbuf[2] = 0x00;
		txbuf[3] = 0x00;
		txbuf[4] = 0x00;

		unsigned long temp;
	
		if ( pedometer.counter > pedometer_last.counter )
		{
			temp = pedometer.counter - pedometer_last.counter;
			
			txbuf[5]  = (temp >> 16) & 0xFF;
			txbuf[6]  = (temp >>  8) & 0xFF;
			txbuf[7]  = (temp >>  0) & 0xFF;
		}
		if ( pedometer.calorie > pedometer_last.calorie )
		{
			temp = pedometer.calorie - pedometer_last.calorie;
			
			txbuf[8] = (temp >> 16) & 0xFF;
			txbuf[9] = (temp >>  8) & 0xFF;
			txbuf[10] = (temp >>  0) & 0xFF;
		}
		if ( pedometer.distance > pedometer_last.distance )
		{
			temp = pedometer.distance - pedometer_last.distance; 
			txbuf[11] = (temp >> 16) & 0xFF;
			txbuf[12] = (temp >>  8) & 0xFF;
			txbuf[13] = (temp >>  0) & 0xFF;
		}
		
		txbuf[14] = (utc>>24) & 0xFF;
		txbuf[15] = (utc>>16) & 0xFF;
		txbuf[16] = (utc>>8) & 0xFF;
		txbuf[17] = (utc>>0) & 0xFF;
		txbuf[18] = 0x3C;		
			
		bt_protocol_tx( txbuf, 20 );
        
        return ( events ^ TASK_UPLOAD_iBand_SEG_PEDO_EVT );
    }	
	    	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载计步数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_PEDO_EVT )
	{
		int ret;
		
		ret = store_queue( FM_ID_PEDOMETER, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_PEDO_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_PEDO_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
 
			txbuf[0] = 0x03;
			txbuf[1] = 0xC0;	

			bt_protocol_tx( txbuf, sizeof(txbuf) );
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_PEDO_EVT );
	}	
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载睡眠数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_SL_EVT )
	{
		int ret;
		
		ret = store_queue( FM_ID_SLEEP, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_SL_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_SL_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
	 
			txbuf[0] = 0x0C; 
			txbuf[1] = 0x01;
			
			bt_protocol_tx( txbuf, sizeof(txbuf) );
		}
		
		return ( events ^ TASK_UPLOAD_STORE_SL_EVT );
	}	
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载心率数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_HR_EVT )
	{       
		int ret;
		
		ret = store_queue( FM_ID_HR, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_HR_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_HR_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
			txbuf[0] = 0x0A;
			txbuf[1] = 0x01;

			bt_protocol_tx( txbuf, sizeof(txbuf) );			
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_HR_EVT );
	}
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载血压数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_B_P_EVT )
	{
		int ret;
		
		ret = store_queue( FM_ID_B_PRESSURE, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_B_P_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_B_PRESSURE_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
		
			txbuf[0] = 0x11;
			txbuf[1] = 0x01;

			bt_protocol_tx( txbuf, sizeof(txbuf) );			
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_B_P_EVT );
	}
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载血氧数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_SPO2_EVT )
	{
		int ret;
		
		ret = store_queue( FM_ID_SPO2, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_SPO2_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_SP02_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
		
			txbuf[0] = 0x11;
			txbuf[1] = 0x01;

			bt_protocol_tx( txbuf, sizeof(txbuf) );			
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_SPO2_EVT );
	}	
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载分段计步数据到队列中                                                    *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_SEG_PEDO_EVT )
	{
		int ret;
		
		ret = store_queue( FM_ID_SEG_PEDO, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_SEG_PEDO_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_SEG_PEDO_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
		
			txbuf[0] = 0x1A;
			txbuf[1] = 0x04;

			bt_protocol_tx( txbuf, sizeof(txbuf) );			
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_SEG_PEDO_EVT );
	}	
	
	/*************************************************************************** 
	 *                                                                         *
 	 * 加载运动数据到队列中                                                *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_UPLOAD_STORE_RUNNING_EVT )
	{
		int ret;
		
		ret = store_queue( FMC_ID_RUNNING, &q, 16);
		switch ( ret )
		{
			case QUEUE_LOCKED:
			case FLASH_LOCKED:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_RUNNING_EVT, 1000);
				break;
			case QUEUE_EMPTY:
				break;
			case 0:
				osal_start_timerEx ( task_id, TASK_UPLOAD_STORE_RUNNING_1_EVT, 100);
				break;
			default:break;
		}
		
		if ( ret == QUEUE_EMPTY )
		{
			memset( txbuf, 0, sizeof(txbuf));
		
			txbuf[0] = 0x1B;
			txbuf[1] = 0x04;

			bt_protocol_tx( txbuf, sizeof(txbuf) );			
		}		
		
		return ( events ^ TASK_UPLOAD_STORE_RUNNING_EVT );
	}	
	
	
	/**
	* 复位队列数据(连接异常时)
	 */
	if ( events & TASK_UPLOAD_RESET_QUEUE_EVT )
	{
		reset_queue();     
		
		return ( events ^ TASK_UPLOAD_RESET_QUEUE_EVT );
	}
	
    return 0;
}

void taskUploadInit( unsigned long task_id )
{
	taskUploadTaskId = task_id;
}

