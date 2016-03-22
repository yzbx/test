#ifndef SHADOWREMOVE_H
#define SHADOWREMOVE_H

#include "IBGS.h"
#include "yzbx_utility.h"
#include "yzbx_config.h"

#define STATIC_POINT 0
#define UNKNOW_POINT 1
#define MOVING_POINT 2
using namespace cv;
using namespace cv;

class shadowRemove_pair{
public:
    shadowRemove_pair(){}
    shadowRemove_pair(cv::Vec3b a,size_t b){
        first=a;
        second=b;
    }

    cv::Vec3b first;
    size_t second;
};

class shadowRemove_listNode{
public:
    shadowRemove_listNode(){}
    std::list<shadowRemove_pair> list;
};

class shadowRemove : public IBGS
{
public:
    shadowRemove();
    ~shadowRemove();
    typedef cv::Vec3b value_t;
    //the size of value_t, for CV8UC1, _value_size=1
    //for CV_8UC3, _value_size=3 ...
    //for hsv, count just h,s
    size_t _value_size=3;
    //    size_t _value_size=yzbx_config::shadowRemove::_value_size;

    //    typedef typename std::pair<value_t, size_t> cache_pair_t;
    //    typedef typename std::list<cache_pair_t>::iterator list_iterator_t;
    //    typedef typename std::list<cache_pair_t> cache_list_t;
    typedef shadowRemove_listNode cache_node_t;
    typedef shadowRemove_pair cache_pair_t;
    typedef typename std::list<cache_pair_t>::iterator list_iterator_t;
    typedef typename std::list<cache_pair_t> cache_list_t;
public:

    void process(const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background);
    void processWithoutUpdate(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_background);
    void updateWithMovingStatic(const cv::Mat &img_input,const cv::Mat &MovingStaticMat);
private:
    void saveConfig();
    void loadConfig();
    void func_init(const Mat &img_input);
    void func_getImgForeground(const Mat &img_input);

    bool firstTime=true;
    bool showOutput=true;
    int frameNum=0;
    bool processedWithoutUpdate=false;
    cv::Mat img_input_prev;
    cv::Mat img_foreground;
    //for each pixel point, there is a cache list to keep the background model.
    //    std::list<cache_pair_t> _cache_one_list,_cache_two_list;
    //for the whole image, there is a map to keep all the background model of pixel points.
    //   std::vector<std::list<cache_pair_t>> _cache_one_vector,_cache_two_vector;
    std::vector<shadowRemove_listNode> _cache_one_vector,_cache_two_vector;
    cv::Mat img_roi;
    //store level one and level two cache in different list.
    //the pixel in level one cache is the background model
    size_t _cache_level_one_size=3;
    size_t _cache_level_two_size=5;
    size_t _cache_one_minFrequency=5;
    int _cache_hit_distance=10;
    size_t img_rows,img_cols;
    cv::Size img_size;


    //shadowRemove kernel function
    //return true for find.
    //return false for not find.
    //if not hit, do nothing.
    bool findAndAdajustCacheOneList(size_t position,value_t input_value){
        //std::list<key_value_pair_t> _cache_one_list,_cache_two_list;
        //       cache_list_t *_cache_one_list=&_cache_one_vector[position];
        shadowRemove_listNode *node=&(_cache_one_vector[position]);
        std::list<shadowRemove_pair> *_cache_one_list=&(node->list);
        bool cache_hit=false;
        cache_list_t::iterator it_before;
        for(auto it=_cache_one_list->begin();it!=_cache_one_list->end();it++) {
            cache_pair_t pair=*it;
            value_t model_value=pair.first;
            size_t frequcency=pair.second;
            size_t distance=0;
            /* NOTE just change the distance and data type, we can easy create another algorithm.
             *
           */
            //FIXME hammin distance
            for(int i=1;i<_value_size;i++){
                distance+=yzbx_d2 (model_value[i],input_value[i]);
            }

            if(distance<=_cache_hit_distance){
                cache_hit=true;
                it_before=it;
                it_before--;
                _cache_one_list->erase (it);
                it=it_before;

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
        //        cache_list_t *_cache_two_list=&_cache_two_vector[position];
        cache_node_t *node=&(_cache_two_vector[position]);
        cache_list_t *_cache_two_list=&(node->list);
        bool cache_hit=false;
        cache_list_t::iterator it_before;
        for(cache_list_t::iterator it=_cache_two_list->begin();it!=_cache_two_list->end();it++) {
            value_t model_value=it->first;
            size_t frequcency=it->second;
            size_t distance=0;

            //FIXME hammin distance
            for(int i=1;i<_value_size;i++){
                //               if(model_value[i]!=input_value[i]){
                //                   distance++;
                //               }
                //               distance+=yzbx_hamdist(model_value[i],input_value[i]);
                distance+=yzbx_d2(model_value[i],input_value[i]);
            }

            if(distance<=_cache_hit_distance){
                cache_hit=true;
                frequcency++;

                if(frequcency>_cache_one_minFrequency&&levelUp){
                    //remove from cache two
                    it_before=it;
                    it_before--;
                    _cache_two_list->erase (it);
                    it=it_before;

                    //level up to cache one
                    cache_node_t *node_one=&(_cache_one_vector[position]);
//                    cache_list_t* _cache_one_list=&_cache_one_vector[position];
                    cache_list_t* _cache_one_list=&(node_one->list);
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
                            it_before=_cache_two_list->end ();
                            it_before--;
                            if(it==it_before) it--;
                            _cache_two_list->pop_back ();
                        }

                    }

                    //                    _cache_one_vector[position]=_cache_one_list;
                }
                else{
                    //move to front in cache two and frequency plus one.
                    it_before=it;
                    it_before--;
                    _cache_two_list->erase (it);
                    it=it_before;
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
};

#endif // SHADOWREMOVE_H
