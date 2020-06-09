#include "object3d.h"


bool Object3d::LoadFromFile(QString objFile, QString mtlFile)
{
    this->pos.setX(1024);
    this->pos.setZ(1024);

    QFile f(objFile);

    f.open(QFile::ReadOnly);

    QString objFileText = f.readAll();

    f.close();

    QFile f2(mtlFile);

    f2.open(QFile::ReadOnly);

    QString mtlFileText = f2.readAll();

    f2.close();

    QStringList mtlLines = mtlFileText.split("\n");

    QString currMtlName;

    QMap<QString, QImage*> textureMap;
    QMap<QString, QRgb> textureColors;


    for(int i = 0; i < mtlLines.length(); i++)
    {
        QString line = mtlLines.at(i);

        QStringList elements = line.split(" ");

        if(elements[0] == "#")
            continue; //Comment

        if(elements[0] == "newmtl")
        {
            currMtlName = elements[1];
        }

        if(elements[0] == "Kd")
        {
            float r = qRound(elements[1].toFloat() * 255);
            float g = qRound(elements[1].toFloat() * 255);
            float b = qRound(elements[1].toFloat() * 255);

            if(currMtlName.length())
            {
                textureColors[currMtlName] = qRgb(r,g,b);
            }
        }

        if(elements[0] == "map_Kd")
        {
            QString fileName = elements[1];

            QStringList parts = fileName.split("\\");
            QString lastBit = parts.at(parts.size()-1);

            if(currMtlName.length())
            {
                QImage* image = new QImage();

                image->load(":/models/VRML/" + lastBit);

                if(!image->isNull())
                    textureMap[currMtlName] = image;

                currMtlName.clear();
            }
        }
    }






    QStringList lines = objFileText.split("\n");

    QList<QVector3D> vertexes;
    QList<QVector2D> uvs;


    Mesh3d* currentMesh = new Mesh3d();
    currentMesh->color = Qt::lightGray;



    for(int i = 0; i < lines.length(); i++)
    {
        QString line = lines.at(i);

        QStringList elements = line.split(" ");

        if(elements[0] == "#")
            continue; //Comment

        //Vertex
        if(elements[0] == "v")
        {
            float x = elements[1].toFloat();
            float y = elements[2].toFloat();
            float z = elements[3].toFloat();

            vertexes.append(QVector3D(x, y, z));
        }

        if(elements[0] == "vt")
        {
            float u = elements[1].toFloat();
            float v = elements[2].toFloat();

            uvs.append(QVector2D(u, v));
        }

        //Face
        if(elements[0] == "f")
        {
            Triangle3d t3d;

            for(int t = 0; t < 3; t++)
            {
                QString tri = elements[t+1];

                QStringList vtx_elelments = tri.split("/");

                t3d.verts[t].pos = vertexes.at(vtx_elelments[0].toInt() - 1);
                t3d.verts[t].uv = uvs.at(vtx_elelments[1].toInt() - 1);
            }

            currentMesh->tris.append(t3d);
        }

        if(elements[0] == "usemtl")
        {
            if(currentMesh->tris.length())
            {
                //Start new mesh when texture changes.
                this->mesh.append(currentMesh);
                currentMesh = new Mesh3d();
                currentMesh->color = Qt::lightGray;
            }

            currentMesh->texture = textureMap.value(elements[1]);
            currentMesh->color = textureColors.value(elements[1]);
        }
    }

    if(currentMesh->tris.length())
        this->mesh.append(currentMesh);

    return true;
}
