#ifndef FP_H
#define FP_H

class FP
{
public:
    FP() {}

    FP(const FP& r) : n(r.n)            {}
    explicit FP(int v)                  {*this=v;}
    explicit FP(float v)                {*this=v;}
    explicit FP(double v)               {*this=v;}

    operator int() const                {return i();}
    operator float() const              {return f();}
    operator double() const             {return d();}

    int i() const                       {return n >> fracbits;}
    float f() const                     {return (float)n / one;}
    float d() const                     {return (double)n / one;}

    static FP fromFPInt(const int r)    {FP v; v.n = r; return v;}
    int toFPInt()                       {return n;}

    FP& operator=(const int r)          {n = (r << fracbits);   return *this;}
    FP& operator=(const float r)        {n = (int)(r * one);    return *this;}
    FP& operator=(const double r)       {n = (int)(r * one);    return *this;}

    //Addition
    FP operator+(const FP& r)           {FP v(r);   return v+=r;}
    FP operator+(const int r)           {FP v(r);   return v+=r;}
    FP operator+(const float r)         {FP v(r);   return v+=r;}
    FP operator+(const double r)        {FP v(r);   return v+=r;}

    FP& operator+=(const FP& r)         {n += r.n;  return *this;}
    FP& operator+=(const int r)         {return *this+=FP(r);}
    FP& operator+=(const float r)       {return *this+=FP(r);}
    FP& operator+=(const double r)      {return *this+=FP(r);}

    //Subtraction
    FP operator-(const FP& r)           {FP v(r);   return v-=r;}
    FP operator-(const int r)           {FP v(r);   return v-=r;}
    FP operator-(const float r)         {FP v(r);   return v-=r;}
    FP operator-(const double r)        {FP v(r);   return v-=r;}


    FP& operator-=(const FP& r)         {n -= r.n;  return *this;}
    FP& operator-=(const int& r)        {return *this-=FP(r);}
    FP& operator-=(const float& r)      {return *this-=FP(r);}
    FP& operator-=(const double& r)     {return *this-=FP(r);}

    //Multiply
    FP operator*(const FP& r)           {FP v(r);   return v*=r;}
    FP operator*(const int r)           {FP v(r);   return v*=r;}
    FP operator*(const float r)         {FP v(r);   return v*=r;}
    FP operator*(const double r)        {FP v(r);   return v*=r;}

    FP& operator*=(const FP& r)         {n = ((n * r.n) >> fracbits);   return *this;}
    FP& operator*=(const int& r)        {return *this*=FP(r);}
    FP& operator*=(const float& r)      {return *this*=FP(r);}
    FP& operator*=(const double& r)     {return *this*=FP(r);}

    //Divide
    FP operator/(const FP& r)           {FP v(r);   return v/=r;}
    FP operator/(const int r)           {FP v(r);   return v/=r;}
    FP operator/(const float r)         {FP v(r);   return v/=r;}
    FP operator/(const double r)        {FP v(r);   return v/=r;}

    FP& operator/=(const FP& r)         {n = ((n << fracbits) / r.n);   return *this;}
    FP& operator/=(const int& r)        {return *this/=FP(r);}
    FP& operator/=(const float& r)      {return *this/=FP(r);}
    FP& operator/=(const double& r)     {return *this/=FP(r);}

    //Equality
    bool operator==(const FP& r)        {return n == r.n;}
    bool operator==(const int& r)       {return *this==FP(r);}
    bool operator==(const float& r)     {return *this==FP(r);}
    bool operator==(const double& r)    {return *this==FP(r);}

    //Inequality
    bool operator!=(const FP& r)        {return n != r.n;}
    bool operator!=(const int& r)       {return *this!=FP(r);}
    bool operator!=(const float& r)     {return *this!=FP(r);}
    bool operator!=(const double& r)    {return *this!=FP(r);}

    //Less than
    bool operator<(const FP& r)         {return n < r.n;}
    bool operator<(const int& r)        {return *this<FP(r);}
    bool operator<(const float& r)      {return *this<FP(r);}
    bool operator<(const double& r)     {return *this<FP(r);}

    //Greater than
    bool operator>(const FP& r)         {return n > r.n;}
    bool operator>(const int& r)        {return *this>FP(r);}
    bool operator>(const float& r)      {return *this>FP(r);}
    bool operator>(const double& r)     {return *this>FP(r);}

    //Greater than=
    bool operator>=(const FP& r)        {return n >= r.n;}
    bool operator>=(const int& r)       {return *this>=FP(r);}
    bool operator>=(const float& r)     {return *this>=FP(r);}
    bool operator>=(const double& r)    {return *this>=FP(r);}

    //Less than=
    bool operator<=(const FP& r)        {return n <= r.n;}
    bool operator<=(const int& r)       {return *this<=FP(r);}
    bool operator<=(const float& r)     {return *this<=FP(r);}
    bool operator<=(const double& r)    {return *this<=FP(r);}

    //&
    FP operator&(const FP& r)           {FP v(r);   return v&=r;}
    FP operator&(const int r)           {FP v(r);   return v&=r;}
    FP& operator&=(const FP& r)         {n &= r.n;  return *this;}
    FP& operator&=(const int& r)        {return *this&=FP(r);}

    //|
    FP operator|(const FP& r)           {FP v(r);   return v|=r;}
    FP operator|(int r)                 {FP v(r);   return v|=r;}
    FP& operator|=(const FP& r)         {n |= r.n;  return *this;}
    FP& operator|=(const int& r)        {return *this|=FP(r);}

    //^
    FP operator^(const FP& r)           {FP v(r);   return v^=r;}
    FP operator^(const int r)           {FP v(r);   return v^=r;}
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
