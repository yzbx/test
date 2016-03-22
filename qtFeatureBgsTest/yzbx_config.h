#ifndef YZBX_CONFIG
#define YZBX_CONFIG

//A-B,B-A,A&B
#define Generate3FG 0
//A-B-C,B-A-C,C-A-B, else
#define Generate4FG 1
//A-B-C,B-A-C,C-A-B,A&B-A&B&C,A&C-A&B&C,B&C-A&B&C,A&B&C
#define Generate7FG 2
//simple mix, A|B|C
#define Generate1FG 3

namespace yzbx_config {

namespace shadowRemove{
namespace Lab_la{
//use only l and a, but the effect is very good.
const size_t _value_size=2;
const size_t _cache_level_one_size=20;
const size_t _cache_level_two_size=25;
const size_t _cache_one_minFrequency=5;
const int _cache_hit_distance=100;
}

namespace Lab_ab{
const size_t _value_size=2;
const size_t _cache_level_one_size=20;
const size_t _cache_level_two_size=25;
const size_t _cache_one_minFrequency=5;
const int _cache_hit_distance=100;
}

}

namespace  npe_bgs {

namespace rgb{
const size_t _value_size=3;
const size_t _cache_level_one_size=20;
const size_t _cache_level_two_size=25;
const size_t _cache_one_minFrequency=5;
const int _cache_hit_distance=50;
}

namespace lab_l1{
const size_t _value_size=3;
const size_t _cache_level_one_size=20;
const size_t _cache_level_two_size=25;
const size_t _cache_one_minFrequency=5;
const int _cache_hit_distance=50;
}

namespace lab_l2{
const size_t _value_size=3;
const size_t _cache_level_one_size=20;
const size_t _cache_level_two_size=25;
const size_t _cache_one_minFrequency=5;
const int _cache_hit_distance=50;
}
}

}
#endif // YZBX_CONFIG

