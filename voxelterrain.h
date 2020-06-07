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

    void DrawObject(const Object3d* object);
    void DrawMesh(const Mesh3d* mesh);
    void DrawTriangle(const Triangle3d* tri, QImage* texture, QRgb color);
    void DrawTransformedTriangle(QPoint points[3], QImage* texture, QRgb color);
    QVector3D TransformVertex(const Vertex3d* vertex);
    bool IsTriangleFrontface(QVector3D screenSpacePoints[3]);

    QImage frameBuffer;

    QImage heightMap;
    QImage colorMap;


    QVector<unsigned int> yBuffer;
    QVector<float> zBuffer;

    const int screenWidth = 1366;
    const int screenHeight = 720;

    const float zNear = 1.0;
    const float zFar = 2048.0;

    const unsigned int heightScale = (1.25 * ((float)screenWidth * ((float)screenHeight/(float)screenWidth)));

    float zAngle = 0;

    QPointF cameraPos;
    float cameraAngle;
    float cameraHeight;

    float viewHeight;

    QList<Object3d*> objects;

    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 modelMatrix;
    QMatrix4x4 transformMatrix;

private:




};

#endif // VOXELTERRAIN_H
