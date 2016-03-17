#include "yzbx_tracking.h"

yzbx_tracking::yzbx_tracking()
{

}

void yzbx_tracking::process(vector<KeyPoint> &keypoints2, Mat &descriptors2,Mat input_gray,Mat input_pre_gray){
    //Note no feature will give up process.
    if(keypoints2.empty ()){
        return;
    }

    if(firstTime){
        init(keypoints2,descriptors2);
        firstTime=false;
    }
    else{
        update(keypoints2,descriptors2,input_gray,input_pre_gray);
    }

    frameNum++;
}

void yzbx_tracking::init(vector<KeyPoint> &keypoints2, Mat &descriptors2)
{
    img_rows=descriptors2.rows;
    img_cols=descriptors2.cols;
    img_size=descriptors2.size();
    img_type=descriptors2.type ();


    for(int i=0;i<keypoints2.size();i++){
        keypoints1.push_back(keypoints2[i]);
        track_node *node=newTrackNode();
        //set keypoints1 index
        node->keyPointIndex=i;

        list<Point2f> *list;
        list=&(node->trackList);
        //set position x,y
        Point2f pt=keypoints2[i].pt;
        list->push_back(pt);

        trackTree.push_back(node);
    }
    descriptors1=descriptors2.clone();

}

void yzbx_tracking::update(vector<KeyPoint> &keypoints2, Mat &descriptors2,Mat input_gray,Mat input_pre_gray)
{
    //match feature point
    BFMatcher matcher(NORM_L2);
    vector<DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);

    if( (!input_gray.empty()) && (!input_pre_gray.empty()) ){
        Mat img_matches;
        drawMatches(input_gray, keypoints1, input_pre_gray, keypoints2, matches, img_matches);
        imshow("tracking matches", img_matches);

        //reverse match
        vector<DMatch> rmatches;
        matcher.match (descriptors2,descriptors1,rmatches);
        drawMatches(input_pre_gray, keypoints2,input_gray, keypoints1, rmatches, img_matches);
        imshow("tracking reverse matches", img_matches);
    }

    //NOTE match(query,train);
    vector<bool> missedFeature;
    for(int i=0;i<keypoints1.size();i++){
        missedFeature.push_back(true);
    }
    vector<bool> newFeature;
    for(int i=0;i<keypoints2.size();i++){
        newFeature.push_back(true);
    }

    ///////////////update track tree

    //1. push matched feature node front
    vector<track_node *> newTree;
    for(int i=0;i<matches.size();i++){
        int queryIdx1=matches[i].queryIdx;
        int trainIdx2=matches[i].trainIdx;
        missedFeature[queryIdx1]=false;
        newFeature[trainIdx2]=false;

        track_node *node;
        node=(trackTree[queryIdx1]);
        node->frequency +=1;
        node->lastAppearTime =frameNum;

        //add point to list.
        list<Point2f> *list;
        list=&(node->trackList);
        //set position x,y
        Point2f pt=keypoints2[trainIdx2].pt;
        list->push_back(pt);
        //NOTE update descriptors
        for(int j=0;j<img_cols;j++){
            switch (img_type) {
            case CV_8UC3:{
                //opencv can assign Vec3b directly!
                Vec3b newB=descriptors2.at<Vec3b>(trainIdx2,j);
                descriptors1.at<Vec3b>(queryIdx1,j)=newB;
                break;
            }
            case CV_32FC1:{
                float newB=descriptors2.at<float>(trainIdx2,j);
                descriptors1.at<float>(queryIdx1,j)=newB;
                break;
            }
            default:{
                cout<<"unexpected img type"<<endl;
                CV_Assert(false);
                break;
            }
            }
        }


        newTree.push_back(node);
    }


    //2. push new feature node back
    // push missed feature node back
    for(int i=0;i<keypoints2.size();i++){
        if(newFeature[i]){
            track_node *node=newTrackNode();
            //set keypoints index=-1-i;
            node->keyPointIndex=-1-i;

            list<Point2f> *list;
            list=&(node->trackList);
            //set position x,y
            Point2f pt=keypoints2[i].pt;
            list->push_back(pt);

            newTree.push_back(node);
        }
    }

    for(int i=0;i<keypoints1.size();i++){
        if(missedFeature[i]&&newTree.size()<=maxTrackNodeNum){
            track_node *node=(trackTree[i]);
            node->missTime +=1;

            newTree.push_back(node);
        }
    }

    trackTree.swap(newTree);
//    newTree.clear();

    //3. recreate descriptors1 and keypoints1
    // according to tree
    Mat newDescriptors(trackTree.size(),img_cols,img_type);
    vector<KeyPoint> newKeyPoints;
    for(int i=0;i<trackTree.size();i++){
        track_node *node;
        node=(trackTree[i]);

        int idx=node->keyPointIndex;
        if(idx>=0){
            //keyPoints1, discriptors1
            newKeyPoints.push_back(keypoints1[idx]);
            for(int j=0;j<img_cols;j++){
                switch (img_type) {
                case CV_8UC3:{
                    //FIXME need update the descriptors!

                    //opencv can assign Vec3b directly!
                    Vec3b newB=descriptors1.at<Vec3b>(i,j);
                    newDescriptors.at<Vec3b>(i,j)=newB;
                    break;
                }
                case CV_32FC1:{
                    float newB=descriptors1.at<float>(i,j);
                    newDescriptors.at<float>(i,j)=newB;
                    break;
                }
                default:{
                    cout<<"unexpected img type"<<endl;
                    CV_Assert(false);
                    break;
                }
                }
            }
        }
        else{
            //keyPoints2, discriptors2
            idx=-idx-1;
            newKeyPoints.push_back(keypoints2[idx]);
            for(int j=0;j<img_cols;j++){
                switch (img_type) {
                case CV_8UC3:{
                    Vec3b newB=descriptors2.at<Vec3b>(i,j);
                    newDescriptors.at<Vec3b>(i,j)=newB;
                    break;
                }
                case CV_32FC1:{
                    float newB=descriptors2.at<float>(i,j);
                    newDescriptors.at<float>(i,j)=newB;
                    break;
                }
                default:{
                    cout<<"unexpected img type"<<endl;
                    CV_Assert(false);
                    break;
                }
                }
            }
        }
    }
    keypoints1.swap(newKeyPoints);
    descriptors1=newDescriptors;

    //4. refresh tree's keyPointIndex
    // according to descriptors1 and keypoints1
    for(int i=0;i<trackTree.size();i++){
        track_node *node=trackTree[i];
        node->keyPointIndex=i;
    }
}

track_node::track_node(size_t featureNumber, size_t frameNumber){
    firstAppearTime=frameNumber;
    lastAppearTime=frameNumber;
    frequency=1;
    featureNum=featureNumber;
}

track_node* yzbx_tracking::newTrackNode(){
    track_node *node=new track_node(featureNum,frameNum);
    featureNum++;
    return node;
}

void yzbx_tracking::showTrajectory(Mat &img){
    Mat showImg=img.clone();
    Point2f pt1,pt2;
    for(int i=0;i<trackTree.size();i++){
        track_node *node=trackTree[i];
        list<Point2f>* list=&(node->trackList);

        for(std::list<Point2f>::iterator j=list->begin();j!=list->end();j++){
            pt2=*j;

            if(j!=list->begin ()){
                cv::line(showImg,pt1,pt2,Scalar(255,0,0),5,8);
            }

            pt1=pt2;
        }
    }

    imshow("trajectory",showImg);

}
