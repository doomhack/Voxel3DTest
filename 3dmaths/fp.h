#ifndef FP_H
#define FP_H

#include <stdio.h>
#include <limits>

#include <QtCore>

#define OVERFLOW_CHECK

class FP
{
public:
    FP() {}

    FP(const FP& r) : n(r.n)            {}
    FP(int v)                           {*this=v;}
    FP(float v)                         {*this=v;}

    operator int() const                {return i();}
    operator float() const              {return f();}

    int i() const                       {return n >> fracbits;}
    float f() const                     {return (float)n / one;}

    static FP fromFPInt(const int r)    {FP v; v.n = r; return v;}
    int toFPInt()                       {return n;}

    int intMul(int r)                   {return ((long long int)n * r) >> fracbits;}

    static int max()
    {
        return std::numeric_limits<short>::max();
    }

    static int min()
    {
        return std::numeric_limits<short>::min();
    }

    FP& operator=(const int r)
    {
#ifdef OVERFLOW_CHECK

        if(r < min() || r > max())
        {
            qDebug() << "int assign oveflow: " <<  r;
        }
#endif
        n = (r << fracbits);   return *this;
    }

    FP& operator=(const float r)
    {
#ifdef OVERFLOW_CHECK

        if(r < min() || r > max())
        {
            qDebug() << "float assign oveflow: " <<  r;
        }
#endif

        n = (int)(r * one);    return *this;
    }

    //Addition
    FP operator+(const FP& r)           {FP v(r);   return v+=*this;}
    FP operator+(const int r)           {FP v(r);   return v+=*this;}
    FP operator+(const float r)         {FP v(r);   return v+=*this;}

    FP& operator+=(const FP& r)
    {
#ifdef OVERFLOW_CHECK

        long long int tmp = r.n + n;

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            qDebug() << "addition oveflow: " <<  r.i() << this->i() << r.i() << this->i();
        }
#endif

        n += r.n;  return *this;
    }

    FP& operator+=(const int r)         {return *this+=FP(r);}
    FP& operator+=(const float r)       {return *this+=FP(r);}

    //Subtraction
    FP operator-(const FP& r)
    {

#ifdef OVERFLOW_CHECK

        long long int tmp = n - r.n;

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            qDebug() << "subtraction oveflow: " <<  r.i() << this->i() << r.i() << this->i();
        }
#endif

        FP v(r); v.n = n - v.n;  return v;
    }


    FP operator-(const int r)           {FP v(r); return *this-v;}
    FP operator-(const float r)         {FP v(r); return *this-v;}


    FP& operator-=(const FP& r)         {n -= r.n;  return *this;}
    FP& operator-=(const int& r)        {return *this-=FP(r);}
    FP& operator-=(const float& r)      {return *this-=FP(r);}

    FP operator-()                     {FP r; r.n = -n; return r;}

    //Multiply
    FP operator*(const FP& r)           {FP v(r);   return v*=*this;}
    FP operator*(const int r)           {FP v(r);   return v*=*this;}
    FP operator*(const float r)         {FP v(r);   return v*=*this;}

    FP& operator*=(const FP& r)
    {
        long long int tmp = (((long long int)n * r.n) >> fracbits);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            qDebug() << "multiply oveflow: " <<  r.i() << this->i() << r.i() * this->i();
        }
#endif

        n = (int)tmp;
        return *this;
    }

    FP& operator*=(const int& r)        {return *this*=FP(r);}
    FP& operator*=(const float& r)      {return *this*=FP(r);}

    //Divide
    FP operator/(const FP& r)
    {
        long long int tmp = (((long long int)n << fracbits) / r.n);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            qDebug() << "divide oveflow: " <<  r.i() << this->i() << r.i() << this->i();
        }
#endif

        FP v(r); v.n = (int)tmp;
        return v;
    }

    FP operator/(const int r)           {FP v(r);   return *this/v;}
    FP operator/(const float r)         {FP v(r);   return *this/v;}

    FP& operator/=(const FP& r)
    {
        long long int tmp = (((long long int)n << fracbits) / r.n);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            qDebug() << "divide oveflow: " <<  r.i() << this->i() << r.i() << this->i();
        }
#endif
        n = (int)tmp;
        return *this;
    }

    FP& operator/=(const int& r)        {return *this/=FP(r);}
    FP& operator/=(const float& r)      {return *this/=FP(r);}

    //Equality
    bool operator==(const FP& r)        {return n == r.n;}
    bool operator==(const int& r)       {return *this==FP(r);}
    bool operator==(const float& r)     {return *this==FP(r);}

    //Inequality
    bool operator!=(const FP& r)        {return n != r.n;}
    bool operator!=(const int& r)       {return *this!=FP(r);}
    bool operator!=(const float& r)     {return *this!=FP(r);}

    //Less than
    bool operator<(const FP& r)         {return n < r.n;}
    bool operator<(const int& r)        {return *this<FP(r);}
    bool operator<(const float& r)      {return *this<FP(r);}

    //Greater than
    bool operator>(const FP& r)         {return n > r.n;}
    bool operator>(const int& r)        {return *this>FP(r);}
    bool operator>(const float& r)      {return *this>FP(r);}

    //Greater than=
    bool operator>=(const FP& r)        {return n >= r.n;}
    bool operator>=(const int& r)       {return *this>=FP(r);}
    bool operator>=(const float& r)     {return *this>=FP(r);}

    //Less than=
    bool operator<=(const FP& r)        {return n <= r.n;}
    bool operator<=(const int& r)       {return *this<=FP(r);}
    bool operator<=(const float& r)     {return *this<=FP(r);}

    //&
    FP operator&(const FP& r)           {FP v(r); v.n &= n;  return v;}
    FP operator&(const int r)           {FP v(r); return v&=*this;}

    FP& operator&=(const FP& r)         {n &= r.n;  return *this;}
    FP& operator&=(const int& r)        {return *this&=FP(r);}

    //|
    FP operator|(const FP& r)           {FP v(r); v.n |= n;  return v;}
    FP operator|(int r)                 {FP v(r);   return v|=*this;}
    FP& operator|=(const FP& r)         {n |= r.n;  return *this;}
    FP& operator|=(const int& r)        {return *this|=FP(r);}

    //^
    FP operator^(const FP& r)           {FP v(r); v.n ^= n;  return v;}
    FP operator^(const int r)           {FP v(r);   return v^=*this;}
    FP& operator^=(const FP& r)         {n ^= r.n;  return *this;}
    FP& operator^=(const int& r)        {return *this^=FP(r);}

    //<<
    FP operator<<(const unsigned int r)     {FP v(n);   return v<<=r;}
    FP& operator<<=(const unsigned int r)   {n <<= r;  return *this;}

    //>>
    FP operator>>(const unsigned int r)     {FP v(n);   return v>>=r;}
    FP& operator>>=(const unsigned int r)   {n >>= r;  return *this;}


private:
    int n;

    static const unsigned int fracbits = 16;
    static const unsigned int one = (1 << fracbits);
};

#endif // FP_H
