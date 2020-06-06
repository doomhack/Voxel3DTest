#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <QtCore>
#include <QImage>

#include "object3d.h"

class VoxelTerrain : public QObject
{
    Q_OBJECT
public:
    explicit VoxelTerrain(QObject *parent = nullptr);

    void Render();
    void Draw3d();

    void BeginFrame();

    QImage frameBuffer;

    QImage heightMap;
    QImage colorMap;


    QVector<unsigned int> yBuffer;
    unsigned int cullDistance;
    QVector<float> zBuffer;

    const int screenWidth = 1366;
    const int screenHeight = 720;

    const unsigned int heightScale = (1.25 * ((float)screenWidth * ((float)screenHeight/(float)screenWidth)));

    float zAngle = 0;

    QPointF cameraPos;
    float cameraAngle;
    float cameraHeight;

    float viewHeight;

    QList<Object3d*> objects;

    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;

private:




};

#endif // VOXELTERRAIN_H
