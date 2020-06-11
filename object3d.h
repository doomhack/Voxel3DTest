#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <QtCore>
#include <QtMath>
#include <QtGui>

class Vertex3d
{
public:
    QVector3D pos;
    QVector2D uv;
};

class Triangle3d
{
public:
    Vertex3d verts[3];
};

class Texture
{
public:
    QImage* texture;

    const QRgb* pixels;
    unsigned int width;
    unsigned int height;
};

class Mesh3d
{
public:
    QRgb color;
    Texture* texture;

    QVector<Triangle3d> tris;
};

class Object3d
{
public:
    QVector3D pos;
    QVector<Mesh3d*> mesh;

    bool LoadFromFile(QString objFile, QString mtlFile);
};

#endif // OBJECT3D_H
