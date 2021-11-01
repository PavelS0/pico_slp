#include <stdio.h>

#include "pico/stdlib.h"
#include "kissfft/kiss_fftr.h"
#include "fft.h"

fft_cfg fft_init(uint16_t size) {
    fft_cfg c;
    c.size = size;
    c.state = kiss_fftr_alloc((int)size, 0, 0, 0);
    return c;
}


void fft(fft_cfg c, int16_t* in, fft_cpx* out) {
    //kiss_fft_scalar rin[nfft+2];
    //kiss_fft_scalar rout[nfft+2];

    kiss_fftr(c.state, in, out);
}


void fft_mag(fft_cpx* in, float* out, uint16_t size) {
    float max = 0.0;
    for (uint16_t i = 0; i < size; i++) {
        out[i] = sqrtf(in[i].r * in[i].r + in[i].i * in[i].i);
        //out[i] = 10 * log10f(fabsf(in[i].r * in[i].r + in[i].i * in[i].i));
        //if (out[i] > max)
            //max = out[i];
    }
}