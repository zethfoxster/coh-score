#include "arda2/math/matFirst.h"
#include "arda2/math/matMatrix4x3T.h"

matMatrix4x3T& matMatrix4x3T::StoreReflect( IN const matPlane& ReflectPlane )
{
    matPlane p(ReflectPlane);
    matBasePlaneNormalize(&p,&p);

    // Uses the following formula to compute returned matrix (adjusted for transpose)
    // In normal (non-transpose) reflect calculation, last column should be zeros
    // P = plane
    // -2 * P.a * P.a + 1       -2 * P.a * P.b      -2 * P.a * P.c      -2 * P.a * P.d
    // -2 * P.a * P.b           -2 * P.b * P.b +1   -2 * P.b * P.c      -2 * P.b * P.d
    // -2 * P.a * P.c           -2 * P.b * P.c      -2 * P.c * P.c + 1  -2 * P.c * P.d

    for ( int i = 0; i < 3; i++ )
        for ( int j = 0; j < 3; j++ )
        {
            m[i][j] = -2;
            if ( i == 0 )
                m[i][j]*=p.a;
            if ( j == 0 )
                m[i][j]*=p.a;
            if ( i == 1 )
                m[i][j]*=p.b;
            if ( j == 1 )
                m[i][j]*=p.b;
            if ( i == 2 )
                m[i][j]*=p.c;
            if ( j == 2 )
                m[i][j]*=p.c;
            if ( i == j )
                m[i][j]+=1;
        }
    _14 = -2 * p.a * p.d;
    _24 = -2 * p.b * p.d;
    _34 = -2 * p.c * p.d;

    return *this;
}

matMatrix4x3T& matMatrix4x3T::StoreQuaternion( const matQuaternion &rOrientation ) 
{
    matBaseQUATERNION q(rOrientation);
    matBaseQuaternionNormalize(&q, &q);

    // Uses following formula to calculate rotation matrix (adjusted for transpose)
    // 1 - 2(x^2 + z^2)     2xy - 2zw           2yw + 2 xz          0
    // 2xy + 2zw            1 - 2(x^2 + z^2)    -2xw + 2zy          0
    // -2yw + 2xz           2xw + 2yz           1 - 2(x^2 + y^2)    0

    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;

    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;

    float zz = q.z * q.z;
    float zw = q.z * q.w;

    _11 = 1.0f - 2.0f * ( yy + zz );
    _12 = 2.0f * ( xy - zw );
    _13 = 2.0f * ( yw + xz );
    _14 = 0.0f;
    _21 = 2.0f * ( xy + zw );
    _22 = 1.0f - 2.0f * ( xx + zz );
    _23 = 2.0f * ( yz - xw );
    _24 = 0.0f;
    _31 = 2.0f * ( xz - yw );
    _32 = 2.0f * ( xw + yz );
    _33 = 1.0f - 2.0f * ( xx + yy );
    _34 = 0.0f;

    return *this;
}

matMatrix4x3T& matMatrix4x3T::StoreEulerAngles( float fYaw, float fPitch, float fRoll ) 
{
    matBaseQUATERNION q;
    matBaseQuaternionRotationYawPitchRoll(&q, fYaw, fPitch, fRoll);

    return StoreQuaternion(q);
}
