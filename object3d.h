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

class Mesh3d
{
public:
    QRgb color;
    QImage* texture;

    QList<Triangle3d> tris;
};


class Object3d
{
public:
    QVector3D pos;
    QList<Mesh3d> mesh;
};

#endif // OBJECT3D_H
