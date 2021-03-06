#ifndef FP_H
#define FP_H

#include <limits>


//#define OVERFLOW_CHECK

class FP
{
public:
    constexpr FP() : n(0)                               {}

    constexpr FP(const FP& r)   : n(r.n)                {}
    constexpr FP(int v)         : n(v << fracbits)      {}
    constexpr FP(float v)       : n((int)(v * one))     {}

    constexpr operator int() const      {return i();}
    constexpr operator float() const    {return f();}

    constexpr int i() const             {return n >> fracbits;}
    constexpr float f() const           {return (float)n / one;}

    static constexpr FP fromFPInt(const int r)    {FP v; v.n = r; return v;}
    constexpr int toFPInt()             {return n;}

    constexpr int intMul(int r)         {return ((long long int)n * r) >> fracbits;}

    constexpr static int max()
    {
        return std::numeric_limits<short>::max();
    }

    constexpr static int min()
    {
        return std::numeric_limits<short>::min();
    }

    constexpr FP& operator=(const int r)
    {
#ifdef OVERFLOW_CHECK

        if(r < min() || r > max())
        {
            //std::cout << "int assign overflow: " <<  r;
        }
#endif
        n = (r << fracbits);   return *this;
    }

    constexpr FP& operator=(const float r)
    {
#ifdef OVERFLOW_CHECK

        if(r < min() || r > max())
        {
            //std::cout << "float assign overflow: " <<  r;
        }
#endif

        n = (int)(r * one);    return *this;
    }

    //Addition
    constexpr FP operator+(const FP& r)           {FP v(r);   return v+=*this;}
    constexpr FP operator+(const int r)           {FP v(r);   return v+=*this;}
    constexpr FP operator+(const float r)         {FP v(r);   return v+=*this;}

    constexpr FP& operator+=(const FP& r)
    {
#ifdef OVERFLOW_CHECK

        long long int tmp = r.n + n;

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            //std::cout << "addition overflow: " <<  r.i() << this->i();
        }
#endif

        n += r.n;  return *this;
    }

    constexpr FP& operator+=(const int r)         {return *this+=FP(r);}
    constexpr FP& operator+=(const float r)       {return *this+=FP(r);}

    //Subtraction
    constexpr FP operator-(const FP& r)
    {

#ifdef OVERFLOW_CHECK

        long long int tmp = n - r.n;

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            //std::cout << "subtraction overflow: " <<  r.i() << this->i();
        }
#endif

        FP v(r); v.n = n - v.n;  return v;
    }


    constexpr FP operator-(const int r)           {FP v(r); return *this-v;}
    constexpr FP operator-(const float r)         {FP v(r); return *this-v;}


    constexpr FP& operator-=(const FP& r)         {n -= r.n;  return *this;}
    constexpr FP& operator-=(const int& r)        {return *this-=FP(r);}
    constexpr FP& operator-=(const float& r)      {return *this-=FP(r);}

    constexpr FP operator-()                      {FP r; r.n = -n; return r;}

    //Multiply
    constexpr FP operator*(const FP& r)           {FP v(r);   return v*=*this;}
    constexpr FP operator*(const int r)           {FP v(r);   return v*=*this;}
    constexpr FP operator*(const float r)         {FP v(r);   return v*=*this;}

    constexpr FP& operator*=(const FP& r)
    {
        long long int tmp = (((long long int)n * r.n) >> fracbits);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            //std::cout << "multiply overflow: " <<  r.i() << this->i() << r.i() * this->i();
        }
#endif

        n = (int)tmp;
        return *this;
    }

    constexpr FP& operator*=(const int& r)        {return *this*=FP(r);}
    constexpr FP& operator*=(const float& r)      {return *this*=FP(r);}

    //Divide
    constexpr FP operator/(const FP& r)
    {
        long long int tmp = (((long long int)n << fracbits) / r.n);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            //std::cout << "divide overflow: " <<  r.i() << this->i();
        }
#endif

        FP v(r); v.n = (int)tmp;
        return v;
    }

    constexpr FP operator/(const int r)           {FP v(r);   return *this/v;}
    constexpr FP operator/(const float r)         {FP v(r);   return *this/v;}

    constexpr FP& operator/=(const FP& r)
    {
        long long int tmp = (((long long int)n << fracbits) / r.n);

#ifdef OVERFLOW_CHECK

        if(tmp < std::numeric_limits<int>::min() || tmp > std::numeric_limits<int>::max())
        {
            //std::cout << "divide overflow: " <<  r.i() << this->i();
        }
#endif
        n = (int)tmp;
        return *this;
    }

    constexpr FP& operator/=(const int& r)        {return *this/=FP(r);}
    constexpr FP& operator/=(const float& r)      {return *this/=FP(r);}

    //Equality
    constexpr bool operator==(const FP& r)        {return n == r.n;}
    constexpr bool operator==(const int& r)       {return *this==FP(r);}
    constexpr bool operator==(const float& r)     {return *this==FP(r);}

    //Inequality
    constexpr bool operator!=(const FP& r)        {return n != r.n;}
    constexpr bool operator!=(const int& r)       {return *this!=FP(r);}
    constexpr bool operator!=(const float& r)     {return *this!=FP(r);}

    //Less than
    constexpr bool operator<(const FP& r)         {return n < r.n;}
    constexpr bool operator<(const int& r)        {return *this<FP(r);}
    constexpr bool operator<(const float& r)      {return *this<FP(r);}

    //Greater than
    constexpr bool operator>(const FP& r)         {return n > r.n;}
    constexpr bool operator>(const int& r)        {return *this>FP(r);}
    constexpr bool operator>(const float& r)      {return *this>FP(r);}

    //Greater than=
    constexpr bool operator>=(const FP& r)        {return n >= r.n;}
    constexpr bool operator>=(const int& r)       {return *this>=FP(r);}
    constexpr bool operator>=(const float& r)     {return *this>=FP(r);}

    //Less than=
    constexpr bool operator<=(const FP& r)        {return n <= r.n;}
    constexpr bool operator<=(const int& r)       {return *this<=FP(r);}
    constexpr bool operator<=(const float& r)     {return *this<=FP(r);}

    //&
    constexpr FP operator&(const FP& r)           {FP v(r); v.n &= n;  return v;}
    constexpr FP operator&(const int r)           {FP v(r); return v&=*this;}

    constexpr FP& operator&=(const FP& r)         {n &= r.n;  return *this;}
    constexpr FP& operator&=(const int& r)        {return *this&=FP(r);}

    //|
    constexpr FP operator|(const FP& r)           {FP v(r); v.n |= n;  return v;}
    constexpr FP operator|(int r)                 {FP v(r);   return v|=*this;}
    constexpr FP& operator|=(const FP& r)         {n |= r.n;  return *this;}
    constexpr FP& operator|=(const int& r)        {return *this|=FP(r);}

    //^
    constexpr FP operator^(const FP& r)           {FP v(r); v.n ^= n;  return v;}
    constexpr FP operator^(const int r)           {FP v(r);   return v^=*this;}
    constexpr FP& operator^=(const FP& r)         {n ^= r.n;  return *this;}
    constexpr FP& operator^=(const int& r)        {return *this^=FP(r);}

    //<<
    constexpr FP operator<<(const unsigned int r)     {FP v(n);   return v<<=r;}
    constexpr FP& operator<<=(const unsigned int r)   {n <<= r;  return *this;}

    //>>
    constexpr FP operator>>(const unsigned int r)     {FP v(n);   return v>>=r;}
    constexpr FP& operator>>=(const unsigned int r)   {n >>= r;  return *this;}


private:
    int n;

    static const unsigned int fracbits = 16;
    static const unsigned int one = (1 << fracbits);
};

#endif // FP_H
