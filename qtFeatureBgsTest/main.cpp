#include <QCoreApplication>
#include "IBGS.h"
#include "AdaptiveBackgroundLearning.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <stdio.h>
#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/nonfree/nonfree.hpp"
#include <iostream>
#include <QDir>
#include "yzbx_tracking.h"
//TODO install opencv_contrib, then use the daisy feature.
//#include "xfeatures2d.hpp"
using namespace std;
using namespace cv;

//BUG unexpected, maybe the bug of qmake in windows10!
//I will try the ubuntu and look the result.
//NOTE maybe cannot change build type from the left down corner of Qt Creator.
int main(int argc, char *argv[])
{
    //    QCoreApplication a(argc, argv);
    //    return a.exec();
    std::cout << "Using OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION << "." << CV_SUBMINOR_VERSION << std::endl;

    /* Background Subtraction Methods */
    IBGS *bgs;
    bgs=new AdaptiveBackgroundLearning;

    QDir qdir(".");
    qdir.mkdir("config");
#ifdef WIN
    VideoCapture cap("D:\\git\\tracking\\data.avi");
#else
    VideoCapture cap("/mnt/hgfs/D/git/tracking/data.avi");
#endif
    if(!cap.isOpened())
    {
        return -1;
    }
    Mat input,input_gray,input_pre_gray;

    int num=0,key=0;
    bool init = false;

    yzbx_tracking* ytrack;
    ytrack=new yzbx_tracking;

    while(key!='q')
    {
        num++;
        cout<<num<<endl;

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
        //        if(!img_bkgmodel.empty())
        //            imshow("bkgmodel",img_bkgmodel);

        cvtColor(input,input_gray,CV_RGB2GRAY);

        if(!input_pre_gray.empty()){
            SurfFeatureDetector detector(10000);
            vector<KeyPoint> keypoints1, keypoints2;
            detector.detect(input_gray, keypoints1);
            detector.detect(input_pre_gray, keypoints2);

            SurfDescriptorExtractor extractor;
            Mat descriptors1, descriptors2;

            extractor.compute(input_gray, keypoints1, descriptors1);
            extractor.compute(input_pre_gray, keypoints2, descriptors2);
            cout<<"descriptors' mat size="<<descriptors1.size()<<","<<descriptors2.size()<<endl;
            cout<<"descriptors's mat type="<<descriptors1.type()<<endl;

            //NOTE match(query,train);
            BFMatcher matcher(NORM_L2);
            vector<DMatch> matches;
            matcher.match(descriptors1, descriptors2, matches);


            Mat img_matches;
            drawMatches(input_gray, keypoints1, input_pre_gray, keypoints2, matches, img_matches);
            imshow("matches", img_matches);

            ytrack->process(keypoints2,descriptors2);
            ytrack->showTrajectory(input);
        }
        else{
            SurfFeatureDetector detector(10000);
            vector<KeyPoint> keypoints1;
            detector.detect(input_gray, keypoints1);

            SurfDescriptorExtractor extractor;
            Mat descriptors1;

            extractor.compute(input_gray, keypoints1, descriptors1);
            ytrack->process(keypoints1,descriptors1);
        }

        input_pre_gray=input_gray;

        key=waitKey(30);
    }
}
