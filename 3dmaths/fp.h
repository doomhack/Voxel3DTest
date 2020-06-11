#ifndef FP_H
#define FP_H

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

    FP& operator=(const int r)          {n = (r << fracbits);   return *this;}
    FP& operator=(const float r)        {n = (int)(r * one);    return *this;}

    //Addition
    FP operator+(const FP& r)           {FP v(r);   return v+=*this;}
    FP operator+(const int r)           {FP v(r);   return v+=*this;}
    FP operator+(const float r)         {FP v(r);   return v+=*this;}

    FP& operator+=(const FP& r)         {n += r.n;  return *this;}
    FP& operator+=(const int r)         {return *this+=FP(r);}
    FP& operator+=(const float r)       {return *this+=FP(r);}

    //Subtraction
    FP operator-(const FP& r)           {FP v(r); v.n = n - v.n;  return v;}
    FP operator-(const int r)           {FP v(r); return *this-v;}
    FP operator-(const float r)         {FP v(r); return *this-v;}


    FP& operator-=(const FP& r)         {n -= r.n;  return *this;}
    FP& operator-=(const int& r)        {return *this-=FP(r);}
    FP& operator-=(const float& r)      {return *this-=FP(r);}

    //Multiply
    FP operator*(const FP& r)           {FP v(r);   return v*=*this;}
    FP operator*(const int r)           {FP v(r);   return v*=*this;}
    FP operator*(const float r)         {FP v(r);   return v*=*this;}

    FP& operator*=(const FP& r)         {n = (int)(((long long int)n * r.n) >> fracbits);   return *this;}
    FP& operator*=(const int& r)        {return *this*=FP(r);}
    FP& operator*=(const float& r)      {return *this*=FP(r);}

    //Divide
    FP operator/(const FP& r)           {FP v(r); v.n = (int)(((long long int)n << fracbits) / r.n);   return v;}
    FP operator/(const int r)           {FP v(r);   return *this/v;}
    FP operator/(const float r)         {FP v(r);   return *this/v;}

    FP& operator/=(const FP& r)         {n = (int)(((long long int)n << fracbits) / r.n);   return *this;}
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
