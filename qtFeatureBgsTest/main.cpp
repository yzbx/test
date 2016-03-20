#include <QCoreApplication>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QDir>
#include "IBGS.h"
#include "main_bgs.h"

//TODO install opencv_contrib, then use the daisy feature.
//#include "xfeatures2d.hpp"
using namespace std;
using namespace cv;

#define E420 1
//BUG unexpected, maybe the bug of qmake in windows10!
//I will try the ubuntu and look the result.
//NOTE maybe cannot change build type from the left down corner of Qt Creator.
int main(int argc, char *argv[])
{
    //    QCoreApplication a(argc, argv);
    //    return a.exec();
    std::cout << "Using OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "." << CV_SUBMINOR_VERSION << std::endl;

    /* Background Subtraction Methods */
    main_bgs *bgs;
    bgs=new main_bgs;

    QDir qdir(".");
    qdir.mkdir("config");
#ifdef WIN
    VideoCapture cap("D:\\git\\tracking\\data.avi");
#else

#ifdef E420
    VideoCapture cap("/home/yzbx/git/video.avi");
#else
    //VM ware
    VideoCapture cap("/mnt/hgfs/D/git/tracking/data.avi");
#endif

#endif
    if(!cap.isOpened())
    {
        return -1;
    }
    Mat input;

    int num=0,key=0;
    bool init = false;

    namedWindow ("input");
    while(key!='q')
    {
        num++;
        cout<<"frame="<<num<<"***************************************************************"<<endl;

        cap>>input;
        if(init){
            if(input.empty()){
                break;
            }
        }
        else{
            if(input.empty()){
                continue;
            }
            init=true;
        }

        cv::Mat img_mask;

#ifdef WIN
        //NOTE windows use the old bgslibrary in UrbanTrack
        bgs->process(input, img_mask); // by default, it shows automatically the foreground mask image
#else
        cv::Mat img_bkgmodel;
        bgs->process(input,img_mask,img_bkgmodel);
#endif

        if(!img_mask.empty())
            cv::imshow("Foreground", img_mask);
        imshow("input",input);
        key=waitKey(30);
    }
}
