 #include <QtMath>
#include "voxelterrain.h"
#include "object3d.h"

#include "3dmaths/f3dmath.h"

int VoxelTerrain::fracToY(float frac)
{
    float y = 1.0-((frac + 1.0) / 2.0);

    return qRound(y * screenHeight);
}

int VoxelTerrain::fracToX(float frac)
{
    float x = (frac + 1.0) / 2.0;

    return qRound(x * screenWidth);
}

VoxelTerrain::VoxelTerrain(QObject *parent) : QObject(parent)
{
    cameraPos = F3D::V2F(mapSize/2,mapSize/2);
    cameraAngle = 0;
    cameraHeight = 50;

    frameBufferImage = QImage(screenWidth, screenHeight, QImage::Format::Format_RGB32);
    frameBuffer = (QRgb*)frameBufferImage.bits();

    heightMapImage.load(":/images/D1.png");
    heightMapImage = heightMapImage.convertToFormat(QImage::Format::Format_Grayscale8);

    heightMap = heightMapImage.bits();

    colorMapImage.load(":/images/C1W.png");
    colorMapImage = colorMapImage.convertToFormat(QImage::Format::Format_ARGB32);

    colorMap = (const QRgb*)colorMapImage.bits();

    zBuffer = new float[screenWidth * screenHeight];

    //55.7954
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(54, 1.888888888, zNear, zFar);


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
}

void VoxelTerrain::BeginFrame()
{
    viewHeight = cameraHeight;

    int xpos = qRound(cameraPos.x);
    int ypos = qRound(cameraPos.y);

    if(xpos < mapSize && ypos < mapSize && xpos >= 0 && ypos >= 0)
    {
        viewHeight = heightMap[ypos * mapSize + xpos] + cameraHeight;
    }

    frameBufferImage.fill(Qt::cyan);

    std::fill_n(zBuffer, screenWidth*screenHeight, 1.0);
    std::fill_n(yBuffer, screenWidth, screenHeight);


    viewMatrix.setToIdentity();
    viewMatrix.rotate(-zAngle, 1,0,0);
    viewMatrix.rotate(qRadiansToDegrees(-cameraAngle), 0,1,0);
    viewMatrix.translate(F3D::V3F(-cameraPos.x, -viewHeight, -cameraPos.y));

    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void VoxelTerrain::Render()
{
    BeginFrame();

    float sinphi = qSin(cameraAngle);
    float cosphi = qCos(cameraAngle);

    float cZstep = zStep;

    for(float z = zNear; z < zFar; z += cZstep, cZstep += zStepD)
    {
        F3D::V2F pleft = F3D::V2F((-cosphi*z - sinphi*z) + cameraPos.x, ( sinphi*z - cosphi*z) + cameraPos.y);
        F3D::V2F pright = F3D::V2F(( cosphi*z - sinphi*z) + cameraPos.x, (-sinphi*z - cosphi*z) + cameraPos.y);

        float dx = (pright.x - pleft.x) / (float)screenWidth;
        float dy = (pright.y - pleft.y) / (float)screenWidth;

        float invz = (1.0 / z);

        float invh = invz * heightScale;


        int yPos = (viewHeight * (invz * yScale)) + (screenHeight / 2);

        float zDepth = 1.0-invz;

        for(int i = 0; i < screenWidth; i++)
        {
            float pointHeight = 0;
            QRgb lineColor = qRgb(0,0,128);

            if((pleft.x < mapSize && pleft.x >= 0) && (pleft.y < mapSize && pleft.y >= 0))
            {
                const int pixelOffs = (int)pleft.y*2048 + (int)pleft.x;

                QRgb color = colorMap[pixelOffs];

                if(qAlpha(color) == 255)
                {
                    pointHeight = heightMap[pixelOffs];

                    lineColor = color;
                }
            }

            int hDiff = qRound((pointHeight * invh));

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

            pleft += F3D::V2F(dx, dy);
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

Vertex3d VoxelTerrain::TransformVertex(const Vertex3d* vertex)
{
    F3D::V3F p = transformMatrix * vertex->pos;

    Vertex3d screenspace;

    screenspace.pos = F3D::V3F
    (
        fracToX(p.x),
        fracToY(p.y),
        p.z
    );

    screenspace.uv = vertex->uv;

    return screenspace;
}


void VoxelTerrain::DrawTriangle(const Triangle3d* tri, Texture* texture, QRgb color)
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

    if(texture)
        DrawTransformedTriangle(transformedPoints, texture);
    else
        DrawTransformedTriangle(transformedPoints, color);
}

bool VoxelTerrain::IsTriangleFrontface(Vertex3d screenSpacePoints[3])
{
    F3D::V3F edge1 = screenSpacePoints[0].pos - screenSpacePoints[1].pos;
    F3D::V3F edge2 = screenSpacePoints[1].pos - screenSpacePoints[2].pos;

    float normal = edge1.CrossProductZ(edge2);

    return normal > 0;
}

bool VoxelTerrain::IsTriangleOnScreen(Vertex3d screenSpacePoints[3])
{

    if  (
            ((screenSpacePoints[0].pos.z >= 1) || (screenSpacePoints[0].pos.z <= 0)) ||
            ((screenSpacePoints[1].pos.z >= 1) || (screenSpacePoints[1].pos.z <= 0)) ||
            ((screenSpacePoints[2].pos.z >= 1) || (screenSpacePoints[2].pos.z <= 0))
         )
        return false;

    if  (
            (screenSpacePoints[0].pos.x < 0) &&
            (screenSpacePoints[1].pos.x < 0) &&
            (screenSpacePoints[2].pos.x < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.x > screenWidth) &&
            (screenSpacePoints[1].pos.x > screenWidth) &&
            (screenSpacePoints[2].pos.x > screenWidth)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.y < 0) &&
            (screenSpacePoints[1].pos.y < 0) &&
            (screenSpacePoints[2].pos.y < 0)
        )
        return false;

    if  (
            (screenSpacePoints[0].pos.y > screenHeight) &&
            (screenSpacePoints[1].pos.y > screenHeight) &&
            (screenSpacePoints[2].pos.y > screenHeight)
        )
        return false;

    return true;
}

void VoxelTerrain::DrawTransformedTriangle(Vertex3d points[3], Texture* texture)
{
    SortPointsByY(points);

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
        float splitFrac = (points[1].pos.y - points[0].pos.y) / (points[2].pos.y - points[0].pos.y);

        //Interpolate new values for new vertex.

        //x pos
        int v4x = qRound(points[0].pos.x + (splitFrac * (points[2].pos.x - points[0].pos.x)));

        //z-depth
        float v4z = points[0].pos.z + (splitFrac * (points[2].pos.z - points[0].pos.z));

        //uv coords.
        float v4u = points[0].uv.x + (splitFrac * (points[2].uv.x - points[0].uv.x));
        float v4v = points[0].uv.y + (splitFrac * (points[2].uv.y - points[0].uv.y));

        Vertex3d triangle[4];

        Vertex3d p4;
        p4.pos = F3D::V3F(v4x, points[1].pos.y, v4z);
        p4.uv = F3D::V2F(v4u, v4v);

        triangle[0] = points[0];
        triangle[1] = points[1];
        triangle[2] = p4;
        triangle[3] = points[2];

        DrawTriangleTop(triangle, texture);

        DrawTriangleBottom(&triangle[1], texture);
    }
}

void VoxelTerrain::DrawTransformedTriangle(Vertex3d points[3], QRgb color)
{
    SortPointsByY(points);

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
        float splitFrac = (points[1].pos.y - points[0].pos.y) / (points[2].pos.y - points[0].pos.y);

        //Interpolate new values for new vertex.

        //x pos
        int v4x = qRound(points[0].pos.x + (splitFrac * (points[2].pos.x - points[0].pos.x)));

        //z-depth
        float v4z = points[0].pos.z + (splitFrac * (points[2].pos.z - points[0].pos.z));

        Vertex3d triangle[4];

        Vertex3d p4;
        p4.pos = F3D::V3F(v4x, points[1].pos.y, v4z);

        triangle[0] = points[0];
        triangle[1] = points[1];
        triangle[2] = p4;
        triangle[3] = points[2];

        DrawTriangleTop(triangle, color);

        DrawTriangleBottom(&triangle[1], color);
    }
}

void VoxelTerrain::SortPointsByY(Vertex3d points[3])
{
    if(points[0].pos.y > points[1].pos.y)
        qSwap(points[0], points[1]);

    if(points[0].pos.y > points[2].pos.y)
        qSwap(points[0], points[2]);

    if(points[1].pos.y > points[2].pos.y)
        qSwap(points[1], points[2]);
}



void VoxelTerrain::DrawTriangleTop(Vertex3d points[3], Texture* texture)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    float inv_height = (1.0/(points[1].pos.y - points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[1].pos.x - points[0].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[1].pos.z - points[0].pos.z) * inv_height;

    step.u_left     = (points[2].uv.x - points[0].uv.x) * inv_height;
    step.u_right    = (points[1].uv.x - points[0].uv.x) * inv_height;

    step.v_left     = (points[2].uv.y - points[0].uv.y) * inv_height;
    step.v_right    = (points[1].uv.y - points[0].uv.y) * inv_height;

    if(step.x_left > step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[0].pos.x;
    pos.z_left = pos.z_right = points[0].pos.z;
    pos.u_left = pos.u_right = points[0].uv.x;
    pos.v_left = pos.v_right = points[0].uv.y;

    int yStart = qRound(points[0].pos.y);
    int yEnd = qRound(points[1].pos.y);

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

        pos.u_left  += step.u_left;
        pos.u_right += step.u_right;

        pos.v_left  += step.v_left;
        pos.v_right += step.v_right;
    }
}

void VoxelTerrain::DrawTriangleTop(Vertex3d points[3], QRgb color)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    float inv_height = (1.0/(points[1].pos.y - points[0].pos.y));

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

    int yStart = qRound(points[0].pos.y);
    int yEnd = qRound(points[1].pos.y);

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

void VoxelTerrain::DrawTriangleBottom(Vertex3d points[], Texture* texture)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    float inv_height = (1.0/(points[2].pos.y - points[0].pos.y));

    step.x_left     = (points[2].pos.x - points[0].pos.x) * inv_height;
    step.x_right    = (points[2].pos.x - points[1].pos.x) * inv_height;

    step.z_left     = (points[2].pos.z - points[0].pos.z) * inv_height;
    step.z_right    = (points[2].pos.z - points[1].pos.z) * inv_height;

    step.u_left     = (points[2].uv.x - points[0].uv.x) * inv_height;
    step.u_right    = (points[2].uv.x - points[1].uv.x) * inv_height;

    step.v_left     = (points[2].uv.y - points[0].uv.y) * inv_height;
    step.v_right    = (points[2].uv.y - points[1].uv.y) * inv_height;

    if(step.x_left < step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[2].pos.x;
    pos.z_left = pos.z_right = points[2].pos.z;
    pos.u_left = pos.u_right = points[2].uv.x;
    pos.v_left = pos.v_right = points[2].uv.y;

    int yStart = qRound(points[2].pos.y);
    int yEnd = qRound(points[0].pos.y);


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

        pos.u_left  -= step.u_left;
        pos.u_right -= step.u_right;

        pos.v_left  -= step.v_left;
        pos.v_right -= step.v_right;

    }
}

void VoxelTerrain::DrawTriangleBottom(Vertex3d points[], QRgb color)
{
    TriEdgeTrace step;
    TriEdgeTrace pos;

    float inv_height = (1.0/(points[2].pos.y- points[0].pos.y));

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

    int yStart = qRound(points[2].pos.y);
    int yEnd = qRound(points[0].pos.y);


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

    float inv_width = 1.0/(pos.x_right - pos.x_left);

    sl_step.z = (pos.z_right - pos.z_left) * inv_width;
    sl_step.u = (pos.u_right - pos.u_left) * inv_width;
    sl_step.v = (pos.v_right - pos.v_left) * inv_width;

    sl_pos.z = pos.z_left;
    sl_pos.u = pos.u_left;
    sl_pos.v = pos.v_left;

    for(int x = pos.x_left; x <= pos.x_right; x++)
    {
        if(x >= screenWidth)
            return;

        if(x >= 0)
        {
            float prevz = zBuffer[ (y*screenWidth) + x];

            if(sl_pos.z < prevz)
            {
                int tx = qRound(sl_pos.u * texture->width);
                int ty = qRound((1.0-sl_pos.v) * texture->height);

                tx = tx & (texture->width - 1);
                ty = ty & (texture->height - 1);

                frameBuffer[y*screenWidth + x] = texture->pixels[ty * texture->width + tx];
                zBuffer[ (y*screenWidth) + x] = sl_pos.z;
            }
        }

        sl_pos.z += sl_step.z;
        sl_pos.u += sl_step.u;
        sl_pos.v += sl_step.v;
    }
}

void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, QRgb color)
{
    TriDrawPos sl_step;
    TriDrawPos sl_pos;

    float inv_width = 1.0/(pos.x_right - pos.x_left);

    sl_step.z = (pos.z_right - pos.z_left) * inv_width;
    sl_pos.z = pos.z_left;

    for(int x = pos.x_left; x <= pos.x_right; x++)
    {
        if(x >= screenWidth)
            return;

        if(x >= 0)
        {
            float prevz = zBuffer[ (y*screenWidth) + x];

            if(sl_pos.z < prevz)
            {
                frameBuffer[y*screenWidth + x] = color;
                zBuffer[ (y*screenWidth) + x] = sl_pos.z;
            }
        }

        sl_pos.z += sl_step.z;
    }
}
