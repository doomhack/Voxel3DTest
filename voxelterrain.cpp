 #include <QtMath>
#include "voxelterrain.h"
#include "object3d.h"

VoxelTerrain::VoxelTerrain(QObject *parent) : QObject(parent)
{
    cameraPos = QPointF(1024,1024);
    cameraAngle = 0;
    cameraHeight = 50;

    frameBuffer = QImage(screenWidth, screenHeight, QImage::Format::Format_RGB888);

    heightMap.load(":/images/D1.png");

    colorMap.load(":/images/C1W.png");

    yBuffer.reserve(screenWidth);

    zBuffer.resize(screenWidth * screenHeight);

    cullDistance = 2048;

    projectionMatrix.perspective(80, 2.4, 1.0, 2048.0);
}

void VoxelTerrain::BeginFrame()
{
    viewHeight = cameraHeight - 128;

    if(cameraPos.x() < heightMap.width() && cameraPos.y() < heightMap.height() && cameraPos.x() >= 0 && cameraPos.y() >= 0)
    {
        viewHeight = qGray(heightMap.pixel(cameraPos.x(), cameraPos.y())) + cameraHeight - 128;
    }

    frameBuffer.fill(Qt::cyan);

    yBuffer.fill(screenHeight, screenWidth);

    zBuffer.fill(0.0, screenWidth * screenHeight);


    viewMatrix.setToIdentity();
    viewMatrix.rotate(-zAngle * 0.55, QVector3D(1,0,0));
    viewMatrix.rotate(qRadiansToDegrees(-cameraAngle), QVector3D(0,1,0));
    viewMatrix.translate(-cameraPos.x(), -viewHeight, -cameraPos.y());

}

void VoxelTerrain::Render()
{
    BeginFrame();

    float horizonPos = 2048 * qTan(qDegreesToRadians(zAngle));
    float maxPos = 2048 * qTan(qDegreesToRadians(40.0));

    horizonPos = (screenHeight / 2) + ((horizonPos / maxPos) * (screenHeight / 2));


    float sinphi = qSin(cameraAngle);
    float cosphi = qCos(cameraAngle);

    float dz = 0.01;
    float z = 1.0;

    while(z < cullDistance)
    {
        QPointF pleft = QPoint((-cosphi*z - sinphi*z) + cameraPos.x(), ( sinphi*z - cosphi*z) + cameraPos.y());
        QPointF pright = QPoint(( cosphi*z - sinphi*z) + cameraPos.x(), (-sinphi*z - cosphi*z) + cameraPos.y());

        float dx = (pright.x() - pleft.x()) / screenWidth;
        float dy = (pright.y() - pleft.y()) / screenWidth;

        float invz = (1.0 / z);
        float invh = invz * heightScale;

        for(unsigned int i = 0; i < screenWidth; i++)
        {
            float pointHeight = -128;
            QRgb lineColor = qRgb(0,0,128);

            if((pleft.x() < heightMap.width() && pleft.x() >= 0) && (pleft.y() < heightMap.height() && pleft.y() >= 0))
            {

                if(qAlpha(colorMap.pixel(pleft.x(),pleft.y())) == 255)
                {
                    pointHeight = qGray(heightMap.pixel(pleft.x(), pleft.y())) - 128;

                    lineColor = colorMap.pixel(pleft.x(),pleft.y());
                }
            }

            int lineHeight = (((viewHeight - pointHeight) * invh) + (int)horizonPos);

            if(lineHeight < 0)
                lineHeight = 0;
            else if(lineHeight >= screenHeight)
                lineHeight = screenHeight-1;

            for(unsigned int y = lineHeight; y < yBuffer[i]; y++)
            {
                frameBuffer.setPixel(i, y, lineColor);
                zBuffer[(y * screenWidth) + i] = invz;
            }

            if (lineHeight < yBuffer[i])
                yBuffer[i] = lineHeight;

            pleft += QPointF(dx, dy);
        }

        z += dz;
        dz += 0.01;
    }

    Draw3d();
}

void VoxelTerrain::Draw3d()
{
    Triangle3d t[4];
    t[0].verts[0].pos = QVector3D(0, 0, 0);
    t[0].verts[1].pos = QVector3D(100, 0, 0);
    t[0].verts[2].pos = QVector3D(50, 100, 50);

    t[1].verts[0].pos = QVector3D(100, 0, 0);
    t[1].verts[1].pos = QVector3D(100, 0, 100);
    t[1].verts[2].pos = QVector3D(50, 100, 50);

    t[2].verts[0].pos = QVector3D(100, 0, 100);
    t[2].verts[1].pos = QVector3D(0, 0, 100);
    t[2].verts[2].pos = QVector3D(50, 100, 50);

    t[3].verts[0].pos = QVector3D(0, 0, 100);
    t[3].verts[1].pos = QVector3D(0, 0, 0);
    t[3].verts[2].pos = QVector3D(50, 100, 50);


    int pointHeight = qGray(heightMap.pixel(1500, 1500)) - 127;


    Mesh3d mesh;
    mesh.color = Qt::red;
    mesh.tris.append(t[0]);
    mesh.tris.append(t[1]);
    mesh.tris.append(t[2]);
    mesh.tris.append(t[3]);

    Object3d obj;

    obj.mesh.append(mesh);
    obj.pos = QVector3D(1500, pointHeight, 1500);

    QMatrix4x4 modelMatrix;
    QMatrix4x4 transformMatrix;


    modelMatrix.translate(obj.pos);





    transformMatrix = projectionMatrix * viewMatrix * modelMatrix;

    QPointF p[3];


    for(int x = 0; x < mesh.tris.count(); x++)
    {
        bool skip = false;

        for(int i = 0; i < 3; i++)
        {
            QVector3D p3 = transformMatrix * t[x].verts[i].pos;

            p[i].setX((p3.x() + 0.5) * screenWidth);
            p[i].setY(((0.0-p3.y() + 0.5)) * screenHeight);

            float z = p3.z();

            if(z > 1)
                skip = true;
            else
            {
                int vx = p[i].x();
                int vy = p[i].y();

                if(vx >= 0 && vx < screenWidth && vy >= 0 && vy < screenHeight)
                {
                    float z2 = 1.0-zBuffer[(vy * screenWidth) + vx];

                    if(z > (z2+0.001))
                        skip = true;
                }
            }
        }

        Qt::GlobalColor colors[4] = {Qt::red, Qt::blue, Qt::yellow, Qt::green};

        if(!skip)
        {
            QPainter ptr(&frameBuffer);

            QPen pen;
            pen.setWidth(5);
            pen.setColor(colors[x]);

            ptr.setPen(pen);
            ptr.drawConvexPolygon(p, 3);
        }
    }



}
