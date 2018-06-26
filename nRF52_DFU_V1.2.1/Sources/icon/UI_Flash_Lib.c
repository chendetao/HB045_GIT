#include "UI_Flash_Lib.h"

const flash_struct_t flash_ui[UI_NR] = 
{
    [0] = {
        .addr   = FLASH_BASE_ADDR+ 0x0000+0x0000,
        //.size   = sizeof(gImage_bycle),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_bycle 
    },
    [1] = {
        .addr   = FLASH_BASE_ADDR+ 0x2000+0x0000,
        //.size   = sizeof(gImage_hr),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_hr 
    },  
    [2] = {
        .addr   = FLASH_BASE_ADDR+ 0x4000+0x0000,
        //.size   = sizeof(gImage_sleep),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_sleep 
    },  
    [3] = {
        .addr   = FLASH_BASE_ADDR+ 0x6000+0x0000,
        //.size   = sizeof(gImage_alarm),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_alarm 
    },     
    [4] = {
        .addr   = FLASH_BASE_ADDR+ 0x8000+0x0000,
        //.size   = sizeof(gImage_find),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_find 
    },   
    [5] = {
        .addr   = FLASH_BASE_ADDR+ 0xa000+0x0000,
        //.size   = sizeof(gImage_info),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_info 
    },   
    [6] = {
        .addr   = FLASH_BASE_ADDR+ 0xc000+0x0000,
        //.size   = sizeof(gImage_power),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_power 
    },  
    [7] = {
        .addr   = FLASH_BASE_ADDR+ 0xe000+0x0000,
        //.size   = sizeof(gImage_back),
        .mode   = MODE_BITMAP|MODE_BITMAP_16,
        .width  = 64,
        .height = 64,
        //.bitmap = (unsigned char *)gImage_back
    },    
};
