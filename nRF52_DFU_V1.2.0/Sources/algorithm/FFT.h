#ifndef __FFT_H__
#define __FFT_H__

#define 		FFT_N  		512
#define         Fs          25.0f

struct compx {float real,imag;};

#define PI 3.1415926535897932384626433832795028841971f // 圆周率

extern void fft(void);
extern void fft_put(short real, short image);
extern void fft_init(void);
extern void fft_reset(void);
extern float get_max_frecency(float min, float max);
void fft_m(void);

#endif
