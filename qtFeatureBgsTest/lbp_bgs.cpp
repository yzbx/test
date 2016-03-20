#include "lbp_bgs.h"
#include <QDir>

using namespace lbp;
//template<typename value_t>
lbp_bgs::lbp_bgs() : firstTime(true), showOutput(true)
{
  std::cout << "lbp_bgs()" << std::endl;
}

//template<typename value_t>
lbp_bgs::~lbp_bgs()
{
  std::cout << "~lbp_bgs()" << std::endl;
}

//template<typename value_t>
void lbp_bgs::process(const cv::Mat &input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{
  if(input.empty())
    return;

  Mat img_input;
  computeLBP (input,img_input);

  if(firstTime){
      QDir qdir(".");
      qdir.mkdir ("config");

      loadConfig();

      firstTime = false;
      frameNum=1;
      func_init(img_input);
     saveConfig();
     img_input.copyTo(img_input_prev);
     return;
  }

  func_getImgForeground(img_input);

  if(showOutput)
    cv::imshow("LBP BGS", img_foreground);

  img_foreground.copyTo(img_output);

  img_input.copyTo(img_input_prev);

  frameNum++;
}

//template<typename value_t>
void lbp_bgs::func_init(const Mat &img_input){
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

}

//template<typename value_t>
void lbp_bgs::func_getImgForeground(const Mat &img_input){
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

  position=0;
  for(size_t i=0;i<img_rows;i++){
      for(size_t j=0;j<img_cols;j++){
          value_t value;
          if(img_roi.empty ()||img_roi.at<uchar>(i,j)!=0){
              value=img_input.at<value_t>(i,j);
              if(img_foreground.at<uchar>(i,j)==0||frameNum<_cache_level_one_size+_cache_level_two_size){
                  findAndAdajustCacheTwoList (position,value,true);
              }
              else{
                  findAndAdajustCacheTwoList (position,value,false);
              }
              position++;
          }
      }

  }

}

//template<typename value_t>
void lbp_bgs::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/lbp_bgs.xml", 0, CV_STORAGE_WRITE);

  cvWriteInt(fs, "showOutput", showOutput);

  cvReleaseFileStorage(&fs);
}

//template<typename value_t>
void lbp_bgs::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/lbp_bgs.xml", 0, CV_STORAGE_READ);

  showOutput = cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}

void testlbp (const Mat &img_input){
        int rad = 3;
        int pts = 8;
        //ri,riu2,hf,u2
        string mapping = "u2";
        Mat imgOrg;
        // Read an (RGB) image and convert to monochrome
//        Mat imgOrg = imread( fileName, -1 );
        // convert to double precision
        img_input.convertTo( imgOrg, CV_64F );

        Mat lbpImg;
        switch( imgOrg.channels() ) {
            case 1:
                lbpImg = Mat( imgOrg.size(), CV_8UC1, Scalar( 0 ) );
                break;
            case 3:
                lbpImg = Mat( imgOrg.size(), CV_8UC3, Scalar( 0 ) );
                break;
            default:
                cerr << "Unsupported number of image channels 1/3 only." << endl;
                exit( 1 );
        }

        // Create an LBP instance of type "mapping" using "pts" support points
        LBP lbp( pts, LBP::strToType( mapping ) );

        for( int i = 0; i < imgOrg.channels(); i++ ) {
            // Copy channel i
            Mat img( imgOrg.size(), imgOrg.depth(), 1 );
            const int from_to1[] = { i, 0 };
            mixChannels( &imgOrg, 1, &img, 1, from_to1, 1 );

            // Calculate the descriptor
            lbp.calcLBP( img, rad, true );

            // Copy lbp image
            const int from_to2[] = {0, i};
            Mat tmpImg = lbp.getLBPImage();
            mixChannels( &tmpImg, 1, &lbpImg, 1, from_to2, 1 );
        }

        imshow("lbp",lbpImg);
        cout<<"CV_64FC3="<<CV_64FC3<<endl;
        cout<<"CV_32FC3="<<CV_32FC3<<endl;
        cout<<"CV_8UC3="<<CV_8UC3<<endl;
        cout<<"lbpImg.type()"<<lbpImg.type ()<<endl;
}

void lbp_bgs::processWithoutUpdate (const Mat &input, Mat &img_output, Mat &img_background){
    CV_Assert(!processedWithoutUpdate);
    processedWithoutUpdate=true;
    CV_Assert(!input.empty ());
    Mat img_input;
    computeLBP (input,img_input);

    if(firstTime){
        firstTime=false;
        func_init(img_input);
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

void lbp_bgs::updateWithMovingStatic (const Mat img_input,const Mat &MovingStaticMat){
    CV_Assert(processedWithoutUpdate);
    processedWithoutUpdate=false;

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

void lbp_bgs::computeLBP (const Mat &img_input, Mat &img_lbp){
    int rad = 3;
    int pts = 8;
    //ri,riu2,hf,u2
    string mapping = "u2";
    Mat imgOrg;
    // Read an (RGB) image and convert to monochrome
//        Mat imgOrg = imread( fileName, -1 );
    // convert to double precision
    img_input.convertTo( imgOrg, CV_64F );

    Mat lbpImg;
    switch( imgOrg.channels() ) {
        case 1:
            lbpImg = Mat( imgOrg.size(), CV_8UC1, Scalar( 0 ) );
            break;
        case 3:
            lbpImg = Mat( imgOrg.size(), CV_8UC3, Scalar( 0 ) );
            break;
        default:
            cerr << "Unsupported number of image channels 1/3 only." << endl;
            exit( 1 );
    }

    // Create an LBP instance of type "mapping" using "pts" support points
    LBP lbp( pts, LBP::strToType( mapping ) );

    for( int i = 0; i < imgOrg.channels(); i++ ) {
        // Copy channel i
        Mat img( imgOrg.size(), imgOrg.depth(), 1 );
        const int from_to1[] = { i, 0 };
        mixChannels( &imgOrg, 1, &img, 1, from_to1, 1 );

        // Calculate the descriptor
        lbp.calcLBP( img, rad, true );

        // Copy lbp image
        const int from_to2[] = {0, i};
        Mat tmpImg = lbp.getLBPImage();
        mixChannels( &tmpImg, 1, &lbpImg, 1, from_to2, 1 );
    }

    img_lbp=lbpImg.clone ();
}
