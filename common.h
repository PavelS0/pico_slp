
#include <stdio.h>

typedef union
{
    struct __attribute__((packed))
    {
       uint8_t color:6;
    };
    uint8_t word;
} ColorByte;

typedef union
{
    struct __attribute__((packed))
    {
        bool r0:1;
        bool g0:1;
        bool b0:1;
        bool r:1;
        bool g:1;
        bool b:1;
    };
    uint8_t color;
} ColorRGBByte;