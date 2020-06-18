#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <QtCore>
#include <QImage>

#include "common.h"

#include "object3d.h"



typedef struct TriEdgeTrace
{
    fp x_left, x_right;
    fp z_left, z_right;
    fp w_left, w_right;
    fp u_left, u_right;
    fp v_left, v_right;
} TriDrawState;

typedef struct TriDrawPos
{
    fp z;
    fp w;
    fp u;
    fp v;
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

    void ClipAndDrawTriangle(Vertex2d clipSpacePoints[], Texture *texture, QRgb color);
    void DrawClippedTriangle(Vertex2d clipSpacePoints[], Texture *texture, QRgb color);

    fp GetLineIntersection(fp v1, fp v2, const fp pos);


    void DrawTransformedTriangle(Vertex2d points[], Texture* texture);
    void DrawTriangleTop(Vertex2d points[3], Texture* texture);
    void DrawTriangleBottom(Vertex2d points[3], Texture* texture);

    void DrawTransformedTriangle(Vertex2d points[], QRgb color);
    void DrawTriangleTop(Vertex2d points[3], QRgb color);
    void DrawTriangleBottom(Vertex2d points[3], QRgb color);

    void DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture);
    void DrawTriangleScanline(int y, TriEdgeTrace& pos, QRgb color);


    void SortPointsByY(Vertex2d points[3]);

    Vertex2d TransformVertex(const Vertex3d* vertex);
    bool IsTriangleFrontface(Vertex2d screenSpacePoints[3]);
    bool IsTriangleOnScreen(Vertex2d screenSpacePoints[3]);
    void DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture, QRgb color);

    int fracToY(fp frac);
    int fracToX(fp frac);

    fp lerp(fp a, fp b, fp frac);

    QRgb* frameBuffer;

    QImage frameBufferImage;

    QImage heightMapImage;
    QImage colorMapImage;

    QImage waterImage;
    const QRgb* water;

    const QRgb* colorMap;
    const quint8* heightMap;

    static const int screenWidth = 720;
    static const int screenHeight = 360;

    const int mapSize = 2048;

    const fp zNear = 5;
    const fp zFar = 1024;
    const fp zStep = 1;
    const fp zStepD = 0.01f;

    fp* zBuffer;
    int yBuffer[screenWidth];



    const fp heightScale = (float)(0.82*((float)screenWidth * ((float)screenHeight/(float)screenWidth)));
    const fp yScale = (float)((float)screenWidth * ((float)screenHeight/(float)screenWidth));



    fp zAngle = 0;

    F3D::V3<fp> cameraPos;
    fp cameraAngle;
    int cameraHeight;

    QList<Object3d*> objects;

    F3D::M4<fp> viewMatrix;
    F3D::M4<fp> projectionMatrix;
    F3D::M4<fp> modelMatrix;

    F3D::M4<fp> viewProjectionMatrix; //P*V

    F3D::M4<fp> transformMatrix; //P*V*M

private:




};

#endif // VOXELTERRAIN_H
