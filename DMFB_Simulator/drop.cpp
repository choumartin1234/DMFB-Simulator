#include "drop.h"
#include <math.h>
using namespace std;
drop::drop(int idx, int _r, int _g, int _b, int _type, double _size):
    index(idx),r(_r), g(_g), b(_b), type(_type), size(_size)
{

}

bool drop::is_blank(){ return size < 0; }
bool drop::operator!=(const drop& a)
{
    return (index!=a.index || (r!=a.r) || (g!=a.g) || (b!=a.b));
}

void drop::set_stable()
{
    is_move = is_split = is_split_success = is_merge_success = is_merge = false;
}
bool drop::is_drop()
{
    return (type > 0) && (size > 0);
}
