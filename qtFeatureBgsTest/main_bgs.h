#ifndef MAIN_BGS_H
#define MAIN_BGS_H
#include "lbp_bgs.h"
#include "npe_bgs.h"
#include "bgslibrary.h"
#include "IBGS.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#define LBP_BGS 0
#define NPE_BGS 1
#define ORG_BGS 2

#define STATIC_POINT 0
#define UNKNOW_POINT 1
#define MOVING_POINT 2

class myVec{
public:
    myVec(){
        for(int i=0;i<3;i++){
            a[i]=0;
        }
    }

    int a[3];
};

class main_bgs : IBGS
{
public:
    main_bgs();
    ~main_bgs();
    void process (const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background);

//private:
    void loadConfig ();
    void saveConfig ();
    void getMS(const cv::Mat &img_input, vector<cv::Mat> &ForeGrounds,vector<cv::Mat> &MovingStatics);
    void getMS(const cv::Mat &labelImg8UC1,cv::Mat &nextMSMask);
    void mixMS(vector<cv::Mat> MovingStatics, Mat &mixedMovingStatic);

    //use lbp texture
//    lbp_bgs *lbpBgs;
    IBGS *lbpBgs;
    //use non-parametric estmation
    npe_bgs *npeBgs;
    //use origin class from bgslibrary
    IBGS *originBgs;

    bool inited=false;
    size_t frameNum=0;
    Mat img_input_gray_previous,img_input_gray;
    Mat descriptors,descriptors_previous;
    vector<KeyPoint> keyPoints,keyPoints_previous;
    vector<DMatch> matches;

    Mat staticPointMat;

    size_t img_cols,img_rows;
    cv::Size img_size;
    size_t img_type;
};

#endif // MAIN_BGS_H
