#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>
#include "dfmbwindow.h"
#include "setwindow.h"

namespace Ui {
class StartWindow;
}

class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

private slots:
    void on_startButton_clicked(); //自动关联
    void on_quitButton_clicked();
    void on_actionStart_triggered();
    void on_actionQuit_triggered();

private:
    Ui::StartWindow *ui;
    DFMBWindow* dfmb_w ;
    SetWindow* set_w;
};

#endif // STARTWINDOW_H
