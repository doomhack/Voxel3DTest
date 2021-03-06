#include <limits>

#include <QtMath>
#include "voxelterrain.h"
#include "object3d.h"

#include "3dmaths/f3dmath.h"

int VoxelTerrain::fracToY(fp frac)
{
    fp y = fp(1)-((frac + fp(1)) / fp(2));

#ifdef USE_FLOAT
    int sy = y * screenHeight;
#else
    int sy = y.intMul(screenHeight);

    if(sy < F3D::FP::min())
        return F3D::FP::min();
    else if(sy > F3D::FP::max())
        return F3D::FP::max();
#endif

    return sy;
}

int VoxelTerrain::fracToX(fp frac)
{
    fp x = (frac + fp(1)) / fp(2);

#ifdef USE_FLOAT
    int sx = x * screenWidth;
#else
    int sx = x.intMul(screenWidth);

    if(sx < F3D::FP::min())
        return F3D::FP::min();
    else if(sx > F3D::FP::max())
        return F3D::FP::max();
#endif

    return sx;
}

VoxelTerrain::VoxelTerrain(QObject *parent) : QObject(parent)
{
    cameraHeight = 50;

    cameraPos = F3D::V3<fp>(mapSize/2, cameraHeight, mapSize/2);
    cameraAngle = 0;

    frameBufferImage = QImage(screenWidth, screenHeight, QImage::Format::Format_RGB32);
    frameBuffer = (QRgb*)frameBufferImage.bits();

    heightMapImage.load(":/images/D1.png");
    heightMapImage = heightMapImage.convertToFormat(QImage::Format::Format_Grayscale8);

    heightMap = heightMapImage.bits();

    colorMapImage.load(":/images/C1W.png");
    colorMapImage = colorMapImage.convertToFormat(QImage::Format::Format_ARGB32);

    colorMap = (const QRgb*)colorMapImage.bits();

    waterImage.load(":/images/water.bmp");
    waterImage = waterImage.convertToFormat(QImage::Format::Format_RGB32);
    water = (const QRgb*)waterImage.bits();


    zBuffer = new fp[screenWidth * screenHeight];

    //55.7954
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(54, 1.888888888f, zNear, zFar);

/*
    Object3d* airport = new Object3d();
    airport->LoadFromFile(":/models/VRML/airport.obj", ":/models/VRML/airport.mtl");
    objects.append(airport);

    Object3d* church = new Object3d();
    church->LoadFromFile(":/models/VRML/church.obj", ":/models/VRML/church.mtl");
    objects.append(church);

    Object3d* hotel = new Object3d();
    hotel->LoadFromFile(":/models/VRML/hotel.obj", ":/models/VRML/hotel.mtl");
    objects.append(hotel);

    Object3d* vilage = new Object3d();
    vilage->LoadFromFile(":/models/VRML/vilage.obj", ":/models/VRML/vilage.mtl");
    objects.append(vilage);

    Object3d* funfair = new Object3d();
    funfair->LoadFromFile(":/models/VRML/funfair.obj", ":/models/VRML/funfair.mtl");
    objects.append(funfair);

    Object3d* rock = new Object3d();
    rock->LoadFromFile(":/models/VRML/rock.obj", ":/models/VRML/rock.mtl");
    objects.append(rock);

    */
    Object3d* world = new Object3d();
    world->LoadFromFile(":/models/VRML/world.obj", ":/models/VRML/world.mtl");
    objects.append(world);

/*
    Object3d* plane = new Object3d();
    plane->LoadFromFile(":/models/VRML/plane.obj", ":/models/VRML/plane.mtl");
    objects.append(plane);
    */
}

void VoxelTerrain::BeginFrame()
{
    cameraPos.y = cameraHeight;

    int xpos = cameraPos.x;
    int zpos = cameraPos.z;

    if(xpos < mapSize && zpos < mapSize && xpos >= 0 && zpos >= 0)
    {
        cameraPos.y = heightMap[zpos * mapSize + xpos] + cameraHeight;
    }

    frameBufferImage.fill(Qt::cyan);

    std::fill_n(zBuffer, screenWidth*screenHeight, 1);
    std::fill_n(yBuffer, screenWidth, screenHeight);


    viewMatrix.setToIdentity();
    viewMatrix.rotateX(-zAngle);
    viewMatrix.rotateY(qRadiansToDegrees(-cameraAngle));
    viewMatrix.translate(F3D::V3<fp>(-cameraPos.x, -cameraPos.y, -cameraPos.z));

    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void VoxelTerrain::Render()
{
    BeginFrame();

    Draw3d();
    return;


    fp sinphi = (float)qSin(cameraAngle);
    fp cosphi = (float)qCos(cameraAngle);

    fp cZstep = zStep;

    for(fp z = zNear; z < zFar; z += cZstep, cZstep += zStepD)
    {
        F3D::V2<fp> pleft = F3D::V2<fp>((-cosphi*z - sinphi*z) + cameraPos.x, ( sinphi*z - cosphi*z) + cameraPos.z);
        F3D::V2<fp> pright = F3D::V2<fp>(( cosphi*z - sinphi*z) + cameraPos.x, (-sinphi*z - cosphi*z) + cameraPos.z);

        F3D::V2<fp> delta((pright.x - pleft.x) / screenWidth, (pright.y - pleft.y) / screenWidth);

        fp invz = (fp(1) / z);

        fp zDepth = fp(1)-invz;

        fp invh = invz * heightScale;
        fp invy = invz * yScale;

#ifdef USE_FLOAT
        int yPos = (invy * cameraPos.y) + (screenHeight / 2);
#else
        int yPos = (invy.intMul(cameraPos.y)) + (screenHeight / 2);
#endif

        for(int i = 0; i < screenWidth; i++)
        {
            int pointHeight = 0;
            QRgb lineColor;

            if((pleft.x < mapSize && pleft.x >= 0) && (pleft.y < mapSize && pleft.y >= 0))
            {
                const int pixelOffs = (int)pleft.y*mapSize + (int)pleft.x;

                QRgb color = colorMap[pixelOffs];

                if(qAlpha(color) == 255)
                {
                    pointHeight = heightMap[pixelOffs];

                    lineColor = color;
                }
                else
                {
                    lineColor = water[((int)pleft.y & 63) * 64 + ((int)pleft.x & 63)];
                }
            }
            else
            {
                lineColor = water[((int)pleft.y & 63) * 64 + ((int)pleft.x & 63)];
            }

#ifdef USE_FLOAT
            int hDiff = invh * pointHeight;
#else
            int hDiff = invh.intMul(pointHeight);
#endif


            int lineTop = yPos - hDiff;

            if(lineTop < 0)
                lineTop = 0;

            for(int y = lineTop; y < yBuffer[i]; y++)
            {
                frameBuffer[y*screenWidth + i] = lineColor;
                zBuffer[(y * screenWidth) + i] = zDepth;
            }

            if (lineTop < yBuffer[i])
                yBuffer[i] = lineTop;

            pleft += delta;
        }
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

    transformMatrix = viewProjectionMatrix * modelMatrix;

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

Vertex2d VoxelTerrain::TransformVertex(const Vertex3d* vertex)
{
    F3D::V4<fp> p = transformMatrix * vertex->pos;

    Vertex2d screenspace;

    screenspace.pos = F3D::V4<fp>
    (
        p.x,
        p.y,
        p.z,
        p.w
    );

    screenspace.uv = vertex->uv;

    return screenspace;
}


void VoxelTerrain::DrawTriangle(const Triangle3d* tri, Texture* texture, QRgb color)
{
    Vertex2d clipSpacePoints[3];

    for(int i = 0; i < 3; i++)
    {
        const Vertex3d* v = &tri->verts[i];

        clipSpacePoints[i] = this->TransformVertex(v);
    }

    ClipAndDrawTriangle(clipSpacePoints, texture, color);
}

void VoxelTerrain::ClipAndDrawTriangle(Vertex2d clipSpacePoints[], Texture *texture, QRgb color)
{
    const fp wClip = zNear;

    fp w0 = clipSpacePoints[0].pos.w;
    fp w1 = clipSpacePoints[1].pos.w;
    fp w2 = clipSpacePoints[2].pos.w;

    fp x0 = clipSpacePoints[0].pos.x;
    fp x1 = clipSpacePoints[1].pos.x;
    fp x2 = clipSpacePoints[2].pos.x;

    fp y0 = clipSpacePoints[0].pos.y;
    fp y1 = clipSpacePoints[1].pos.y;
    fp y2 = clipSpacePoints[2].pos.y;

    fp z0 = clipSpacePoints[0].pos.z;
    fp z1 = clipSpacePoints[1].pos.z;
    fp z2 = clipSpacePoints[2].pos.z;

    if(x0 > w0 && x1 > w1 && x2 > w2)
        return;

    if(-x0 > w0 && -x1 > w1 && -x2 > w2)
        return;

    if(y0 > w0 && y1 > w1 && y2 > w2)
        return;

    if(-y0 > w0 && -y1 > w1 && -y2 > w2)
        return;

    if(z0 > w0 && z1 > w1 && z2 > w2)
        return;

    if(-z0 > w0 && -z1 > w1 && -z2 > w2)
        return;

    //All points behind clipping plane.
    if(w0 < wClip && w1 < wClip && w2 < wClip)
        return;

    //All points in valid space.
    if(w0 >= wClip && w1 >= wClip && w2 >= wClip)
    {
        DrawClippedTriangle(clipSpacePoints, texture, color);
        return;
    }

    Vertex2d outputVx[4];
    int vp = 0;

    //qDebug() << w0 << w1 << w2;

    for(int i = 0; i < 3; i++)
    {
        if(clipSpacePoints[i].pos.w >= wClip)
        {
            outputVx[vp] = clipSpacePoints[i];
            vp++;
        }

        int i2 = i < 2 ? i+1 : 0;

        fp frac = GetLineIntersection(clipSpacePoints[i].pos.w, clipSpacePoints[i2].pos.w, wClip);

        if(frac > 0)
        {
            //qDebug() << "Clipfrac = " << frac;

            Vertex2d newVx;

            newVx.pos.x = F3D::lerp(clipSpacePoints[i].pos.x, clipSpacePoints[i2].pos.x, frac);
            newVx.pos.y = F3D::lerp(clipSpacePoints[i].pos.y, clipSpacePoints[i2].pos.y, frac);
            newVx.pos.z = F3D::lerp(clipSpacePoints[i].pos.z, clipSpacePoints[i2].pos.z, frac);
            newVx.pos.w = wClip;

            newVx.uv.x = F3D::lerp(clipSpacePoints[i].uv.x, clipSpacePoints[i2].uv.x, frac);
            newVx.uv.y = F3D::lerp(clipSpacePoints[i].uv.y, clipSpacePoints[i2].uv.y, frac);

            outputVx[vp] = newVx;
            vp++;
        }
    }

    if(vp == 3)
    {
        DrawClippedTriangle(outputVx, texture, color);
        //DrawClippedTriangle(outputVx, nullptr, qrand());
    }
    else if(vp == 4)
    {
        DrawClippedTriangle(outputVx, texture, color);
        //DrawClippedTriangle(outputVx, nullptr, qrand());
        outputVx[1] = outputVx[0];
        DrawClippedTriangle(&outputVx[1], texture, color);
        //DrawClippedTriangle(&outputVx[1], nullptr, qrand());

    }
    else
    {
        qDebug() << "Got" << vp << "vertexes. Thats not right!";
    }
}


//Return -1 == both <= pos.
//Return -2 == both >= pos.
fp VoxelTerrain::GetLineIntersection(fp v1, fp v2, const fp pos)
{
    if(v1 >= pos && v2 >= pos)
        return -2;
    else if(v1 <= pos && v2 <= pos)
        return -1;
    else if(v1 == v2)
    {
        if(v1 >= pos)
            return -2;

        return -1;
    }
    else if(v1 > v2)
    {
        fp len = (v1 - v2);

        fp splitFrac = (v1 - pos) / len;

        return splitFrac;
    }

    fp len = (v2 - v1);

    fp splitFrac = (v2 - pos) / len;

    return fp(1) - splitFrac;
}


void VoxelTerrain::DrawClippedTriangle(Vertex2d clipSpacePoints[], Texture *texture, QRgb color)
{
    Vertex2d screenSpacePoints[3];

    for(int i = 0; i < 3; i++)
    {
        screenSpacePoints[i].pos = clipSpacePoints[i].pos.ToScreenSpace();

        screenSpacePoints[i].pos.x = fracToX(screenSpacePoints[i].pos.x);
        screenSpacePoints[i].pos.y = fracToY(screenSpacePoints[i].pos.y);

        screenSpacePoints[i].uv = clipSpacePoints[i].uv;
    }

    //Backface cull here.
    if(!IsTriangleFrontface(screenSpacePoints))
        return;

    //Reject offscreen polys
    if(!IsTriangleOnScreen(screenSpacePoints))
        return;

    SortPointsByY(screenSpacePoints);

    if(texture)
        DrawTransformedTriangle(screenSpacePoints, texture);
    else
        DrawTransformedTriangle(screenSpacePoints, color);
}

bool VoxelTerrain::IsTriangleFrontface(Vertex2d screenSpacePoints[])
{
    int x1 = (screenSpacePoints[0].pos.x - screenSpacePoints[1].pos.x);
    int y1 = (screenSpacePoints[0].pos.y - screenSpacePoints[1].pos.y);

    int x2 = (screenSpacePoints[1].pos.x - screenSpacePoints[2].pos.x);
    int y2 = (screenSpacePoints[1].pos.y - screenSpacePoints[2].pos.y);

    return ((x1 * y2) - (y1 * x2)) > 0;
}

bool VoxelTerrain::IsTriangleOnScreen(Vertex2d screenSpacePoints[])
{
    int lowx = screenWidth, highx = -1, lowy = screenHeight, highy = -1;

    for(int i = 0; i < 3; i++)
    {
        int x = screenSpacePoints[i].pos.x;
        int y = screenSpacePoints[i].pos.y;

        if(x < lowx)
            lowx = x;

        if(x > highx)
            highx = x;

        if(y < lowy)
            lowy = y;

        if(y > highy)
            highy = y;
    }

    if(lowx == highx || lowy == highy)
        return false;

    if((lowx >= screenWidth) || (highx < 0) || (lowy >= screenHeight) || (highy < 0))
        return false;

    return true;
}

void VoxelTerrain::DrawTransformedTriangle(Vertex2d points[], Texture* texture)
{

#ifdef PERSPECTIVE_CORRECT
    points[0].toPerspectiveCorrect();
    points[1].toPerspectiveCorrect();
    points[2].toPerspectiveCorrect();
#endif

    if(points[1].pos.y == points[2].pos.y)
    {
        DrawTriangleTop(points, texture);
    }
    else if(points[0].pos.y == points[1].pos.y)
    {
        DrawTriangleBottom(points, texture);
    }
    else
    {
        //Now we split the polygon into two triangles.
        //A flat top and flat bottom triangle.

        //How far down between vx0 -> vx2 are we spliting?
        fp splitFrac = (points[1].pos.y - points[0].pos.y) / (points[2].pos.y - points[0].pos.y);

        //Interpolate new values for new vertex.
        Vertex2d triangle[4];

        triangle[0] = points[0];
        triangle[1] = points[1];

        //x pos
        triangle[2].pos.x = F3D::lerp(points[0].pos.x, points[2].pos.x, splitFrac);
        triangle[2].pos.y = points[1].pos.y;
        triangle[2].pos.z = F3D::lerp(points[0].pos.z, points[2].pos.z, splitFrac);
        triangle[2].pos.w = F3D::lerp(points[0].pos.w, points[2].pos.w, splitFrac);

        //uv coords.
        triangle[2].uv.x = F3D::lerp(points[0].uv.x, points[2].uv.x, splitFrac);
        triangle[2].uv.y = F3D::lerp(points[0].uv.y, points[2].uv.y, splitFrac);

        triangle[3] = points[2];

        DrawTriangleTop(triangle, texture);

        DrawTriangleBottom(&triangle[1], texture);
    }
}

void VoxelTerrain::DrawTransformedTriangle(Vertex2d points[], QRgb color)
{
    if(points[1].pos.y == points[2].pos.y)
    {
        DrawTriangleTop(points, color);
    }
    else if(points[0].pos.y == points[1].pos.y)
    {
        DrawTriangleBottom(points, color);
    }
    else
    {
        //Now we split the polygon into two triangles.
        //A flat top and flat bottom triangle.

        //How far down between vx0 -> vx2 are we spliting?
        fp splitFrac = (points[1].pos.y - points[0].pos.y) / (points[2].pos.y - points[0].pos.y);

        //Interpolate new values for new vertex.
        Vertex2d triangle[4];

        triangle[0] = points[0];
        triangle[1] = points[1];

        //x pos
        triangle[2].pos.x = F3D::lerp(points[0].pos.x, points[2].pos.x, splitFrac);
        triangle[2].pos.y = points[1].pos.y;
        triangle[2].pos.z = F3D::lerp(points[0].pos.z, points[2].pos.z, splitFrac);

        triangle[3] = points[2];

        DrawTriangleTop(triangle, color);

        DrawTriangleBottom(&triangle[1], color);
    }
}

void VoxelTerrain::SortPointsByY(Vertex2d points[])
{
    if(points[0].pos.y > points[1].pos.y)
        qSwap(points[0], points[1]);

    if(points[0].pos.y > points[2].pos.y)
        qSwap(points[0], points[2]);

    if(points[1].pos.y > points[2].pos.y)
        qSwap(points[1], points[2]);
}



void VoxelTerrain::DrawTriangleTop(Vertex2d points[], Texture* texture)
{
    TriEdgeTrace pos;

    Vertex2d *top, *left, *right;
    top = &points[0];

    if(points[1].pos.x < points[2].pos.x)
    {
        left = &points[1];
        right = &points[2];
    }
    else
    {
        left = &points[2];
        right = &points[1];
    }

    const fp yFracScale = 1024;
    fp inv_height = (fp(yFracScale)/(points[1].pos.y - points[0].pos.y));
    fp yFracScaled = inv_height;

    int yStart = top->pos.y;
    int yEnd = left->pos.y;

    if(yStart < 0)
    {
        yFracScaled = (fp(-yStart) * inv_height);
        yStart = 0;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(top->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(top->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(top->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(top->pos.z, right->pos.z, yFrac);

        pos.w_left = F3D::lerp(top->pos.w, left->pos.w, yFrac);
        pos.w_right = F3D::lerp(top->pos.w, right->pos.w, yFrac);

        pos.u_left = F3D::lerp(top->uv.x, left->uv.x, yFrac);
        pos.u_right = F3D::lerp(top->uv.x, right->uv.x, yFrac);

        pos.v_left = F3D::lerp(top->uv.y, left->uv.y, yFrac);
        pos.v_right = F3D::lerp(top->uv.y, right->uv.y, yFrac);
    }
    else
    {
        pos.x_left = pos.x_right = top->pos.x;
        pos.z_left = pos.z_right = top->pos.z;
        pos.w_left = pos.w_right = top->pos.w;

        pos.u_left = pos.u_right = top->uv.x;
        pos.v_left = pos.v_right = top->uv.y;

        yFracScaled = 0;
    }

    if(yEnd >= screenHeight)
        yEnd = screenHeight-1;

    for (int y = yStart; y <= yEnd; y++)
    {

        DrawTriangleScanline(y, pos, texture);

        yFracScaled += inv_height;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(top->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(top->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(top->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(top->pos.z, right->pos.z, yFrac);

        pos.w_left = F3D::lerp(top->pos.w, left->pos.w, yFrac);
        pos.w_right = F3D::lerp(top->pos.w, right->pos.w, yFrac);

        pos.u_left = F3D::lerp(top->uv.x, left->uv.x, yFrac);
        pos.u_right = F3D::lerp(top->uv.x, right->uv.x, yFrac);

        pos.v_left = F3D::lerp(top->uv.y, left->uv.y, yFrac);
        pos.v_right = F3D::lerp(top->uv.y, right->uv.y, yFrac);
    }
}

void VoxelTerrain::DrawTriangleTop(Vertex2d points[], QRgb color)
{
    TriEdgeTrace pos;

    Vertex2d *top, *left, *right;
    top = &points[0];

    if(points[1].pos.x < points[2].pos.x)
    {
        left = &points[1];
        right = &points[2];
    }
    else
    {
        left = &points[2];
        right = &points[1];
    }

    const fp yFracScale = 1024;
    fp inv_height = (fp(yFracScale)/(points[1].pos.y - points[0].pos.y));
    fp yFracScaled = inv_height;

    int yStart = top->pos.y;
    int yEnd = left->pos.y;

    if(yStart < 0)
    {
        yFracScaled = (fp(-yStart) * inv_height);
        yStart = 0;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(top->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(top->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(top->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(top->pos.z, right->pos.z, yFrac);
    }
    else
    {
        pos.x_left = pos.x_right = top->pos.x;
        pos.z_left = pos.z_right = top->pos.z;

        yFracScaled = 0;
    }

    if(yEnd >= screenHeight)
        yEnd = screenHeight-1;

    for (int y = yStart; y <= yEnd; y++)
    {

        DrawTriangleScanline(y, pos, color);

        yFracScaled += inv_height;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(top->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(top->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(top->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(top->pos.z, right->pos.z, yFrac);
    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex2d points[], Texture* texture)
{
    TriEdgeTrace pos;

    Vertex2d *bottom, *left, *right;
    bottom = &points[2];

    if(points[0].pos.x < points[1].pos.x)
    {
        left = &points[0];
        right = &points[1];
    }
    else
    {
        left = &points[1];
        right = &points[0];
    }

    const fp yFracScale = 1024;
    fp inv_height = (fp(yFracScale)/(bottom->pos.y - left->pos.y));

    fp yFracScaled;

    int yStart = bottom->pos.y;
    int yEnd = left->pos.y;

    if(yStart >= screenHeight)
    {
        yFracScaled = (fp(yStart-(screenHeight-1)) * inv_height);
        yStart = screenHeight-1;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(bottom->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(bottom->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(bottom->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(bottom->pos.z, right->pos.z, yFrac);

        pos.w_left = F3D::lerp(bottom->pos.w, left->pos.w, yFrac);
        pos.w_right = F3D::lerp(bottom->pos.w, right->pos.w, yFrac);

        pos.u_left = F3D::lerp(bottom->uv.x, left->uv.x, yFrac);
        pos.u_right = F3D::lerp(bottom->uv.x, right->uv.x, yFrac);

        pos.v_left = F3D::lerp(bottom->uv.y, left->uv.y, yFrac);
        pos.v_right = F3D::lerp(bottom->uv.y, right->uv.y, yFrac);
    }
    else
    {
        pos.x_left = pos.x_right = bottom->pos.x;
        pos.z_left = pos.z_right = bottom->pos.z;
        pos.w_left = pos.w_right = bottom->pos.w;

        pos.u_left = pos.u_right = bottom->uv.x;
        pos.v_left = pos.v_right = bottom->uv.y;

        yFracScaled = 0;
    }

    if(yEnd < 0)
        yEnd = 0;


    for (int y = yStart; y >= yEnd; y--)
    {
        DrawTriangleScanline(y, pos, texture);

        yFracScaled += inv_height;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(bottom->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(bottom->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(bottom->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(bottom->pos.z, right->pos.z, yFrac);

        pos.w_left = F3D::lerp(bottom->pos.w, left->pos.w, yFrac);
        pos.w_right = F3D::lerp(bottom->pos.w, right->pos.w, yFrac);

        pos.u_left = F3D::lerp(bottom->uv.x, left->uv.x, yFrac);
        pos.u_right = F3D::lerp(bottom->uv.x, right->uv.x, yFrac);

        pos.v_left = F3D::lerp(bottom->uv.y, left->uv.y, yFrac);
        pos.v_right = F3D::lerp(bottom->uv.y, right->uv.y, yFrac);
    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex2d points[], QRgb color)
{
    TriEdgeTrace pos;

    Vertex2d *bottom, *left, *right;
    bottom = &points[2];

    if(points[0].pos.x < points[1].pos.x)
    {
        left = &points[0];
        right = &points[1];
    }
    else
    {
        left = &points[1];
        right = &points[0];
    }

    const fp yFracScale = 1024;
    fp inv_height = (fp(yFracScale)/(bottom->pos.y - left->pos.y));

    fp yFracScaled;

    int yStart = bottom->pos.y;
    int yEnd = left->pos.y;

    if(yStart >= screenHeight)
    {
        yFracScaled = (fp(yStart-(screenHeight-1)) * inv_height);
        yStart = screenHeight-1;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(bottom->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(bottom->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(bottom->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(bottom->pos.z, right->pos.z, yFrac);
    }
    else
    {
        pos.x_left = pos.x_right = bottom->pos.x;
        pos.z_left = pos.z_right = bottom->pos.z;

        yFracScaled = 0;
    }

    if(yEnd < 0)
        yEnd = 0;


    for (int y = yStart; y >= yEnd; y--)
    {
        DrawTriangleScanline(y, pos, color);

        yFracScaled += inv_height;

        fp yFrac = yFracScaled / yFracScale;

        pos.x_left = F3D::lerp(bottom->pos.x, left->pos.x, yFrac);
        pos.x_right = F3D::lerp(bottom->pos.x, right->pos.x, yFrac);

        pos.z_left = F3D::lerp(bottom->pos.z, left->pos.z, yFrac);
        pos.z_right = F3D::lerp(bottom->pos.z, right->pos.z, yFrac);
    }
}


void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture)
{
    TriDrawPos sl_pos;

    int x_start = pos.x_left;
    int x_end = pos.x_right;

    fp inv_width = 0;
    const fp xFracScale = 1024;
    fp xFracScaled;

    if(x_start < x_end)
    {
        inv_width = fp(xFracScale)/(x_end - x_start);
    }

    if(x_start < 0)
    {
        xFracScaled = (fp(-x_start) * inv_width);
        x_start = 0;

        fp xFrac = xFracScaled / xFracScale;

        sl_pos.z = F3D::lerp(pos.z_left, pos.z_right, xFrac);
        sl_pos.w = F3D::lerp(pos.w_left, pos.w_right, xFrac);
        sl_pos.u = F3D::lerp(pos.u_left, pos.u_right, xFrac);
        sl_pos.v = F3D::lerp(pos.v_left, pos.v_right, xFrac);
    }
    else
    {
        sl_pos.w = pos.w_left;
        sl_pos.z = pos.z_left;
        sl_pos.u = pos.u_left;
        sl_pos.v = pos.v_left;

        xFracScaled = 0;
    }

    if(x_end >= screenWidth)
        x_end = screenWidth-1;

    int buffOffset = ((y * screenWidth) + x_start);
    fp* zb = &zBuffer[buffOffset];
    QRgb* fb = &frameBuffer[buffOffset];

    for(int x = x_start; x <= x_end; x++)
    {
        if(sl_pos.z < *zb)
        {
#ifdef PERSPECTIVE_CORRECT
            fp invw = fp(1) / sl_pos.w;

            int tx = ((sl_pos.u * fp((int)texture->width)) * invw);
            int ty = fp((int)texture->height) - ((sl_pos.v * fp((int)texture->height)) * invw);
#else
            int tx = sl_pos.u * (int)texture->width;
            int ty = (fp(1)-sl_pos.v) * (int)texture->height;
#endif
            tx = tx & (texture->width - 1);
            ty = ty & (texture->height - 1);

            *fb = texture->pixels[ty * texture->width + tx];
            *zb = sl_pos.z;
        }

        xFracScaled += inv_width;
        zb++;
        fb++;

        fp xFrac = xFracScaled / xFracScale;

        sl_pos.z = F3D::lerp(pos.z_left, pos.z_right, xFrac);
        sl_pos.w = F3D::lerp(pos.w_left, pos.w_right, xFrac);
        sl_pos.u = F3D::lerp(pos.u_left, pos.u_right, xFrac);
        sl_pos.v = F3D::lerp(pos.v_left, pos.v_right, xFrac);
    }
}

void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, QRgb color)
{
    fp zPos;

    int x_start = pos.x_left;
    int x_end = pos.x_right;

    fp inv_width = 0;
    const fp xFracScale = 1024;
    fp xFracScaled;

    if(x_start < x_end)
    {
        inv_width = fp(xFracScale)/(x_end - x_start);
    }
    else if(x_start > x_end)
        return;

    if(x_start < 0)
    {
        xFracScaled = (fp(-x_start) * inv_width);
        x_start = 0;

        fp xFrac = xFracScaled / xFracScale;

        zPos = F3D::lerp(pos.z_left, pos.z_right, xFrac);
    }
    else
    {
        zPos = pos.z_left;
        xFracScaled = 0;
    }

    if(x_end >= screenWidth)
        x_end = screenWidth-1;

    int buffOffset = ((y * screenWidth) + x_start);
    fp* zb = &zBuffer[buffOffset];
    QRgb* fb = &frameBuffer[buffOffset];


    for(int i = x_start; i <= x_end; i++)
    {
        if(zPos < *zb)
        {
            *fb = color;
            *zb = zPos;
        }

        xFracScaled += inv_width;
        zb++;
        fb++;

        fp xFrac = xFracScaled / xFracScale;
        zPos = F3D::lerp(pos.z_left, pos.z_right, xFrac);

    }
}
