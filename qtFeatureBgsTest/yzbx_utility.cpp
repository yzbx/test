#include "yzbx_utility.h"

unsigned yzbx_hamdist(unsigned x, unsigned y)
{
    //  11111 11100
    unsigned dist = 0, val = x ^ y; // XOR

    // Count the number of set bits
    while(val)
    {
        ++dist;
        val &= val - 1;
    }

    return dist;
}

unsigned yzbx_d1(unsigned x,unsigned y){
    if(x>y) return x-y;
    else return y-x;
}

unsigned yzbx_d2(unsigned x,unsigned y){
    if(x>y) return (x-y)*(x-y);
    else return (y-x)*(y-x);
}
