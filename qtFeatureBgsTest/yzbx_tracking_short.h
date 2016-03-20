/*
 * this is a class write to do temporary tracking
 * input: the FG Mask for bgslibrary, the input frame
 * output: the motion component and the static component for FG Mask
 * theory:
 */
#ifndef YZBX_TRACKING_SHORT_H
#define YZBX_TRACKING_SHORT_H

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>

using namespace cv;
using namespace std;

#define UNKNOW_POINT 1
#define STATIC_POINT 0
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

class yzbx_tracking_short
{
public:
    yzbx_tracking_short();
    void process(Mat &nextFrame,Mat &nextFGMask,Mat &nextMSMask);
//private:
    void update(Mat &nextFrame,Mat &nextFGMask,Mat &nextMSMask);
    //TODO: better detector and better match
    void detectAndMatch(Mat &nextFrame);
    //split the FGMask into connected component!
    void connectedCompoentSplit(Mat &nextFGMask,Mat &labelImg8UC1);
    void debug();
    vector<int> movingCountVec,staticCountVec;
    Mat currentFrame,currentFGMask,currentMSMask;
    Mat currentDescriptor,nextDescriptor;
    Mat currentGray,nextGray;
    vector<KeyPoint> currentKeyPoint,nextKeyPoint;
    vector<DMatch> currentMatch,nextMatch;

    Mat probability32FC1,MSCount8UC3;
    size_t img_cols,img_rows;
    size_t img_type;
    cv::Size img_size;
    bool inited=false;
};

//TODO choose featureThreshold
/*
 * while(featureThreshold>1000&&featureNum<10){
                if(!keypoints1.empty ()){
                    keypoints1.clear ();
                }
                SurfFeatureDetector detector(featureThreshold);
                detector.detect(input_gray, keypoints1);
                featureNum=keypoints1.size ();
                featureThreshold=featureThreshold-1000;

                if(featureNum>=10)
                    detector.detect(input_pre_gray, keypoints2);
            }

            if(featureNum<10){
                featureThreshold=1000;
                while(featureThreshold>100&&featureNum<10){
                    if(!keypoints1.empty ()){
                        keypoints1.clear ();
                    }
                    SurfFeatureDetector detector(featureThreshold);
                    detector.detect(input_gray, keypoints1);
                    featureNum=keypoints1.size ();
                    featureThreshold=featureThreshold-100;

                    if(featureNum>=10){
                        SurfFeatureDetector newDetector(featureThreshold*5);
                        newDetector.detect(input_pre_gray, keypoints2);
                    }
                }
            }

 */
#endif // YZBX_TRACKING_SHORT_H
