#include <stdio.h>
#include "math.h"
#include "pico/stdlib.h"
#include "fft.h"

fft_cfg fft_init(uint16_t size) {
    fft_cfg c;
    c.size = size;
    c.cbuf = malloc(sizeof(kiss_fft_cpx) * size);
    c.state = kiss_fftr_alloc((int)size, 0, 0, 0);
    //Allocate the memory for the constants
    c.wnd_coef = malloc(sizeof(float) * size);
    
    // create the constants for a hamming window
    const float arg =  (2.0 * M_PI) / size;
    for (int i = 0; i < size; i++)
    {
        float n_coef_value = 0.54 - (0.46 * cos(arg * (i+ 0.5)));
        c.wnd_coef[i] = n_coef_value;
    }

    return c;
}


void fft(fft_cfg c, float* in, float* out) {
    //kiss_fft_scalar rin[nfft+2];
    //kiss_fft_scalar rout[nfft+2];
    for (int i = 0; i < c.size; i++)
        in[i] = in[i] * 4 * c.wnd_coef[i];

    kiss_fftr(c.state, in, c.cbuf);
    for (uint16_t i = 0; i < c.size; i++) {
        out[i] = sqrtf(c.cbuf[i].r * c.cbuf[i].r + c.cbuf[i].i * c.cbuf[i].i);
        //out[i] = 10 * log10f(fabsf(c.cbuf[i].r * c.cbuf[i].r + c.cbuf[i].i * c.cbuf[i].i));
        //if (out[i] > max)
            //max = out[i];
    }
}