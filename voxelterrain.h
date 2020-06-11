#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <QtCore>
#include <QImage>

#include "3dmaths/f3dmath.h"

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

typedef struct ZYMap
{
    float z;
    int y;
} ZYMap;

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

    int fracToY(float frac);
    int fracToX(float frac);

    QRgb* frameBuffer;

    QImage frameBufferImage;

    QImage heightMapImage;
    QImage colorMapImage;

    const QRgb* colorMap;
    const quint8* heightMap;

    static const int screenWidth = 1360;
    static const int screenHeight = 720;

    const int mapSize = 2048;

    const float zNear = 1.0;
    const float zFar = 2048.0;
    const float zStep = 1.0;
    const float zStepD = 0.01;

    float* zBuffer;
    int yBuffer[screenWidth];



    const float heightScale = (0.82*((float)screenWidth * ((float)screenHeight/(float)screenWidth)));
    const float yScale = ((float)screenWidth * ((float)screenHeight/(float)screenWidth));



    float zAngle = 0;

    F3D::V2F cameraPos;
    float cameraAngle;
    float cameraHeight;

    float viewHeight;

    QList<Object3d*> objects;

    F3D::M4F viewMatrix;
    F3D::M4F projectionMatrix;
    F3D::M4F modelMatrix;

    F3D::M4F viewProjectionMatrix; //P*V

    F3D::M4F transformMatrix; //P*V*M

private:




};

#endif // VOXELTERRAIN_H
