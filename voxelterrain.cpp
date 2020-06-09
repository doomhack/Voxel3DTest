 #include <QtMath>
#include "voxelterrain.h"
#include "object3d.h"

float XFovtoYFov(float xfov, float aspect)
{
    xfov = qDegreesToRadians(xfov);
    float yfov = 2.0 * qAtan(qTan(xfov * 0.5f) / aspect);

    return qRadiansToDegrees(yfov);
}

float YFovtoXFov(float yfov, float aspect)
{
    yfov = qDegreesToRadians(yfov);
    float xfov = 2.0 * qAtan(qTan(yfov * 0.5f) * aspect);

    return qRadiansToDegrees(xfov);
}

int VoxelTerrain::fracToY(float frac)
{
    float y1 = frac;
    y1 = y1 + 1.0;

    y1 = y1 / 2;

    y1 = 1.0-y1;

    return qRound(y1 * screenHeight);
}

int VoxelTerrain::fracToX(float frac)
{
    return qRound(((frac + 1.0)/2.0) * screenWidth);
}

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

    float aspect = (float)screenWidth/(float)screenHeight;
    float vfov = XFovtoYFov(90.0, aspect);

    //projectionMatrix.perspective(vfov, aspect, zNear, zFar);

    projectionMatrix.perspective(56, 1.9, zNear, zFar);


    Object3d* airport = new Object3d();
    airport->LoadFromFile(":/models/VRML/airport.obj", ":/models/VRML/airport.mtl");
    airport->pos.setY(-(128));
    objects.append(airport);

    Object3d* church = new Object3d();
    church->LoadFromFile(":/models/VRML/church.obj", ":/models/VRML/church.mtl");
    church->pos.setY(-(128));
    objects.append(church);

    Object3d* hotel = new Object3d();
    hotel->LoadFromFile(":/models/VRML/hotel.obj", ":/models/VRML/hotel.mtl");
    hotel->pos.setY(-(128));
    objects.append(hotel);

    Object3d* vilage = new Object3d();
    vilage->LoadFromFile(":/models/VRML/vilage.obj", ":/models/VRML/vilage.mtl");
    vilage->pos.setY(-(128));
    objects.append(vilage);

    Object3d* funfair = new Object3d();
    funfair->LoadFromFile(":/models/VRML/funfair.obj", ":/models/VRML/funfair.mtl");
    funfair->pos.setY(-(128));
    objects.append(funfair);

    Object3d* rock = new Object3d();
    rock->LoadFromFile(":/models/VRML/rock.obj", ":/models/VRML/rock.mtl");
    rock->pos.setY(-(128));
    objects.append(rock);

    /*
    Object3d* plane = new Object3d();
    plane->LoadFromFile(":/models/VRML/plane.obj", ":/models/VRML/plane.mtl");
    plane->pos.setY(-128);
    objects.append(plane);
    */
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
    viewMatrix.rotate(-zAngle, QVector3D(1,0,0));
    viewMatrix.rotate(qRadiansToDegrees(-cameraAngle), QVector3D(0,1,0));
    viewMatrix.translate(-cameraPos.x(), -viewHeight, -cameraPos.y());

    viewProjectionMatrix = projectionMatrix * viewMatrix;

    RecalculateZToY();
}

void VoxelTerrain::RecalculateZToY()
{
    zToY.clear();

    QMatrix4x4 zMatrix;
    zMatrix.rotate(-zAngle, QVector3D(1,0,0));
    zMatrix.translate(0.0, -viewHeight, 0.0);

    zMatrix = projectionMatrix * zMatrix;

    float zs = zStep;

    for(int i = zNear; i <= zFar; i+= zs)
    {
        QVector3D pt(0, -128, -i);

        pt = zMatrix * pt;


        if(pt.z() > 0.0 && pt.z() < 1.0)
        {
            int y = fracToY(pt.y());

            if(y >= 0)
                zToY[i] = y;
        }

        zs += zStepD;
    }
}

void VoxelTerrain::Render()
{
    BeginFrame();

    if(render3d)
    {
        Draw3d();
        //return;
    }

    float sinphi = qSin(cameraAngle);
    float cosphi = qCos(cameraAngle);

    float z = zNear;

    QList<float> zSteps = zToY.keys();

    for(int w = 0; w < zSteps.length(); w++)
    {
        z = zSteps[w];
        int yPos = zToY[z];

        QPointF pleft = QPoint((-cosphi*z - sinphi*z) + cameraPos.x(), ( sinphi*z - cosphi*z) + cameraPos.y());
        QPointF pright = QPoint(( cosphi*z - sinphi*z) + cameraPos.x(), (-sinphi*z - cosphi*z) + cameraPos.y());

        float dx = (pright.x() - pleft.x()) / (float)screenWidth;
        float dy = (pright.y() - pleft.y()) / (float)screenWidth;

        float invz = (1.0 / z);
        float invh = invz * heightScale;

        float zDepth = 1.0-invz;

        for(int i = 0; i < screenWidth; i++)
        {
            float pointHeight = 0;
            QRgb lineColor = qRgb(0,0,128);

            if((pleft.x() < heightMap.width() && pleft.x() >= 0) && (pleft.y() < heightMap.height() && pleft.y() >= 0))
            {

                if(qAlpha(colorMap.pixel(pleft.x(),pleft.y())) == 255)
                {
                    pointHeight = qGray(heightMap.pixel(pleft.x(), pleft.y()));

                    lineColor = colorMap.pixel(pleft.x(),pleft.y());
                }
            }

            int hDiff = qRound((pointHeight * invh));

            int lineHeight = yPos - hDiff;

            if(lineHeight < 0)
                lineHeight = 0;
            else if(lineHeight >= screenHeight)
                lineHeight = screenHeight;

            for(int y = lineHeight; y < yBuffer[i]; y++)
            {
                frameBuffer.setPixel(i, y, lineColor);
                zBuffer[(y * screenWidth) + i] = zDepth;
            }

            if (lineHeight < yBuffer[i])
                yBuffer[i] = lineHeight;

            pleft += QPointF(dx, dy);
        }

        z += zStep;
    }

    if(render3d)
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
    QVector3D p = transformMatrix * vertex->pos;

    Vertex3d screenspace;

    float z = p.z();

    if(z < 0.0)
        z = 0;
    else if(z > 1.0)
        z = 1.0;


    screenspace.pos = QVector3D
    (
        fracToX(p.x()),
        fracToY(p.y()),
        z
    );

    screenspace.uv = vertex->uv;

    return screenspace;
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
            ((screenSpacePoints[0].pos.z() >= 1) || (screenSpacePoints[0].pos.z() <= 0)) ||
            ((screenSpacePoints[1].pos.z() >= 1) || (screenSpacePoints[1].pos.z() <= 0)) ||
            ((screenSpacePoints[2].pos.z() >= 1) || (screenSpacePoints[2].pos.z() <= 0))
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
        //Now we split the polygon into two triangles.
        //A flat top and flat bottom triangle.

        //How far down between vx0 -> vx2 are we spliting?
        float splitFrac = (points[1].pos.y() - points[0].pos.y()) / (points[2].pos.y() - points[0].pos.y());

        //Interpolate new values for new vertex.

        //x pos
        int v4x = qRound(points[0].pos.x() + (splitFrac * (points[2].pos.x() - points[0].pos.x())));

        //z-depth
        float v4z = points[0].pos.z() + (splitFrac * (points[2].pos.z() - points[0].pos.z()));

        //uv coords.
        float v4u = points[0].uv.x() + (splitFrac * (points[2].uv.x() - points[0].uv.x()));
        float v4v = points[0].uv.y() + (splitFrac * (points[2].uv.y() - points[0].uv.y()));

        Vertex3d triangle[3];

        Vertex3d p4;
        p4.pos = QVector3D(v4x, points[1].pos.y(), v4z);
        p4.uv = QVector2D(v4u, v4v);

        triangle[0] = points[0];
        triangle[1] = points[1];
        triangle[2] = p4;

        DrawTriangleTop(triangle, texture, color);

        triangle[0] = points[1];
        triangle[1] = p4;
        triangle[2] = points[2];

        DrawTriangleBottom(triangle, texture, color);
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
    float inv_height = (1.0/(points[1].pos.y() - points[0].pos.y()));

    TriEdgeTrace step;
    TriEdgeTrace pos;

    step.x_left     = (points[2].pos.x() - points[0].pos.x()) * inv_height;
    step.x_right    = (points[1].pos.x() - points[0].pos.x()) * inv_height;

    step.z_left     = (points[2].pos.z() - points[0].pos.z()) * inv_height;
    step.z_right    = (points[1].pos.z() - points[0].pos.z()) * inv_height;

    step.u_left     = (points[2].uv.x() - points[0].uv.x()) * inv_height;
    step.u_right    = (points[1].uv.x() - points[0].uv.x()) * inv_height;

    step.v_left     = (points[2].uv.y() - points[0].uv.y()) * inv_height;
    step.v_right    = (points[1].uv.y() - points[0].uv.y()) * inv_height;

    if(step.x_left > step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[0].pos.x();
    pos.z_left = pos.z_right = points[0].pos.z();
    pos.u_left = pos.u_right = points[0].uv.x();
    pos.v_left = pos.v_right = points[0].uv.y();

    int yStart = qRound(points[0].pos.y());
    int yEnd = qRound(points[1].pos.y());

    for (int y = yStart; y <= yEnd; y++)
    {
        if(y >= screenHeight)
            break;

        if(y > 0)
        {
            DrawTriangleScanline(y, pos, texture, color);
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

void VoxelTerrain::DrawTriangleBottom(Vertex3d points[], QImage* texture, QRgb color)
{
    float inv_height = (1.0/(points[2].pos.y() - points[0].pos.y()));

    TriEdgeTrace step;
    TriEdgeTrace pos;

    step.x_left     = (points[2].pos.x() - points[0].pos.x()) * inv_height;
    step.x_right    = (points[2].pos.x() - points[1].pos.x()) * inv_height;

    step.z_left     = (points[2].pos.z() - points[0].pos.z()) * inv_height;
    step.z_right    = (points[2].pos.z() - points[1].pos.z()) * inv_height;

    step.u_left     = (points[2].uv.x() - points[0].uv.x()) * inv_height;
    step.u_right    = (points[2].uv.x() - points[1].uv.x()) * inv_height;

    step.v_left     = (points[2].uv.y() - points[0].uv.y()) * inv_height;
    step.v_right    = (points[2].uv.y() - points[1].uv.y()) * inv_height;

    if(step.x_left < step.x_right)
    {
        qSwap(step.x_left, step.x_right);
        qSwap(step.z_left, step.z_right);
        qSwap(step.u_left, step.u_right);
        qSwap(step.v_left, step.v_right);
    }

    pos.x_left = pos.x_right = points[2].pos.x();
    pos.z_left = pos.z_right = points[2].pos.z();
    pos.u_left = pos.u_right = points[2].uv.x();
    pos.v_left = pos.v_right = points[2].uv.y();

    int yStart = qRound(points[2].pos.y());
    int yEnd = qRound(points[0].pos.y());


    for (int y = yStart; y > yEnd; y--)
    {
        if(y < 0)
            break;

        if(y < screenHeight)
        {
            DrawTriangleScanline(y, pos, texture, color);
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

void VoxelTerrain::DrawTriangleScanline(int y, TriEdgeTrace& pos, QImage* texture, QRgb color)
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
                if(texture)
                {
                    int tx = qRound(sl_pos.u * texture->width());
                    int ty = qRound((1.0-sl_pos.v) * texture->height());

                    tx = tx & (texture->width() - 1);
                    ty = ty & (texture->height() - 1);

                    frameBuffer.setPixel(x, y, texture->pixel(tx, ty));
                }
                else
                {
                    frameBuffer.setPixel(x, y, color);
                }

                zBuffer[ (y*screenWidth) + x] = sl_pos.z;
            }
        }

        sl_pos.z += sl_step.z;
        sl_pos.u += sl_step.u;
        sl_pos.v += sl_step.v;
    }
}
