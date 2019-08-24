#ifndef DROP_H
#define DROP_H
#include <utility>
#include <string>
using namespace std;

class drop
{
public:
    int index;
    int r,g,b;
    int type; // type : 0 = blank,  1 = normal_drop, 2 = merge_horizon ; 3 = merge_verticle ; 4 = stretch; -1 = pollute
    double size;
    drop(int idx = -1, int _r = 255, int _g = 255, int _b = 255, int _type = 0, double _size = -1);
    bool is_blank();
    bool is_move = false;
    bool is_merge = false;
    bool is_merge_success = false;
    bool is_split = false;
    bool is_split_success = false;
    void set_stable();
    bool is_drop();
    bool operator!=(const drop& a);
};

#endif // DROP_H
