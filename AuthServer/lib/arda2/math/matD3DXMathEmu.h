/*****************************************************************************
created:	2002-09-19
copyright:	2002, NCSoft. All Rights Reserved
author(s):	Jason Beardsley

purpose:	Emulation of d3dx9math.h types and functions, for non-Win32.
Note that this is not a complete implementation, just enough to
get the math library to build.
*****************************************************************************/

#ifndef INCLUDED_matD3DXMathEmu
#define INCLUDED_matD3DXMathEmu

#include <cstring>
#include <cmath>
#include <cfloat>
#include <algorithm>

// enable this define to always used the matBase versions of these math functions (not D3DX)
#define NO_MATBASE_REMAPPING


//
// Note: if you're looking for *documentation* on these classes and
// functions, check your friendly neighborhood DirectX SDK.  There
// certainly isn't any here.
//
#if (defined(NO_MATBASE_REMAPPING) || !CORE_SYSTEM_WIN32)

//*
//* Declare types (forwards)
//*

struct matBaseVECTOR3;
struct matBaseVECTOR4;
struct matBaseMATRIX;
struct matBasePLANE;
struct matBaseQUATERNION;

//*
//* Declare API
//*

// VECTOR3

matBaseVECTOR3* matBaseVec3BaryCentric // NOT IMPLEMENTED
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2, const matBaseVECTOR3* pV3, float f, float g);

matBaseVECTOR3* matBaseVec3CatmullRom // NOT IMPLEMENTED
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2, const matBaseVECTOR3* pV3, const matBaseVECTOR3* pV4, float s);

matBaseVECTOR3* matBaseVec3Hermite // NOT IMPLEMENTED
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pT1, const matBaseVECTOR3* pV2, const matBaseVECTOR3* pT2, float s);

matBaseVECTOR3* matBaseVec3Lerp
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2, float s);

matBaseVECTOR3* matBaseVec3Maximize
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2);

matBaseVECTOR3* matBaseVec3Minimize
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2);

matBaseVECTOR3* matBaseVec3Scale
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV, float s);

matBaseVECTOR3* matBaseVec3TransformCoord
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV, const matBaseMATRIX* pM);

matBaseVECTOR3* matBaseVec3TransformNormal
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV, const matBaseMATRIX* pM);

matBaseVECTOR3* matBaseVec3Normalize
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV);

float matBaseVec3Length
(const matBaseVECTOR3* pV);

float matBaseVec3LengthSq
(const matBaseVECTOR3* pV);

float matBaseVec3Dot
(const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2);

matBaseVECTOR3* matBaseVec3Cross
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2);

// VECTOR4

matBaseVECTOR4* matBaseVec4Minimize
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2);

matBaseVECTOR4* matBaseVec4Maximize
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2);

matBaseVECTOR4* matBaseVec4Lerp
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2, float s);

matBaseVECTOR4* matBaseVec4Hermite // NOT IMPLEMENTED
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pT1, const matBaseVECTOR4* pV2, const matBaseVECTOR4* pT2, float s);

matBaseVECTOR4* matBaseVec4CatmullRom // NOT IMPLEMENTED
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV0, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2, const matBaseVECTOR4* pV3, float s);

matBaseVECTOR4* matBaseVec4BaryCentric // NOT IMPLEMENTED
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2, const matBaseVECTOR4* pV3, float f, float g);

matBaseVECTOR4* matBaseVec4Transform
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV, const matBaseMATRIX* pM);

float matBaseVec4Length
(const matBaseVECTOR4* pV);

float matBaseVec4LengthSq
(const matBaseVECTOR4* pV);

float matBaseVec4Dot
(const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2);

matBaseVECTOR4* matBaseVec4Scale
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV, float s);

matBaseVECTOR4* matBaseVec4Cross // NOT IMPLEMENTED
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2, const matBaseVECTOR4* pV3);

matBaseVECTOR4* matBaseVec4Normalize
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV);

// MATRIX

matBaseMATRIX* matBaseMatrixIdentity
(matBaseMATRIX* pOut);

bool matBaseMatrixIsIdentity
(const matBaseMATRIX* pM);

matBaseMATRIX* matBaseMatrixAffineTransformation
(matBaseMATRIX* pOut, float Scaling, const matBaseVECTOR3* pRotationCenter, const matBaseQUATERNION* pRotation, const matBaseVECTOR3* pTranslation);

matBaseMATRIX* matBaseMatrixInverse
(matBaseMATRIX* pOut, float* pDeterminant, const matBaseMATRIX* pM);

float matBaseMatrixDeterminant
(const matBaseMATRIX* pM);

matBaseMATRIX* matBaseMatrixLookAtRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, const matBaseVECTOR3* pEye, const matBaseVECTOR3* pAt, const matBaseVECTOR3* pUp);

matBaseMATRIX* matBaseMatrixLookAtLH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, const matBaseVECTOR3* pEye, const matBaseVECTOR3* pAt, const matBaseVECTOR3* pUp);

matBaseMATRIX* matBaseMatrixMultiply
(matBaseMATRIX* pOut, const matBaseMATRIX* pM1, const matBaseMATRIX* pM2);

matBaseMATRIX* matBaseMatrixMultiplyTranspose
(matBaseMATRIX* pOut, const matBaseMATRIX* pM1, const matBaseMATRIX* pM2);

matBaseMATRIX* matBaseMatrixOrthoRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float w, float h, float zn, float zf);

matBaseMATRIX* matBaseMatrixOrthoLH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float w, float h, float zn, float zf);

matBaseMATRIX* matBaseMatrixOrthoOffCenterRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float l, float r, float b, float t, float zn, float zf);

matBaseMATRIX* matBaseMatrixOrthoOffCenterLH
(matBaseMATRIX* pOut, float l, float r, float b, float t, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float w, float h, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveLH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float w, float h, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveFovRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float fovy, float Aspect, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveFovLH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float fovy, float Aspect, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveOffCenterRH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float l, float r, float b, float t, float zn, float zf);

matBaseMATRIX* matBaseMatrixPerspectiveOffCenterLH // NOT IMPLEMENTED
(matBaseMATRIX* pOut, float l, float r, float b, float t, float zn, float zf);

matBaseMATRIX* matBaseMatrixReflect
(matBaseMATRIX* pOut, const matBasePLANE* pPlane);

matBaseMATRIX* matBaseMatrixRotationX
(matBaseMATRIX* pOut, float Angle);

matBaseMATRIX* matBaseMatrixRotationY
(matBaseMATRIX* pOut, float Angle);

matBaseMATRIX* matBaseMatrixRotationZ
(matBaseMATRIX* pOut, float Angle);

matBaseMATRIX* matBaseMatrixRotationAxis
(matBaseMATRIX* pOut, const matBaseVECTOR3* pV, float Angle);

matBaseMATRIX* matBaseMatrixRotationQuaternion
(matBaseMATRIX* pOut, const matBaseQUATERNION* pQ);

matBaseMATRIX* matBaseMatrixRotationYawPitchRoll
(matBaseMATRIX* pOut, float Yaw, float Pitch, float Roll);

matBaseMATRIX* matBaseMatrixScaling
(matBaseMATRIX* pOut, float sx, float sy, float sz);

matBaseMATRIX* matBaseMatrixShadow // NOT IMPLEMENTED
(matBaseMATRIX* pOut, const matBaseVECTOR4* pLight, const matBasePLANE* pPlane);

matBaseMATRIX* matBaseMatrixTransformation
(matBaseMATRIX* pOut, const matBaseVECTOR3* pScalingCenter, const matBaseQUATERNION* pScalingRotation, const matBaseVECTOR3* pScaling, const matBaseVECTOR3* pRotationCenter, const matBaseQUATERNION* pRotation, const matBaseVECTOR3* pTranslation);

matBaseMATRIX* matBaseMatrixTransformation
(matBaseMATRIX* pOut, const matBaseVECTOR3* pScaling, const matBaseQUATERNION* pRotation, const matBaseVECTOR3* pTranslation);

matBaseMATRIX* matBaseMatrixTranslation
(matBaseMATRIX* pOut, float x, float y, float z);

matBaseMATRIX* matBaseMatrixTranspose
(matBaseMATRIX* pOut, const matBaseMATRIX* pM);

// QUATERNION

matBaseQUATERNION* matBaseQuaternionRotationMatrix
(matBaseQUATERNION* pOut, const matBaseMATRIX* pM);

matBaseQUATERNION* matBaseQuaternionSlerp
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ1, const matBaseQUATERNION* pQ2, float t);

matBaseQUATERNION* matBaseQuaternionMultiply
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ1, const matBaseQUATERNION* pQ2);

matBaseQUATERNION* matBaseQuaternionInverse
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ);

matBaseQUATERNION* matBaseQuaternionNormalize
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ);

matBaseQUATERNION* matBaseQuaternionBaryCentric // NOT IMPLEMENTED
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ1, const matBaseQUATERNION* pQ2, const matBaseQUATERNION* pQ3, float f, float g);

matBaseQUATERNION* matBaseQuaternionConjugate
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ);

float matBaseQuaternionDot
(const matBaseQUATERNION* pQ1, const matBaseQUATERNION* pQ2);

matBaseQUATERNION* matBaseQuaternionExp
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ);

matBaseQUATERNION* matBaseQuaternionIdentity
(matBaseQUATERNION* pOut);

bool matBaseQuaternionIsIdentity
(const matBaseQUATERNION* pQ);

float matBaseQuaternionLength
(const matBaseQUATERNION* pQ);

float matBaseQuaternionLengthSq
(const matBaseQUATERNION* pQ);

matBaseQUATERNION* matBaseQuaternionLn
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ);

matBaseQUATERNION* matBaseQuaternionRotationAxis
(matBaseQUATERNION* pOut, const matBaseVECTOR3* pV, float Angle);

matBaseQUATERNION* matBaseQuaternionRotationYawPitchRoll
(matBaseQUATERNION* pOut, float Yaw, float Pitch, float Roll);

void matBaseQuaternionToAxisAngle
(const matBaseQUATERNION* pQ, matBaseVECTOR3* pAxis, float* pAngle);

// PLANE    

matBasePLANE* matBasePlaneFromPointNormal
(matBasePLANE* pOut, const matBaseVECTOR3* pPoint, const matBaseVECTOR3* pNormal);

matBasePLANE* matBasePlaneFromPoints
(matBasePLANE* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2, const matBaseVECTOR3* pV3);

matBasePLANE* matBasePlaneNormalize
(matBasePLANE* pOut, const matBasePLANE* pP); 


//*
//* Define types
//*
//* Note: this is after the API, so that API functions can be used
//* inside the inlined implementation of class methods.
//*
//* (Less typing than duplicating everything in a .inl file...)
//*

struct matBaseVECTOR3
{
    float x, y, z;

    matBaseVECTOR3() { }

    matBaseVECTOR3(const matBaseVECTOR3& v)
    { x = v.x; y = v.y; z = v.z; }

    matBaseVECTOR3(float fx, float fy, float fz)
    { x = fx; y = fy; z = fz; }

    // casting
    operator float* ()
    {
        return (float *) &x;
    }

    operator const float* () const
    {
        return (const float *) &x;
    }

    matBaseVECTOR3& operator += (const matBaseVECTOR3& v)
    { x += v.x; y += v.y; z += v.z; return *this; }

    matBaseVECTOR3& operator -= (const matBaseVECTOR3& v)
    { x -= v.x; y -= v.y; z -= v.z; return *this; }

    matBaseVECTOR3& operator *= (float f)
    { x *= f; y *= f; z *= f; return *this; }

    matBaseVECTOR3& operator /= (float f)
    { return operator*=(1.0f / f); }

    matBaseVECTOR3 operator + () const
    { return *this; }

    matBaseVECTOR3 operator - () const
    { return matBaseVECTOR3(-x, -y, -z); }

    matBaseVECTOR3 operator + (const matBaseVECTOR3& v) const
    { return matBaseVECTOR3(x + v.x, y + v.y, z + v.z); }

    matBaseVECTOR3 operator - (const matBaseVECTOR3& v) const
    { return matBaseVECTOR3(x - v.x, y - v.y, z - v.z); }

    matBaseVECTOR3 operator * (float f) const
    { return matBaseVECTOR3(x * f, y * f, z * f); }

    matBaseVECTOR3 operator / (float f) const
    { return operator*(1.0f / f); }

    bool operator == (const matBaseVECTOR3& v) const
    { return (x == v.x) && (y == v.y) && (z == v.z); }

    bool operator != (const matBaseVECTOR3& v) const
    { return !operator==(v); }
};

inline matBaseVECTOR3 operator * (float f, const matBaseVECTOR3& v)
{
    return v * f;
}

// VECTOR4

struct matBaseVECTOR4
{
    float x, y, z, w;

    matBaseVECTOR4() { }

    matBaseVECTOR4(float fx, float fy, float fz, float fw)
    { x = fx; y = fy; z = fz; w = fw; }

    matBaseVECTOR4& operator += (const matBaseVECTOR4& v)
    { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }

    matBaseVECTOR4& operator -= (const matBaseVECTOR4& v)
    { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }

    matBaseVECTOR4& operator *= (float f)
    { x *= f; y *= f; z *= f; w *= f; return *this; }

    matBaseVECTOR4& operator /= (float f)
    { return operator*=(1.0f / f); }

    matBaseVECTOR4 operator + () const
    { return *this; }

    matBaseVECTOR4 operator - () const
    { return matBaseVECTOR4(-x, -y, -z, -w); }

    matBaseVECTOR4 operator + (const matBaseVECTOR4& v) const
    { return matBaseVECTOR4(x + v.x, y + v.y, z + v.z, w + v.w); }

    matBaseVECTOR4 operator - (const matBaseVECTOR4& v) const
    { return matBaseVECTOR4(x - v.x, y - v.y, z - v.z, w - v.w); }

    matBaseVECTOR4 operator * (float f) const
    { return matBaseVECTOR4(x * f, y * f, z * f, w * f); }

    matBaseVECTOR4 operator / (float f) const
    { return operator*(1.0f / f); }

    bool operator == (const matBaseVECTOR4& v) const
    { return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w); }

    bool operator != (const matBaseVECTOR4& v) const
    { return !operator==(v); }
};

inline matBaseVECTOR4 operator * (float f, const matBaseVECTOR4& v)
{
    return v * f;
}

// MATRIX (4x4)

struct matBaseMATRIX
{
#if CORE_COMPILER_MSVC
#pragma warning ( push )
#pragma warning ( disable: 4201 )   // no warning about nameless union
#endif
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
#if CORE_COMPILER_MSVC
#pragma warning ( pop )
#endif

    matBaseMATRIX() { }

    matBaseMATRIX(const matBaseMATRIX& mat)
    { std::memcpy(&_11, &mat, sizeof(matBaseMATRIX)); }

    matBaseMATRIX(float f11, float f12, float f13, float f14,
        float f21, float f22, float f23, float f24,
        float f31, float f32, float f33, float f34,
        float f41, float f42, float f43, float f44)
    { _11 = f11; _12 = f12; _13 = f13; _14 = f14;
    _21 = f21; _22 = f22; _23 = f23; _24 = f24;
    _31 = f31; _32 = f32; _33 = f33; _34 = f34;
    _41 = f41; _42 = f42; _43 = f43; _44 = f44; }

    float& operator () (unsigned row, unsigned col)
    {
        static float BIG_FLOAT = FLT_MAX;
        if (row > 3 || col > 3)
        {
            ERR_REPORT(ES_Error, "Invalid matrix indices");
            return BIG_FLOAT;
        }

        return m[row][col];
    }

    float  operator () (unsigned row, unsigned col) const
    {
        if (row > 3 || col > 3)
        {
            ERR_REPORT(ES_Error, "Invalid matrix indices");
            return FLT_MAX;
        }
        return m[row][col];
    }

    matBaseMATRIX& operator *= (const matBaseMATRIX& mat)
    { matBaseMatrixMultiply(this, this, &mat); return *this; }

    matBaseMATRIX& operator += (const matBaseMATRIX& mat)
    { for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 4; ++c)
        m[r][c] += mat.m[r][c];
    return *this; }

    matBaseMATRIX& operator -= (const matBaseMATRIX& mat)
    { for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 4; ++c)
        m[r][c] -= mat.m[r][c];
    return *this; }

    matBaseMATRIX& operator *= (float f)
    { for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 4; ++c)
        m[r][c] *= f;
    return *this; }

    matBaseMATRIX& operator /= (float f)
    { return operator*=(1.0f / f); }

    matBaseMATRIX operator + () const
    { return *this; }

    matBaseMATRIX operator - () const
    { return matBaseMATRIX(-_11, -_12, -_13, -_14,
    -_21, -_22, -_23, -_24,
    -_31, -_32, -_33, -_34,
    -_41, -_42, -_43, -_44); }

    matBaseMATRIX operator * (const matBaseMATRIX& mat) const
    { matBaseMATRIX ret(*this); ret *= mat; return ret; }

    matBaseMATRIX operator + (const matBaseMATRIX& mat) const
    { matBaseMATRIX ret(*this); ret += mat; return ret; }

    matBaseMATRIX operator - (const matBaseMATRIX& mat) const
    { matBaseMATRIX ret(*this); ret -= mat; return ret; }

    matBaseMATRIX operator * (float f) const
    { matBaseMATRIX ret(*this); ret *= f; return ret; }

    matBaseMATRIX operator / (float f) const
    { matBaseMATRIX ret(*this); ret /= f; return ret; }

    bool operator == (const matBaseMATRIX& mat) const
    { return std::memcmp(this, &mat, sizeof(matBaseMATRIX)) == 0; }

    bool operator != (const matBaseMATRIX& mat) const
    { return !operator==(mat); }
};

inline matBaseMATRIX operator*(float f, const matBaseMATRIX& mat)
{
    matBaseMATRIX ret(mat);
    ret *= f;
    return ret;
}

// QUATERNION

struct matBaseQUATERNION
{
    float x, y, z, w;

    matBaseQUATERNION() { }

    explicit
        matBaseQUATERNION(const float* pf)
    { x = pf[0]; y = pf[1]; z = pf[2]; w = pf[3]; }

    matBaseQUATERNION(float fx, float fy, float fz, float fw)
    { x = fx; y = fy; z = fz; w = fw; }

    matBaseQUATERNION& operator += (const matBaseQUATERNION& q)
    { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }

    matBaseQUATERNION& operator -= (const matBaseQUATERNION& q)
    { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }

    matBaseQUATERNION& operator *= (const matBaseQUATERNION& q)
    { matBaseQuaternionMultiply(this, this, &q); return *this; }

    matBaseQUATERNION& operator *= (float f)
    { x *= f; y *= f; z *= f; w *= f; return *this; }

    matBaseQUATERNION& operator /= (float f)
    { return operator*=(1.0f / f); }

    matBaseQUATERNION operator + () const
    { return *this; }

    matBaseQUATERNION operator - () const
    { return matBaseQUATERNION(-x, -y, -z, -w); }

    matBaseQUATERNION operator + (const matBaseQUATERNION& q) const
    { return matBaseQUATERNION(x + q.x, y + q.y, z + q.z, w + q.w); }

    matBaseQUATERNION operator - (const matBaseQUATERNION& q) const
    { return matBaseQUATERNION(x - q.x, y - q.y, z - q.z, w - q.w); }

    matBaseQUATERNION operator * (const matBaseQUATERNION& q) const
    { matBaseQUATERNION ret(*this); ret *= q; return ret; }

    matBaseQUATERNION operator * (float f) const
    { return matBaseQUATERNION(x * f, y * f, z * f, w * f); }

    matBaseQUATERNION operator / (float f) const
    { return operator*(1.0f / f); }

    bool operator == (const matBaseQUATERNION& q) const
    { return (x == q.x) && (y == q.y) && (z == q.z) && (w == q.w); }

    bool operator != (const matBaseQUATERNION& q) const
    { return !operator==(q); }
};

inline matBaseQUATERNION operator*(float f, const matBaseQUATERNION& q)
{
    matBaseQUATERNION ret(q);
    ret *= f;
    return ret;
}

// PLANE

struct matBasePLANE
{
    float a, b, c, d;

    matBasePLANE() { }

    matBasePLANE(float fa, float fb, float fc, float fd)
    { a = fa; b = fb; c = fc; d = fd; }

    matBasePLANE operator + () const
    { return *this; }

    matBasePLANE operator - () const
    { return matBasePLANE(-a, -b, -c, -d); }

    bool operator == (const matBasePLANE& p) const
    { return (a == p.a) && (b == p.b) && (c == p.c) && (d == p.d); }

    bool operator != (const matBasePLANE& p) const
    { return !operator==(p); }
};


//*
//* Define inline API functions
//*

// VECTOR3

inline float matBaseVec3Dot
(const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2)
{
    return (pV1->x * pV2->x) + (pV1->y * pV2->y) + (pV1->z * pV2->z);
}

inline float matBaseVec3LengthSq
(const matBaseVECTOR3* pV)
{
    return matBaseVec3Dot(pV, pV);
}

inline float matBaseVec3Length
(const matBaseVECTOR3* pV)
{
    return std::sqrt(matBaseVec3LengthSq(pV));
}

inline matBaseVECTOR3* matBaseVec3Minimize
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2)
{
    // note: aliasing is OK
    pOut->x = Min(pV1->x, pV2->x);
    pOut->y = Min(pV1->y, pV2->y);
    pOut->z = Min(pV1->z, pV2->z);
    return pOut;
}

inline matBaseVECTOR3* matBaseVec3Maximize
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2)
{
    // note: aliasing is OK
    pOut->x = Max(pV1->x, pV2->x);
    pOut->y = Max(pV1->y, pV2->y);
    pOut->z = Max(pV1->z, pV2->z);
    return pOut;
}

inline matBaseVECTOR3* matBaseVec3Scale
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV, float s)
{
    // Out = V * s;
    // note: aliasing is OK
    pOut->x = pV->x * s;
    pOut->y = pV->y * s;
    pOut->z = pV->z * s;
    return pOut;
}

inline matBaseVECTOR3* matBaseVec3Lerp
(matBaseVECTOR3* pOut, const matBaseVECTOR3* pV1, const matBaseVECTOR3* pV2, float s)
{
    // Out = V1 + s * (V2 - V1);
    // note: aliasing is OK
    pOut->x = pV1->x + s * (pV2->x - pV1->x);
    pOut->y = pV1->y + s * (pV2->y - pV1->y);
    pOut->z = pV1->z + s * (pV2->z - pV1->z);
    return pOut;
}

// VECTOR4

inline float matBaseVec4Dot
(const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2)
{
    return (pV1->x * pV2->x) + (pV1->y * pV2->y) + (pV1->z * pV2->z) + (pV1->w * pV2->w);
}

inline float matBaseVec4LengthSq
(const matBaseVECTOR4* pV)
{
    return matBaseVec4Dot(pV, pV);
}

inline float matBaseVec4Length
(const matBaseVECTOR4* pV)
{
    return std::sqrt(matBaseVec4LengthSq(pV));
}

inline matBaseVECTOR4* matBaseVec4Minimize
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2)
{
    // note: aliasing is OK
    pOut->x = Min(pV1->x, pV2->x);
    pOut->y = Min(pV1->y, pV2->y);
    pOut->z = Min(pV1->z, pV2->z);
    pOut->w = Min(pV1->w, pV2->w);
    return pOut;
}

inline matBaseVECTOR4* matBaseVec4Maximize
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2)
{
    // note: aliasing is OK
    pOut->x = Max(pV1->x, pV2->x);
    pOut->y = Max(pV1->y, pV2->y);
    pOut->z = Max(pV1->z, pV2->z);
    pOut->w = Max(pV1->w, pV2->w);
    return pOut;
}

inline matBaseVECTOR4* matBaseVec4Scale
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV, float s)
{
    // Out = V * s;
    // note: aliasing is OK
    pOut->x = pV->x * s;
    pOut->y = pV->y * s;
    pOut->z = pV->z * s;
    pOut->w = pV->w * s;
    return pOut;
}

inline matBaseVECTOR4* matBaseVec4Lerp
(matBaseVECTOR4* pOut, const matBaseVECTOR4* pV1, const matBaseVECTOR4* pV2, float s)
{
    // Out = V1 + s * (V2 - V1);
    // note: aliasing is OK
    pOut->x = pV1->x + s * (pV2->x - pV1->x);
    pOut->y = pV1->y + s * (pV2->y - pV1->y);
    pOut->z = pV1->z + s * (pV2->z - pV1->z);
    pOut->w = pV1->w + s * (pV2->w - pV1->w);
    return pOut;
}

// MATRIX

static const matBaseMATRIX s_identityMat(1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f);

inline matBaseMATRIX* matBaseMatrixIdentity
(matBaseMATRIX* pOut)
{
    *pOut = s_identityMat;
    return pOut;
}

inline bool matBaseMatrixIsIdentity
(const matBaseMATRIX* pM)
{
    return *pM == s_identityMat;
}

inline matBaseMATRIX* matBaseMatrixScaling
(matBaseMATRIX* pOut,
 float sx,
 float sy,
 float sz)
{
    // sx  0  0  0
    //  0 sy  0  0
    //  0  0 sz  0
    //  0  0  0  1
    matBaseMatrixIdentity(pOut);
    pOut->_11 = sx;
    pOut->_22 = sy;
    pOut->_33 = sz;
    return pOut;
}

inline matBaseMATRIX* matBaseMatrixTranslation
(matBaseMATRIX* pOut,
 float x,
 float y,
 float z)
{
    //  1  0  0  0
    //  0  1  0  0
    //  0  0  1  0
    // tx ty tz  1
    matBaseMatrixIdentity(pOut);
    pOut->_41 = x;
    pOut->_42 = y;
    pOut->_43 = z;
    return pOut;
}

// QUATERNION

inline float matBaseQuaternionDot
(const matBaseQUATERNION* pQ1, const matBaseQUATERNION* pQ2)
{
    return (pQ1->x * pQ2->x) + (pQ1->y * pQ2->y) + (pQ1->z * pQ2->z) + (pQ1->w * pQ2->w);
}

inline float matBaseQuaternionLengthSq
(const matBaseQUATERNION* pQ)
{
    return matBaseQuaternionDot(pQ, pQ);
}

inline float matBaseQuaternionLength
(const matBaseQUATERNION* pQ)
{
    return std::sqrt(matBaseQuaternionLengthSq(pQ));
}

inline matBaseQUATERNION* matBaseQuaternionIdentity
(matBaseQUATERNION* pOut)
{
    pOut->x = 0.0f;
    pOut->y = 0.0f;
    pOut->z = 0.0f;
    pOut->w = 1.0f;
    return pOut;
}

inline bool matBaseQuaternionIsIdentity
(const matBaseQUATERNION* pQ)
{
    return (pQ->x == 0.0f) && (pQ->y == 0.0f) && (pQ->z == 0.0f) && (pQ->w == 1.0f);
}

inline matBaseQUATERNION* matBaseQuaternionConjugate
(matBaseQUATERNION* pOut, const matBaseQUATERNION* pQ)
{
    // note: aliasing is OK
    pOut->x = -pQ->x;
    pOut->y = -pQ->y;
    pOut->z = -pQ->z;
    pOut->w = pQ->w;
    return pOut;
}



#else
#include <d3dx9math.h>

// remap all matBase functions and types onto D3DX
#define matBaseVECTOR3          D3DXVECTOR3             
#define matBaseVECTOR4          D3DXVECTOR4             
#define matBaseMATRIX           D3DXMATRIX              
#define matBasePLANE            D3DXPLANE               
#define matBaseQUATERNION       D3DXQUATERNION          

// VECTOR3
#define matBaseVec3BaryCentric      D3DXVec3BaryCentric     
#define matBaseVec3CatmullRom       D3DXVec3CatmullRom      
#define matBaseVec3Hermite          D3DXVec3Hermite         
#define matBaseVec3Lerp             D3DXVec3Lerp            
#define matBaseVec3Maximize         D3DXVec3Maximize        
#define matBaseVec3Minimize         D3DXVec3Minimize        
#define matBaseVec3Scale            D3DXVec3Scale           
#define matBaseVec3TransformCoord   D3DXVec3TransformCoord  
#define matBaseVec3TransformNormal  D3DXVec3TransformNormal 
#define matBaseVec3Normalize        D3DXVec3Normalize       
#define matBaseVec3Length           D3DXVec3Length          
#define matBaseVec3LengthSq         D3DXVec3LengthSq        
#define matBaseVec3Dot              D3DXVec3Dot             
#define matBaseVec3Cross            D3DXVec3Cross           

// VECTOR4

#define matBaseVec4Minimize         D3DXVec4Minimize        
#define matBaseVec4Maximize         D3DXVec4Maximize        
#define matBaseVec4Lerp             D3DXVec4Lerp            
#define matBaseVec4Hermite          D3DXVec4Hermite         
#define matBaseVec4CatmullRom       D3DXVec4CatmullRom      
#define matBaseVec4BaryCentric      D3DXVec4BaryCentric     
#define matBaseVec4Transform        D3DXVec4Transform       
#define matBaseVec4Length           D3DXVec4Length          
#define matBaseVec4LengthSq         D3DXVec4LengthSq        
#define matBaseVec4Dot              D3DXVec4Dot             
#define matBaseVec4Scale            D3DXVec4Scale           
#define matBaseVec4Cross            D3DXVec4Cross           
#define matBaseVec4Normalize        D3DXVec4Normalize       

// MATRIX

#define matBaseMatrixIdentity                   D3DXMatrixIdentity                  
#define matBaseMatrixIsIdentity                 D3DXMatrixIsIdentity                
#define matBaseMatrixAffineTransformation       D3DXMatrixAffineTransformation      
#define matBaseMatrixInverse                    D3DXMatrixInverse                   
#define matBaseMatrixDeterminant                D3DXMatrixDeterminant               
#define matBaseMatrixLookAtRH                   D3DXMatrixLookAtRH                  
#define matBaseMatrixLookAtLH                   D3DXMatrixLookAtLH                  
#define matBaseMatrixMultiply                   D3DXMatrixMultiply                  
#define matBaseMatrixMultiplyTranspose          D3DXMatrixMultiplyTranspose         
#define matBaseMatrixOrthoRH                    D3DXMatrixOrthoRH                   
#define matBaseMatrixOrthoLH                    D3DXMatrixOrthoLH                   
#define matBaseMatrixOrthoOffCenterRH           D3DXMatrixOrthoOffCenterRH          
#define matBaseMatrixOrthoOffCenterLH           D3DXMatrixOrthoOffCenterLH          
#define matBaseMatrixPerspectiveRH              D3DXMatrixPerspectiveRH             
#define matBaseMatrixPerspectiveLH              D3DXMatrixPerspectiveLH             
#define matBaseMatrixPerspectiveFovRH           D3DXMatrixPerspectiveFovRH          
#define matBaseMatrixPerspectiveFovLH           D3DXMatrixPerspectiveFovLH          
#define matBaseMatrixPerspectiveOffCenterRH     D3DXMatrixPerspectiveOffCenterRH    
#define matBaseMatrixPerspectiveOffCenterLH     D3DXMatrixPerspectiveOffCenterLH    
#define matBaseMatrixReflect                    D3DXMatrixReflect                   
#define matBaseMatrixRotationX                  D3DXMatrixRotationX                 
#define matBaseMatrixRotationY                  D3DXMatrixRotationY                 
#define matBaseMatrixRotationZ                  D3DXMatrixRotationZ                 
#define matBaseMatrixRotationAxis               D3DXMatrixRotationAxis              
#define matBaseMatrixRotationQuaternion         D3DXMatrixRotationQuaternion        
#define matBaseMatrixRotationYawPitchRoll       D3DXMatrixRotationYawPitchRoll      
#define matBaseMatrixScaling                    D3DXMatrixScaling                   
#define matBaseMatrixShadow                     D3DXMatrixShadow                    
#define matBaseMatrixTransformation             D3DXMatrixTransformation            
#define matBaseMatrixTranslation                D3DXMatrixTranslation               
#define matBaseMatrixTranspose                  D3DXMatrixTranspose                 
//#define D3DXMatrixTransformation(pOut, pS, pR, pT) D3DXMatrixTransformation((pOut), NULL, NULL, (pS), NULL, (pR), (pT))
inline matBaseMATRIX* D3DXMatrixTransformation(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pScaling,
    const matBaseQUATERNION* pRotation,
    const matBaseVECTOR3* pTranslation)
{
    return D3DXMatrixTransformation(pOut, NULL, NULL, pScaling, NULL, pRotation, pTranslation);
}

// QUATERNION

#define matBaseQuaternionRotationMatrix         D3DXQuaternionRotationMatrix        
#define matBaseQuaternionSlerp                  D3DXQuaternionSlerp                 
#define matBaseQuaternionMultiply               D3DXQuaternionMultiply              
#define matBaseQuaternionInverse                D3DXQuaternionInverse               
#define matBaseQuaternionNormalize              D3DXQuaternionNormalize             
#define matBaseQuaternionBaryCentric            D3DXQuaternionBaryCentric           
#define matBaseQuaternionConjugate              D3DXQuaternionConjugate             
#define matBaseQuaternionDot                    D3DXQuaternionDot                   
#define matBaseQuaternionExp                    D3DXQuaternionExp                   
#define matBaseQuaternionIdentity               D3DXQuaternionIdentity              
#define matBaseQuaternionIsIdentity             D3DXQuaternionIsIdentity            
#define matBaseQuaternionLength                 D3DXQuaternionLength                
#define matBaseQuaternionLengthSq               D3DXQuaternionLengthSq              
#define matBaseQuaternionLn                     D3DXQuaternionLn                    
#define matBaseQuaternionRotationAxis           D3DXQuaternionRotationAxis          
#define matBaseQuaternionRotationYawPitchRoll   D3DXQuaternionRotationYawPitchRoll  
#define matBaseQuaternionToAxisAngle            D3DXQuaternionToAxisAngle           

// PLANE    

#define matBasePlaneFromPointNormal             D3DXPlaneFromPointNormal    
#define matBasePlaneFromPoints                  D3DXPlaneFromPoints         
#define matBasePlaneNormalize                   D3DXPlaneNormalize          

static const matBaseMATRIX s_identityMat(1.0f, 0.0f, 0.0f, 0.0f,
										 0.0f, 1.0f, 0.0f, 0.0f,
										 0.0f, 0.0f, 1.0f, 0.0f,
										 0.0f, 0.0f, 0.0f, 1.0f);

#endif

#endif // INCLUDED_matD3DXMathEmu
