#ifndef SETWINDOW_H
#define SETWINDOW_H

#include <QWidget>
#include <vector>
#include <utility>
#include <dfmbwindow.h>
using namespace std;

namespace Ui {
class SetWindow;
}

class SetWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SetWindow(QWidget *parent = nullptr);
    ~SetWindow();



private slots:
    void on_cleanBox_stateChanged();
    void on_defaultBox_stateChanged();
    void on_pushButton_clicked();
    void on_case0_clicked();
    void on_case1_clicked();
    void on_case2_clicked();
    void on_case3_clicked();
    void on_error_clicked();
    void on_wash_clicked();

private:
    Ui::SetWindow *ui;
    DFMBWindow* dfmb_w;
    void Warning(QString title, QString msg);
    bool checked_on_boarder(pair<int, int> p);
    int row;
    int column;
    vector<pair<int,int> > inputPos;
    vector<pair<int,int> > cleanPos;
    pair<int,int> outputPos;
    pair<int,int> wastePos;


};

#endif // SETWINDOW_H
