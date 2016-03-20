#include "yzbx_tracking_short.h"

yzbx_tracking_short::yzbx_tracking_short()
{

}

void yzbx_tracking_short::process (Mat &nextFrame, Mat &nextFGMask, Mat &nextMSMask){
    if(nextFGMask.empty ()||nextFGMask.empty ()){
        cout<<"empty nextFrame or nextFGMask"<<endl;
        return;
    }

    if(inited){
       update(nextFrame,nextFGMask,nextMSMask);
       debug();
    }
    else{
        img_cols=nextFrame.cols;
        img_rows=nextFrame.rows;
        img_type=nextFrame.type ();
        img_size=nextFrame.size();

        probability32FC1.create (img_size,CV_32FC1);
        probability32FC1=Scalar::all(0.5);
        MSCount8UC3.create (img_size,CV_8UC3);
        MSCount8UC3=Scalar::all(0);

        detectAndMatch (nextFrame);
        //update
        currentKeyPoint.swap (nextKeyPoint);
        currentDescriptor=nextDescriptor;
        currentMatch.swap (nextMatch);
        currentGray=nextGray;

        nextMSMask.release ();
        nextMSMask.create (img_size,CV_8UC1);
        nextMSMask=Scalar::all(UNKNOW_POINT);

        inited=true;
    }

    currentFrame=nextFrame.clone();
    currentFGMask=nextFGMask.clone();
    currentMSMask=nextMSMask.clone();
}

void yzbx_tracking_short::update(Mat &nextFrame,Mat &nextFGMask,Mat &nextMSMask){
    Mat labelImg8UC1;
    connectedCompoentSplit (nextFGMask,labelImg8UC1);
    detectAndMatch (nextFrame);

    //use labelImg8UC1 and nextMatch to update probability32FC1 and MSCount8UC3

    //1. count the blob type of unknow, moving and static
    double minVal,maxVal;
    minMaxIdx (labelImg8UC1,&minVal,&maxVal);
    size_t blobNum=(size_t)maxVal;

    vector<myVec> blobTypeCount;
    for(int i=0;i<blobNum+1;i++){
        myVec v;
        blobTypeCount.push_back (v);
    }

    float speedThresholdHigh=25.0;
    float speedThresholdLow=4;
    for(int i=0;i<nextMatch.size ();i++){
        DMatch dm=nextMatch[i];
        int currentIdx=dm.queryIdx;
        int nextIdx=dm.trainIdx;
        CV_Assert(currentIdx<currentKeyPoint.size ());
        CV_Assert(nextIdx<nextKeyPoint.size());
        KeyPoint currentKP=currentKeyPoint[currentIdx];
        KeyPoint nextKP=nextKeyPoint[nextIdx];
        Point2f currentP=currentKP.pt;
        Point2f nextP=nextKP.pt;
        float speedSquare=(currentP.x-nextP.x)*(currentP.x-nextP.x)+(currentP.y-nextP.y)*(currentP.y-nextP.y);
        int x=(int)nextP.x;
        int y=(int)nextP.y;
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
            blobType[i]=STATIC_POINT;
        }
    }

    for(int i=0;i<blobType.size ();i++){
        cout<<"blobType[i]="<<blobType[i]<<endl;
    }

    //3. generate nextMSMask
    //BUG
    nextMSMask.release ();
    nextMSMask.release ();
    nextMSMask.create (img_size,CV_8UC1);
    for(int i=0;i<img_rows;i++){
        for(int j=0;j<img_cols;j++){
            char c=labelImg8UC1.at<uchar>(i,j);
            if(c!=0){
                char ch=(char)blobType[c];
                nextMSMask.at<uchar>(i,j)=ch;
            }
            else{
                nextMSMask.at<uchar>(i,j)=0;
            }
        }
    }

    //TODO 4. update probability32FC1 and MSCount8UC3

    //update
    currentKeyPoint.swap (nextKeyPoint);
    currentDescriptor=nextDescriptor;
    currentMatch.swap (nextMatch);
    currentGray=nextGray;

}

void yzbx_tracking_short::detectAndMatch (Mat &nextFrame){
    int featureThreshold=400;
//    Mat nextGray;
    //NOTE nextFrame may change from rgb to gray!!!
    if(nextFrame.type()==CV_8UC3){
        cvtColor (nextFrame,nextGray,CV_RGB2GRAY);
    }
    else{
        nextGray=nextFrame;
    }

    if(inited){
//        vector<KeyPoint> nextKeyPoint;
//        Mat nextDescriptor;
//        vector<DMatch> nextMatch;
        nextKeyPoint.clear ();
        nextDescriptor.release ();
        nextMatch.clear ();

        SurfFeatureDetector detector(featureThreshold);
        detector.detect(nextGray,nextKeyPoint);
        SurfDescriptorExtractor extractor;
        extractor.compute(nextGray,nextKeyPoint,nextDescriptor);
        BFMatcher matcher(NORM_L2,true);
        matcher.match (currentDescriptor,nextDescriptor,nextMatch);

        //show
        Mat img_matches;
//        cout<<"type="<<currentGray.type ()<<" "<<nextGray.type ()<<endl;
//        cout<<"size="<<currentGray.size()<<" "<<nextGray.size()<<endl;
//        cout<<"length="<<currentDescriptor.size()<<" "<<nextDescriptor.size()<<endl;
//        cout<<"match="<<nextMatch.size ();
//        cout<<"keypooint="<<currentKeyPoint.size ()<<" "<<nextKeyPoint.size();
//        imshow("currentGray",currentGray);
//        imshow("nextGray",nextGray);
//        imshow("currentD",currentDescriptor);
//        imshow("nextD",nextDescriptor);

        drawMatches(currentGray, currentKeyPoint, nextGray, nextKeyPoint, nextMatch, img_matches);
        imshow("img_matches", img_matches);
    }
    else{
        //init the feature and other things.
        SurfFeatureDetector detector(featureThreshold);
        detector.detect(nextGray,nextKeyPoint);
        SurfDescriptorExtractor extractor;
        extractor.compute(nextGray,nextKeyPoint,nextDescriptor);
    }
}

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

void yzbx_tracking_short::connectedCompoentSplit (Mat &nextFGMask, Mat &labelImg8UC1){
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

void yzbx_tracking_short::debug (){
//    cout<<"1 "<<currentKeyPoint.size ()<<endl;
//    cout<<"2 "<<nextKeyPoint.size()<<endl;
//    cout<<"3 "<<currentMatch.size()<<endl;
//    cout<<"4 "<<nextMatch.size()<<endl;
//    cout<<"5 "<<currentDescriptor.size()<<endl;
//    cout<<"6 "<<nextDescriptor.size()<<endl;
//    cout<<"7 "<<currentGray.size()<<endl;
//    cout<<"8 "<<nextGray.size()<<endl;

}
