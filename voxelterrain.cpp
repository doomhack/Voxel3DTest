#include <limits>

#include <QtMath>
#include "voxelterrain.h"
#include "object3d.h"

#include "3dmaths/f3dmath.h"

fp VoxelTerrain::lerp(fp a, fp b, fp frac)
{
    fp ifrac = fp(1) - frac;

    return (a * ifrac) + (b * frac);
}

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

            newVx.pos.x = lerp(clipSpacePoints[i].pos.x, clipSpacePoints[i2].pos.x, frac);
            newVx.pos.y = lerp(clipSpacePoints[i].pos.y, clipSpacePoints[i2].pos.y, frac);
            newVx.pos.z = lerp(clipSpacePoints[i].pos.z, clipSpacePoints[i2].pos.z, frac);
            newVx.pos.w = wClip;

            newVx.uv.x = lerp(clipSpacePoints[i].uv.x, clipSpacePoints[i2].uv.x, frac);
            newVx.uv.y = lerp(clipSpacePoints[i].uv.y, clipSpacePoints[i2].uv.y, frac);

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
        triangle[2].pos.x = lerp(points[0].pos.x, points[2].pos.x, splitFrac);
        triangle[2].pos.y = points[1].pos.y;
        triangle[2].pos.z = lerp(points[0].pos.z, points[2].pos.z, splitFrac);
        triangle[2].pos.w = lerp(points[0].pos.w, points[2].pos.w, splitFrac);

        //uv coords.
        triangle[2].uv.x = lerp(points[0].uv.x, points[2].uv.x, splitFrac);
        triangle[2].uv.y = lerp(points[0].uv.y, points[2].uv.y, splitFrac);

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
        triangle[2].pos.x = lerp(points[0].pos.x, points[2].pos.x, splitFrac);
        triangle[2].pos.y = points[1].pos.y;
        triangle[2].pos.z = lerp(points[0].pos.z, points[2].pos.z, splitFrac);

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
    TriEdgeTrace step;
    TriEdgeTrace pos;

    fp inv_height = (fp(1)/(points[1].pos.y - points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[1].pos.x - points[0].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[1].pos.z - points[0].pos.z) * inv_height;

    step.w_left     = (points[2].pos.w - points[0].pos.w) * inv_height;
    step.w_right    = (points[1].pos.w - points[0].pos.w) * inv_height;

    step.u_left     = (points[2].uv.x - points[0].uv.x) * inv_height;
    step.u_right    = (points[1].uv.x - points[0].uv.x) * inv_height;

    step.v_left     = (points[2].uv.y - points[0].uv.y) * inv_height;
    step.v_right    = (points[1].uv.y - points[0].uv.y) * inv_height;

    if(step.x_left > step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.w_left, step.w_right);

        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[0].pos.x;
    pos.z_left = pos.z_right = points[0].pos.z;
    pos.w_left = pos.w_right = points[0].pos.w;

    pos.u_left = pos.u_right = points[0].uv.x;
    pos.v_left = pos.v_right = points[0].uv.y;

    int yStart = points[0].pos.y;
    int yEnd = points[1].pos.y;

    for (int y = yStart; y <= yEnd; y++)
    {
        if(y >= screenHeight)
            break;

        if(y > 0)
        {
            DrawTriangleScanline(y, pos, texture);
        }

        pos.x_left  += step.x_left;
        pos.x_right += step.x_right;

        pos.z_left  += step.z_left;
        pos.z_right += step.z_right;

        pos.w_left  += step.w_left;
        pos.w_right += step.w_right;

        pos.u_left  += step.u_left;
        pos.u_right += step.u_right;

        pos.v_left  += step.v_left;
        pos.v_right += step.v_right;
    }
}

void VoxelTerrain::DrawTriangleTop(Vertex2d points[], QRgb color)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    fp inv_height = (fp(1)/(points[1].pos.y - points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[1].pos.x - points[0].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[1].pos.z - points[0].pos.z) * inv_height;

    if(step.x_left > step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
    }

    pos.x_left = pos.x_right = points[0].pos.x;
    pos.z_left = pos.z_right = points[0].pos.z;

    int yStart = points[0].pos.y;
    int yEnd = points[1].pos.y;

    for (int y = yStart; y <= yEnd; y++)
    {
        if(y >= screenHeight)
            break;

        if(y > 0)
        {
            DrawTriangleScanline(y, pos, color);
        }

        pos.x_left  += step.x_left;
        pos.x_right += step.x_right;

        pos.z_left  += step.z_left;
        pos.z_right += step.z_right;
    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex2d points[], Texture* texture)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    fp inv_height = (fp(1)/(points[2].pos.y - points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[2].pos.x - points[1].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[2].pos.z - points[1].pos.z) * inv_height;

    step.w_left     = (points[2].pos.w - points[0].pos.w) * inv_height;
    step.w_right    = (points[2].pos.w - points[1].pos.w) * inv_height;

    step.u_left     = (points[2].uv.x - points[0].uv.x) * inv_height;
    step.u_right    = (points[2].uv.x - points[1].uv.x) * inv_height;

    step.v_left     = (points[2].uv.y - points[0].uv.y) * inv_height;
    step.v_right    = (points[2].uv.y - points[1].uv.y) * inv_height;

    if(step.x_left < step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.w_left, step.w_right);
        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[2].pos.x;
    pos.z_left = pos.z_right = points[2].pos.z;
    pos.w_left = pos.w_right = points[2].pos.w;

    pos.u_left = pos.u_right = points[2].uv.x;
    pos.v_left = pos.v_right = points[2].uv.y;

    int yStart = points[2].pos.y;
    int yEnd = points[0].pos.y;


    for (int y = yStart; y > yEnd; y--)
    {
        if(y < 0)
            break;

        if(y < screenHeight)
        {
            DrawTriangleScanline(y, pos, texture);
        }

        pos.x_left  -= step.x_left;
        pos.x_right -= step.x_right;

        pos.z_left  -= step.z_left;
        pos.z_right -= step.z_right;

        pos.w_left  -= step.w_left;
        pos.w_right -= step.w_right;

        pos.u_left  -= step.u_left;
        pos.u_right -= step.u_right;

        pos.v_left  -= step.v_left;
        pos.v_right -= step.v_right;

    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex2d points[], QRgb color)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    fp inv_height = (fp(1)/(points[2].pos.y- points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[2].pos.x - points[1].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[2].pos.z - points[1].pos.z) * inv_height;

    if(step.x_left < step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
    }

    pos.x_left = pos.x_right = points[2].pos.x;
    pos.z_left = pos.z_right = points[2].pos.z;

    int yStart = points[2].pos.y;
    int yEnd = points[0].pos.y;


    for (int y = yStart; y > yEnd; y--)
    {
        if(y < 0)
            break;

        if(y < screenHeight)
        {
            DrawTriangleScanline(y, pos, color);
        }

        pos.x_left  -= step.x_left;
        pos.x_right -= step.x_right;

        pos.z_left  -= step.z_left;
        pos.z_right -= step.z_right;
    }
}


void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, Texture* texture)
{
    TriDrawPos sl_step;
    TriDrawPos sl_pos;

    if(pos.x_left < pos.x_right)
    {
        fp inv_width = fp(1)/(pos.x_right - pos.x_left);

        sl_step.w = (pos.w_right - pos.w_left) * inv_width;
        sl_step.z = (pos.z_right - pos.z_left) * inv_width;
        sl_step.u = (pos.u_right - pos.u_left) * inv_width;
        sl_step.v = (pos.v_right - pos.v_left) * inv_width;
    }

    sl_pos.w = pos.w_left;
    sl_pos.z = pos.z_left;
    sl_pos.u = pos.u_left;
    sl_pos.v = pos.v_left;

    int x_start = pos.x_left;
    int x_end = pos.x_right;

    for(int x = x_start; x <= x_end; x++)
    {
        if(x >= screenWidth)
            return;

        if(x >= 0)
        {
            fp prevz = zBuffer[ (y*screenWidth) + x];

            if(sl_pos.z < prevz)
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

                frameBuffer[y*screenWidth + x] = texture->pixels[ty * texture->width + tx];
                zBuffer[ (y*screenWidth) + x] = sl_pos.z;
            }
        }

        sl_pos.w += sl_step.w;
        sl_pos.z += sl_step.z;
        sl_pos.u += sl_step.u;
        sl_pos.v += sl_step.v;
    }
}

void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, QRgb color)
{
    TriDrawPos sl_step;
    TriDrawPos sl_pos;

    if(pos.x_left < pos.x_right)
    {
        fp inv_width = fp(1)/(pos.x_right - pos.x_left);

        sl_step.z = (pos.z_right - pos.z_left) * inv_width;
    }

    sl_pos.z = pos.z_left;

    int x_start = pos.x_left;
    int x_end = pos.x_right;

    for(int x = x_start; x <= x_end; x++)
    {
        if(x >= screenWidth)
            return;

        if(x >= 0)
        {   
            fp prevz = zBuffer[ (y*screenWidth) + x];

            if(sl_pos.z < prevz)
            {
                frameBuffer[y*screenWidth + x] = color;
                zBuffer[ (y*screenWidth) + x] = sl_pos.z;
            }
        }

        sl_pos.z += sl_step.z;
    }
}
