#ifndef YZBX_UTILITY_H
#define YZBX_UTILITY_H
#include <opencv2/opencv.hpp>
#include <iostream>
//set qt sync with file.

using namespace std;
using namespace cv;
/*
    version: 1.0
    author: hellogiser
    blog: http://www.cnblogs.com/hellogiser
    date: 2014/5/30
*/
// hamming distance of two integer 0-1 bits
unsigned yzbx_hamdist(unsigned x, unsigned y);
unsigned yzbx_d1(unsigned x,unsigned y);
unsigned yzbx_d2(unsigned x,unsigned y);
//0 hamdist, 1 d1, 2 d2,...
unsigned yzbx_distance(unsigned x,unsigned y,int distance_type);
void showImgInLoop(const cv::Mat img,int i);

//TODO 2016/03/21
bool separatedMatCheck(const Mat &a,const Mat& b);
bool separatedMatsCheck(vector<Mat> &va, vector<Mat> &vb);
#endif // UTILITY_HPP
