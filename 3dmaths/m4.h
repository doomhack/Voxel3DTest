#ifndef M4_H
#define M4_H


#include "v3.h"

#define M_PI (3.14159265358979323846)

template <class T> class M4
{
public:
    explicit M4() {}

    void setToIdentity()
    {
        m[0][0] = 1;
        m[0][1] = 0;
        m[0][2] = 0;
        m[0][3] = 0;
        m[1][0] = 0;
        m[1][1] = 1;
        m[1][2] = 0;
        m[1][3] = 0;
        m[2][0] = 0;
        m[2][1] = 0;
        m[2][2] = 1;
        m[2][3] = 0;
        m[3][0] = 0;
        m[3][1] = 0;
        m[3][2] = 0;
        m[3][3] = 1;
    }

    M4& operator+=(const M4& other)
    {
        m[0][0] += other.m[0][0];
        m[0][1] += other.m[0][1];
        m[0][2] += other.m[0][2];
        m[0][3] += other.m[0][3];
        m[1][0] += other.m[1][0];
        m[1][1] += other.m[1][1];
        m[1][2] += other.m[1][2];
        m[1][3] += other.m[1][3];
        m[2][0] += other.m[2][0];
        m[2][1] += other.m[2][1];
        m[2][2] += other.m[2][2];
        m[2][3] += other.m[2][3];
        m[3][0] += other.m[3][0];
        m[3][1] += other.m[3][1];
        m[3][2] += other.m[3][2];
        m[3][3] += other.m[3][3];
        return *this;
    }

    M4& operator-=(const M4& other)
    {
        m[0][0] -= other.m[0][0];
        m[0][1] -= other.m[0][1];
        m[0][2] -= other.m[0][2];
        m[0][3] -= other.m[0][3];
        m[1][0] -= other.m[1][0];
        m[1][1] -= other.m[1][1];
        m[1][2] -= other.m[1][2];
        m[1][3] -= other.m[1][3];
        m[2][0] -= other.m[2][0];
        m[2][1] -= other.m[2][1];
        m[2][2] -= other.m[2][2];
        m[2][3] -= other.m[2][3];
        m[3][0] -= other.m[3][0];
        m[3][1] -= other.m[3][1];
        m[3][2] -= other.m[3][2];
        m[3][3] -= other.m[3][3];
        return *this;
    }

    M4& operator*=(const M4& other)
    {
        T m0, m1, m2;
        m0 = m[0][0] * other.m[0][0]
                + m[1][0] * other.m[0][1]
                + m[2][0] * other.m[0][2]
                + m[3][0] * other.m[0][3];
        m1 = m[0][0] * other.m[1][0]
                + m[1][0] * other.m[1][1]
                + m[2][0] * other.m[1][2]
                + m[3][0] * other.m[1][3];
        m2 = m[0][0] * other.m[2][0]
                + m[1][0] * other.m[2][1]
                + m[2][0] * other.m[2][2]
                + m[3][0] * other.m[2][3];
        m[3][0] = m[0][0] * other.m[3][0]
                + m[1][0] * other.m[3][1]
                + m[2][0] * other.m[3][2]
                + m[3][0] * other.m[3][3];
        m[0][0] = m0;
        m[1][0] = m1;
        m[2][0] = m2;

        m0 = m[0][1] * other.m[0][0]
                + m[1][1] * other.m[0][1]
                + m[2][1] * other.m[0][2]
                + m[3][1] * other.m[0][3];
        m1 = m[0][1] * other.m[1][0]
                + m[1][1] * other.m[1][1]
                + m[2][1] * other.m[1][2]
                + m[3][1] * other.m[1][3];
        m2 = m[0][1] * other.m[2][0]
                + m[1][1] * other.m[2][1]
                + m[2][1] * other.m[2][2]
                + m[3][1] * other.m[2][3];
        m[3][1] = m[0][1] * other.m[3][0]
                + m[1][1] * other.m[3][1]
                + m[2][1] * other.m[3][2]
                + m[3][1] * other.m[3][3];
        m[0][1] = m0;
        m[1][1] = m1;
        m[2][1] = m2;

        m0 = m[0][2] * other.m[0][0]
                + m[1][2] * other.m[0][1]
                + m[2][2] * other.m[0][2]
                + m[3][2] * other.m[0][3];
        m1 = m[0][2] * other.m[1][0]
                + m[1][2] * other.m[1][1]
                + m[2][2] * other.m[1][2]
                + m[3][2] * other.m[1][3];
        m2 = m[0][2] * other.m[2][0]
                + m[1][2] * other.m[2][1]
                + m[2][2] * other.m[2][2]
                + m[3][2] * other.m[2][3];
        m[3][2] = m[0][2] * other.m[3][0]
                + m[1][2] * other.m[3][1]
                + m[2][2] * other.m[3][2]
                + m[3][2] * other.m[3][3];
        m[0][2] = m0;
        m[1][2] = m1;
        m[2][2] = m2;

        m0 = m[0][3] * other.m[0][0]
                + m[1][3] * other.m[0][1]
                + m[2][3] * other.m[0][2]
                + m[3][3] * other.m[0][3];
        m1 = m[0][3] * other.m[1][0]
                + m[1][3] * other.m[1][1]
                + m[2][3] * other.m[1][2]
                + m[3][3] * other.m[1][3];
        m2 = m[0][3] * other.m[2][0]
                + m[1][3] * other.m[2][1]
                + m[2][3] * other.m[2][2]
                + m[3][3] * other.m[2][3];
        m[3][3] = m[0][3] * other.m[3][0]
                + m[1][3] * other.m[3][1]
                + m[2][3] * other.m[3][2]
                + m[3][3] * other.m[3][3];
        m[0][3] = m0;
        m[1][3] = m1;
        m[2][3] = m2;
        return *this;
    }

    M4 operator*(const M4& m2)
    {
        M4 m3;
        m3.m[0][0] = m[0][0] * m2.m[0][0]
                  + m[1][0] * m2.m[0][1]
                  + m[2][0] * m2.m[0][2]
                  + m[3][0] * m2.m[0][3];
        m3.m[0][1] = m[0][1] * m2.m[0][0]
                  + m[1][1] * m2.m[0][1]
                  + m[2][1] * m2.m[0][2]
                  + m[3][1] * m2.m[0][3];
        m3.m[0][2] = m[0][2] * m2.m[0][0]
                  + m[1][2] * m2.m[0][1]
                  + m[2][2] * m2.m[0][2]
                  + m[3][2] * m2.m[0][3];
        m3.m[0][3] = m[0][3] * m2.m[0][0]
                  + m[1][3] * m2.m[0][1]
                  + m[2][3] * m2.m[0][2]
                  + m[3][3] * m2.m[0][3];

        m3.m[1][0] = m[0][0] * m2.m[1][0]
                  + m[1][0] * m2.m[1][1]
                  + m[2][0] * m2.m[1][2]
                  + m[3][0] * m2.m[1][3];
        m3.m[1][1] = m[0][1] * m2.m[1][0]
                  + m[1][1] * m2.m[1][1]
                  + m[2][1] * m2.m[1][2]
                  + m[3][1] * m2.m[1][3];
        m3.m[1][2] = m[0][2] * m2.m[1][0]
                  + m[1][2] * m2.m[1][1]
                  + m[2][2] * m2.m[1][2]
                  + m[3][2] * m2.m[1][3];
        m3.m[1][3] = m[0][3] * m2.m[1][0]
                  + m[1][3] * m2.m[1][1]
                  + m[2][3] * m2.m[1][2]
                  + m[3][3] * m2.m[1][3];

        m3.m[2][0] = m[0][0] * m2.m[2][0]
                  + m[1][0] * m2.m[2][1]
                  + m[2][0] * m2.m[2][2]
                  + m[3][0] * m2.m[2][3];
        m3.m[2][1] = m[0][1] * m2.m[2][0]
                  + m[1][1] * m2.m[2][1]
                  + m[2][1] * m2.m[2][2]
                  + m[3][1] * m2.m[2][3];
        m3.m[2][2] = m[0][2] * m2.m[2][0]
                  + m[1][2] * m2.m[2][1]
                  + m[2][2] * m2.m[2][2]
                  + m[3][2] * m2.m[2][3];
        m3.m[2][3] = m[0][3] * m2.m[2][0]
                  + m[1][3] * m2.m[2][1]
                  + m[2][3] * m2.m[2][2]
                  + m[3][3] * m2.m[2][3];

        m3.m[3][0] = m[0][0] * m2.m[3][0]
                  + m[1][0] * m2.m[3][1]
                  + m[2][0] * m2.m[3][2]
                  + m[3][0] * m2.m[3][3];
        m3.m[3][1] = m[0][1] * m2.m[3][0]
                  + m[1][1] * m2.m[3][1]
                  + m[2][1] * m2.m[3][2]
                  + m[3][1] * m2.m[3][3];
        m3.m[3][2] = m[0][2] * m2.m[3][0]
                  + m[1][2] * m2.m[3][1]
                  + m[2][2] * m2.m[3][2]
                  + m[3][2] * m2.m[3][3];
        m3.m[3][3] = m[0][3] * m2.m[3][0]
                  + m[1][3] * m2.m[3][1]
                  + m[2][3] * m2.m[3][2]
                  + m[3][3] * m2.m[3][3];
        return m3;
    }

    V3<T> operator*(const V3<T>& vector)
    {
        T x, y, z, w;

            x = vector.x * m[0][0] +
                vector.y * m[1][0] +
                vector.z * m[2][0] +
                m[3][0];
            y = vector.x * m[0][1] +
                vector.y * m[1][1] +
                vector.z * m[2][1] +
                m[3][1];
            z = vector.x * m[0][2] +
                vector.y * m[1][2] +
                vector.z * m[2][2] +
                m[3][2];
            w = vector.x * m[0][3] +
                vector.y * m[1][3] +
                vector.z * m[2][3] +
                m[3][3];
            if (w == 1)
                return V3<T>(x, y, z);
            else
                return V3<T>(x / w, y / w, z / w);
    }

    void translate(const V3<T>& vector)
    {
        T vx = vector.x;
        T vy = vector.y;
        T vz = vector.z;

        m[3][0] += m[0][0] * vx + m[1][0] * vy + m[2][0] * vz;
        m[3][1] += m[0][1] * vx + m[1][1] * vy + m[2][1] * vz;
        m[3][2] += m[0][2] * vx + m[1][2] * vy + m[2][2] * vz;
        m[3][3] += m[0][3] * vx + m[1][3] * vy + m[2][3] * vz;
    }


    void rotate(T angle, T x, T y, T z)
    {
        if (angle == 0)
            return;

        T c, s;

        if (angle == 90 || angle == -270)
        {
            s = 1;
            c = 0;
        }
        else if (angle == -90 || angle == 270)
        {
            s = -1;
            c = 0;
        }
        else if (angle == 180 || angle == -180)
        {
            s = 0;
            c = -1;
        }
        else
        {
            T a = d2r(angle);
            c = std::cos(a);
            s = std::sin(a);

        }
        if (x == 0)
        {
            if (y == 0)
            {
                if (z != 0)
                {
                    // Rotate around the Z axis.
                    if (z < 0)
                        s = -s;
                    T tmp;
                    m[0][0] = (tmp = m[0][0]) * c + m[1][0] * s;
                    m[1][0] = m[1][0] * c - tmp * s;
                    m[0][1] = (tmp = m[0][1]) * c + m[1][1] * s;
                    m[1][1] = m[1][1] * c - tmp * s;
                    m[0][2] = (tmp = m[0][2]) * c + m[1][2] * s;
                    m[1][2] = m[1][2] * c - tmp * s;
                    m[0][3] = (tmp = m[0][3]) * c + m[1][3] * s;
                    m[1][3] = m[1][3] * c - tmp * s;
                    return;
                }
            }
            else if (z == 0)
            {
                // Rotate around the Y axis.
                if (y < 0)
                    s = -s;
                T tmp;
                m[2][0] = (tmp = m[2][0]) * c + m[0][0] * s;
                m[0][0] = m[0][0] * c - tmp * s;
                m[2][1] = (tmp = m[2][1]) * c + m[0][1] * s;
                m[0][1] = m[0][1] * c - tmp * s;
                m[2][2] = (tmp = m[2][2]) * c + m[0][2] * s;
                m[0][2] = m[0][2] * c - tmp * s;
                m[2][3] = (tmp = m[2][3]) * c + m[0][3] * s;
                m[0][3] = m[0][3] * c - tmp * s;

                return;
            }
        }
        else if (y == 0 && z == 0)
        {
            // Rotate around the X axis.
            if (x < 0)
                s = -s;
            T tmp;
            m[1][0] = (tmp = m[1][0]) * c + m[2][0] * s;
            m[2][0] = m[2][0] * c - tmp * s;
            m[1][1] = (tmp = m[1][1]) * c + m[2][1] * s;
            m[2][1] = m[2][1] * c - tmp * s;
            m[1][2] = (tmp = m[1][2]) * c + m[2][2] * s;
            m[2][2] = m[2][2] * c - tmp * s;
            m[1][3] = (tmp = m[1][3]) * c + m[2][3] * s;
            m[2][3] = m[2][3] * c - tmp * s;
            return;
        }

        T len = x * x + y * y + z * z;

        len = std::sqrt(len);
        x = (x / len);
        y = (y / len);
        z = (z / len);

        T ic = 1 - c;
        M4 rot; // The "1" says to not load the identity.
        rot.m[0][0] = x * x * ic + c;
        rot.m[1][0] = x * y * ic - z * s;
        rot.m[2][0] = x * z * ic + y * s;
        rot.m[3][0] = 0.0f;
        rot.m[0][1] = y * x * ic + z * s;
        rot.m[1][1] = y * y * ic + c;
        rot.m[2][1] = y * z * ic - x * s;
        rot.m[3][1] = 0.0f;
        rot.m[0][2] = x * z * ic - y * s;
        rot.m[1][2] = y * z * ic + x * s;
        rot.m[2][2] = z * z * ic + c;
        rot.m[3][2] = 0.0f;
        rot.m[0][3] = 0.0f;
        rot.m[1][3] = 0.0f;
        rot.m[2][3] = 0.0f;
        rot.m[3][3] = 1.0f;
        *this *= rot;
    }



    void perspective(T verticalAngle, T aspectRatio, T nearPlane, T farPlane)
    {
        // Bail out if the projection volume is zero-sized.
        if (nearPlane == farPlane || aspectRatio == 0)
            return;

        // Construct the projection.
        M4 m;
        T radians = d2r(verticalAngle / 2);
        T sine = std::sin(radians);

        if (sine == 0)
            return;

        T cotan = std::cos(radians) / sine;
        T clip = farPlane - nearPlane;

        m.m[0][0] = cotan / aspectRatio;
        m.m[1][0] = 0;
        m.m[2][0] = 0;
        m.m[3][0] = 0;
        m.m[0][1] = 0;
        m.m[1][1] = cotan;
        m.m[2][1] = 0;
        m.m[3][1] = 0;
        m.m[0][2] = 0;
        m.m[1][2] = 0;
        m.m[2][2] = -(nearPlane + farPlane) / clip;
        m.m[3][2] = -(2 * nearPlane * farPlane) / clip;
        m.m[0][3] = 0;
        m.m[1][3] = 0;
        m.m[2][3] = -1;
        m.m[3][3] = 0;

        // Apply the projection.
        *this *= m;
    }

private:
    T m[4][4];

    T d2r(T degrees)
    {
        return degrees * T(M_PI/180);
    }
};

typedef M4<float> M4F;
typedef M4<double> M4D;
typedef M4<FP> M4FP;

#endif // M4_H
