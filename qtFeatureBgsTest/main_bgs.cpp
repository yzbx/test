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
    CV_Assert(!labelImg8UC1.empty ());
    imshow("label image",labelImg8UC1);

    Mat colorLabelImg;
    //input mat must be 32SC1 in this label function
    icvprLabelColor (labelImg32SC1,colorLabelImg);
    //BUG empty mat!!
    imshow("color label img",colorLabelImg);
}

main_bgs::main_bgs()
{
    //    lbpBgs=new lbp_bgs;
//        lbpBgs = new LOBSTERBGS;
//        lbpBgs = new SuBSENSEBGS;
    lbpBgs=new shadowRemove;
    npeBgs=new npe_bgs;
//    npeBgs=new FrameDifferenceBGS;
    //    originBgs=new FrameDifferenceBGS;
    originBgs=new LOBSTERBGS;
}

main_bgs::~main_bgs (){

}

void main_bgs::process (const Mat &img_input, Mat &img_foreground, Mat &img_background){
    //step 1: set img_input_gray
    CV_Assert(!img_input.empty ());
    if(img_input.type ()==CV_8UC3){
        //NOTE in opencv, the true sequence is BGR.
        cvtColor (img_input,img_input_gray,CV_BGR2GRAY);
    }
    else{
        CV_Assert(img_input.type ()==CV_8UC1);
        img_input_gray=img_input.clone();
    }

    //step 2: set MovingStaticMats, BackGroundModelMats, ForeGroundMats.
    //vector.push_back may cause memory error!!!
    vector<Mat>  matVector_MS,matVector_BGM,matVector_FG;
    for(int i=0;i<3;i++){
        Mat fg,bgm;
        matVector_BGM.push_back (bgm);
        matVector_FG.push_back (fg);
    }
    lbpBgs->processWithoutUpdate(img_input,matVector_FG[LBP_BGS],matVector_BGM[LBP_BGS]);
//        lbpBgs->process (img_input,matVector_FG[LBP_BGS],matVector_BGM[LBP_BGS]);
    npeBgs->processWithoutUpdate(img_input,matVector_FG[NPE_BGS],matVector_BGM[NPE_BGS]);
//    npeBgs->process(img_input,matVector_FG[NPE_BGS],matVector_BGM[NPE_BGS]);
    originBgs->process (img_input,matVector_FG[ORG_BGS],matVector_BGM[ORG_BGS]);

    if(matVector_FG[ORG_BGS].empty()){
        matVector_FG[ORG_BGS].create(img_input.size(),CV_8UC1);
        matVector_FG[ORG_BGS]=Scalar::all (0);
    }
    if(matVector_FG[LBP_BGS].empty()){
        matVector_FG[LBP_BGS].create(img_input.size(),CV_8UC1);
        matVector_FG[LBP_BGS]=Scalar::all (0);
    }
    if(matVector_FG[NPE_BGS].empty()){
        matVector_FG[NPE_BGS].create(img_input.size(),CV_8UC1);
        matVector_FG[NPE_BGS]=Scalar::all (0);
    }
    CV_Assert(!matVector_FG[ORG_BGS].empty());
    CV_Assert(!matVector_FG[LBP_BGS].empty());
    CV_Assert(!matVector_FG[NPE_BGS].empty());

    //DEBUG
    imshow("LBP_BGS",matVector_FG[LBP_BGS]);
    imshow("NPE_BGS",matVector_FG[NPE_BGS]);

    //if the False Postive is normal, so it's may better to open the FG.
    Mat kernel=getStructuringElement (MORPH_ELLIPSE,Size(5,5));
    for(int i=0;i<matVector_FG.size ();i++){
        morphologyEx (matVector_FG[i],matVector_FG[i],MORPH_CLOSE,kernel);
        yzbx_imfill(matVector_FG[i]);
    }

    //step 3:
    //NOTE light change pollution check
    lightChangePollutionCheck (matVector_FG);

    //step 4: separate the FGs
    vector<Mat> matVector_separatedFG;
    //the FG can consider as final FG.
//    Mat believableFG(img_input.size(),CV_8UC1);
//    believableFG=Scalar::all (0);

    int separateStrategy=Generate1FG;
    if(separateStrategy==Generate3FG){
        for(int i=0;i<3;i++){
            Mat m,ms;
            matVector_separatedFG.push_back (m);
            matVector_MS.push_back (ms);
        }
        //strategy: input FG: A,B
        //generate 3 FG: A-B, B-A, A&B
        //NOTE consider A&B as FG, add it to last FG.
        matVector_separatedFG[0]=matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS];
        matVector_separatedFG[1]=matVector_FG[NPE_BGS]-matVector_FG[LBP_BGS];
        matVector_separatedFG[2]=matVector_FG[NPE_BGS]&matVector_FG[LBP_BGS];
    }
    else if(separateStrategy==Generate4FG){
        for(int i=0;i<4;i++){
            Mat m,ms;
            matVector_separatedFG.push_back (m);
            matVector_MS.push_back (ms);
        }
        //strategy: input FG: A,B,C
        //generate 4 FG which has no common area: A-B-C,B-A-C,C-A-B,else.
        //idea: split the FG and track them separatly
        matVector_separatedFG[0]=matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS]-matVector_FG[ORG_BGS];
        matVector_separatedFG[1]=matVector_FG[NPE_BGS]-matVector_FG[LBP_BGS]-matVector_FG[ORG_BGS];
        matVector_separatedFG[2]=matVector_FG[ORG_BGS]-matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS];
        matVector_separatedFG[3]=matVector_FG[LBP_BGS]&matVector_FG[NPE_BGS];
        matVector_separatedFG[3]|=(matVector_FG[NPE_BGS]&matVector_FG[ORG_BGS]);
        matVector_separatedFG[3]|=(matVector_FG[LBP_BGS]&matVector_FG[ORG_BGS]);
    }
    else if(separateStrategy==Generate7FG){
        for(int i=0;i<7;i++){
            Mat m,ms;
            matVector_separatedFG.push_back (m);
            matVector_MS.push_back (ms);
        }
        //strategy: input FG: A,B,C
        //generate 7 FG which has no common area: A-B-C,B-A-C,C-A-B,A&B&C,A&B-A&B&C,A&C-A&B&C,B&C-A&B&C
        //idea: split the FG and track them separatly
        matVector_separatedFG[0]=matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS]-matVector_FG[ORG_BGS];
        matVector_separatedFG[1]=matVector_FG[NPE_BGS]-matVector_FG[LBP_BGS]-matVector_FG[ORG_BGS];
        matVector_separatedFG[2]=matVector_FG[ORG_BGS]-matVector_FG[LBP_BGS]-matVector_FG[NPE_BGS];
        matVector_separatedFG[3]=matVector_FG[LBP_BGS]&matVector_FG[NPE_BGS]&matVector_FG[ORG_BGS];
        matVector_separatedFG[4]=matVector_FG[LBP_BGS]&matVector_FG[NPE_BGS]-matVector_separatedFG[3];
        matVector_separatedFG[5]=matVector_FG[LBP_BGS]&matVector_FG[ORG_BGS]-matVector_separatedFG[3];
        matVector_separatedFG[6]=matVector_FG[ORG_BGS]&matVector_FG[NPE_BGS]-matVector_separatedFG[3];
    }
    else if(separateStrategy==Generate1FG){
        Mat M=matVector_FG[0]|matVector_FG[1]|matVector_FG[2];
        matVector_separatedFG.push_back (M);
        Mat ms;
        matVector_MS.push_back (ms);
    }

    //step 5: use connected component anlysis and feature tracking
    CV_Assert(matVector_separatedFG.size()==matVector_MS.size());
    getMS (img_input,matVector_separatedFG,matVector_MS);

    //DEBUG
//    for(int i=0;i<matVector_MS.size();i++){
//        showImgInLoop (matVector_MS[i]*100,i);
//    }

    //step 6: get ForeGround and update my submodels.
    Mat mixedMovingStatic;
    mixMS(matVector_MS,mixedMovingStatic);
    img_foreground=mixedMovingStatic>0;
    //NOTE consider believable as FG
//    img_foreground|=believableFG;
    lbpBgs->updateWithMovingStatic(img_input,mixedMovingStatic);
    npeBgs->updateWithMovingStatic(img_input,mixedMovingStatic);
    //NOTE: orgBgs cannot be edited and do not have update() funciton.

    if(inited) debug();

    inited=true;
    //step 7: update history information
    frameNum++;
    img_input_gray_previous=img_input_gray.clone();
    //update history information
    descriptors_previous=descriptors.clone();
    keyPoints_previous.clear ();
    keyPoints_previous.swap (keyPoints);
    img_fgMask_previous=img_foreground.clone ();

    //step 8: memory manage
    for(int i=0;i<matVector_BGM.size();i++){
        matVector_BGM[i].release();
    }
    matVector_BGM.clear ();

    for(int i=0;i<matVector_FG.size ();i++){
        matVector_FG[i].release();
    }
    matVector_FG.clear ();

    for(int i=0;i<matVector_MS.size();i++){
        matVector_MS[i].release();
    }
    matVector_MS.clear ();

    for(int i=0;i<matVector_separatedFG.size();i++){
        matVector_separatedFG[i].release();
    }
    matVector_separatedFG.clear ();
}

void main_bgs::getMS (const Mat &img_input, vector<Mat> &ForeGrounds, vector<Mat> &MovingStatics){
    //input: img_input, FG
    //output: MS
    //in fact: also need img_input_previous to track.
    //TODO: use history information such as: probablity map, FG_previous
    int featureThreshold=400;
    keyPoints.clear ();
    descriptors.release ();

    //step 1: detect keypoint
    SurfFeatureDetector detector(featureThreshold);
    detector.detect(img_input_gray,keyPoints);
    //DEBUG, no problem for empty() keyPoints.
//    keyPoints.clear ();

    //step 2
    //NOTE filter the background keyPoints after inited.
    Mat KeyPointMask;
    if(inited){
        Mat dilatedMixFG=ForeGrounds[0].clone();
        for(int i=1;i<ForeGrounds.size ();i++){
            dilatedMixFG|=ForeGrounds[i];
        }
        Mat dilatedKernel=getStructuringElement (MORPH_RECT,Size(15,15));
        dilate (dilatedMixFG,dilatedMixFG,dilatedKernel);
        CV_Assert(!img_fgMask_previous.empty ());
        KeyPointMask=dilatedMixFG|img_fgMask_previous;

        //WARNING filter the keypoints out of dilatedMixFG
        vector<KeyPoint>::iterator it,it_before;
        for(it=keyPoints.begin ();it!=keyPoints.end ();it++){
            Point2f p=it->pt;
            int x=(int)p.x;
            int y=(int)p.y;
            if(x<0||x>=img_input.cols||y<0||y>=img_input.rows){
                cout<<"x="<<p.x<<endl;
                cout<<"y="<<p.y<<endl;
                cout<<"rows="<<img_input.rows<<endl;
                cout<<"cols="<<img_input.cols<<endl;
            }
            CV_Assert(x>=0&&x<img_input.cols);
            CV_Assert(y>=0&&y<img_input.rows);
            it_before=it-1;
            if(dilatedMixFG.at<uchar>(y,x)==0){
                keyPoints.erase (it);
                it=it_before;
            }
        }
    }

    //step 3: compute descriptors
    SurfDescriptorExtractor extractor;
    extractor.compute(img_input_gray,keyPoints,descriptors);

    //step 4: match descriptors  after inited.
    if(inited){
        //CONFIG: whole img or part img;
        CV_Assert(!KeyPointMask.empty ());
        keyPoints_previous.clear ();
        detector.detect (img_input_gray_previous,keyPoints_previous,KeyPointMask);
        descriptors_previous.release ();
        extractor.compute (img_input_gray_previous,keyPoints_previous,descriptors_previous);

        matches.clear ();
        //FIXME true or false ?
        BFMatcher matcher(NORM_L2,true);
        if(!descriptors_previous.empty ()&&!descriptors.empty ()){
            CV_Assert(descriptors_previous.type ()==descriptors.type ());
            CV_Assert(descriptors_previous.cols==descriptors.cols);
            matcher.match (descriptors_previous,descriptors,matches);
        }

        Mat img_matches;
        drawMatches(img_input_gray_previous, keyPoints_previous, img_input_gray, keyPoints, matches, img_matches);
        imshow("img_matches", img_matches);

        //step 5: decide the type of all keypoints after filter.
        decideAllKeyPointTypes ();

        //step 6: compute MS
        for(int i=0;i<ForeGrounds.size ();i++){
            CV_Assert(!ForeGrounds[i].empty ());

            Mat labelImg8UC1;
            connectedCompoentSplit (ForeGrounds[i],labelImg8UC1);
            //0 static, 1 unknow, 2 moving
            getMS(labelImg8UC1,MovingStatics[i]);

            double maxVal,minVal;
            minMaxIdx (MovingStatics[i],&minVal,&maxVal);
            if(maxVal>=3){
                std::cout<<"maxVal="<<maxVal<<std::endl;
            }
            CV_Assert(maxVal<3);
        }
    }
    else{
        //init class
        img_cols=img_input.cols;
        img_rows=img_input.rows;
        img_size=img_input.size();
        img_type=img_input.type ();

        for(int i=0;i<MovingStatics.size ();i++){
            MovingStatics[i].create(img_size,CV_8UC1);
            MovingStatics[i]=Scalar::all (0);
        }
    }
}

void main_bgs::mixMS (vector<Mat> MovingStatics, Mat &mixedMovingStatic){
    //strategy: input FG: A,B,C
    //generate 4 FG which has no common area: A-B,B-A,A&B,C-A-B
    //idea: split the FG and track them separatly
    if(inited){
        CV_Assert(!MovingStatics[0].empty());
        mixedMovingStatic=MovingStatics[0].clone();
        for(int i=1;i<MovingStatics.size ();i++){
            mixedMovingStatic+=MovingStatics[i];
        }

        //NOTE Check the separated FG
        double maxVal,minVal;
        minMaxIdx (mixedMovingStatic,&minVal,&maxVal);
        if(maxVal>=3){
            imshow("mixedMovingStatic",mixedMovingStatic*100);
            std::cout<<"maxVal="<<std::endl;
            waitKey(0);
        }
        CV_Assert(maxVal<3);
    }
    else{
        mixedMovingStatic.release ();
        mixedMovingStatic.create (img_size,CV_8UC1);
        mixedMovingStatic=Scalar::all(0);
    }
}

void main_bgs::getMS(const Mat &labelImg8UC1,Mat &MS){
    //addtion input: matches, keypoints, keypoints_previous

    //step 1: count the blob type of unknow, moving and static
    double minVal,maxVal;
    minMaxIdx (labelImg8UC1,&minVal,&maxVal);
    size_t blobNum=(size_t)maxVal;

    //NOTE erode the FG when count for point type.
    //in this way, we can keep the component analysis right.
    //and remove the affect of feature point radius.
    //    Mat kernelFG=labelImg8UC1>0;
    //    Mat kernel_erode=getStructuringElement (MORPH_ELLIPSE,Size(10,10));
    //    erode(kernelFG,kernelFG,kernel_erode);

    //NOTE can I use vec3f ???
    //step 1.1: init blobTypeCount.
    vector<myVec> blobTypeCount;
    for(int i=0;i<blobNum+1;i++){
        myVec v;
        blobTypeCount.push_back (v);
    }

    //step 1.2: assign blobTypeCount by allKeyPointTypes.
    int featureNumCount=0;
    for(int i=0;i<allKeyPointTypes.size ();i++){
        int pointType=allKeyPointTypes[i];
        KeyPoint kp=keyPoints[i];
        Point2f p=kp.pt;
        int x=(int)p.x;
        int y=(int)p.y;
        CV_Assert(x>=0&&x<img_cols);
        CV_Assert(y>=0&&y<img_rows);
        int blobIdx=labelImg8UC1.at<uchar>(y,x);

        //        uchar isKernelFG=kernelFG.at<uchar>(y,x);
        //0 for background, other for foreground
        if(blobIdx!=0){
            //MOVING_POINT,UNKNOW_POINT,STATIC_POINT belong to 0~2
            //MOVING_POINT need be care to accumulate.
            //            if(pointType!=MOVING_POINT||isKernelFG>0){

            //            }
            CV_Assert(blobIdx<blobTypeCount.size ());
            CV_Assert(pointType<3);
            myVec v=(blobTypeCount[blobIdx]);
            v.a[pointType]+=1;
            blobTypeCount[blobIdx]=v;
            featureNumCount++;
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
        featureNumCount-=(v.a[0]+v.a[1]+v.a[2]);
        int blobTypeDecision=BlobTypePreferMoving;
        if(blobTypeDecision==BlobTypeRatio){
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
        else if(blobTypeDecision==BlobTypePreferMoving){
            if(v.a[MOVING_POINT]>0){
                blobType[i]=MOVING_POINT;
            }
            else if(v.a[STATIC_POINT]>0){
                blobType[i]=STATIC_POINT;
            }
            else if(v.a[UNKNOW_POINT]>0){
                blobType[i]=UNKNOW_POINT;
            }
            else{
                blobType[i]=STATIC_POINT;
            }
        }
    }

    CV_Assert(featureNumCount==0);

    //3. generate MS
    MS.release ();
    MS.create (img_size,CV_8UC1);
    for(int i=0;i<img_rows;i++){
        for(int j=0;j<img_cols;j++){
            uchar c=labelImg8UC1.at<uchar>(i,j);
            if(c!=0){
                //NOTE STATIC_POINT must be 0!!!
                uchar ch=(uchar)blobType[c];

                if(ch>=3){
                    cout<<"ch="<<(int)ch<<endl;
                    cout<<"blobType[c]="<<blobType[c]<<endl;
                }
                CV_Assert(c<blobNum+1);
                CV_Assert(ch<3);

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

void main_bgs::lightChangePollutionCheck (vector<Mat> &ForeGrounds){
    CV_Assert(ForeGrounds.size ()==3);

    vector<double> areas;
    double maxArea=0;
    int maxIdx=0;
    for(int i=0;i<ForeGrounds.size();i++){
        Scalar area=cv::sum(ForeGrounds[i]);
        areas.push_back (area[0]);
        if(maxArea<area[0]){
            maxArea=area[0];
            maxIdx=i;
        }
    }

    int img_rows=ForeGrounds[0].rows;
    int img_cols=ForeGrounds[0].cols;
    if(maxArea>255.0*img_rows*img_cols*0.1){
        double totalArea=0.0;
        for(int i=0;i<areas.size ();i++){
            totalArea+=areas[i];
        }

        //CONFIG
        if(maxArea/totalArea>0.5){
            std::cout<<"light change cause pollution, remove the pollution FG automatically"<<std::endl;
            ForeGrounds[maxIdx]=Scalar::all (0);
        }
    }
}

void main_bgs::decideAllKeyPointTypes (){
    CV_Assert(inited);

    allKeyPointTypes.clear ();
    for(int i=0;i<keyPoints.size ();i++){
        allKeyPointTypes.push_back (UNKNOW_POINT);
    }

    //show the point type mat.
    if(staticPointMat.empty ()){
        staticPointMat.create (img_size,CV_8UC1);
    }
    staticPointMat=Scalar::all (0);
    if(unknowPointMat.empty ()){
        unknowPointMat.create (img_size,CV_8UC1);
    }
    unknowPointMat=Scalar::all (0);
    if(movingPointMat.empty ()){
        movingPointMat.create (img_size,CV_8UC1);
    }
    movingPointMat=Scalar::all(0);

    float speedThresholdHigh=1.0;
    float speedThresholdLow=0.1;

    //void DescriptorMatcher::match(const Mat& queryDescriptors=preivious, const Mat& trainDescriptors, \
    // vector<DMatch>& matches, const Mat& mask=Mat() )
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
        if(speedSquare>=speedThresholdHigh) pointType=MOVING_POINT;
        else if(speedSquare>=speedThresholdLow) pointType=UNKNOW_POINT;
        else pointType=STATIC_POINT;

        //TODO refresh staticPointMat
        //in each match(P1,P2), if P1 or P2 is a static point, then the match is invalid like two static point matched.
        int x_previous=(int)P_previous.x;
        int y_previous=(int)P_previous.y;
        CV_Assert(x_previous>=0&&x_previous<img_cols);
        CV_Assert(y_previous>=0&&y_previous<img_rows);
//        if(staticPointMat.at<uchar>(y,x)==1||staticPointMat.at<uchar>(y_previous,x_previous)==1){
//            pointType=STATIC_POINT;
//        }


        //set allKeyPointTypes.
        allKeyPointTypes[idx]=pointType;
        if(pointType==MOVING_POINT){
            movingPointMat.at<uchar>(y,x)=1;
        }
        else if(pointType==STATIC_POINT){
            staticPointMat.at<uchar>(y,x)=1;
        }
    }

    for(int i=0;i<allKeyPointTypes.size();i++){
        int pointType=allKeyPointTypes[i];
        if(pointType==UNKNOW_POINT){
            KeyPoint kp=keyPoints[i];
            Point2f p=kp.pt;
            int x=(int)p.x;
            int y=(int)p.y;
            unknowPointMat.at<uchar>(y,x)=(uchar)1;
        }
    }
}

void main_bgs::debug (){
    Mat dilatedKernel=getStructuringElement (MORPH_ELLIPSE,Size(10,10));
    Mat M;
    dilate(staticPointMat,M,dilatedKernel);
    imshow("staic point mat",M>0);

    dilate(movingPointMat,M,dilatedKernel);
    imshow("moving point mat",M>0);

    dilate(unknowPointMat,M,dilatedKernel);
    imshow("unknow point mat",M>0);
}

