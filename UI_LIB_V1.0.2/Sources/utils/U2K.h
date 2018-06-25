
#ifndef __U2K_H__
#define __U2K_H__

extern unsigned char enc_get_utf8_size(const unsigned char *pInput);
extern int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic);

#endif
