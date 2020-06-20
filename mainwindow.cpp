#include <QtGui>
#include "mainwindow.h"

#include "3dmaths/f3dmath.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->resize(1360, 720);
    this->update();

    fpsTimer.start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::OnRedraw()
{

}


void MainWindow::paintEvent(QPaintEvent *event)
{
    static unsigned int frameCount = 0;
    static unsigned int currentFps = 0;

    vt.Render();

    QPainter p(this);

    p.drawImage(this->rect(), vt.frameBufferImage);

    frameCount++;

    unsigned int elapsed = fpsTimer.elapsed();

    if(elapsed > 1000)
    {
        currentFps = qRound((double)frameCount / ((double)elapsed / 1000.0));
        frameCount = 0;
        fpsTimer.restart();
    }

    p.drawText(32,32, QString("FPS: %1").arg(currentFps));

    this->update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        vt.cameraAngle += 0.05f;
    }
    else if(event->key() == Qt::Key_Right)
    {
        vt.cameraAngle -= 0.05f;
    }
    else if(event->key() == Qt::Key_Up)
    {
        F3D::V3<fp> d((float)-(qSin(vt.cameraAngle) *10), 0, (float)-(qCos(vt.cameraAngle) *10));

        vt.cameraPos += d;
    }
    else if(event->key() == Qt::Key_Down)
    {
        F3D::V3<fp> d((float)-(qSin(vt.cameraAngle) *10), 0, (float)-(qCos(vt.cameraAngle) *10));

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

}











