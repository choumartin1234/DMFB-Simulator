#include "op.h"

op::op(QString _name, int _t, int _time_cost, vector<pair<int, int>> _points):
    name(_name), time(_t), time_cost(_time_cost), points(_points)
{

}


bool op::operator>(const op &a)const
{
    if(time > a.time)
        return true;
    if (time_cost > a.time_cost)
        return true;
    return false;
}
