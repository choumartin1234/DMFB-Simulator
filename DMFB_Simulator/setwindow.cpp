#include "setwindow.h"
#include "ui_setwindow.h"
#include <QString>
#include <QMessageBox>
#include <QDialog>
#include <QDebug>
#include <set>
#include <QStringBuilder>



SetWindow::SetWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetWindow)
{
    ui->setupUi(this);
    setFixedSize(800, 600);
    setWindowTitle("DMFB Simulator -- Set Parameters");
}
void SetWindow::on_cleanBox_stateChanged()
{
    if(ui->cleanBox->isChecked())
    {
        ui->cleanLine->clear();
        ui->cleanLine->setEnabled(true);
        ui->wasteLine->clear();
        ui->wasteLine->setEnabled(true);
        ui->defaultBox->setEnabled(true);
    }else
    {
        ui->cleanLine->clear();
        ui->cleanLine->setEnabled(false);
        ui->wasteLine->clear();
        ui->wasteLine->setEnabled(false);
        ui->defaultBox->setChecked(false);
        ui->defaultBox->setEnabled(false);
    }
}

void SetWindow::on_defaultBox_stateChanged()
{
    if(ui->defaultBox->isChecked())
    {
        ui->cleanLine->clear();
        ui->cleanLine->setEnabled(false);
        ui->wasteLine->clear();
        ui->wasteLine->setEnabled(false);
    }else if (ui->cleanBox->isChecked())
    {
        ui->cleanLine->clear();
        ui->cleanLine->setEnabled(true);
        ui->wasteLine->clear();
        ui->wasteLine->setEnabled(true);
    }
}

void SetWindow::Warning(QString title, QString msg){
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Warning, title, msg, nullptr, this);
    msgBox->show();
}

bool SetWindow::checked_on_boarder(pair<int, int> p){
    return (((p.first == 0 || p.first == column+1) && p.second > 0 && p.second <= row) || //电极端口在左右
            ((p.second == 0 || p.second == row+1) && p.first > 0 && p.first <= column));  //电极端口在上下
}



void SetWindow::on_pushButton_clicked()
{
   set<pair<int,int> > cnt; //记录该位置是否已有电极
   row = ui->rspinBox->text().toInt();
   column = ui->cspinBox->text().toInt();
   if (row == 0 || column == 0 )
   {
        Warning("输入错误", "长、宽不得为0"); return;
   }
   if (row <= 3 && column <= 3)
   {
       {
           Warning( "输入错误", "长、宽不得同时小于3");  return;
       }
   }
   QStringList inputList = ui->inputLine->text().split(',');
   //for(int i = 0; i < inputList.length(); i++) qDebug() << inputList[i] << "\n";
   if (inputList.length() % 2 != 0)
   {
       Warning("输入端口错误","输入端口坐标不成对" ); return;
   }
   for (int i = 0; i < inputList.length()/2 ; i++ )
   {
       pair<int,int> tmp = make_pair(inputList[2*i].toInt(), inputList[2*i+1].toInt());
       if(!checked_on_boarder(tmp))
       {
           Warning("输入端口错误", "第" % QString::number(i+1) + "个坐标(" + inputList[2*i] +"," + inputList[2*i+1]+ ")并不在边界上！");
           return;
       }
       if(cnt.count(tmp) != 0)
       {
           Warning("输入端口错误","坐标(" + inputList[2*i] +"," + inputList[2*i+1]+ ")已被占用!" );
           return;
       }
       inputPos.push_back(tmp);
       cnt.insert(tmp);
   }
   QStringList outputList = ui->outputLine->text().split(',');
   if(outputList.length()!= 2)
   {
       Warning("输出端口错误","参数个数(" % QString::number(outputList.length())+")不正确, 请输入两个整数" ); return;
   }
   pair<int,int> tmp = make_pair(outputList[0].toInt(), outputList[1].toInt());
   if(!checked_on_boarder(tmp))
   {
       Warning("输出端口错误", "坐标(" + outputList[0] +"," + outputList[1]+ ")并不在边界上！");
       return;
   }
   if(cnt.count(tmp) != 0)
   {
       Warning("输出端口错误","坐标(" + outputList[0] +"," + outputList[1]+ ")已被占用!" );
       return;
   }
   outputPos = tmp;
   cnt.insert(tmp);

   if(ui->cleanBox->isChecked())
   {
       if (ui->defaultBox->isChecked())
       {
           if(cnt.count(pair<int,int>(0,1)) != 0)
           {
               Warning("预设位置错误","左下角(预设的清洁端口位置)已被占用"); return;
           }
           if(cnt.count(pair<int,int>(column+1, row)) != 0)
           {
               Warning("预设位置错误", "右上角(预设的废液端口位置)已被占用"); return;
           }
           cleanPos.push_back(make_pair(0,1));
           wastePos = make_pair(column+1, row);
       }else
       {
           QStringList cleanList = ui->cleanLine->text().split(',');
           //for(int i = 0; i < inputList.length(); i++) qDebug() << inputList[i] << "\n";
           if (cleanList.length() % 2 != 0)
           {
               Warning("清洁端口错误","清洁端口坐标不成对" ); return;
           }
           for (int i = 0; i < cleanList.length()/2 ; i++ )
           {
               pair<int,int> tmp = make_pair(cleanList[2*i].toInt(), cleanList[2*i+1].toInt());
               if(!checked_on_boarder(tmp))
               {
                   Warning("清洁端口错误", "第" % QString::number(i+1) + "个坐标(" + cleanList[2*i] +"," + cleanList[2*i+1]+ ")并不在边界上！");
                   return;
               }
               if(cnt.count(tmp) != 0)
               {
                   Warning("清洁端口错误","坐标(" + cleanList[2*i] +"," + cleanList[2*i+1]+ ")已被占用!" );
                   return;
               }
               cleanPos.push_back(tmp);
               cnt.insert(tmp);
           }
           QStringList wasteList = ui->wasteLine->text().split(',');
           if(wasteList.length()!= 2)
           {
               Warning("废液端口错误","参数个数(" % QString::number(wasteList.length())+")不正确, 请输入两个整数" ); return;
           }
           pair<int,int> tmp = make_pair(wasteList[0].toInt(), wasteList[1].toInt());
           if(!checked_on_boarder(tmp))
           {
               Warning("废液端口错误", "坐标(" + wasteList[0] +"," + wasteList[1]+ ")并不在边界上！");
               return;
           }
           if(cnt.count(tmp) != 0)
           {
               Warning("废液端口错误","坐标(" + wasteList[0] + "," + wasteList[1]+ ")已被占用!" );
               return;
           }
           wastePos = tmp;
           cnt.insert(tmp);
       }
   }
   else
   {
       cleanPos.clear();
       wastePos = make_pair(-1,-1);
   }
   QString PATH = ui->PATHLine->text();
   dfmb_w = new DFMBWindow(PATH, inputPos, outputPos, cleanPos, wastePos, row, column);
   dfmb_w -> show();
   this -> hide();
}

void SetWindow::on_case0_clicked()
{
    ui->rspinBox->setValue(6);
    ui->cspinBox->setValue(2);
    ui->inputLine->setText("2,7");
    ui->outputLine->setText("2,0");
    ui->PATHLine->setText(":/Input/testcase0.txt");
    ui->defaultBox->setEnabled(false);
    ui->cleanLine->setEnabled(false);
    ui->wasteLine->setEnabled(false);
}
void SetWindow::on_case1_clicked()
{
    ui->rspinBox->setValue(6);
    ui->cspinBox->setValue(4);
    ui->inputLine->setText("1,7,4,7");
    ui->outputLine->setText("5,5");
    ui->PATHLine->setText(":/Input/testcase1.txt");
    ui->defaultBox->setEnabled(false);
    ui->cleanLine->setEnabled(false);
    ui->wasteLine->setEnabled(false);

}
void SetWindow::on_case2_clicked()
{
    ui->rspinBox->setValue(8);
    ui->cspinBox->setValue(6);
    ui->inputLine->setText("0,2,0,4,4,9");
    ui->outputLine->setText("7,4");
    ui->PATHLine->setText(":/Input/testcase2.txt");
    ui->defaultBox->setEnabled(false);
    ui->cleanLine->setEnabled(false);
    ui->wasteLine->setEnabled(false);

}
void SetWindow::on_case3_clicked()
{
    ui->rspinBox->setValue(8);
    ui->cspinBox->setValue(8);
    ui->inputLine->setText("0,8,4,9");
    ui->outputLine->setText("9,1");
    ui->PATHLine->setText(":/Input/testcase3.txt");
    ui->defaultBox->setEnabled(false);
    ui->cleanLine->setEnabled(false);
    ui->wasteLine->setEnabled(false);
}
void SetWindow::on_error_clicked()
{
    ui->rspinBox->setValue(8);
    ui->cspinBox->setValue(8);
    ui->inputLine->setText("0,4,4,9");
    ui->outputLine->setText("9,3");
    ui->PATHLine->setText(":/Input/testcaseerror.txt");
    ui->defaultBox->setEnabled(false);
    ui->cleanLine->setEnabled(false);
    ui->wasteLine->setEnabled(false);
}
void SetWindow::on_wash_clicked()
{
    ui->rspinBox->setValue(8);
    ui->cspinBox->setValue(8);
    ui->inputLine->setText("0,5,4,9");
    ui->outputLine->setText("9,3");
    ui->PATHLine->setText(":/Input/testcasewash.txt");
    ui->cleanBox->setChecked(true);
    ui->defaultBox->setChecked(true);
}
SetWindow::~SetWindow()
{
    delete ui;
}
