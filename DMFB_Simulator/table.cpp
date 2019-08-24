#include "table.h"

table::table(int _row, int _column):row(_row), column(_column)
{
    for (int i = 0; i <= row * column+10; i++)
        droplist.push_back(drop());
}
