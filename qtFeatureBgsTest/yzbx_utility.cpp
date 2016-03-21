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

unsigned yzbx_distance (unsigned x, unsigned y, int distance_type){
    CV_Assert(distance_type<=2&&distance_type>=0);
    if(distance_type==0) return yzbx_hamdist (x,y);
    if(distance_type==1) return yzbx_d1 (x,y);
    if(distance_type==2) return yzbx_d2(x,y);
}

void showImgInLoop (const cv::Mat img, int i){
    stringstream ss;
    ss<<i;
    string name;
    ss>>name;
    cv::imshow (name,img);
}
