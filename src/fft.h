#ifndef FFT_H
#define FFT_H
#include "kiss_fftr.h"

typedef struct 
{
    uint16_t size;
    kiss_fftr_cfg state;
    kiss_fft_cpx* cbuf;
    float* wnd_coef;
} fft_cfg;



fft_cfg fft_init(uint16_t size);
void fft(fft_cfg c, float* in, float* out);


#endif