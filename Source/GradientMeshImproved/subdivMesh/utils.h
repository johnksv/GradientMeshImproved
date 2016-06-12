#ifndef UTILS_H
#define UTILS_H

#include <cmath>

namespace subdivMesh {

static inline double powJiri(double b, double e)
{
    if (b < 0)
    {
        return(-pow(-b, e));
    }
    else
    {
        return(pow(b,e));
    }
}

static inline void XYZ2LAB(double X, double Y, double Z, double &L, double &a ,double &b)
{
    double var_X,var_Y,var_Z;

    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 100.0;

    var_X = X / ref_X;          //ref_X =  95.047   Observer= 2°, Illuminant= D65
    var_Y = Y / ref_Y;          //ref_Y = 100.000
    var_Z = Z / ref_Z;          //ref_Z = 108.883

    if ( var_X > 0.008856 )
    {
        var_X = powJiri(var_X, 1.0/3.0);
    }
    else
    {
        var_X = ( 7.787 * var_X ) + ( 16.0 / 116.0 );
    }

    if ( var_Y > 0.008856 )
    {
        var_Y = powJiri(var_Y, 1.0/3.0);
    }
    else
    {
        var_Y = ( 7.787 * var_Y ) + ( 16.0 / 116.0 );
    }

    if ( var_Z > 0.008856 )
    {
        var_Z = powJiri(var_Z, 1.0/3.0);
    }
    else
    {
        var_Z = ( 7.787 * var_Z ) + ( 16.0 / 116.0 );
    }

    L = ( 116.0 * var_Y ) - 16.0;
    a = 500.0 * ( var_X - var_Y );
    b = 200.0 * ( var_Y - var_Z );
}

static inline void LAB2XYZ(double L, double a, double b, double &X, double &Y, double &Z)
{
    double var_X, var_Y, var_Z, x3, y3, z3;

    var_Y = ( L + 16.0 ) / 116.0;
    var_X = a / 500.0 + var_Y;
    var_Z = var_Y - b / 200.0;

    x3 = var_X * var_X * var_X;
    y3 = var_Y * var_Y * var_Y;
    z3 = var_Z * var_Z * var_Z;

    if ( y3 > 0.008856 )
    {
        var_Y = y3;
    }
    else
    {
        var_Y = ( var_Y - 16.0 / 116.0 ) / 7.787;
    }

    if ( x3 > 0.008856 )
    {
        var_X = x3;
    }
    else
    {
        var_X = ( var_X - 16.0 / 116.0 ) / 7.787;
    }

    if ( z3 > 0.008856 )
    {
        var_Z = z3;
    }
    else
    {
        var_Z = ( var_Z - 16.0 / 116.0 ) / 7.787;
    }

    double ref_X = 95.047;
    double ref_Y = 100.0;
    double ref_Z = 100.0;

    X = ref_X * var_X;     //ref_X =  95.047     Observer= 2°, Illuminant= D65
    Y = ref_Y * var_Y;     //ref_Y = 100.000
    Z = ref_Z * var_Z;     //ref_Z = 108.883
}

static inline void RGB2XYZ(double R, double G, double B, double &X, double &Y, double &Z)
{
    double var_R,var_G,var_B;

    var_R = R;
    var_G = G;
    var_B = B;

    if ( var_R > 0.04045 )
    {
        var_R = powJiri( ( var_R + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_R = var_R / 12.92;
    }

    if ( var_G > 0.04045 )
    {
        var_G = powJiri( ( var_G + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_G = var_G / 12.92;
    }

    if ( var_B > 0.04045 )
    {
        var_B = powJiri( ( var_B + 0.055 ) / 1.055, 2.4);
    }
    else
    {
        var_B = var_B / 12.92;
    }

    var_R = var_R * 100.0;
    var_G = var_G * 100.0;
    var_B = var_B * 100.0;

    //Observer. = 2°, Illuminant = D65
    X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
    Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
    Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
}

static inline void XYZ2RGB(double X, double Y, double Z, double &R, double &G, double &B)
{
    double var_X,var_Y,var_Z,var_R,var_G,var_B;

    var_X = X / 100.0;        //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
    var_Y = Y / 100.0;        //Y from 0 to 100.000
    var_Z = Z / 100.0;        //Z from 0 to 108.883

    var_R = var_X *  3.2406 + var_Y * (-1.5372) + var_Z * (-0.4986);
    var_G = var_X * (-0.9689) + var_Y *  1.8758 + var_Z *  0.0415;
    var_B = var_X *  0.0557 + var_Y * (-0.2040) + var_Z *  1.0570;

    if ( var_R > 0.0031308 )
    {
        var_R = 1.055 * ( powJiri(var_R, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_R = 12.92 * var_R;
    }

    if ( var_G > 0.0031308 )
    {
        var_G = 1.055 * ( powJiri(var_G, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_G = 12.92 * var_G;
    }

    if ( var_B > 0.0031308 )
    {
        var_B = 1.055 * ( powJiri(var_B, 1.0 / 2.4) ) - 0.055;
    }
    else
    {
        var_B = 12.92 * var_B;
    }

    R = var_R; // * 255;
    G = var_G; // * 255;
    B = var_B; // * 255;
}

static inline void RGB2LAB(double R, double G,double B, double &L, double &a, double &b)
{
    double x,y,z;
    RGB2XYZ(R,G,B,x,y,z);
    XYZ2LAB(x,y,z,L,a,b);
}

//reimplemented from http://www.easyrgb.com/index.php?X=MATH&H=01#text1
static inline void LAB2RGB(double L, double a, double b, double &R, double &G, double &B)
{
    double x,y,z;
    LAB2XYZ(L,a,b,x,y,z);
    XYZ2RGB(x,y,z,R,G,B);
}

static inline void RGB2LCH(double R, double G,double B, double &L, double &C, double &H)
{
    double x,y,z,a=0,b=0;
    RGB2XYZ(R,G,B,x,y,z);
    XYZ2LAB(x,y,z,L,a,b);

    C = sqrt(pow(a,2)+pow(b,2));
    H = atan2(b,a);
    if(H<0) H += 2*3.14159265358979;
    if(H>=2*3.14159265358979) H -= 2*3.14159265358979;
}

//reimplemented from http://www.easyrgb.com/index.php?X=MATH&H=01#text1
static inline void LCH2RGB(double L, double C, double H, double &R, double &G, double &B)
{
    double x,y,z;

    double a = C*cos(H), b = C*sin(H);

    LAB2XYZ(L,a,b,x,y,z);
    XYZ2RGB(x,y,z,R,G,B);
}

} // end namespace subdivMesh

#endif // UTILS_H
