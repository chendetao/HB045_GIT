#ifndef __UI_LIB_H__
#define __UI_LIB_H__

#define UI_NR                      6
#define FLASH_BASE_ADDR     0x200000
#define USED_BY_APP                0 /* app使用时, 将该宏定设定为1 */

/**
 * mode bit filed means
 * 1111 1111 1111 1111 1111 1111 1111  1111 
 *                              format  bits
 * format : 0001 bitmap
 *          0010 jpeg
 * bits : 0001    1bit bitmap       (when format is 0001)
 *        0010    16bits bitmap   
 */
#define MODE_BITMAP       (0x1<<4)
#define MODE_JPEG         (0x1<<5)
#define MODE_BITMAP_1     (0x1<<0)
#define MODE_BITMAP_16    (0x1<<1)

typedef struct 
{
    unsigned long addr;              /** Flash存储地址 */
    unsigned long size;              /** 图片大小 */
    unsigned long mode;              /** 用于描述图片一些属性 */
    unsigned short width;             /** 宽 */
    unsigned short height;            /** 高 */
    const unsigned char *bitmap;     /** 图片缓存区 */
}flash_struct_t;

extern const flash_struct_t flash_ui[UI_NR];

#endif
