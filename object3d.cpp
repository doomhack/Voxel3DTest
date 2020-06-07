#include "object3d.h"


bool Object3d::LoadFromFile(QString filename)
{
    this->pos.setX(1024);
    this->pos.setZ(1024);
    this->pos.setY(-136);

    QFile f(filename);

    f.open(QFile::ReadOnly);

    QString fileText = f.readAll();

    f.close();

    QStringList lines = fileText.split("\n");

    QList<QVector3D> vertexes;

    Mesh3d mesh;

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

        //Face
        if(elements[0] == "f")
        {
            Triangle3d t3d;

            for(int t = 0; t < 3; t++)
            {
                QString tri = elements[t+1];

                QStringList verts = tri.split("/");

                t3d.verts[t].pos = vertexes.at(verts[0].toInt() - 1);
            }

            mesh.tris.append(t3d);
        }
    }

    this->mesh.append(mesh);

    return true;
}
