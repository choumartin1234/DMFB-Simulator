#include "startwindow.h"
#include "ui_startwindow.h"

StartWindow::StartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow)
{
    ui->setupUi(this);
    ui->textEdit->setStyleSheet("QTextEdit { background: black}");
    setFixedSize(800, 600);
    setWindowTitle("DMFB Simulator -- Menu");
}

void StartWindow::on_startButton_clicked()
{
    this -> hide();
    set_w = new SetWindow();
    set_w -> show(); //跳入下个视窗
}

void StartWindow::on_quitButton_clicked()
{
    this->close();
}

void StartWindow::on_actionStart_triggered()
{
    on_startButton_clicked();
}
void StartWindow::on_actionQuit_triggered()
{
    on_quitButton_clicked();
}

StartWindow::~StartWindow()
{
    delete ui;
}
