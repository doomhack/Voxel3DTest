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

    projectionMatrix.perspective(80, 2.4, zNear, zFar);



    Object3d* airport = new Object3d();
    airport->LoadFromFile(":/models/VRML/airport.obj");
    objects.append(airport);
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

    zBuffer.fill(1.0, screenWidth * screenHeight);


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
    float z = zNear;


    while(z < zFar)
    {
        QPointF pleft = QPoint((-cosphi*z - sinphi*z) + cameraPos.x(), ( sinphi*z - cosphi*z) + cameraPos.y());
        QPointF pright = QPoint(( cosphi*z - sinphi*z) + cameraPos.x(), (-sinphi*z - cosphi*z) + cameraPos.y());

        float dx = (pright.x() - pleft.x()) / screenWidth;
        float dy = (pright.y() - pleft.y()) / screenWidth;

        float invz = (1.0 / z);
        float invh = invz * heightScale;

        float zDepth = (((invz) - (1.0 / zNear) ) / ( (1.0 / zFar) - (1.0 / zNear)));

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
                zBuffer[(y * screenWidth) + i] = zDepth;
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
    for(int i = 0; i < objects.length(); i++)
    {
        const Object3d* obj = objects.at(i);

        this->DrawObject(obj);
    }
}

void VoxelTerrain::DrawObject(const Object3d* obj)
{
    modelMatrix.setToIdentity();

    modelMatrix.translate(obj->pos);

    transformMatrix = projectionMatrix * viewMatrix * modelMatrix;

    for(int i = 0; i < obj->mesh.length(); i++)
    {
        const Mesh3d* mesh = &obj->mesh.at(i);

        this->DrawMesh(mesh);
    }
}

void VoxelTerrain::DrawMesh(const Mesh3d* mesh)
{
    Qt::GlobalColor colors[4] = {Qt::red, Qt::blue, Qt::yellow, Qt::green};

    for(int i = 0; i < mesh->tris.count(); i++)
    {
        const Triangle3d* tri = &mesh->tris.at(i);

        QColor c = QColor(colors[i & 3]);

        this->DrawTriangle(tri, mesh->texture, c.rgb());
    }
}

void VoxelTerrain::DrawTriangle(const Triangle3d* tri, QImage* texture, QRgb color)
{
    QVector3D transformedPoints[3];

    for(int i = 0; i < 3; i++)
    {
        const Vertex3d* v = &tri->verts[i];

        transformedPoints[i] = this->TransformVertex(v);
    }

    //Reject offscreen polys
    if(!IsTriangleOnScreen(transformedPoints))
        return;

    //Backface cull here.
    if(!IsTriangleFrontface(transformedPoints))
        return;

    DrawTransformedTriangle(transformedPoints, texture, color);
}

bool VoxelTerrain::IsTriangleFrontface(QVector3D screenSpacePoints[3])
{
    QVector3D edge1 = screenSpacePoints[0] - screenSpacePoints[1];
    QVector3D edge2 = screenSpacePoints[1] - screenSpacePoints[2];

    QVector3D normal = QVector3D::crossProduct(edge1, edge2);

    return normal.z() > 0;
}

bool VoxelTerrain::IsTriangleOnScreen(QVector3D screenSpacePoints[3])
{
    if  (
            ((screenSpacePoints[0].z() >= 1) || (screenSpacePoints[0].z() < 0)) ||
            ((screenSpacePoints[1].z() >= 1) || (screenSpacePoints[1].z() < 0)) ||
            ((screenSpacePoints[2].z() >= 1) || (screenSpacePoints[2].z() < 0))
         )
        return false;


    if  (
            (screenSpacePoints[0].x() < 0) &&
            (screenSpacePoints[1].x() < 0) &&
            (screenSpacePoints[1].x() < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].x() > screenWidth) &&
            (screenSpacePoints[1].x() > screenWidth) &&
            (screenSpacePoints[1].x() > screenWidth)
        )
        return false;

    if  (
            (screenSpacePoints[0].y() < 0) &&
            (screenSpacePoints[1].y() < 0) &&
            (screenSpacePoints[1].y() < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].y() > screenHeight) &&
            (screenSpacePoints[1].y() > screenHeight) &&
            (screenSpacePoints[1].y() > screenHeight)
        )
        return false;

    return true;
}

void VoxelTerrain::DrawTransformedTriangle(QVector3D points[3], QImage* texture, QRgb color)
{
    SortPointsByY(points);

    if(points[1].y() == points[2].y())
    {
        DrawTriangleTop(points, texture, color);
    }
    else if(points[0].y() == points[1].y())
    {
        DrawTriangleBottom(points, texture, color);
    }
    else
    {
        QVector3D px[3];

        int v4x =(int)(points[0].x() + ((float)(points[1].y() - points[0].y()) / (float)(points[2].y() - points[0].y())) * (points[2].x() - points[0].x()));

        QVector3D p4 = QVector3D(v4x, points[1].y(), points[1].z());

        px[0] = points[0];
        px[1] = points[1];
        px[2] = p4;

        DrawTriangleTop(px, texture, color);

        px[0] = points[1];
        px[1] = p4;
        px[2] = points[2];

        DrawTriangleBottom(px, texture, color);
    }
}

void VoxelTerrain::SortPointsByY(QVector3D points[3])
{
    if(points[0].y() > points[1].y())
        qSwap(points[0], points[1]);

    if(points[0].y() > points[2].y())
        qSwap(points[0], points[2]);

    if(points[1].y() > points[2].y())
        qSwap(points[1], points[2]);
}

void VoxelTerrain::DrawTriangleTop(QVector3D points[3], QImage* texture, QRgb color)
{
    float invslope1 = (float)(points[1].x() - points[0].x()) / (float)(points[1].y() - points[0].y());
    float invslope2 = (float)(points[2].x() - points[0].x()) / (float)(points[2].y() - points[0].y());

    float zStepLeft = (float)(points[1].z() - points[0].z()) / (float)(points[1].y() - points[0].y());
    float zStepRight = (float)(points[2].z() - points[0].z()) / (float)(points[2].y() - points[0].y());

    if(invslope1 > invslope2)
    {
        qSwap(invslope1, invslope2);
        qSwap(zStepLeft, zStepRight);
    }

    float curx1 = points[0].x();
    float curx2 = points[0].x();

    float curZLeft = points[0].z();
    float curZRight = points[0].z();


    int yStart = qRound(points[0].y());
    int yEnd = qRound(points[1].y());

    for (int scanlineY = yStart; scanlineY <= yEnd; scanlineY++)
    {
        if(scanlineY < 0)
            continue;

        if(scanlineY >= screenHeight)
            break;

        float zStep = (curZLeft - curZRight) / (curx1 - curx2);

        float currZ = curZLeft;

        for(int x = curx1; x <= curx2; x++)
        {
            if(x < 0)
                continue;

            if(x >= screenWidth)
                break;

            float oldZ = zBuffer[ (scanlineY*screenWidth) + x];

            if(currZ <= oldZ)
            {
                frameBuffer.setPixel(x, scanlineY, color);
                zBuffer[ (scanlineY*screenWidth) + x] = currZ;
            }

            currZ += zStep;
        }

        curx1 += invslope1;
        curx2 += invslope2;

        curZLeft+=zStepLeft;
        curZRight+=zStepRight;
    }
}

void VoxelTerrain::DrawTriangleBottom(QVector3D points[], QImage* texture, QRgb color)
{
    float invslope1 = (float)(points[2].x() - points[0].x()) / (float)(points[2].y() - points[0].y());
    float invslope2 = (float)(points[2].x() - points[1].x()) / (float)(points[2].y() - points[1].y());

    float zStepLeft = (float)(points[2].z() - points[0].z()) / (float)(points[2].y() - points[0].y());
    float zStepRight = (float)(points[2].z() - points[1].z()) / (float)(points[2].y() - points[1].y());


    if(invslope1 < invslope2)
    {
        qSwap(invslope1, invslope2);
        qSwap(zStepLeft, zStepRight);
    }

    float curx1 = points[2].x();
    float curx2 = points[2].x();

    float curZLeft = points[2].z();
    float curZRight = points[2].z();

    int yStart = qRound(points[2].y());
    int yEnd = qRound(points[0].y());

    for (int scanlineY = yStart; scanlineY > yEnd; scanlineY--)
    {
        if(scanlineY < 0)
            continue;

        if(scanlineY >= screenHeight)
            break;

        float zStep = (curZLeft - curZRight) / (curx1 - curx2);

        float currZ = curZLeft;

        for(int x = curx1; x <= curx2; x++)
        {
            if(x < 0)
                continue;

            if(x >= screenWidth)
                break;

            float oldZ = zBuffer[ (scanlineY*screenWidth) + x];

            if(currZ <= oldZ)
            {
                frameBuffer.setPixel(x, scanlineY, color);
                zBuffer[ (scanlineY*screenWidth) + x] = currZ;
            }

            currZ += zStep;
        }

        curx1 -= invslope1;
        curx2 -= invslope2;

        curZLeft-=zStepLeft;
        curZRight-=zStepRight;
    }
}

QVector3D VoxelTerrain::TransformVertex(const Vertex3d* vertex)
{
    QVector3D p = transformMatrix * vertex->pos;

    QVector3D screenspace(
                    qRound((p.x() + 0.5) * screenWidth),
                    qRound(((0.0-p.y() + 0.5)) * screenHeight),
                    p.z()
                );

    return screenspace;
}
