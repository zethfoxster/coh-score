/*****************************************************************************
    created:    2001/09/20
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matQuaternion
#define   INCLUDED_matQuaternion
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTypes.h"
#include "matVector3.h"

class matMatrix4x4;

class matQuaternion : public matBaseQUATERNION
{
public:
    matQuaternion() : matBaseQUATERNION() {};
    matQuaternion(const matBaseQUATERNION q) : matBaseQUATERNION(q) {};
    matQuaternion(const float *f) : matBaseQUATERNION(f) {};
    matQuaternion(float x, float y, float z, float w) : matBaseQUATERNION(x, y, z, w) {};
    matQuaternion(matIdentityType) : matBaseQUATERNION(0.0f, 0.0f, 0.0f, 1.0f) {}

    matQuaternion operator +() const;
    matQuaternion operator -() const;

    bool IsEqual(   const matQuaternion& q, 
        float fRelError = kDefaultRelError, 
        float fAbsError = kDefaultAbsError ) const
    {
        return
           (::IsEqual(w, q.w, fRelError, fAbsError) && 
            ::IsEqual(x, q.x, fRelError, fAbsError) && 
            ::IsEqual(y, q.y, fRelError, fAbsError) && 
            ::IsEqual(z, q.z, fRelError, fAbsError)) ||
           (::IsEqual(w, -q.w, fRelError, fAbsError) && 
            ::IsEqual(x, -q.x, fRelError, fAbsError) && 
            ::IsEqual(y, -q.y, fRelError, fAbsError) && 
            ::IsEqual(z, -q.z, fRelError, fAbsError));
    }

    void StoreWeightedAverage(const matQuaternion* QuatArray, const float* Weight, int NumSets);

    void StoreNormalizedLerp(const matQuaternion &rStart, const matQuaternion &rEnd, float fTheta);

    void StoreFromMatrix(const matMatrix4x4 &rMat)  
    { matBaseQuaternionRotationMatrix(this, (matBaseMATRIX*)&rMat); }
    
    void StoreSlerp(const matQuaternion &rStart, const matQuaternion &rEnd, float fTheta) 
    { matBaseQuaternionSlerp( this, &rStart, &rEnd, fTheta ); }
    
    void StoreMultiply(const matQuaternion &rSource1, const matQuaternion &rSource2)    
    { matBaseQuaternionMultiply(this, (matBaseQUATERNION*)&rSource1, (matBaseQUATERNION*)&rSource2); }

    void StoreInverse( const matQuaternion &rSource )
    { matBaseQuaternionInverse(this, &rSource); }
    
    void Multiply(const matQuaternion &rSource)     
    { matBaseQuaternionMultiply(this, this, (matBaseQUATERNION*)&rSource); }
    
    void PreMultiply(const matQuaternion &rSource)  
    { matBaseQuaternionMultiply(this, (matBaseQUATERNION*)&rSource, this); }

    void Normalize()                                
    { matBaseQuaternionNormalize( this, this ); }

    // interp methods
    void StoreBarycentric(  const matQuaternion& q1, const matQuaternion& q2,
                            const matQuaternion& q3, float f, float g)
    { matBaseQuaternionBaryCentric(this, &q1, &q2, &q3, f, g); }

    void StoreConjugate()
    { matBaseQuaternionConjugate(this, this); }

    float Dot( const matQuaternion& quat ) const
    { return matBaseQuaternionDot(this, &quat);}
    
    void StoreExponential()
    { matBaseQuaternionExp(this, this); }

    bool IsNormal( float eps = kDefaultAbsError ) const;

    bool IsIdentity() 
    { return matBaseQuaternionIsIdentity(this) != 0; }

    void StoreIdentity()
    { matBaseQuaternionIdentity(this); }

    void Invert()
    { matBaseQuaternionInverse(this, this); }

    float Length() const
    { return matBaseQuaternionLength(this); }

    float LengthSq() const
    { return matBaseQuaternionLengthSq(this); }

    void StoreNaturalLog()
    { matBaseQuaternionLn(this, this); }
    
    void StoreRotationAxis( const matVector3& axis, float angle)
    { matBaseQuaternionRotationAxis(this, &axis, angle); }

    void StoreEulerAngles(float yaw, float pitch, float roll)
    { matBaseQuaternionRotationYawPitchRoll(this, yaw, pitch, roll); }

    void StoreLookDir( const matVector3 &vDir, const matVector3 &vUp );

    void ToAxisAngle( matVector3& Axis, float& Angle) const
    { matBaseQuaternionToAxisAngle(this, &Axis, &Angle);}

    matVector3 GetRightVector() const;
    matVector3 GetUpVector() const;
    matVector3 GetForwardVector() const;

    static const matQuaternion Identity;
};

inline bool IsEqual( const matQuaternion& lhs, const matQuaternion& rhs,
                        float fRelError = kDefaultRelError, 
                        float fAbsError = kDefaultAbsError )
{
    return lhs.IsEqual(rhs, fRelError, fAbsError);
}

inline matQuaternion operator*( const matQuaternion& lhs, const matQuaternion &rhs )
{
    matQuaternion r;
    r.StoreMultiply(lhs,rhs);
    return r;
}

inline matQuaternion matQuaternion::operator +() const
{
    return (matQuaternion)matBaseQUATERNION::operator+();
}

inline matQuaternion matQuaternion::operator -() const
{
    return (matQuaternion)matBaseQUATERNION::operator-();
}

inline matQuaternion FromMatrix(const matMatrix4x4 &rMat)   
{ 
    matQuaternion temp;
    temp.StoreFromMatrix(rMat);
    return temp;
}

inline matQuaternion Slerp(const matQuaternion &rStart, const matQuaternion &rEnd, float fTheta) 
{ 
    matQuaternion temp;
    temp.StoreSlerp(rStart, rEnd, fTheta);
    return temp;
}

inline matQuaternion Multiply(const matQuaternion &rSource1, const matQuaternion &rSource2) 
{   
    matQuaternion temp;
    temp.StoreMultiply(rSource1, rSource2);
    return temp;
}

inline matQuaternion Normalize(const matQuaternion& quat)                               
{ 
    matQuaternion temp(quat);
    temp.Normalize();
    return temp;
}

inline matQuaternion Barycentric(   const matQuaternion& q1, const matQuaternion& q2,
                                    const matQuaternion& q3, float f, float g)
{ 
    matQuaternion temp;
    temp.StoreBarycentric(q1, q2, q3, f, g);
    return temp;
}

inline matQuaternion Conjugate(const matQuaternion& quat)
{ 
    matQuaternion temp(quat);
    temp.StoreConjugate();
    return temp;
}

inline float Dot( const matQuaternion& quat, const matQuaternion& quat2 )
{ return matBaseQuaternionDot(&quat2, &quat);}

inline matQuaternion Exponential(const matQuaternion& quat)
{ 
    matQuaternion temp(quat);
    temp.StoreExponential();
    return temp;
}

inline matQuaternion Inverse(const matQuaternion& quat)
{ 
    matQuaternion temp(quat);
    temp.Invert();
    return temp;
}

inline matQuaternion  NaturalLog(const matQuaternion& quat)
{ 
    matQuaternion temp(quat);
    temp.StoreNaturalLog();
    return temp;
}

inline matQuaternion RotationAxis(  const matVector3& axis, 
                                    float angle)
{ 
    matQuaternion temp;
    temp.StoreRotationAxis(axis, angle);
    return temp;
}

inline matQuaternion EulerAngles(float yaw, 
                                 float pitch, 
                                 float roll)
{ 
    matQuaternion temp;
    temp.StoreEulerAngles(yaw, pitch, roll);
    return temp;
}


inline matVector3 matQuaternion::GetRightVector() const
{
    return matVector3(1 - 2 * (y*y + z*z), 2 * (x*y + w*z), 2 * (x*z - w*y));
}

inline matVector3 matQuaternion::GetUpVector() const
{
    return matVector3(2 * (x*y - w*z), 1 - 2 * (x*x + z*z), 2 * (y*z + w*x));
}

inline matVector3 matQuaternion::GetForwardVector() const
{
    return matVector3(2 * (x*z + w*y), 2 * (y*z - w*x), 1 - 2 * (x*x + y*y));
}

inline void matQuaternion::StoreWeightedAverage(const matQuaternion*    pQuatArray, 
                                                const float*            pWeight, 
                                                int                     NumSets)
{
    errAssert(pQuatArray && pWeight && (pWeight[0] > 0.0f) );
    errAssert(NumSets > 1);

    float cumwt = pWeight[0];
    *this       = pQuatArray[0];

    for(int i=1; i<NumSets; ++i){
        this->StoreSlerp( *this, pQuatArray[i], pWeight[i]/(cumwt+pWeight[i]) );
        cumwt += pWeight[i];
    }
}

inline void matQuaternion::StoreNormalizedLerp(const matQuaternion &rStart, const matQuaternion &rEnd, float fTheta)
{
    x = matLerp(rStart.x, rEnd.x, fTheta);
    y = matLerp(rStart.y, rEnd.y, fTheta);
    z = matLerp(rStart.z, rEnd.z, fTheta);
    w = matLerp(rStart.w, rEnd.w, fTheta);
    Normalize();
}

inline bool matQuaternion::IsNormal(float eps /*=QUAT_LEN_SQ_EPS*/) const
{ 
    float lenSq = LengthSq(); 
    return (fabsf(1.0f - lenSq) < eps);
}


#endif // INCLUDED_matQuaternion///////////////
