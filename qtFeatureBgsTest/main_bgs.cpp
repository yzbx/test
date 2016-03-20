#include "main_bgs.h"
void icvprCcaByTwoPass(const cv::Mat& _binImg, cv::Mat& _lableImg)
{
    // connected component analysis (4-component)
    // use two-pass algorithm
    // 1. first pass: label each foreground pixel with a label
    // 2. second pass: visit each labeled pixel and merge neighbor labels
    //
    // foreground pixel: _binImg(x,y) = 1
    // background pixel: _binImg(x,y) = 0


    if (_binImg.empty() ||
            _binImg.type() != CV_8UC1)
    {
        return ;
    }

    // 1. first pass

    _lableImg.release() ;
    _binImg.convertTo(_lableImg, CV_32SC1) ;

    int label = 1 ;  // start by 2
    std::vector<int> labelSet ;
    labelSet.push_back(0) ;   // background: 0
    labelSet.push_back(1) ;   // foreground: 1

    int rows = _binImg.rows - 1 ;
    int cols = _binImg.cols - 1 ;
    for (int i = 1; i < rows; i++)
    {
        int* data_preRow = _lableImg.ptr<int>(i-1) ;
        int* data_curRow = _lableImg.ptr<int>(i) ;
        for (int j = 1; j < cols; j++)
        {
            if (data_curRow[j] == 1)
            {
                std::vector<int> neighborLabels ;
                neighborLabels.reserve(2) ;
                int leftPixel = data_curRow[j-1] ;
                int upPixel = data_preRow[j] ;
                if ( leftPixel > 1)
                {
                    neighborLabels.push_back(leftPixel) ;
                }
                if (upPixel > 1)
                {
                    neighborLabels.push_back(upPixel) ;
                }

                if (neighborLabels.empty())
                {
                    labelSet.push_back(++label) ;  // assign to a new label
                    data_curRow[j] = label ;
                    labelSet[label] = label ;
                }
                else
                {
                    std::sort(neighborLabels.begin(), neighborLabels.end()) ;
                    int smallestLabel = neighborLabels[0] ;
                    data_curRow[j] = smallestLabel ;

                    // save equivalence
                    for (size_t k = 1; k < neighborLabels.size(); k++)
                    {
                        int tempLabel = neighborLabels[k] ;
                        int& oldSmallestLabel = labelSet[tempLabel] ;
                        if (oldSmallestLabel > smallestLabel)
                        {
                            labelSet[oldSmallestLabel] = smallestLabel ;
                            oldSmallestLabel = smallestLabel ;
                        }
                        else if (oldSmallestLabel < smallestLabel)
                        {
                            labelSet[smallestLabel] = oldSmallestLabel ;
                        }
                    }
                }
            }
        }
    }

    // update equivalent labels
    // assigned with the smallest label in each equivalent label set
    for (size_t i = 2; i < labelSet.size(); i++)
    {
        int curLabel = labelSet[i] ;
        int preLabel = labelSet[curLabel] ;
        while (preLabel != curLabel)
        {
            curLabel = preLabel ;
            preLabel = labelSet[preLabel] ;
        }
        labelSet[i] = curLabel ;
    }


    // 2. second pass
    for (int i = 0; i < rows; i++)
    {
        int* data = _lableImg.ptr<int>(i) ;
        for (int j = 0; j < cols; j++)
        {
            int& pixelLabel = data[j] ;
            pixelLabel = labelSet[pixelLabel] ;
        }
    }
}

void compressMat(Mat &mCV32SC1,Mat &mCV8UC1){
    mCV8UC1.release ();
    mCV8UC1.create (mCV32SC1.size(),CV_8UC1);
    mCV8UC1=Scalar::all (0);

    //push the number in mCV32SC1 to vector by sort
    //then compress it.
    //for example:srcVec 1 100 232 400 ...
    //desVec: 1,2,3,...
    //then get pair(1,1),(100,2),(232,3)...
    //convert the pixel in mCV32SC1 to mCV8UC1 according to this!
    vector<int> srcVec;
    size_t img_cols=mCV32SC1.cols,img_rows=mCV32SC1.rows;
    for(int i=0;i<img_rows;i++){
        for(int j=0;j<img_cols;j++){
            int v=mCV32SC1.at<int>(i,j);
            //find before sort
            //search after sort
            //            std::vector<int>::iterator it;
            //            it = find (srcVec.begin(), srcVec.end(), v);
            //            if (it == srcVec.end()){
            //                //not find it
            //                srcVec.push_back (v);
            //            }

            //no background
            if(v!=0&&!binary_search(srcVec.begin (),srcVec.end (),v)){
                srcVec.push_back (v);
                sort(srcVec.begin (),srcVec.end ());
            }
        }
    }

    //compress to CV8UC1
    for(int i=0;i<img_rows;i++){
        for(int j=0;j<img_cols;j++){
            int v=mCV32SC1.at<int>(i,j);
            if(v!=0){
                std::vector<int>::iterator it;
                it=search_n(srcVec.begin (),srcVec.end (),1,v);
                int des=1+(it-srcVec.begin ());
                mCV8UC1.at<uchar>(i,j)=des;
            }
        }
    }
}

cv::Scalar icvprGetRandomColor(){
    uchar r = 255 * (rand()/(1.0 + RAND_MAX));
    uchar g = 255 * (rand()/(1.0 + RAND_MAX));
    uchar b = 255 * (rand()/(1.0 + RAND_MAX));
    return cv::Scalar(b,g,r) ;
}

void icvprLabelColor(const cv::Mat& _labelImg, cv::Mat& _colorLabelImg){
    if (_labelImg.empty() ||
            _labelImg.type() != CV_32SC1)
    {
        return ;
    }

    std::map<int, cv::Scalar> colors ;

    int rows = _labelImg.rows ;
    int cols = _labelImg.cols ;

    _colorLabelImg.release() ;
    _colorLabelImg.create(rows, cols, CV_8UC3) ;
    _colorLabelImg = cv::Scalar::all(0) ;

    for (int i = 0; i < rows; i++)
    {
        const int* data_src = (int*)_labelImg.ptr<int>(i) ;
        uchar* data_dst = _colorLabelImg.ptr<uchar>(i) ;
        for (int j = 0; j < cols; j++)
        {
            int pixelValue = data_src[j] ;
            if (pixelValue > 1)
            {
                if (colors.count(pixelValue) <= 0)
                {
                    colors[pixelValue] = icvprGetRandomColor() ;
                }
                cv::Scalar color = colors[pixelValue] ;
                *data_dst++   = color[0] ;
                *data_dst++ = color[1] ;
                *data_dst++ = color[2] ;
            }
            else
            {
                data_dst++ ;
                data_dst++ ;
                data_dst++ ;
            }
        }
    }
}

void connectedCompoentSplit (Mat &nextFGMask, Mat &labelImg8UC1){
    Mat binImage;
    cv::threshold (nextFGMask,binImage,1,1,CV_THRESH_BINARY);
    Mat labelImg32SC1;
    icvprCcaByTwoPass (binImage,labelImg32SC1);
    compressMat (labelImg32SC1,labelImg8UC1);
    imshow("label image",labelImg8UC1);

    Mat colorLabelImg;
    //input mat must be 32SC1 in this label function
    icvprLabelColor (labelImg32SC1,colorLabelImg);
    //BUG empty mat!!
    imshow("color label img",colorLabelImg);
}

main_bgs::main_bgs()
{
    lbpBgs=new lbp_bgs;
    npeBgs=new npe_bgs;
    originBgs=new FrameDifferenceBGS;
}

main_bgs::~main_bgs (){

}

void main_bgs::process (const Mat &img_input, Mat &img_foreground, Mat &img_background){
    CV_Assert(!img_input.empty ());
    if(img_input.type ()==CV_8UC3){
        cvtColor (img_input,img_input_gray,CV_RGB2GRAY);
    }
    else{
        CV_Assert(img_input.type ()==CV_8UC1);
        img_input_gray=img_input;
    }

    vector<Mat>  matVector_MS,matVector_BGM,matVector_FG;
    for(int i=0;i<3;i++){
        Mat fg,bgm;
        matVector_BGM.push_back (bgm);
        matVector_FG.push_back (fg);
    }
    lbpBgs->processWithoutUpdate(img_input,matVector_FG[LBP_BGS],matVector_BGM[LBP_BGS]);
    npeBgs->processWithoutUpdate(img_input,matVector_FG[NPE_BGS],matVector_BGM[NPE_BGS]);
    originBgs->process (img_input,matVector_FG[ORG_BGS],matVector_BGM[ORG_BGS]);

    vector<Mat> matVector_separatedFG;
    for(int i=0;i<4;i++){
        Mat m,ms;
        matVector_separatedFG.push_back (m);
        matVector_MS.push_back (ms);
    }
    //strategy: input FG: A,B,C
    //generate 4 FG which has no common area: A-B,B-A,A&B,C-A-B
    //idea: split the FG and track them separatly
    matVector_separatedFG[0]=matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS];
    matVector_separatedFG[1]=matVector_FG[NPE_BGS]-matVector_FG[LBP_BGS];
    matVector_separatedFG[2]=matVector_FG[LBP_BGS]&matVector_FG[NPE_BGS];
    matVector_separatedFG[3]=matVector_FG[ORG_BGS]-matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS];

     //use connected component anlysis and feature tracking
    getMS (img_input,matVector_separatedFG,matVector_MS);

    Mat mixedMovingStatic;
    mixMS(matVector_MS,mixedMovingStatic);
    img_foreground=mixedMovingStatic>0;
    lbpBgs->updateWithMovingStatic(img_input,mixedMovingStatic);
    npeBgs->updateWithMovingStatic(img_input,mixedMovingStatic);
    //NOTE: orgBgs cannot be edited and do not have update() funciton.

    //update history information
    frameNum++;
    img_input_gray_previous=img_input_gray;
    //update history information
    descriptors_previous=descriptors.clone();
    keyPoints_previous.swap (keyPoints);
}

void main_bgs::getMS (const Mat &img_input, vector<Mat> &ForeGrounds, vector<Mat> &MovingStatics){
    //input: img_input, FG
    //output: MS
    //in fact: also need img_input_previous to track.
    //TODO: use history information such as: probablity map, FG_previous
    int featureThreshold=400;
    keyPoints.clear ();
    descriptors.release ();

    SurfFeatureDetector detector(featureThreshold);
    detector.detect(img_input_gray,keyPoints);
    SurfDescriptorExtractor extractor;
    extractor.compute(img_input_gray,keyPoints,descriptors);

    if(inited){
        matches.clear ();
        BFMatcher matcher(NORM_L2,true);
        matcher.match (descriptors_previous,descriptors,matches);

        Mat img_matches;
        drawMatches(img_input_gray_previous, keyPoints_previous, img_input_gray, keyPoints, matches, img_matches);
        imshow("img_matches", img_matches);

        //compute MS
        for(int i=0;i<ForeGrounds.size ();i++){
            CV_Assert(!ForeGrounds[i].empty ());

            Mat labelImg8UC1;
            connectedCompoentSplit (ForeGrounds[i],labelImg8UC1);
            //0 static, 1 unknow, 2 moving
            getMS(labelImg8UC1,MovingStatics[i]);
        }
    }
    else{
        //init class
        img_cols=img_input.cols;
        img_rows=img_input.rows;
        img_size=img_input.size();
        img_type=img_input.type ();
        inited=true;
    }
}

void main_bgs::mixMS (vector<Mat> MovingStatics, Mat &mixedMovingStatic){
    //strategy: input FG: A,B,C
    //generate 4 FG which has no common area: A-B,B-A,A&B,C-A-B
    //idea: split the FG and track them separatly

    CV_Assert(!MovingStatics[0].empty());
    mixedMovingStatic=MovingStatics[0].clone();
    for(int i=1;i<MovingStatics.size ();i++){
        mixedMovingStatic+=MovingStatics[i];
    }

    //NOTE Check the separated FG
    double maxVal,minVal;
    minMaxIdx (mixedMovingStatic,&minVal,&maxVal);
    CV_Assert(maxVal<3);
}

void main_bgs::getMS(const Mat &labelImg8UC1,Mat &MS){
    //addtion input: matches, keypoints, keypoints_previous

    //1. count the blob type of unknow, moving and static
    double minVal,maxVal;
    minMaxIdx (labelImg8UC1,&minVal,&maxVal);
    size_t blobNum=(size_t)maxVal;

    //NOTE can I use vec3f ???
    vector<myVec> blobTypeCount;
    for(int i=0;i<blobNum+1;i++){
        myVec v;
        blobTypeCount.push_back (v);
    }

    float speedThresholdHigh=25.0;
    float speedThresholdLow=4;
    for(int i=0;i<matches.size ();i++){
        DMatch dm=matches[i];
        int idx_previous=dm.queryIdx;
        int idx=dm.trainIdx;
        CV_Assert(idx_previous<keyPoints_previous.size ());
        CV_Assert(idx<keyPoints.size());

        KeyPoint KP_previous=keyPoints_previous[idx_previous];
        KeyPoint KP=keyPoints[idx];
        Point2f P_previous=KP_previous.pt;
        Point2f P=KP.pt;
        float speedSquare=(P_previous.x-P.x)*(P_previous.x-P.x)+(P_previous.y-P.y)*(P_previous.y-P.y);
        int x=(int)P.x;
        int y=(int)P.y;
        CV_Assert(x>=0&&x<img_cols);
        CV_Assert(y>=0&&y<img_rows);

        int pointType;
        if(speedSquare>speedThresholdHigh) pointType=MOVING_POINT;
        else if(speedSquare>speedThresholdLow) pointType=UNKNOW_POINT;
        else pointType=STATIC_POINT;

        int blobIdx=labelImg8UC1.at<uchar>(y,x);
        //0 for background, other for foreground
        if(blobIdx!=0){
            //MOVING_POINT,UNKNOW_POINT,STATIC_POINT belong to 0~2
            myVec v=(blobTypeCount[blobIdx]);
            v.a[pointType]+=1;
            blobTypeCount[blobIdx]=v;
        }
    }

    //2. decide the blobType
//    vector<int> blobType[blobNum+1];
    vector<int> blobType;
    for(int i=0;i<blobNum+1;i++){
        blobType.push_back (0);
    }

    for(int i=0;i<blobNum+1;i++){
        myVec v=blobTypeCount[i];

        float ratio=(2*v.a[MOVING_POINT]+v.a[UNKNOW_POINT])/(2*v.a[STATIC_POINT]+v.a[UNKNOW_POINT]+0.1);
        if(ratio>1){
            blobType[i]=MOVING_POINT;
        }
        else if(ratio>0.5){
            blobType[i]=UNKNOW_POINT;
        }
        else{
            //NOTE STATIC_POINT must be 0!!!
            blobType[i]=STATIC_POINT;
        }
    }

    //3. generate MS
    MS.release ();
    MS.create (img_size,CV_8UC1);
    for(int i=0;i<img_rows;i++){
        for(int j=0;j<img_cols;j++){
            char c=labelImg8UC1.at<uchar>(i,j);
            if(c!=0){
                //NOTE STATIC_POINT must be 0!!!
                char ch=(char)blobType[c];
                MS.at<uchar>(i,j)=ch;
            }
            else{
                //NOTE STATIC_POINT must be 0!!!
                MS.at<uchar>(i,j)=0;
            }
        }
    }

}

void main_bgs::loadConfig (){
    std::cout<<"load config wait todo!"<<std::endl;
}

void main_bgs::saveConfig (){
    std::cout<<"save config wati todo!"<<std::endl;
}
