#ifndef YZBX_FRAMEINPUT_H
#define YZBX_FRAMEINPUT_H

#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

//define how to get the input from file system easyly
#define FromDevice 0
#define FromVideoFile 1
#define FromCDNet 2
class yzbx_frameInput
{
public:
    yzbx_frameInput(int deviceNum=0);
    yzbx_frameInput(string videopath);
    yzbx_frameInput(string frameFloderPath,int startFrameNum,int lastFrameNum=-1);
    void getNextFrame(Mat &frame,int method);

private:
    VideoCapture cap;
    string rootPath;
    int frameNum;
    int lastFrameNum;
};

#endif // YZBX_FRAMEINPUT_H
