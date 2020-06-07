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
    airport->LoadFromFile(":/models/VRML/airport.obj", ":/models/VRML/airport.mtl");
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

        float zDepth = 1.0-invz;

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
        const Mesh3d* mesh = obj->mesh.at(i);

        this->DrawMesh(mesh);
    }
}

void VoxelTerrain::DrawMesh(const Mesh3d* mesh)
{
    for(int i = 0; i < mesh->tris.count(); i++)
    {
        const Triangle3d* tri = &mesh->tris.at(i);

        this->DrawTriangle(tri, mesh->texture, mesh->color);
    }
}

void VoxelTerrain::DrawTriangle(const Triangle3d* tri, QImage* texture, QRgb color)
{
    Vertex3d transformedPoints[3];

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

bool VoxelTerrain::IsTriangleFrontface(Vertex3d screenSpacePoints[3])
{
    QVector3D edge1 = screenSpacePoints[0].pos - screenSpacePoints[1].pos;
    QVector3D edge2 = screenSpacePoints[1].pos - screenSpacePoints[2].pos;

    QVector3D normal = QVector3D::crossProduct(edge1, edge2);

    return normal.z() > 0;
}

bool VoxelTerrain::IsTriangleOnScreen(Vertex3d screenSpacePoints[3])
{

    if  (
            ((screenSpacePoints[0].pos.z() >= 1) || (screenSpacePoints[0].pos.z() < 0)) ||
            ((screenSpacePoints[1].pos.z() >= 1) || (screenSpacePoints[1].pos.z() < 0)) ||
            ((screenSpacePoints[2].pos.z() >= 1) || (screenSpacePoints[2].pos.z() < 0))
         )
        return false;

    if  (
            (screenSpacePoints[0].pos.x() < 0) &&
            (screenSpacePoints[1].pos.x() < 0) &&
            (screenSpacePoints[2].pos.x() < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.x() > screenWidth) &&
            (screenSpacePoints[1].pos.x() > screenWidth) &&
            (screenSpacePoints[2].pos.x() > screenWidth)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.y() < 0) &&
            (screenSpacePoints[1].pos.y() < 0) &&
            (screenSpacePoints[2].pos.y() < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.y() > screenHeight) &&
            (screenSpacePoints[1].pos.y() > screenHeight) &&
            (screenSpacePoints[2].pos.y() > screenHeight)
        )
        return false;

    return true;
}

void VoxelTerrain::DrawTransformedTriangle(Vertex3d points[3], QImage* texture, QRgb color)
{
    SortPointsByY(points);

    if(points[1].pos.y() == points[2].pos.y())
    {
        DrawTriangleTop(points, texture, color);
    }
    else if(points[0].pos.y() == points[1].pos.y())
    {
        DrawTriangleBottom(points, texture, color);
    }
    else
    {
        Vertex3d px[3];

        int v4x =(int)(points[0].pos.x() + ((float)(points[1].pos.y() - points[0].pos.y()) / (float)(points[2].pos.y() - points[0].pos.y())) * (points[2].pos.x() - points[0].pos.x()));
        float v4z = (points[0].pos.z() + ((float)(points[1].pos.y() - points[0].pos.y()) / (float)(points[2].pos.y() - points[0].pos.y())) * (points[2].pos.z() - points[0].pos.z()));

        float v4u = (points[0].uv.x() + ((float)(points[1].pos.y() - points[0].pos.y()) / (float)(points[2].pos.y() - points[0].pos.y())) * (points[2].uv.x() - points[0].uv.x()));
        float v4v = (points[0].uv.y() + ((float)(points[1].pos.y() - points[0].pos.y()) / (float)(points[2].pos.y() - points[0].pos.y())) * (points[2].uv.y() - points[0].uv.y()));

        Vertex3d p4;
        p4.pos = QVector3D(v4x, points[1].pos.y(), v4z);
        p4.uv = QVector2D(v4u, v4v);

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

void VoxelTerrain::SortPointsByY(Vertex3d points[3])
{
    if(points[0].pos.y() > points[1].pos.y())
        qSwap(points[0], points[1]);

    if(points[0].pos.y() > points[2].pos.y())
        qSwap(points[0], points[2]);

    if(points[1].pos.y() > points[2].pos.y())
        qSwap(points[1], points[2]);
}

void VoxelTerrain::DrawTriangleTop(Vertex3d points[3], QImage* texture, QRgb color)
{
    float invslope1 = (float)(points[1].pos.x() - points[0].pos.x()) / (float)(points[1].pos.y() - points[0].pos.y());
    float invslope2 = (float)(points[2].pos.x() - points[0].pos.x()) / (float)(points[2].pos.y() - points[0].pos.y());

    float zStepLeft = (float)(points[1].pos.z() - points[0].pos.z()) / (float)(points[1].pos.y() - points[0].pos.y());
    float zStepRight = (float)(points[2].pos.z() - points[0].pos.z()) / (float)(points[2].pos.y() - points[0].pos.y());

    float uStepLeft = (float)(points[1].uv.x() - points[0].uv.x()) / (float)(points[1].pos.y() - points[0].pos.y());
    float uStepRight = (float)(points[2].uv.x() - points[0].uv.x()) / (float)(points[2].pos.y() - points[0].pos.y());

    float vStepLeft = (float)(points[1].uv.y() - points[0].uv.y()) / (float)(points[1].pos.y() - points[0].pos.y());
    float vStepRight = (float)(points[2].uv.y() - points[0].uv.y()) / (float)(points[2].pos.y() - points[0].pos.y());

    if(invslope1 > invslope2)
    {
        qSwap(invslope1, invslope2);
        qSwap(zStepLeft, zStepRight);
        qSwap(uStepLeft, uStepRight);
        qSwap(vStepLeft, vStepRight);
    }

    float curx1 = points[0].pos.x();
    float curx2 = points[0].pos.x();

    float curZLeft = points[0].pos.z();
    float curZRight = points[0].pos.z();

    float currULeft = points[0].uv.x();
    float currURight = points[0].uv.x();

    float currVLeft = points[0].uv.y();
    float currVRight = points[0].uv.y();

    int yStart = qRound(points[0].pos.y());
    int yEnd = qRound(points[1].pos.y());

    for (int scanlineY = yStart; scanlineY <= yEnd; scanlineY++)
    {
        if(scanlineY < 0)
            continue;

        if(scanlineY >= screenHeight)
            break;

        float zStep = (curZLeft - curZRight) / (curx1 - curx2);

        float uStep = (currULeft - currURight) / (curx1 - curx2);
        float vStep = (currVLeft - currVRight) / (curx1 - curx2);

        float currZ = curZLeft;

        float currU = currULeft;
        float currV = currVLeft;

        for(int x = curx1; x <= curx2; x++)
        {
            if(x < 0)
                continue;

            if(x >= screenWidth)
                break;

            if(currZ < 0 || currZ > 1.0)
                continue;

            float oldZ = zBuffer[ (scanlineY*screenWidth) + x];

            if(currZ < oldZ)
            {
                if(texture)
                {
                    int tx = qRound(currU * texture->width());
                    int ty = qRound((1.0-currV) * texture->height());

                    tx = tx & (texture->width() - 1);
                    ty = ty & (texture->height() - 1);

                    frameBuffer.setPixel(x, scanlineY, texture->pixel(tx, ty));
                }
                else
                {
                    frameBuffer.setPixel(x, scanlineY, color);
                }

                zBuffer[ (scanlineY*screenWidth) + x] = currZ;
            }

            currZ += zStep;

            currU += uStep;
            currV += vStep;
        }

        curx1 += invslope1;
        curx2 += invslope2;

        curZLeft+=zStepLeft;
        curZRight+=zStepRight;

        currULeft += uStepLeft;
        currURight += uStepRight;

        currVLeft += vStepLeft;
        currVRight += vStepRight;
    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex3d points[], QImage* texture, QRgb color)
{
    float invslope1 = (float)(points[2].pos.x() - points[0].pos.x()) / (float)(points[2].pos.y() - points[0].pos.y());
    float invslope2 = (float)(points[2].pos.x() - points[1].pos.x()) / (float)(points[2].pos.y() - points[1].pos.y());

    float zStepLeft = (float)(points[2].pos.z() - points[0].pos.z()) / (float)(points[2].pos.y() - points[0].pos.y());
    float zStepRight = (float)(points[2].pos.z() - points[1].pos.z()) / (float)(points[2].pos.y() - points[1].pos.y());

    float uStepLeft = (float)(points[2].uv.x() - points[0].uv.x()) / (float)(points[2].pos.y() - points[0].pos.y());
    float uStepRight = (float)(points[2].uv.x() - points[1].uv.x()) / (float)(points[2].pos.y() - points[1].pos.y());

    float vStepLeft = (float)(points[2].uv.y() - points[0].uv.y()) / (float)(points[2].pos.y() - points[0].pos.y());
    float vStepRight = (float)(points[2].uv.y() - points[1].uv.y()) / (float)(points[2].pos.y() - points[1].pos.y());


    if(invslope1 < invslope2)
    {
        qSwap(invslope1, invslope2);
        qSwap(zStepLeft, zStepRight);

        qSwap(uStepLeft, uStepRight);
        qSwap(vStepLeft, vStepRight);
    }

    float curx1 = points[2].pos.x();
    float curx2 = points[2].pos.x();

    float curZLeft = points[2].pos.z();
    float curZRight = points[2].pos.z();

    float currULeft = points[2].uv.x();
    float currURight = points[2].uv.x();

    float currVLeft = points[2].uv.y();
    float currVRight = points[2].uv.y();


    int yStart = qRound(points[2].pos.y());
    int yEnd = qRound(points[0].pos.y());

    for (int scanlineY = yStart; scanlineY > yEnd; scanlineY--)
    {
        if(scanlineY < 0)
            break;

        if(scanlineY >= screenHeight)
            continue;

        float zStep = (curZLeft - curZRight) / (curx1 - curx2);

        float uStep = (currULeft - currURight) / (curx1 - curx2);
        float vStep = (currVLeft - currVRight) / (curx1 - curx2);


        float currZ = curZLeft;

        float currU = currULeft;
        float currV = currVLeft;

        for(int x = curx1; x <= curx2; x++)
        {
            if(x < 0)
                continue;

            if(x >= screenWidth)
                break;

            if(currZ < 0 || currZ > 1.0)
                continue;

            float oldZ = zBuffer[ (scanlineY*screenWidth) + x];

            if(currZ < oldZ)
            {
                if(texture)
                {
                    int tx = qRound(currU * texture->width());
                    int ty = qRound((1.0 - currV) * texture->height());

                    tx = tx & (texture->width() - 1);
                    ty = ty & (texture->height() - 1);

                    frameBuffer.setPixel(x, scanlineY, texture->pixel(tx, ty));
                }
                else
                {
                    frameBuffer.setPixel(x, scanlineY, color);
                }

                zBuffer[ (scanlineY*screenWidth) + x] = currZ;
            }

            currZ += zStep;

            currU += uStep;
            currV += vStep;
        }

        curx1 -= invslope1;
        curx2 -= invslope2;

        curZLeft-=zStepLeft;
        curZRight-=zStepRight;

        currULeft -= uStepLeft;
        currURight -= uStepRight;

        currVLeft -= vStepLeft;
        currVRight -= vStepRight;
    }
}

Vertex3d VoxelTerrain::TransformVertex(const Vertex3d* vertex)
{
    QVector3D p = transformMatrix * vertex->pos;

    Vertex3d screenspace;

    screenspace.pos = QVector3D
    (
        qRound((p.x() + 0.5) * screenWidth),
        qRound(((0.0-p.y() + 0.5)) * screenHeight),
        p.z()
    );

    screenspace.uv = vertex->uv;

    return screenspace;
}
