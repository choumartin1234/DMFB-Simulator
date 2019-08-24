#ifndef OPERATION_H
#define OPERATION_H
#include <QString>
#include <vector>
#include <utility>
using namespace std;

class op
{
public:
    op(QString _name, int _t, int _time_cost, vector<pair<int, int> > _points);
    ~op(){}
    QString name;
    int time;
    int time_cost;
    vector<pair<int, int> > points;
    bool operator>(const op& a)const;
};



#endif // OPERATION_H
