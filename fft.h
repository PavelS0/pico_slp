#ifndef FFT_H
#define FFT_H
#include "kissfft/kiss_fftr.h"

typedef struct 
{
    uint16_t size;
    kiss_fftr_cfg state;
} fft_cfg;

typedef kiss_fft_cpx fft_cpx;

fft_cfg fft_init(uint16_t size);
void fft(fft_cfg c, int16_t* in, fft_cpx* out);
void fft_mag(fft_cpx* in, float* out, uint16_t size);


#endif