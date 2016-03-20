#ifndef LBP_BGS_H
#define LBP_BGS_H
#include "IBGS.h"
#include "LBP.hpp"
#include "yzbx_utility.h"

using namespace cv;
using namespace lbp;
#define STATIC_POINT 0
#define UNKNOW_POINT 1
#define MOVING_POINT 2
void testlbp(const Mat &img_input);

//NOTE value: the descriptor for LBP, the value in cache list.
//cv::Vec3b, cv::Vec3f ...
//template<typename value_t>
class lbp_bgs : public IBGS
{
    typedef cv::Vec3b value_t;
    //LBP
    size_t _value_size=3;

    typedef typename std::pair<value_t, size_t> cache_pair_t;
    typedef typename std::list<cache_pair_t>::iterator list_iterator_t;
    typedef typename std::list<cache_pair_t> cache_list_t;

private:
    void computeLBP(const cv::Mat &img_input,cv::Mat &img_lbp);
   void saveConfig();
   void loadConfig();
   void func_init(const Mat &img_input);
   void func_getImgForeground(const Mat &img_input);

   bool firstTime=false;
   bool showOutput;
   bool processedWithoutUpdate=false;
   int frameNum=0;
   cv::Mat img_input_prev;
   cv::Mat img_foreground;
   //for each pixel point, there is a cache list to keep the background model.
//    std::list<cache_pair_t> _cache_one_list,_cache_two_list;
   //for the whole image, there is a map to keep all the background model of pixel points.
   std::vector<std::list<cache_pair_t>> _cache_one_vector,_cache_two_vector;
   cv::Mat img_roi;
   //store level one and level two cache in different list.
   //the pixel in level one cache is the background model
   size_t _cache_level_one_size=3;
   size_t _cache_level_two_size=10;
   size_t _cache_one_minFrequency=5;
   int _cache_hit_distance=8;
   size_t img_rows,img_cols;
   cv::Size img_size;


//lbp_bgs kernel function
   //return true for find.
   //return false for not find.
   //if not hit, do nothing.
   bool findAndAdajustCacheOneList(size_t position,value_t input_value){
       //std::list<key_value_pair_t> _cache_one_list,_cache_two_list;
       cache_list_t *_cache_one_list=&_cache_one_vector[position];
       bool cache_hit=false;
       for(auto it=_cache_one_list->begin();it!=_cache_one_list->end();it++) {
           cache_pair_t pair=*it;
           value_t model_value=pair.first;
           size_t frequcency=pair.second;
           size_t distance=0;
           /* NOTE just change the distance and data type, we can easy create another algorithm.
             *
           */
           //FIXME hammin distance
           for(int i=0;i<_value_size;i++){
//               if(model_value[i]!=input_value[i]){
//                   distance++;
//               }
               distance+=yzbx_hamdist(model_value[i],input_value[i]);
           }

           if(distance<=_cache_hit_distance){
               cache_hit=true;
               _cache_one_list->erase (it);
               _cache_one_list->push_front (cache_pair_t(model_value,frequcency+1));
               break;
           }
       }

//        _cache_one_vector[position]=_cache_one_list;
       return cache_hit;
   }

   //return true for find.
   //return false for not find.
   //if not hit, whether add it to cache one depends the final result.
   bool findAndAdajustCacheTwoList(size_t position,value_t input_value,bool levelUp){
       //std::list<key_value_pair_t> _cache_one_list,_cache_two_list;
       cache_list_t *_cache_two_list=&_cache_two_vector[position];
       bool cache_hit=false;
       for(auto it=_cache_two_list->begin();it!=_cache_two_list->end();it++) {
           value_t model_value=it->first;
           size_t frequcency=it->second;
           size_t distance=0;

           //FIXME hammin distance
           for(int i=0;i<_value_size;i++){
//               if(model_value[i]!=input_value[i]){
//                   distance++;
//               }
               distance+=yzbx_hamdist(model_value[i],input_value[i]);
           }

           if(distance<=_cache_hit_distance){
               cache_hit=true;
               frequcency++;

               if(frequcency>_cache_one_minFrequency&&levelUp){
                   //remove from cache two
                   _cache_two_list->erase (it);

                   //level up to cache one
                   cache_list_t* _cache_one_list=&_cache_one_vector[position];
                   _cache_one_list->push_front (cache_pair_t(model_value,frequcency));

                   //keep the size of cache one
                   if(_cache_one_list->size ()>_cache_level_one_size){
                       auto pair_back=_cache_one_list->back();
                       _cache_one_list->pop_back ();

                       //level down to cache two
                       _cache_two_list->push_front (pair_back);
                       //keep the size of cache two
                       if(_cache_two_list->size()>_cache_level_two_size){
//                            _cache_two_list->erase (_cache_two_list->end());
                           _cache_two_list->pop_back ();
                       }

                   }

//                    _cache_one_vector[position]=_cache_one_list;
               }
               else{
                   //move to front in cache two and frequency plus one.
                   _cache_two_list->erase (it);
                   _cache_two_list->push_front (cache_pair_t(model_value,frequcency));
               }

               break;
           }
       }

       //add new pair to cache two.
       if(!cache_hit){
           _cache_two_list->push_front (cache_pair_t(input_value,1));
           if(_cache_two_list->size()>_cache_level_two_size){
//                _cache_two_list->erase (_cache_two_list->back());
               _cache_two_list->pop_back ();
           }
       }

//        _cache_two_vector[position]=_cache_two_list;
       return cache_hit;
   }
public:
    lbp_bgs();
   ~lbp_bgs();
    void process(const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background);
    void processWithoutUpdate(const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background);
    void updateWithMovingStatic(const Mat img_input, const cv::Mat &MovingStaticMat);
};

#endif // LBP_BGS_H
