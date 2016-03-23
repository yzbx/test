#include <QCoreApplication>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QDir>
#include "IBGS.h"
#include "main_bgs.h"
#include "yzbx_frameinput.h"
#include "yzbx_cdnetbenchmark.h"
#include <QtCore>

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

    QDir qdir(".");
    qdir.mkdir("config");

    string inputRoot="/media/yzbx/D/firefoxDownload/matlab/dataset2012/dataset";
    string outputRoot="/media/yzbx/D/firefoxDownload/matlab/dataset2012/main_bgs";
    yzbx_CDNetBenchMark benchmark(QString::fromStdString (inputRoot),QString::fromStdString (outputRoot));

    CV_Assert(benchmark.InputPathList.size ()==benchmark.OutputPathList.size ());
    for(int benchmarkNum=0;benchmarkNum<benchmark.InputPathList.size ();benchmarkNum++){
        qDebug()<<"input="<<benchmark.InputPathList.at(benchmarkNum);
        qDebug()<<"output="<<benchmark.OutputPathList.at(benchmarkNum);

        /* Background Subtraction Methods */
        main_bgs *bgs;
        bgs=new main_bgs;

        QString qrootin=benchmark.InputPathList.at(benchmarkNum);
        QString qrootout=benchmark.OutputPathList.at(benchmarkNum);
        string rootin=qrootin.toStdString ()+"/input";
        string rootout=qrootout.toStdString ();
        yzbx_frameInput yfInput(rootin,1,-1);
        Mat input,fgMask,bgModel;
        namedWindow ("input");
        for(int i=1;;i++){
            cout<<"i="<<i<<" *****************"<<endl;
            yfInput.getNextFrame (input,FromCDNet);
            bgs->process (input,fgMask,bgModel);
            if(!input.empty ()){
                imshow("input",input);
            }
            else{
                break;
            }
            if(!fgMask.empty ()){
                imshow("fgMask",fgMask);
                stringstream ss;
                ss<<i;
                char cstr[10];
                sprintf (cstr,"%06d",i);
                string numstr(cstr);

                string filename=rootout+"/bin"+numstr+".png";
                cout<<"save img "<<filename<<endl;
                imwrite (filename,fgMask);
            }

            int key=waitKey (30);
            if(key=='q'){
                break;
            }
        }
    }

//    yzbx_frameInput yfInput("/media/yzbx/D/firefoxDownload/matlab/dataset2012/dataset/baseline/highway/input",1,-1);

}
//use video.
/*
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
            cv::imshow("foreground", img_mask);
        imshow("input",input);
        key=waitKey(30);
    }
*/
