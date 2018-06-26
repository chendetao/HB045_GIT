// http://blog.csdn.net/tge7618291/article/details/7599902

unsigned char enc_get_utf8_size(const unsigned char *pInput)
{
    unsigned char size = 1;
    
    if ( (pInput[0]>>7) == 0 )
    {
        size = 1;
    }else if (( (pInput[0]>>0x5) == 0x6) && ((pInput[1]>>6) == 0x2))
    {   
        size = 2;
    }else if (((pInput[0]>>4)==0xE) && ((pInput[1]>>6)==0x2) && ((pInput[2]>>6)==0x2))
    {
        size = 3;
    }else if (((pInput[0]>>4)==0xF) && ((pInput[1]>>6)==0x2) && ((pInput[2]>>6)==0x2) && ((pInput[3]>>6)==0x2))
	{
		size = 4;
	}
    
    return size;
}

/***************************************************************************** 
 * 将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码. 
 * 
 * 参数: 
 *    pInput      指向输入缓冲区, 以UTF-8编码 
 *    Unic        指向输出缓冲区, 其保存的数据即是Unicode编码值, 
 *                类型为unsigned long . 
 * 
 * 返回值: 
 *    成功则返回该字符的UTF8编码所占用的字节数; 失败则返回0. 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 ****************************************************************************/  
int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic)  
{   
    // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...  
    char b1, b2, b3, b4, b5, b6;  
  
    *Unic = 0x0; // 把 *Unic 初始化为全零  
    int utfbytes = enc_get_utf8_size(pInput);  
    unsigned char *pOutput = (unsigned char *) Unic;  
  
    switch ( utfbytes )  
    {  
        case 1:  
            *pOutput     = *pInput;  
            utfbytes    += 1;  
            break;  
        case 2:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            if ( (b2 & 0xE0) != 0x80 )  
                return 0;  
            *pOutput     = (b1 << 6) + (b2 & 0x3F);  
            *(pOutput+1) = (b1 >> 2) & 0x07;  
            break;  
        case 3:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b2 << 6) + (b3 & 0x3F);  
            *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
            break;  
        case 4:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b3 << 6) + (b4 & 0x3F);  
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
            *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
            break;  
        case 5:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b4 << 6) + (b5 & 0x3F);  
            *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
            *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
            *(pOutput+3) = (b1 << 6);  
            break;  
        case 6:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            b6 = *(pInput + 5);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
                    || ((b6 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b5 << 6) + (b6 & 0x3F);  
            *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
            *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
            *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
            break;  
        default:  
            return 0;
    }  
  
    return utfbytes;  
}  
