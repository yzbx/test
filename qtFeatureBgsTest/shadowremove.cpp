#include "shadowremove.h"

shadowRemove::shadowRemove()
{
    std::cout<<"shadowRemove"<<std::endl;
}

shadowRemove::~shadowRemove (){

}

void shadowRemove::process (const Mat &img_input, Mat &img_foreground, Mat &img_background){

}

void shadowRemove::processWithoutUpdate (const Mat &input, Mat &img_output, Mat &img_background){
    CV_Assert(!processedWithoutUpdate);
    processedWithoutUpdate=true;
    CV_Assert(!input.empty ());
    CV_Assert(input.type ()==CV_8UC3);

    //shadow remove
    Mat img_input,img_hsv;
    cvtColor (input,img_hsv,CV_BGR2Lab);
    img_input=img_hsv;
//    Mat imgs[3];
//    split(img_hsv,imgs);
//    imgs[2]=Mat(img_hsv.size(),CV_8UC1,200);
//    merge(imgs,3,img_hsv);
//    cvtColor (img_hsv,img_input,CV_HSV2BGR);
    imshow("shadow remove",img_input);

    if(firstTime){
        firstTime=false;

        img_rows=img_input.rows;
        img_cols=img_input.cols;
        img_size=img_input.size();
        size_t position=0;
        for(size_t i=0;i<img_rows;i++){
            for(size_t j=0;j<img_cols;j++){
                value_t value;
                if(img_roi.empty ()){
                    value=img_input.at<value_t>(i,j);
                    std::list<cache_pair_t> list1,list2;
                    _cache_one_vector.push_back (list1);
                    list2.push_front (cache_pair_t(value,1));
                    _cache_two_vector.push_back (list2);
                    position++;
                }
                else{
                    //NOTE 0 for out of roi, other for filed of roi.
                    if(img_roi.at<uchar>(i,j)!=0){
                        value=img_input.at<value_t>(i,j);
                        std::list<cache_pair_t> list1,list2;
                        _cache_one_vector.push_back (list1);
                        list2.push_front (cache_pair_t(value,1));
                        _cache_two_vector.push_back (list2);
                        position++;
      //                  img_roi_vector.push_back (cv::Vec2i(i,j));
                    }
                }
            }
        }

        img_foreground.create (img_size,CV_8UC1);
        img_foreground=Scalar::all (0);
    }
    else{
//        func_getImgForeground (img_input);
        cv::Mat img_raw_foreground(img_size,CV_8UC1);
        size_t position=0;
        for(size_t i=0;i<img_rows;i++){
            for(size_t j=0;j<img_cols;j++){
                value_t value;
                if(img_roi.empty ()||img_roi.at<uchar>(i,j)!=0){
                    value=img_input.at<value_t>(i,j);
                    bool cache_hit=findAndAdajustCacheOneList (position,value);
                    if(cache_hit){
                        img_raw_foreground.at<uchar>(i,j)=0;
                    }
                    else{
                        img_raw_foreground.at<uchar>(i,j)=255;
                    }
                    position++;
                }
            }

        }

        cv::medianBlur (img_raw_foreground,img_foreground,5);

    }

//    img_foreground.copyTo (img_output);
//    img_input.copyTo (img_input_prev);
    img_output=img_foreground.clone ();
    img_input_prev=img_input.clone ();
    frameNum++;
}

void shadowRemove::updateWithMovingStatic (const Mat &input,const Mat &MovingStaticMat){
    CV_Assert(processedWithoutUpdate);
    processedWithoutUpdate=false;
    CV_Assert(!input.empty ());
    CV_Assert(input.type ()==CV_8UC3);

    //shadow remove
    //NOTE : Lab is better than HSV
    Mat img_input,img_hsv;
    cvtColor (input,img_hsv,CV_BGR2Lab);
    img_input=img_hsv;

    CV_Assert(!MovingStaticMat.empty ());
    CV_Assert(MovingStaticMat.type ()==CV_8UC1);

    size_t position=0;
    for(size_t i=0;i<img_rows;i++){
        for(size_t j=0;j<img_cols;j++){
            value_t value;
            if(img_roi.empty ()||img_roi.at<uchar>(i,j)!=0){
                value=img_input.at<value_t>(i,j);
                //TODO: A better way to init the background.
                if(MovingStaticMat.at<uchar>(i,j)==STATIC_POINT||frameNum<_cache_level_one_size+_cache_level_two_size){
                    findAndAdajustCacheTwoList (position,value,true);
                }
                else if(MovingStaticMat.at<uchar>(i,j)==UNKNOW_POINT){
                    findAndAdajustCacheTwoList (position,value,false);
                }
                position++;
            }
        }

    }

}

void shadowRemove::loadConfig (){
    std::cout<<"load config wait todo!"<<std::endl;
}

void shadowRemove::saveConfig (){
    std::cout<<"save config wati todo!"<<std::endl;
}
