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
 * ݫһٶؖػքUTF8ҠëתۻԉUnicode(UCS-2ۍUCS-4)Ҡë. 
 * 
 * ӎ˽: 
 *    pInput      ָвˤɫۺԥȸ, ӔUTF-8Ҡë 
 *    Unic        ָвˤԶۺԥȸ, Ǥѣզք˽ߝܴˇUnicodeҠëֵ, 
 *                `эΪunsigned long . 
 * 
 * ׵ܘֵ: 
 *    ԉ٦ղ׵ܘكؖػքUTF8Ҡë̹ռԃքؖޚ˽; ʧќղ׵ܘ0. 
 * 
 * עӢ: 
 *     1. UTF8ûԐؖޚѲϊ͢, իˇUnicodeԐؖޚѲҪȳ; 
 *        ؖޚѲؖΪճ׋(Big Endian)ۍС׋(Little Endian)}ז; 
 *        ՚IntelԦmǷאӉԃС׋רҭʾ, ՚ՋӉԃС׋רҭʾ. (֍ַ֘զ֍λ) 
 ****************************************************************************/  
int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic)  
{   
    // b1 ҭʾUTF-8ҠëքpInputאքٟؖޚ, b2 ҭʾՎٟؖޚ, ...  
    char b1, b2, b3, b4, b5, b6;  
  
    *Unic = 0x0; // ё *Unic ԵʼۯΪȫ£  
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
