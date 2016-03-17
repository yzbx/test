#ifndef YZBX_TRACKING_H
#define YZBX_TRACKING_H
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class track_node{
public:
    track_node(size_t featureNumber,size_t frameNumber);
    size_t featureNum;
    //NOTE for new feature, keyPointIndex<0
    //or -1-i, here i is the idx of keypoints2.
    //for other feature:
    //keyPointIndex <==> Mat discriptor[row ith,:]<==>
    //vector<KeyPoint> keyPoints[idx]
    //Mat will have fixed col size(yzbx_tracking::img_cols is constant).
    int keyPointIndex;
    size_t objectNum;
    size_t frequency;
    size_t firstAppearTime,lastAppearTime;
    size_t missTime=0;
    list<Point2f> trackList;
};

class yzbx_tracking
{
    typedef std::vector<track_node *> track_tree_t;
public:
    yzbx_tracking();
    void process(vector<KeyPoint> &keypoints2,Mat &descriptors2,Mat input_gray=Mat(),Mat input_pre_gray=Mat());
//    void showMatch(Mat &img1,vector<KeyPoint> &keyP1,Mat &img2, \
//                   vector<KeyPoint> &keyP2, vector<DMatch> &matches);
    void showTrajectory(Mat &img);
private:
    bool firstTime=true;
    size_t frameNum=0;
    size_t featureNum=0;
    //tracking tree, include a lot of tracking vector
    track_tree_t trackTree;
    //create an specify tracking vector
    track_node *newTrackNode();

    void init(vector<KeyPoint> &keypoints2,Mat &descriptors2);
    void update(vector<KeyPoint> &keypoints2, Mat &descriptors2, Mat input_gray=Mat(), Mat input_pre_gray=Mat());

    vector<KeyPoint> keypoints1;
    Mat descriptors1;
    size_t img_rows,img_cols;
    Size img_size;
    size_t img_type;

    //keep the size of feature list
    size_t maxTrackNodeNum=500;
    //for static feature!!!
    size_t maxTrackListLength=200;

};

#endif // YZBX_TRACKING_H
