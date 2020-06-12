#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <QtCore>
#include <QImage>

#include "3dmaths/f3dmath.h"

#include "object3d.h"

typedef struct TriEdgeTrace
{
    F3D::FP x_left, x_right;
    F3D::FP z_left, z_right;
    F3D::FP u_left, u_right;
    F3D::FP v_left, v_right;
} TriDrawState;

typedef struct TriDrawPos
{
    F3D::FP z;
    F3D::FP u;
    F3D::FP v;
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
    void DrawTriangle(const Triangle3d* tri, Texture *texture, QRgb color);

    void DrawTransformedTriangle(Vertex3d points[], Texture* texture);
    void DrawTriangleTop(Vertex3d points[3], Texture* texture);
    void DrawTriangleBottom(Vertex3d points[3], Texture* texture);

    void DrawTransformedTriangle(Vertex3d points[], QRgb color);
    void DrawTriangleTop(Vertex3d points[3], QRgb color);
    void DrawTriangleBottom(Vertex3d points[3], QRgb color);

    void DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture);
    void DrawTriangleScanline(int y, TriEdgeTrace& pos, QRgb color);


    void SortPointsByY(Vertex3d points[3]);

    Vertex3d TransformVertex(const Vertex3d* vertex);
    bool IsTriangleFrontface(Vertex3d screenSpacePoints[3]);
    bool IsTriangleOnScreen(Vertex3d screenSpacePoints[3]);
    void DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture, QRgb color);

    int fracToY(F3D::FP frac);
    int fracToX(F3D::FP frac);

    QRgb* frameBuffer;

    QImage frameBufferImage;

    QImage heightMapImage;
    QImage colorMapImage;

    const QRgb* colorMap;
    const quint8* heightMap;

    static const int screenWidth = 160;
    static const int screenHeight = 128;

    const int mapSize = 2048;

    const F3D::FP zNear = 1;
    const F3D::FP zFar = 2048;
    const F3D::FP zStep = 1;
    const F3D::FP zStepD = 0.01f;

    F3D::FP* zBuffer;
    int yBuffer[screenWidth];



    const F3D::FP heightScale = (float)(0.82*((float)screenWidth * ((float)screenHeight/(float)screenWidth)));
    const F3D::FP yScale = (float)((float)screenWidth * ((float)screenHeight/(float)screenWidth));



    F3D::FP zAngle = 0;

    F3D::V3FP cameraPos;
    F3D::FP cameraAngle;
    int cameraHeight;

    QList<Object3d*> objects;

    F3D::M4FP viewMatrix;
    F3D::M4FP projectionMatrix;
    F3D::M4FP modelMatrix;

    F3D::M4FP viewProjectionMatrix; //P*V

    F3D::M4FP transformMatrix; //P*V*M

private:




};

#endif // VOXELTERRAIN_H
