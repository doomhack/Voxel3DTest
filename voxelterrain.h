#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <QtCore>
#include <QImage>

#include "object3d.h"

typedef struct TriEdgeTrace
{
    float x_left, x_right;
    float z_left, z_right;
    float u_left, u_right;
    float v_left, v_right;
} TriDrawState;

typedef struct TriDrawPos
{
    float z;
    float u;
    float v;
} TriDrawPos;

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
    void DrawTransformedTriangle(Vertex3d points[], QImage* texture, QRgb color);
    void DrawTriangleTop(Vertex3d points[3], QImage* texture, QRgb color);
    void DrawTriangleBottom(Vertex3d points[3], QImage* texture, QRgb color);

    void SortPointsByY(Vertex3d points[3]);

    Vertex3d TransformVertex(const Vertex3d* vertex);
    bool IsTriangleFrontface(Vertex3d screenSpacePoints[3]);
    bool IsTriangleOnScreen(Vertex3d screenSpacePoints[3]);
    void DrawTriangleScanline(int y, TriEdgeTrace& pos, QImage* texture, QRgb color);

    QImage frameBuffer;

    QImage heightMap;
    QImage colorMap;


    QVector<int> yBuffer;
    QVector<float> zBuffer;

    const int screenWidth = 1360;
    const int screenHeight = 720;

    const float zNear = 1.0;
    const float zFar = 2048.0;

    unsigned int heightScale = (0.75*((float)screenWidth * ((float)screenHeight/(float)screenWidth)));

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
