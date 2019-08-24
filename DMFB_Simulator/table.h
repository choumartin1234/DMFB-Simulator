#ifndef TABLE_H
#define TABLE_H
#include "drop.h"
#include <vector>
#include <utility>
using namespace std;

class table
{
public:
    table(int _row, int _column);
    int row;
    int column;
    vector<drop> droplist;

};

#endif // TABLE_H
