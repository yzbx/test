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

void yzbx_imfill(Mat &input){
    //assume input is uint8 B & W (0 or FG)
    //this function imitates imfill(image,'hole')

    const int FG=255;
    const int FLOODED=100;
    cv::Mat holes=input.clone();

    int x=0,y=0;
    size_t img_rows=input.rows,img_cols=input.cols;
    for(x=0;x<img_rows;x++){
        //holes.at<uchar>(x,y)==FLOODED or FG, will not be flooded again or flooded.
        //NOTE: Mat(x,y) vs Point(y,x)
        y=0;
        if(holes.at<uchar>(x,y)==0){
            cv::floodFill(holes,cv::Point2i(y,x),cv::Scalar(FLOODED));
        }
        y=img_cols-1;
        if(holes.at<uchar>(x,y)==0){
            cv::floodFill(holes,cv::Point2i(y,x),cv::Scalar(FLOODED));
        }
    }

    for(y=0;y<img_cols;y++){
        //holes.at<uchar>(x,y)==FLOODED or FG, will not be flooded again or flooded.
        x=0;
        if(holes.at<uchar>(x,y)==0){
            cv::floodFill(holes,cv::Point2i(y,x),cv::Scalar(FLOODED));
        }
        x=img_rows-1;
        if(holes.at<uchar>(x,y)==0){
            cv::floodFill(holes,cv::Point2i(y,x),cv::Scalar(FLOODED));
        }
    }

    input=input|(holes==0);
    //    for(int i=0; i<input.rows*input.cols; i++)
    //    {
    //        if(holes.data[i]==0)
    //            input.data[i]=255;
    //    }
}

void yzbx_lbp(Mat &input, Mat &lbp){
    int radius=1;
    int neighbors=8;
    Mat dst;
    cvtColor(input, dst, CV_BGR2GRAY);
    GaussianBlur(dst, dst, Size(7,7), 5, 3, BORDER_CONSTANT); // tiny bit of smoothing is always a good idea
    // comment the following lines for original size
//    resize(input, input, Size(), 0.5, 0.5);
//    resize(dst,dst,Size(), 0.5, 0.5);
    int lbp_operator=1;
    switch(lbp_operator) {
    case 0:
        lbp::ELBP(dst, lbp, radius, neighbors); // use the extended operator
        break;
    case 1:
        lbp::OLBP(dst, lbp); // use the original operator
        break;
    case 2:
        lbp::VARLBP(dst, lbp, radius, neighbors);
        break;
    }
    // now to show the patterns a normalization is necessary
    // a simple min-max norm will do the job...
    normalize(lbp, lbp, 0, 255, NORM_MINMAX, CV_8UC1);

    imshow("original", input);
    imshow("lbp", lbp);
}
