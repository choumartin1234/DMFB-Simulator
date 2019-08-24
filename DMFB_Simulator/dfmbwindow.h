#ifndef DFMBWINDOW_H
#define DFMBWINDOW_H
#include "op.h"
#include "table.h"

#include <QTimer>
#include <QMainWindow>
#include <vector>
#include <queue>
#include <utility>
#include <QPainter>
#include <QDir>
#include <QFile>
#include <set>
#include <QString>
#include <QSound>
using namespace std;

namespace Ui {
class DFMBWindow;
}

class DFMBWindow : public QMainWindow
{
    Q_OBJECT


public:
    DFMBWindow(QString Path, vector<pair<int,int> > _inputPos, pair<int,int> _outputPos,
                        vector<pair<int,int>> _cleanPos, pair<int,int> _wastePos,
                        int _row, int _column, QWidget *parent = nullptr);
    ~DFMBWindow();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *ev);


private slots:
    void on_nextButton_clicked();
    void on_prevButton_clicked();
    void on_autoButton_clicked();
    void on_restartButton_clicked();
    void on_soundButton_clicked();
    void on_warningButton_clicked();
    void on_proButton_clicked();
    void on_shuffleButton_clicked();
    void startAuto();
    void startClean();

private:
    void start_cleaning();
    void end_cleaning();
    void set_clean_widget(bool a);
    void set_barrier(QPoint p);
    vector<bool> blocked;
    vector<int> G;   //存图求解清洗路徑的图的节点编号, 编号为0代表不可达。
    vector<int> path;
    vector<int> clean_path;
    vector<op> oplist; //管理指令集
    queue<op> todolist[200]; //管理第i天执行中的指令集
    vector<table> tablelist;
    vector<set<int>> counter[200]; //第i天的污染集合
    QTimer* autotimer;
    QTimer* cleantimer;
    QSound* move_p;
    QSound* split_p;
    QSound* splitsuccess_p;
    QSound* mergesuccess_p ;
    void playsound(int time);
    void move(int t, int a, int b);
    void pollute(int t, int a);
    void merge(int t, int a, int b, int type);
    void split(int t, int a, int b, int type);
    void mix(int t, vector<int> idx);
    void clean(int time);
    vector<int> target; //要清理的污染序列
    int width, height;
    int x_margin, y_margin;
    Ui::DFMBWindow *ui;
    int row;
    int column;
    vector<pair<int,int> > inputPos;
    vector<pair<int,int> > cleanPos;
    pair<int,int> outputPos;
    pair<int,int> wastePos;
    pair<int,int> index_to_pix(pair<int, int> p);
    int pos_to_index(pair<int,int> pos);
    pair<int,int> index_to_pos(int index);
    void stable_checked(int time);
    void mobile_checked(int a, int time);
    void read_file(QString path);
    void parsing(QString s);
    void solve();
    void solve_table();
    void draw(int time);
    void draw_water_drop();
    void set_background();
    void draw_background();
    void draw_input();
    void draw_output();
    void draw_clean();
    void draw_barrier();
    set<pair<int,int>> wrong_signal;
    void play(int time);
    void BFS_search(int start, int end);
    bool auto_play = false;
    bool sound_on = true;
    bool clean_on = false;
    bool is_valid_move = true;
    int cur = 0;
    int max_time = 0;
    void Warning(QString title, QString msg);
    int cnt = 0;
    int water_drop_pos = -1;
    int Cpos;
    int Wpos;
    void initial_pos();
    bool auto_and_clean = false;
    int auto_time = 500;
    int cleantime = 30;
};

#endif // DFMBWINDOW_H
