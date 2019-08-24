#include "dfmbwindow.h"
#include "ui_dfmbwindow.h"
#include <math.h>
#include <QDebug>
#include <QMessageBox>
#include <functional>
#include <QTimer>
#include <QSound>
#include <windows.h>
#include <QStringBuilder>
#include <stack>
#include <QMouseEvent>

using namespace std;

DFMBWindow::DFMBWindow(QString PATH, vector<pair<int,int> > _inputPos, pair<int,int> _outputPos,
                       vector<pair<int,int> > _cleanPos, pair<int,int> _wastePos,
                       int _row, int _column,  QWidget* parent):
   QMainWindow(parent),ui(new Ui::DFMBWindow), row(_row), column(_column),inputPos(_inputPos), cleanPos(_cleanPos), outputPos(_outputPos), wastePos(_wastePos)
{
    blocked.resize(row*column+1);
    ui->setupUi(this);
    autotimer = new QTimer(this);
    cleantimer = new QTimer(this);
    if(cleanPos.size() == 0)
        ui->cleanButton->setVisible(false);
    initial_pos();
    set_clean_widget(clean_on);
    connect(autotimer, SIGNAL(timeout()), this, SLOT(startAuto()));
    connect(cleantimer, SIGNAL(timeout()), this, SLOT(startClean()));
    move_p = new QSound(":/Sound/move.wav");
    split_p = new QSound(":/Sound/split.wav");
    splitsuccess_p = new QSound(":/Sound/splitsuccess.wav");
    mergesuccess_p = new QSound(":/Sound/mergesuccess.wav");
    srand (time(nullptr));
    for (int i = 0; i < 200; i++)
        counter[i].resize(row*column+1);
    set_background();
    read_file(PATH); //读入文件并剖析指令
    solve(); //解出每个时刻需进行的指令
    solve_table(); //依照每个时刻正在进行的指令, 解出每时刻分布图
}
void DFMBWindow::initial_pos()
{
    if(cleanPos.size())
    {
    if(cleanPos[0].first == 0)
        Cpos = pos_to_index(cleanPos[0])+1;
    else if(cleanPos[0].first == column+1)
        Cpos = pos_to_index(cleanPos[0])-1;
    else if (cleanPos[0].second == 0)
        Cpos = pos_to_index(cleanPos[0])+column;
    else
        Cpos = pos_to_index(cleanPos[0])-column;
    if(wastePos.first == 0)
        Wpos = pos_to_index(wastePos)+1;
    else if(wastePos.first == column+1)
        Wpos = pos_to_index(wastePos)-1;
    else if (wastePos.second == 0)
         Wpos = pos_to_index(wastePos)+column;
    else
         Wpos = pos_to_index(wastePos)-column;
    }
}
void DFMBWindow::set_clean_widget(bool a)
{
    ui->cleanlabel->setVisible(a);
    QWidget* movelist[5] = {ui->nextBox, ui->prevBox, ui->restartBox, ui->autoButton, ui->advanceBox};
    for (int i = 0; i < 5; i++)
        movelist[i]->setVisible(!a);
    if (a)
        ui->cleanButton->setStyleSheet(tr("border-image: url(:/pic/clean.png);"));
    if (!a)
        ui->cleanButton->setStyleSheet(tr("border-image: url(:/pic/washoff.png);"));
}
void DFMBWindow::Warning(QString title, QString msg){
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Warning,title,msg, nullptr, this);
    msgBox->show();
}

int DFMBWindow::pos_to_index(pair<int,int> pos)
{
    return (pos.second-1)*column + pos.first;
}


pair<int, int> DFMBWindow::index_to_pos(int index)
{
    int x = index % column;
    if (x == 0) x += column;
    int y = (index-1)/column + 1 ;
    return make_pair(x,y);
}

//change time t's index a to pollutant
void DFMBWindow::pollute(int t, int a)
{
    tablelist[t].droplist[a].type = -1;
    for (int i = t; i <= max_time; i++)
        counter[i][a].insert(tablelist[t].droplist[a].index);
}
//第t秒的指令
void DFMBWindow::merge(int t, int a, int b, int _type)
{
    auto temp1 = tablelist[t-1].droplist[a];
    auto temp2 = tablelist[t-1].droplist[b];
    tablelist[t+1].droplist[(a+b)/2] = tablelist[t].droplist[(a+b)/2] = drop(cnt++,(temp1.r+temp2.r)/2, (temp1.g+temp2.g)/2, (temp1.b+temp2.g/2),_type, sqrt(temp1.size+temp2.size));
    pollute(t,a); pollute(t,b);
    tablelist[t+1].droplist[a] = tablelist[t].droplist[a];
    tablelist[t+1].droplist[b] = tablelist[t].droplist[b];
    tablelist[t].droplist[a].type = tablelist[t].droplist[b].type = 4;
    tablelist[t].droplist[a].index = tablelist[t].droplist[b].index = tablelist[t].droplist[(a+b)/2].index;
    tablelist[t+1].droplist[(a+b)/2].type = 1;
    //merge sound
    tablelist[t+1].droplist[(a+b)/2].set_stable();
    tablelist[t].droplist[a].is_merge = tablelist[t].droplist[b].is_merge = true;
    tablelist[t+1].droplist[(a+b)/2].is_merge_success = true;


}
void DFMBWindow::split(int t, int a, int b, int _type)
{
    auto temp = tablelist[t].droplist[(a+b)/2] = tablelist[t-1].droplist[(a+b)/2];
    tablelist[t].droplist[(a+b)/2].type = _type;
    tablelist[t].droplist[a].type = tablelist[t].droplist[b].type = 4;
    tablelist[t+1].droplist[a] = drop(cnt++,rand()%256,rand()%256,rand()%256, 1, temp.size*temp.size/2);
    tablelist[t+1].droplist[b] =  drop(cnt++,rand()%256,rand()%256,rand()%256, 1, temp.size*temp.size/2);
    tablelist[t+1].droplist[(a+b)/2] = tablelist[t].droplist[(a+b)/2];
    tablelist[t].droplist[a].index = tablelist[t].droplist[b].index = tablelist[t].droplist[(a+b)/2].index;
    pollute(t+1,(a+b)/2);
    for (int i = t; i <= max_time; i++)
    {
        auto it = counter[i][a].find(tablelist[t].droplist[a].index);
                if (it != counter[i][a].end())
                    counter[i][a].erase(it);
        it = counter[i][b].find(tablelist[t].droplist[b].index);
                if (it != counter[i][b].end())
                    counter[i][b].erase(it);
    }
    //sound
    tablelist[t].droplist[a].set_stable(); tablelist[t].droplist[b].set_stable();
    tablelist[t].droplist[(a+b)/2].set_stable();
    tablelist[t].droplist[(a+b)/2].is_split = true;
    tablelist[t].droplist[a].is_split = true;
    tablelist[t].droplist[b].is_split = true;
    tablelist[t+1].droplist[a].set_stable(); tablelist[t+1].droplist[b].set_stable();
    tablelist[t+1].droplist[(a+b)/2].set_stable();
    tablelist[t+1].droplist[a].is_split_success = tablelist[t+1].droplist[b].is_split_success = true;
}
void DFMBWindow::move(int t, int a, int b)
{
    tablelist[t].droplist[b] = tablelist[t].droplist[a] = tablelist[t-1].droplist[a];
    pollute(t, a);
    //sound
    tablelist[t].droplist[b].set_stable();
    tablelist[t].droplist[b].is_move = true;
}
void DFMBWindow::mix(int t, vector<int> idx)
{
    for(int i = 0; i < idx.size()-1; i++)
        move(t+i, idx[i], idx[i+1]);
}

void DFMBWindow::solve_table()
{
    for (int i = 1; i <= max_time+1; i++)
    {
        for (int j = 1; j <= row * column; j++)
            if (tablelist[i].droplist[j].is_blank() && !tablelist[i-1].droplist[j].is_blank())
            {
                tablelist[i].droplist[j] = tablelist[i-1].droplist[j];
                tablelist[i].droplist[j].set_stable();
            }
        while(!todolist[i-1].empty())
        {
           // qDebug("Loop:\n");
            op todo = todolist[i-1].front();
            if (todo.name == "Move")
            {
                int prev_idx = pos_to_index(todo.points[0]);
                int next_idx = pos_to_index(todo.points[1]);
                move(i, prev_idx, next_idx);
               // qDebug("Move success\n");
            }else if (todo.name == "Input")
            {
                auto pos = todo.points[0];
                bool mark = false;
                for (auto it = inputPos.begin(); it != inputPos.end(); it++)
                    if (abs((it->first - pos.first)+(it->second - pos.second)) == 1)
                    {
                        mark = true;
                        break;
                    }
                if (mark)
                {
                    tablelist[i].droplist[pos_to_index(pos)] = drop(cnt, rand()%256, rand()%256,rand()%256,1,1);
                    cnt++;
                   // qDebug("Input success\n");
                }else
                    Warning("Input指令错误", "坐标(" % QString::number(pos.first) + "," % QString::number(pos.second)+ ")并不在输入端口旁边上！");
            }else if (todo.name == "Output")
            {
                auto pos = todo.points[0];
                if (abs((outputPos.first - pos.first) + (outputPos.second - pos.second)) == 1)
                {
                    pollute(i, pos_to_index(pos));
                    // qDebug("Output success\n");
                }
                else
                    Warning("Output指令错误", "坐标(" % QString::number(pos.first) + "," % QString::number(pos.second)+ ")并不在输出端口旁边上！");
            }else if (todo.name == "Merge")
            {
                int type;
                if(todo.points[0].first != todo.points[1].first)
                    type = 2;
                else
                    type = 3;
                int idx1 = pos_to_index(todo.points[0]); int idx2 = pos_to_index(todo.points[1]);
                merge(i, idx1, idx2, type);
               // qDebug("Merge success\n");
            }else if (todo.name == "Split")
            {
                int type;
                if(todo.points[2].first != todo.points[1].first)
                    type = 2;
                else
                    type = 3;
                int idx1 = pos_to_index(todo.points[1]); int idx2 = pos_to_index(todo.points[2]);
                split(i, idx1, idx2, type);
               //  qDebug("split success\n");

            }else if (todo.name == "Mix")
            {
                vector<int> idx;
                for(auto it = todo.points.begin(); it != todo.points.end(); it++)
                    idx.push_back(pos_to_index(*it));
                mix(i, idx);
               // qDebug("mix success\n");
            }
            todolist[i-1].pop();
        }
    }
}

void DFMBWindow::parsing(QString s)
{
    s.chop(1);
    QStringList a = s.split(" ");
    QStringList b = a[1].split(",");
    QString name = a[0];
    int t = b[0].toInt();
    vector<pair<int,int>> points;
    for (int i = 0; i < (b.length()-1)/2 ;  i++)
    {
        pair<int,int> tmp = make_pair(b[2*i+1].toInt(), b[2*i+2].toInt());
        points.push_back(tmp);
    }
    int time_cost;
    if (name == "Input" || name == "Output" || name == "Move")
        time_cost = 1;
    else if (name == "Merge" || name == "Split")
        time_cost = 2;
    else {
        time_cost = points.size()-1;
    }
    op tmp(name, t, time_cost, points);
    oplist.push_back(tmp);
}

void DFMBWindow::read_file(QString path)
{
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr,"error",f.errorString());
    }
    QTextStream in(&f);
    QString s;
    while(in.readLineInto(&s))
        parsing(s);
}
void DFMBWindow::solve()
{
    int cnt = 0; int n = oplist.size();
    for(int i = 0 ; i <= 200; i++)
    {
        for (auto it = oplist.begin(); it != oplist.end(); it++)
        {
            if (it->time == i)
            {
                todolist[i].push(*it);
                cnt++;
            }
        }
        if (cnt == n)
        {
            max_time = i+1;
            break;
        }
   }
   tablelist.clear();
   for (int i = 0 ; i <= max_time+10; i++)
       tablelist.push_back(table(row,column));
}

void DFMBWindow::set_background()
{
    setFixedSize(1720, 960);
    int Width = 800; int Height = 800;
    setWindowTitle("DMFB Simulator");
    width = Width / (column+2);
    height = Height / (row+2);
    x_margin = y_margin = (960 - (row+2)*height)/2;
}

pair<int,int> DFMBWindow::index_to_pix(pair<int, int> p)
{
    //回传坐标(x,y)方格的左上角
    int x = x_margin + p.first * width;
    int y = y_margin + (row+1 - p.second)*height;
    return make_pair(x,y);
}


//静态约束
void DFMBWindow::stable_checked(int time){
    for (int i = 1; i <= row*column; i++)
        for(int j = i+1; j <= row*column; j++)
            if (tablelist[time].droplist[i].is_drop() && tablelist[time].droplist[j].is_drop() && tablelist[time].droplist[i].index != tablelist[time].droplist[j].index)
            {
                auto p1 = index_to_pos(i);
                auto p2 = index_to_pos(j);
                if(sqrt((p1.first-p2.first)*(p1.first-p2.first)+ (p1.second-p2.second)*(p1.second-p2.second))< 2 ){

                    Warning("违反静态约束", "坐标(" % QString::number(p1.first) + "," % QString::number(p1.second) +")与坐标("
                                                 % QString::number(p2.first) + "," % QString::number(p2.second)
                                                 % ")的滴液距离小于2, 有可能会意外污染!");
                    auto_play = is_valid_move = false;
                }
            }
}
void DFMBWindow::mobile_checked(int a, int time){
    if (time >= 1)
        if (tablelist[time].droplist[a] != tablelist[time-1].droplist[a])
            for (int i = 1; i <= row*column ; i++)
                if (abs(a-i) != 1 && abs(a-i) != column) //两者非相邻水滴
                {
                    if ((tablelist[time-1].droplist[i] != tablelist[time].droplist[a])  &&  tablelist[time-1].droplist[i].type > 0)
                    {
                        auto p1 = index_to_pos(a);
                        auto p2 = index_to_pos(i);
                        if(sqrt((p1.first-p2.first)*(p1.first-p2.first)+ (p1.second-p2.second)*(p1.second-p2.second)) < 2 ){
                            wrong_signal.insert(make_pair(a,i));
                             auto_play = is_valid_move =false;

                        }
                    }
                }
}
void DFMBWindow::playsound(int time)
{
    if(sound_on)
    {
        for(int i = 1; i <= row * column; i++)
        {
            drop a = tablelist[time].droplist[i];
            if (a.is_drop())
            {
                if (a.is_move)
                    move_p -> play();
                else if (a.is_split)
                    split_p -> play();
                else if (a.is_merge_success)
                    mergesuccess_p->play();
                else if (a.is_split_success)
                    splitsuccess_p->play();
            }

        }
    }
}

void DFMBWindow::play(int time)
{
    is_valid_move = true;

    wrong_signal.clear();
    for (int i = 1 ; i <= row * column; i++)
        if (tablelist[time].droplist[i].is_move)
            mobile_checked(i, time);
    if(!wrong_signal.empty())
    {
        for (auto it : wrong_signal)
        {
            auto p1 = index_to_pos(it.first);
            auto p2 = index_to_pos(it.second);
            Warning("违反动态约束", QString::number(time) +"时刻坐标(" % QString::number(p1.first) + "," % QString::number(p1.second) +
                                   ")与"% QString::number(time-1) +"时刻坐标("% QString::number(p2.first) +
                                    "," % QString::number(p2.second)+ ")的滴液距离小于2, 有可能会意外污染!");
        }
    }
    if (!is_valid_move){
        ui->nextButton->setStyleSheet(tr("border-image: url(:/pic/stop.png);"));
        ui->nextButton->setEnabled(false);
        cur--;
        return;
    }

    stable_checked(time);
    if (!is_valid_move){
        ui->nextButton->setStyleSheet(tr("border-image: url(:/pic/stop.png);"));
        ui->nextButton->setEnabled(false);
        cur--;
        return;
    }
    ui->lcdtime->display(time);
    this->repaint();
    playsound(time);
    if(cleanPos.size()!=0)
        clean(time);

}

void DFMBWindow::on_nextButton_clicked()
{
    if(cur <= max_time)
        play(++cur);
}
void DFMBWindow::on_prevButton_clicked()
{
    if(cur > 0)
    {
        ui->nextButton->setStyleSheet(tr("border-image: url(:/pic/next.png);"));
        ui->nextButton->setEnabled(true);
        play(--cur);
    }
}
void DFMBWindow::on_autoButton_clicked()
{
    auto_play = !auto_play;
    if(auto_play)
    {
        ui->autoButton->setStyleSheet(tr("border-image: url(:/pic/pause.png);"));
        ui->nextBox->setVisible(false); ui->prevBox->setVisible(false); ui->restartBox->setVisible(false);
    }else
    {
        ui->autoButton->setStyleSheet(tr("border-image: url(:/pic/auto.png);"));
        ui->nextBox->setVisible(true); ui->prevBox->setVisible(true); ui->restartBox->setVisible(true);
    }
    autotimer->start(auto_time);
}

void DFMBWindow::startAuto()
{
    if(cur <= max_time && auto_play)
        play(++cur);
    else
    {
        autotimer->stop();
        if(cur > max_time )
        {
            auto_play = false;
            ui->autoButton->setStyleSheet(tr("border-image: url(:/pic/auto.png);"));
            ui->nextBox->setVisible(true); ui->prevBox->setVisible(true); ui->restartBox->setVisible(true);
        }else if (!auto_play && ui->autoButton->isVisible())
        {
            ui->autoButton->setStyleSheet(tr("border-image: url(:/pic/auto.png);"));
            ui->nextBox->setVisible(true); ui->prevBox->setVisible(true); ui->restartBox->setVisible(true);
        }
    }

}

void DFMBWindow::on_restartButton_clicked()
{
    ui->nextButton->setStyleSheet(tr("border-image: url(:/pic/next.png);"));
    ui->nextButton->setEnabled(true);
    ui->prevButton->setStyleSheet(tr("border-image: url(:/pic/prev.png);"));
    ui->prevButton->setEnabled(true);
    if(cleanPos.size())
    {
        solve();
        solve_table();
    }
    play(cur=0);
}
void DFMBWindow::on_soundButton_clicked()
{
    if (sound_on)
    {
        sound_on = false;
        ui->soundButton->setStyleSheet(tr("border-image: url(:/pic/soundoff.png);"));
    }else
    {
        sound_on = true;
        ui->soundButton->setStyleSheet(tr("border-image: url(:/pic/soundon.png);"));
    }
}
void DFMBWindow::on_warningButton_clicked()
{
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Information,
                                          "功能待解锁",
                                          "附加功能1：此功能可以限制清洗滴液的容量\n请付费购买完整版以解锁此功能 (误)", nullptr, this);
    msgBox->show();

}
void DFMBWindow::on_proButton_clicked()
{
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Information,
                                          "功能待解锁",
                                          "附加功能2：此功能可以优化清洗时间与滴液数量\n请付费购买完整版以解锁此功能 (误)", nullptr, this);
    msgBox->show();

}
void DFMBWindow::on_shuffleButton_clicked()
{
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Information,
                                          "功能待解锁",
                                          "附加功能3：此功能可以重新规划最优解\n请付费购买完整版以解锁此功能 (误)", nullptr, this);
    msgBox->show();

}


void DFMBWindow::start_cleaning()
{
    clean_on = true;
    set_clean_widget(clean_on);
}
void DFMBWindow::end_cleaning()
{
   //判断下秒是否有无法清洗到会造成滴液污染
    if (cur != max_time)
        for (int i = 1; i <= row*column; i++)
            if(tablelist[cur+1].droplist[i].is_drop() && //下一时刻是水滴
                   (tablelist[cur+1].droplist[i].is_move || tablelist[cur+1].droplist[i].is_split) && //准备要动了(move或是split)
                         counter[cur+1][i].size() > 0 && //下一时刻该位置有污染
                            counter[cur+1][i].find(tablelist[cur+1].droplist[i].index) ==  counter[cur+1][i].end()) //污染不同种
                Warning("污染预警", "位于(" % QString::number(index_to_pos(i).first) % ","
                                    % QString::number(index_to_pos(i).second) %")的污染无法去除,下一时刻会造成滴液污染!");
    clean_on = false;
    set_clean_widget(clean_on);
}

//绘图
void DFMBWindow::draw_background()
{
    QPainter p(this);
    for (int i = 1; i <= column; i++ )
        for (int j = 1; j <= row; j ++)
            p.drawRect(x_margin+i*width, y_margin+j*height, width, height);
}
void DFMBWindow::draw_input()
{
    QPainter inputp(this);
    inputp.setBrush(QColor("pink"));
    //qDebug() << "Input Painter created\n" ;
    for (auto it = inputPos.begin(); it != inputPos.end(); it ++)
    {
        pair<int, int> pos = index_to_pix(*it);
        inputp.drawRect(pos.first, pos.second, width, height);
        inputp.drawText(pos.first, pos.second, width, height, Qt::AlignCenter, "Input");
    }
    //qDebug() << "Input Painter successfully drawing\n" ;
}
void DFMBWindow::draw_output()
{
    QPainter outputp(this);
    //qDebug() << "output Painter created\n" ;
    pair<int, int> pos = index_to_pix(outputPos);
    outputp.setBrush(QColor("lightgreen"));
    outputp.drawRect(pos.first, pos.second, width, height);
    outputp.drawText(pos.first,pos.second,width,height, Qt::AlignCenter, "Output");
    //qDebug() << "output Painter successfully drawing\n" ;
}
void DFMBWindow::draw_clean()
{
    if(cleanPos.size() != 0)
    {
        QPainter cleanp(this);
        cleanp.setBrush(QColor("lightblue"));
        for (auto it = cleanPos.begin(); it != cleanPos.end(); it ++)
        {
            pair<int, int> pos = index_to_pix(*it);
            cleanp.drawRect(pos.first, pos.second, width, height);
            cleanp.drawText(pos.first, pos.second, width, height, Qt::AlignCenter, "Wash");
        }
        QPainter wastep(this);
        wastep.setBrush(QColor("orange"));
        pair<int, int> pos = index_to_pix(wastePos);
        wastep.drawRect(pos.first, pos.second, width, height);
        cleanp.drawText(pos.first, pos.second,width, height, Qt::AlignCenter, "Waste");
    }

}
void DFMBWindow::draw(int time)
{
    QPainter tablep(this);
    int unit = min(width, height)/2;
    if (time <= max_time)
    {
        for (int i = 1; i <= row * column; i++)
        {
            auto temp = tablelist[time].droplist[i];
            if (temp.is_blank())
                continue;
            int radi = unit * temp.size / 2 ;
            pair<int, int> pos = index_to_pix(index_to_pos(i));
            pair<int, int> center = make_pair(pos.first + width/2, pos.second + height/2);
            tablep.setBrush(QColor(temp.r, temp.g, temp.b));
            switch(tablelist[time].droplist[i].type){
                case 1 :
                    tablep.drawEllipse(center.first-radi, center.second - radi, 2*radi, 2*radi);
                    break;
                case 2 :
                    tablep.drawEllipse(center.first- 4 * radi, center.second - 0.8*radi , 8 * radi, 1.6*radi);
                    break;
                case 3 :
                    tablep.drawEllipse(center.first - 0.8 * radi, center.second - 4*radi, 1.6*radi, 8*radi);
                    break;
                case -1 : //画污染
                    if (counter[time][i].size()!=0)
                    {
                    tablep.setOpacity(0.3);
                    tablep.drawEllipse(center.first-radi*0.6, center.second-radi*0.6, 1.2*radi, 1.2*radi);
                    tablep.drawText(center.first-radi*0.6, center.second-radi*0.6, 1.2*radi, 1.2*radi, Qt::AlignCenter, QString::number(temp.index));
                    tablep.setOpacity(1);
                    }
                    break;
            }
        }
    }else //输出污染次数。
    {
         tablep.setFont(QFont("times",18));
         for (int i = 1; i <= row * column; i++)
         {
             pair<int, int> pos = index_to_pix(index_to_pos(i));
             tablep.drawText(pos.first,pos.second,width,height,Qt::AlignCenter, QString::number(counter[max_time][i].size()));
         }
    }
}
void DFMBWindow::draw_barrier()
{
    QPainter barrierp(this);
    barrierp.setBrush(QColor("lightyellow"));
    for (int i = 1; i <= row * column; i++)
    {
        if(blocked[i])
        {
            pair<int, int> pos = index_to_pix(index_to_pos(i));
            barrierp.drawRect(pos.first, pos.second, width, height);
        }
    }
}
void DFMBWindow::set_barrier(QPoint p)
{
    int x = p.x(); int y = p.y();
    auto min = index_to_pix(index_to_pos(column*(row-1)+1));
    auto max = index_to_pix(index_to_pos(column));
    if( x  > min.first && y > min.second && x < (max.first+width) && y < (max.second+height))
    {
            int px = (x - x_margin)/width ;
            int py = row - ((y - y_margin)/height) + 1;
            blocked[pos_to_index(make_pair(px, py))] = ! blocked[pos_to_index(make_pair(px, py))];
    }
    this->update();


}
void DFMBWindow::mousePressEvent(QMouseEvent *ev)
{
    if (!clean_on && !auto_play && cleanPos.size())
        set_barrier(ev->pos());
}


void DFMBWindow::paintEvent(QPaintEvent *)
{
    draw_background();
    draw_barrier();
    draw_input(); draw_output(); draw_clean();
    draw(cur);
    draw_water_drop();
}
void DFMBWindow::draw_water_drop()
{
    if (water_drop_pos > 0)
    {
        QPainter waterp(this);
        int radi = min(width,height)/2;
        pair<int, int> pos = index_to_pix(index_to_pos(water_drop_pos));
        pair<int, int> center = make_pair(pos.first + width/2, pos.second + height/2);
        waterp.setBrush(QColor("Blue"));
        waterp.drawEllipse(center.first-radi, center.second - radi, 2*radi, 2*radi);
    }
}


DFMBWindow::~DFMBWindow()
{
    delete ui;
}

//用BFS求解清洁路径 算法参考 https://blog.csdn.net/huanhuan_Coder/article/details/79197181
struct node
{
    int x;
    int y;
    int id;
    int parent=0;
    node(int a,int b,int c):x(a),y(b),id(c){}
};

void DFMBWindow::BFS_search(int start, int end)
{
    clean_path.clear();
    vector<int> path(row*column+1);
    vector<bool> visited(row*column+1);
    int vx[4] = {1,0,-1,0};
    int vy[4] = {0,1,0,-1};
    queue<node> q;
    auto pos = index_to_pos(start);
    node v=node(pos.first, pos.second,start);
    q.push(v);
    visited[start] = true;
    while(!q.empty())
    {
         node u=q.front();
         q.pop();
         path[u.id]=u.parent;
         for(int i=0; i<=3; i++)
         {
             int tx=u.x+vx[i];
             int ty=u.y+vy[i];
             if (tx >= 1 && tx <= column && ty >=1 && ty <= row)
             {
                 int idx = pos_to_index(make_pair(tx,ty));
                 if(G[idx]!= 0 &&  !visited[idx])
                 {
                     visited[idx] = true;
                     node v=node(tx,ty,G[idx]);
                     v.parent=u.id;
                     q.push(v);
                 }
             }
         }
    }
     while(end)
     {
         clean_path.push_back(end);
         end = path[end];
     }
\
}
void DFMBWindow::clean(int time)
{
    target.clear();
    G.clear();
    G.resize(column*row+1);
    for (int i = 1; i <= row * column; i ++)
    {
        if (blocked[i])
            G[i] = 0;
        else
            G[i] = i;
    }
    for (int i = 1; i <= row * column; i++)
    {//静态约束
        if (tablelist[time].droplist[i].is_drop())
        {
            auto pos = index_to_pos(i);
            for (int dx = -1; dx <=1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                    if (pos.first + dx >=1 && pos.first + dx <= column && pos.second + dy >= 1 && pos.second + dy <= row)
                        G[pos_to_index(make_pair(pos.first+dx, pos.second+dy))] = 0;
        }

    }

    //判断可以清洗的点
    for (int i = 1;  i <= row*column; i++)
    {
        if (counter[cur][i].size()  && G[i] != 0)
            target.push_back(i);
    }
  /*  for (auto it : target)
        qDebug() << it;*/

    start_cleaning();
    int cur_pos = Cpos;
    int waste_pos = Wpos;
    if (G[cur_pos])
        {
        vector<int> CLEANPATH;
        CLEANPATH.push_back(cur_pos);
        if (target.size()!=0)
        {
            int cnt = target.size();
            reverse(target.begin(), target.end());
            while(target.size()!= 0){
                int n = target.size();
                int goal = target[n-1];
               // qDebug() << "cur_pos = " << cur_pos << "\t goal = " << goal;
                if (cur_pos != goal)
                {
                    BFS_search(cur_pos, goal);
                    if(clean_path.size()>1)
                    {
                        for(int i = clean_path.size()-2; i >=0; i--)
                            CLEANPATH.push_back(clean_path[i]);
                        cur_pos = goal;
                    }
                    else
                    {
                        cnt--;
                        clean_path.clear();
                        //qDebug() << "没有可行解! 换下一个污染点";
                    }
                }
                target.pop_back();
            }
            if(cnt)
                BFS_search(cur_pos,waste_pos);
            if (clean_path.size() > 1)
            {
                for(int i = clean_path.size()-2; i >=0; i--)
                    CLEANPATH.push_back(clean_path[i]);
                reverse(CLEANPATH.begin(),CLEANPATH.end());
                clean_path = CLEANPATH;
              /*  QDebug debug = qDebug();
                for (auto it : CLEANPATH)
                    debug << it << " ";*/
                cleantimer->start(cleantime);
                ui->prevButton->setStyleSheet(tr("border-image: url(:/pic/stop.png);"));
                ui->prevButton->setEnabled(false);
                return;
            }
            else
            {
                end_cleaning();
                return;
            }
        }
    }
    end_cleaning();
}
void DFMBWindow::startClean()
{
    if(autotimer->isActive())
    {
        autotimer->stop();
        auto_and_clean = true;
    }
    if (clean_path.size()>0)
    {
        int pos = water_drop_pos = *(clean_path.end()-1);
        tablelist[cur].droplist[pos]  = drop();
        for (int i = cur+1; i <= max_time; i++)
        {
            for (auto item : counter[cur][pos])
            {
                auto it = counter[i][pos].find(item);
                if (it != counter[i][pos].end())
                    counter[i][pos].erase(it);
            }
        }
        counter[cur][pos].clear();
        this->repaint();
        clean_path.pop_back();
    }
    else
    {
        water_drop_pos = -1;
        this->repaint();
        cleantimer->stop();
        end_cleaning();
        if(auto_and_clean)
        {
            autotimer->start(auto_time);
            auto_and_clean = false;
        }
    }
}

