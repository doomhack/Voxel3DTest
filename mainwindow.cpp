#include <QtGui>
#include "mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->resize(1360, 720);
    this->update();
}

MainWindow::~MainWindow()
{
}

void MainWindow::OnRedraw()
{

}


void MainWindow::paintEvent(QPaintEvent *event)
{
    vt.Render();

    QPainter p(this);

    p.drawImage(this->rect(), vt.frameBufferImage);

    this->update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        vt.cameraAngle += 0.05;
    }
    else if(event->key() == Qt::Key_Right)
    {
        vt.cameraAngle -= 0.05;
    }
    else if(event->key() == Qt::Key_Up)
    {
        QPointF d(-(qSin(vt.cameraAngle) *10), -(qCos(vt.cameraAngle) *10));

        vt.cameraPos += d;
    }
    else if(event->key() == Qt::Key_Down)
    {
        QPointF d(-(qSin(vt.cameraAngle) *10), -(qCos(vt.cameraAngle) *10));

        vt.cameraPos -= d;
    }
    else if(event->key() == Qt::Key_Z)
    {
        if(vt.zAngle < 40)
            vt.zAngle+=1;
    }
    else if(event->key() == Qt::Key_X)
    {
        if(vt.zAngle > -40)
            vt.zAngle-=1;
    }
    else if(event->key() == Qt::Key_Q)
    {
        vt.cameraHeight++;
    }
    else if(event->key() == Qt::Key_W)
    {
        vt.cameraHeight--;
    }

    else if(event->key() == Qt::Key_Comma)
    {
        vt.heightScale-=10;
    }
    else if(event->key() == Qt::Key_Period)
    {
        vt.heightScale+=10;
    }
    else if(event->key() == Qt::Key_V)
    {
        vt.render3d = !vt.render3d;
    }

}











