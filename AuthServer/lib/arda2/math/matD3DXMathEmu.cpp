/*****************************************************************************
created:    2002/09/20
copyright:  2002, NCsoft. All Rights Reserved
author(s):  Jason Beardsley

purpose:	
*****************************************************************************/

#define NO_MATBASE_REMAPPING
#include "arda2/math/matFirst.h"
#include "arda2/math/matD3DXMathEmu.h"

//
// Helpers
//

// Return true if given float is approximately zero

#define IsEqual_ZERO(f) (std::fabs(f) < kDefaultAbsError)

// Return true if given matrix is a standard transformation matrix,
// i.e. last column is [0 0 0 1]T.

#define IS_TRANSFORM_MATRIX(m) (((m)._14 == 0.0f) && ((m)._24 == 0.0f) && ((m)._34 == 0.0f) && ((m)._44 == 1.0f))


//*
//* API functions
//*

// VECTOR3

matBaseVECTOR3* matBaseVec3BaryCentric(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV1,
    const matBaseVECTOR3* pV2,
    const matBaseVECTOR3* pV3,
    float f,
    float g)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pV2);
    UNREF(pV3);
    UNREF(f);
    UNREF(g);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR3* matBaseVec3CatmullRom(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV1,
    const matBaseVECTOR3* pV2,
    const matBaseVECTOR3* pV3,
    const matBaseVECTOR3* pV4,
    float s)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pV2);
    UNREF(pV3);
    UNREF(pV4);
    UNREF(s);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR3* matBaseVec3Hermite(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV1,
    const matBaseVECTOR3* pT1,
    const matBaseVECTOR3* pV2,
    const matBaseVECTOR3* pT2,
    float s)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pT1);
    UNREF(pV2);
    UNREF(pT2);
    UNREF(s);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR3* matBaseVec3TransformCoord(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV,
    const matBaseMATRIX* pM )
{
    // transform (x, y, z, 1)
    matBaseVECTOR4 v4(pV->x, pV->y, pV->z, 1.0f);
    matBaseVec4Transform(&v4, &v4, pM);
    // copy back, making sure w==1
    if (IsEqual(v4.w, 1.0f))
    {
        // w is already 1
        pOut->x = v4.x;
        pOut->y = v4.y;
        pOut->z = v4.z;
    }
    else if (!IsEqual_ZERO(v4.w))
    {
        // w != 1, but it's not zero
        float wInv = 1.0f / v4.w;
        pOut->x = v4.x * wInv;
        pOut->y = v4.y * wInv;
        pOut->z = v4.z * wInv;
    }
    else
    {
        // w == 0? that's unpossible!
        pOut->x = v4.x;
        pOut->y = v4.y;
        pOut->z = v4.z;
    }
    return pOut;
}

matBaseVECTOR3* matBaseVec3TransformNormal(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV,
    const matBaseMATRIX* pM )
{
    // transform (x, y, z, 0)
    matBaseVECTOR4 v4(pV->x, pV->y, pV->z, 0.0f);
    matBaseVec4Transform(&v4, &v4, pM);
    // copy back
    pOut->x = v4.x;
    pOut->y = v4.y;
    pOut->z = v4.z;
    return pOut;
}

matBaseVECTOR3* matBaseVec3Normalize(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV )
{
    float vLen = matBaseVec3Length(pV);
    if (IsEqual_ZERO(vLen))
    {
        return pOut; // zero-length vector
    }
    matBaseVec3Scale(pOut, pV, (1.0f / vLen));
    return pOut;
}

matBaseVECTOR3* matBaseVec3Cross(
    matBaseVECTOR3* pOut,
    const matBaseVECTOR3* pV1,
    const matBaseVECTOR3* pV2 )
{
    matBaseVECTOR3 out; // temporary, because of possible aliasing
    out.x = (pV1->y * pV2->z) - (pV1->z * pV2->y);
    out.y = (pV1->z * pV2->x) - (pV1->x * pV2->z);
    out.z = (pV1->x * pV2->y) - (pV1->y * pV2->x);
    *pOut = out;
    return pOut;
}

// VECTOR4

matBaseVECTOR4* matBaseVec4Hermite(
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV1,
    const matBaseVECTOR4* pT1,
    const matBaseVECTOR4* pV2,
    const matBaseVECTOR4* pT2,
    float s)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pT1);
    UNREF(pV2);
    UNREF(pT2);
    UNREF(s);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR4* matBaseVec4CatmullRom( 
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV0,
    const matBaseVECTOR4* pV1,
    const matBaseVECTOR4* pV2,
    const matBaseVECTOR4* pV3,
    float s)
{
    // TODO: implement iff necessary
    UNREF(pV0);
    UNREF(pV1);
    UNREF(pV2);
    UNREF(pV3);
    UNREF(s);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR4* matBaseVec4BaryCentric(
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV1,
    const matBaseVECTOR4* pV2,
    const matBaseVECTOR4* pV3,
    float f,
    float g)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pV2);
    UNREF(pV3);
    UNREF(f);
    UNREF(g);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR4* matBaseVec4Transform(
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV,
    const matBaseMATRIX* pM )
{
    // Out = V * M
    matBaseVECTOR4 out; // temporary, because of possible aliasing
    // check for common optimizable cases
    if (IS_TRANSFORM_MATRIX(*pM))
    {
        // last column of M = [0 0 0 1]T; i.e. _14=_24=_34=0, _44=1
        if (IsEqual_ZERO(pV->w))
        {
            out.x = (pV->x * pM->_11) + (pV->y * pM->_21) + (pV->z * pM->_31);
            out.y = (pV->x * pM->_12) + (pV->y * pM->_22) + (pV->z * pM->_32);
            out.z = (pV->x * pM->_13) + (pV->y * pM->_23) + (pV->z * pM->_33);
            out.w = 0.0f;
        }
        else if (IsEqual(pV->w, 1.0f))
        {
            out.x = (pV->x * pM->_11) + (pV->y * pM->_21) + (pV->z * pM->_31) + pM->_41;
            out.y = (pV->x * pM->_12) + (pV->y * pM->_22) + (pV->z * pM->_32) + pM->_42;
            out.z = (pV->x * pM->_13) + (pV->y * pM->_23) + (pV->z * pM->_33) + pM->_43;
            out.w = 1.0f;
        }
        else
        {
            out.x = (pV->x * pM->_11) + (pV->y * pM->_21) + (pV->z * pM->_31) + (pV->w * pM->_41);
            out.y = (pV->x * pM->_12) + (pV->y * pM->_22) + (pV->z * pM->_32) + (pV->w * pM->_42);
            out.z = (pV->x * pM->_13) + (pV->y * pM->_23) + (pV->z * pM->_33) + (pV->w * pM->_43);
            out.w = pV->w;
        }
    }
    else
    {
        // multiply it all out
        out.x = (pV->x * pM->_11) + (pV->y * pM->_21) + (pV->z * pM->_31) + (pV->w * pM->_41);
        out.y = (pV->x * pM->_12) + (pV->y * pM->_22) + (pV->z * pM->_32) + (pV->w * pM->_42);
        out.z = (pV->x * pM->_13) + (pV->y * pM->_23) + (pV->z * pM->_33) + (pV->w * pM->_43);
        out.w = (pV->x * pM->_14) + (pV->y * pM->_24) + (pV->z * pM->_34) + (pV->w * pM->_44);
    }
    *pOut = out;
    return pOut;
}

matBaseVECTOR4* matBaseVec4Cross(
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV1,
    const matBaseVECTOR4* pV2,
    const matBaseVECTOR4* pV3)
{
    // TODO: implement iff necessary
    UNREF(pV1);
    UNREF(pV2);
    UNREF(pV3);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseVECTOR4* matBaseVec4Normalize(
    matBaseVECTOR4* pOut,
    const matBaseVECTOR4* pV )
{
    float vLen = matBaseVec4Length(pV);
    if (IsEqual_ZERO(vLen))
    {
        return pOut;
    }
    matBaseVec4Scale(pOut, pV, (1.0f / vLen));
    return pOut;
}

// MATRIX

matBaseMATRIX* matBaseMatrixAffineTransformation
    (   matBaseMATRIX* pOut,
        float Scaling,
        const matBaseVECTOR3* pRotationCenter,
        const matBaseQUATERNION* pRotation,
        const matBaseVECTOR3* pTranslation )
{
    // Logically:
    //   Ms = scaling matrix
    //   Mrc = center of rotation
    //   Mr = rotation
    //   Mt = translation
    //
    // Out = Ms * (Mrc-1 * (Mr * (Mrc * Mt)))

#if 1

    // Mt
    if ( pTranslation != NULL )
    {
        // add translation to output
        pOut->_41 = pTranslation->x;
        pOut->_42 = pTranslation->y;
        pOut->_43 = pTranslation->z;
    }
    else
    {
        // reset translation
        pOut->_41 = 0.0f;
        pOut->_42 = 0.0f;
        pOut->_43 = 0.0f;
    }

    // Mrc
    bool needMrcInv = false;
    if ((pRotationCenter != NULL) && 
        ((pRotationCenter->x != 0.0f) || (pRotationCenter->y != 0.0f) || (pRotationCenter->z != 0.0f)))
    {
        // add translation to output - it is only a translation matrix by here
        pOut->_41 += pRotationCenter->x;
        pOut->_42 += pRotationCenter->y;
        pOut->_43 += pRotationCenter->z;
        needMrcInv = true;
    }

    // Mr
    bool didRotation = false;
    if ( pRotation != NULL && !matBaseQuaternionIsIdentity(pRotation) )
    {
        matBaseMATRIX Mr;
        matBaseMatrixRotationQuaternion(&Mr, pRotation);
        // since we can only be a translation matrix, no need to do the
        // full multiply, just copy the rotational part
        pOut->_11 = Mr._11; pOut->_12 = Mr._12; pOut->_13 = Mr._13;
        pOut->_21 = Mr._21; pOut->_22 = Mr._22; pOut->_23 = Mr._23;
        pOut->_31 = Mr._31; pOut->_32 = Mr._32; pOut->_33 = Mr._33;
        didRotation = true;
    }
    else
    {
        // reset orientation to identity
        pOut->_11 = 1.0f; pOut->_12 = 0.0f; pOut->_13 = 0.0f;
        pOut->_21 = 0.0f; pOut->_22 = 1.0f; pOut->_23 = 0.0f;
        pOut->_31 = 0.0f; pOut->_32 = 0.0f; pOut->_33 = 1.0f;
    }

    // Mrc-1
    if ( needMrcInv )
    {
        if (!didRotation)
        {
            // still just a translation matrix, excellent
            pOut->_41 -= pRotationCenter->x;
            pOut->_42 -= pRotationCenter->y;
            pOut->_43 -= pRotationCenter->z;
        }
        else
        {
            // have to do a real multiply
            matBaseMATRIX MrcI;
            matBaseMatrixTranslation(&MrcI, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
            matBaseMatrixMultiply(pOut, &MrcI, pOut);
        }
    }

    // Ms
    if (!IsEqual(Scaling, 1.0f))
    {
        pOut->_11 *= Scaling;
        pOut->_12 *= Scaling;
        pOut->_13 *= Scaling;
        pOut->_14 *= Scaling;
        pOut->_21 *= Scaling;
        pOut->_22 *= Scaling;
        pOut->_23 *= Scaling;
        pOut->_24 *= Scaling;
        pOut->_31 *= Scaling;
        pOut->_32 *= Scaling;
        pOut->_33 *= Scaling;
        pOut->_34 *= Scaling;
    }

    return pOut;

#endif

#if 0

    // This is the really slow method.  Amazing what profiling shows
    // you...matrix multiplication is slow and you should avoid it
    // whenever possible.

    // build scaling matrix
    matBaseMATRIX Ms;
    matBaseMATRIX* pMs = NULL;
    if (!IsEqual(Scaling, 1.0f))
    {
        matBaseMatrixScaling(&Ms, Scaling, Scaling, Scaling);
        pMs = &Ms;
    }

    // build rotation center matrices
    matBaseMATRIX Mrc, MrcI;
    matBaseMATRIX* pMrc = NULL;
    matBaseMATRIX* pMrcI = NULL;
    if ((pRotationCenter != NULL) && 
        ((pRotationCenter->x != 0.0f) || (pRotationCenter->y != 0.0f) || (pRotationCenter->z != 0.0f)))
    {
        matBaseMatrixTranslation(&Mrc, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
        matBaseMatrixTranslation(&MrcI, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
        pMrc = &Mrc;
        pMrcI = &MrcI;
    }

    // build rotation matrix
    matBaseMATRIX Mr;
    matBaseMATRIX* pMr = NULL;
    if ((pRotation != NULL) && !matBaseQuaternionIsIdentity(pRotation))
    {
        matBaseMatrixRotationQuaternion(&Mr, pRotation);
        pMr = &Mr;
    }

    // build translation matrix
    matBaseMATRIX Mt;
    matBaseMATRIX* pMt = NULL;
    if ((pTranslation != NULL) &&
        ((pTranslation->x != 0.0f) || (pTranslation->y != 0.0f) || (pTranslation->z != 0.0f)))
    {
        matBaseMatrixTranslation(&Mt, pTranslation->x, pTranslation->y, pTranslation->z);
        pMt = &Mt;
    }

    // Ms * (Mrc-1 * (Mr * (Mrc * Mt)))

    // initialize output
    if (pMt != NULL)
    {
        *pOut = *pMt;
    }
    else
    {
        matBaseMatrixIdentity(pOut);
    }

    // multiply
    if (pMrc  != NULL) matBaseMatrixMultiply(pOut, pMrc, pOut);
    if (pMr   != NULL) matBaseMatrixMultiply(pOut, pMr, pOut);
    if (pMrcI != NULL) matBaseMatrixMultiply(pOut, pMrcI, pOut);
    if (pMs   != NULL) matBaseMatrixMultiply(pOut, pMs, pOut);

    // done
    return pOut;

#endif
}

static inline float _ComputeSubDeterminant(const matBaseMATRIX* pM, int rx, int cx)
{
    // compute determinant of 3x3 matrix that you get if you remove row
    // r and col c from M.
    // first get the 3x3 matrix
    float a, b, c, d, e, f, g, h, i;
    float* m2[3][3] = { { &a, &b, &c },
        { &d, &e, &f },
        { &g, &h, &i } };
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            int r2 = (row < rx) ? row : row+1;
            int c2 = (col < cx) ? col : col+1;
            *m2[row][col] = pM->m[r2][c2];
        }
    }
    // then compute its determinant
    float ret = 0.0f;
    ret += a * (e*i - f*h);
    ret -= b * (d*i - f*g);
    ret += c * (d*h - e*g);
    return ret;
}

matBaseMATRIX* matBaseMatrixInverse
    (   matBaseMATRIX* pOut,
        float* pDeterminant,
        const matBaseMATRIX* pM )
{
    // compute determinant
    float det = matBaseMatrixDeterminant(pM);
    if (pDeterminant)
    {
        *pDeterminant = det;
    }
    // zero -> no inverse
    if (IsEqual_ZERO(det))
    {
        return pOut;
    }
    // compute adjoint matrix
    matBaseMATRIX adj;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            float val = _ComputeSubDeterminant(pM, col, row);
            if (((row + col) % 2) != 0) // row+col odd -> negate
            {
                if (val != 0.0f) // avoid -0
                    val = -val;
            }
            adj.m[row][col] = val;
        }
    }
    // multiply by inverse determinant
    adj *= (1.0f / det);
    *pOut = adj;
    return pOut;
}

float matBaseMatrixDeterminant
    ( const matBaseMATRIX* pM )
{
    // reduce typing...
    // a b c d
    // e f g h
    // i j k l
    // m n o p
    float a = pM->_11, b = pM->_12, c = pM->_13, d = pM->_14;
    float e = pM->_21, f = pM->_22, g = pM->_23, h = pM->_24;
    float i = pM->_31, j = pM->_32, k = pM->_33, l = pM->_34;
    float m = pM->_41, n = pM->_42, o = pM->_43, p = pM->_44;

    // compute 2x2 determinants
    float x1 = k*p - l*o;
    float x2 = j*p - l*n;
    float x3 = j*o - k*n;
    float x4 = i*p - l*m;
    float x5 = i*o - k*m;
    float x6 = i*n - j*m;

    // finally compute it
    float ret = 0.0f;
    ret += a * (f*x1 - g*x2 + h*x3);
    ret -= b * (e*x1 - g*x4 + h*x5);
    ret += c * (e*x2 - f*x4 + h*x6);
    ret -= d * (e*x3 - f*x5 + g*x6);
    return ret;
}

matBaseMATRIX* matBaseMatrixLookAtRH(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pEye,
    const matBaseVECTOR3* pAt,
    const matBaseVECTOR3* pUp)
{
    // TODO: implement iff necessary
    UNREF(pEye);
    UNREF(pAt);
    UNREF(pUp);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixLookAtLH(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pEye,
    const matBaseVECTOR3* pAt,
    const matBaseVECTOR3* pUp)
{
    // TODO: implement iff necessary
    UNREF(pEye);
    UNREF(pAt);
    UNREF(pUp);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixMultiply(
    matBaseMATRIX* pOut,
    const matBaseMATRIX* pM1,
    const matBaseMATRIX* pM2)
{
    // Out = M1 * M2;

#if 0
    // this isn't fast
    matBaseMATRIX out; // temporary, because of possible aliasing
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            out.m[row][col] =
                (pM1->m[row][0] * pM2->m[0][col]) +
                (pM1->m[row][1] * pM2->m[1][col]) +
                (pM1->m[row][2] * pM2->m[2][col]) +
                (pM1->m[row][3] * pM2->m[3][col]);
        }
    }
    *pOut = out;
    return pOut;
#endif

#if 1
    // this has the loops unrolled, and is faster (?)

    // copy M1 and M2 into temporaries, in case of aliasing (and let the
    // compiler also (hopefully) not worry about aliasing...)
    float a11 = pM1->_11, a12 = pM1->_12, a13 = pM1->_13, a14 = pM1->_14;
    float a21 = pM1->_21, a22 = pM1->_22, a23 = pM1->_23, a24 = pM1->_24;
    float a31 = pM1->_31, a32 = pM1->_32, a33 = pM1->_33, a34 = pM1->_34;
    float a41 = pM1->_41, a42 = pM1->_42, a43 = pM1->_43, a44 = pM1->_44;

    float b11 = pM2->_11, b12 = pM2->_12, b13 = pM2->_13, b14 = pM2->_14;
    float b21 = pM2->_21, b22 = pM2->_22, b23 = pM2->_23, b24 = pM2->_24;
    float b31 = pM2->_31, b32 = pM2->_32, b33 = pM2->_33, b34 = pM2->_34;
    float b41 = pM2->_41, b42 = pM2->_42, b43 = pM2->_43, b44 = pM2->_44;

    if (!IS_TRANSFORM_MATRIX(*pM1) || !IS_TRANSFORM_MATRIX(*pM2))
    {
        // general matrices
        pOut->_11 = (a11 * b11) + (a12 * b21) + (a13 * b31) + (a14 * b41);
        pOut->_12 = (a11 * b12) + (a12 * b22) + (a13 * b32) + (a14 * b42);
        pOut->_13 = (a11 * b13) + (a12 * b23) + (a13 * b33) + (a14 * b43);
        pOut->_14 = (a11 * b14) + (a12 * b24) + (a13 * b34) + (a14 * b44);

        pOut->_21 = (a21 * b11) + (a22 * b21) + (a23 * b31) + (a24 * b41);
        pOut->_22 = (a21 * b12) + (a22 * b22) + (a23 * b32) + (a24 * b42);
        pOut->_23 = (a21 * b13) + (a22 * b23) + (a23 * b33) + (a24 * b43);
        pOut->_24 = (a21 * b14) + (a22 * b24) + (a23 * b34) + (a24 * b44);

        pOut->_31 = (a31 * b11) + (a32 * b21) + (a33 * b31) + (a34 * b41);
        pOut->_32 = (a31 * b12) + (a32 * b22) + (a33 * b32) + (a34 * b42);
        pOut->_33 = (a31 * b13) + (a32 * b23) + (a33 * b33) + (a34 * b43);
        pOut->_34 = (a31 * b14) + (a32 * b24) + (a33 * b34) + (a34 * b44);

        pOut->_41 = (a41 * b11) + (a42 * b21) + (a43 * b31) + (a44 * b41);
        pOut->_42 = (a41 * b12) + (a42 * b22) + (a43 * b32) + (a44 * b42);
        pOut->_43 = (a41 * b13) + (a42 * b23) + (a43 * b33) + (a44 * b43);
        pOut->_44 = (a41 * b14) + (a42 * b24) + (a43 * b34) + (a44 * b44);
    }
    else
    {
        // transformation matrices (last column = [0 0 0 1])
        pOut->_11 = (a11 * b11) + (a12 * b21) + (a13 * b31);
        pOut->_12 = (a11 * b12) + (a12 * b22) + (a13 * b32);
        pOut->_13 = (a11 * b13) + (a12 * b23) + (a13 * b33);
        pOut->_14 = 0.0f;

        pOut->_21 = (a21 * b11) + (a22 * b21) + (a23 * b31);
        pOut->_22 = (a21 * b12) + (a22 * b22) + (a23 * b32);
        pOut->_23 = (a21 * b13) + (a22 * b23) + (a23 * b33);
        pOut->_24 = 0.0f;

        pOut->_31 = (a31 * b11) + (a32 * b21) + (a33 * b31);
        pOut->_32 = (a31 * b12) + (a32 * b22) + (a33 * b32);
        pOut->_33 = (a31 * b13) + (a32 * b23) + (a33 * b33);
        pOut->_34 = 0.0f;

        pOut->_41 = (a41 * b11) + (a42 * b21) + (a43 * b31) + b41;
        pOut->_42 = (a41 * b12) + (a42 * b22) + (a43 * b32) + b42;
        pOut->_43 = (a41 * b13) + (a42 * b23) + (a43 * b33) + b43;
        pOut->_44 = 1.0f;
    }

#endif

    return pOut;
}

matBaseMATRIX* matBaseMatrixMultiplyTranspose(
    matBaseMATRIX* pOut,
    const matBaseMATRIX* pM1,
    const matBaseMATRIX* pM2)
{
    // Out = T(M1* M2)
    // no doubt there's an optimization for this, but so what
    matBaseMatrixMultiply(pOut, pM1, pM2);
    matBaseMatrixTranspose(pOut, pOut);
    return pOut;
}

matBaseMATRIX* matBaseMatrixOrthoRH(
    matBaseMATRIX* pOut,
    float w, float h, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(w);
    UNREF(h);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixOrthoLH(
    matBaseMATRIX* pOut,
    float w, float h, float zn, float zf)
{
//    2/w  0    0           0
//    0    2/h  0           0
//    0    0    1/(zf-zn)   0
//    0    0    zn/(zn-zf)  1

    pOut->_11 = 2.0f / w;
    pOut->_12 = 0.0f;
    pOut->_13 = 0.0f;
    pOut->_14 = 0.0f;

    pOut->_21 = 0.0f;
    pOut->_22 = 2.0f / h;
    pOut->_23 = 0.0f;
    pOut->_24 = 0.0f;

    pOut->_31 = 0.0f;
    pOut->_32 = 0.0f;
    pOut->_33 = 1.0f / (zf - zn);
    pOut->_34 = 0.0f;

    pOut->_41 = 0.0f;
    pOut->_42 = 0.0f;
    pOut->_43 = zn / (zn - zf);
    pOut->_44 = 1.0f;

    return pOut;
}

matBaseMATRIX* matBaseMatrixOrthoOffCenterRH(
    matBaseMATRIX* pOut,
    float l, float r, float b, float t, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(l);
    UNREF(r);
    UNREF(b);
    UNREF(t);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixOrthoOffCenterLH(
    matBaseMATRIX* pOut,
    float l, float r, float b, float t, float zn, float zf)
{
//        2/(r-l)      0            0           0
//        0            2/(t-b)      0           0
//        0            0            1/(zf-zn)   0
//        (l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1

    pOut->_11 = 2.0f / (r - l);
    pOut->_12 = 0.0f;
    pOut->_13 = 0.0f;
    pOut->_14 = 0.0f;

    pOut->_21 = 0.0f;
    pOut->_22 = 2.0f / (t - b);
    pOut->_23 = 0.0f;
    pOut->_24 = 0.0f;

    pOut->_31 = 0.0f;
    pOut->_32 = 0.0f;
    pOut->_33 = 1.0f / (zf - zn);
    pOut->_34 = 0.0f;

    pOut->_41 = (l + r) / (l - r);
    pOut->_42 = (b + t) / (b - t);
    pOut->_43 = zn / (zn - zf);
    pOut->_44 = 1.0f;

    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveRH(
    matBaseMATRIX* pOut,
    float w, float h, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(w);
    UNREF(h);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveLH(
    matBaseMATRIX* pOut,
    float w, float h, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(w);
    UNREF(h);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveFovRH(
    matBaseMATRIX* pOut,
    float fovy, float aspect, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(fovy);
    UNREF(aspect);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveFovLH(
    matBaseMATRIX* pOut,
    float fovy, float aspect, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(fovy);
    UNREF(aspect);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveOffCenterRH(
    matBaseMATRIX* pOut,
    float l, float r, float b, float t, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(l);
    UNREF(r);
    UNREF(b);
    UNREF(t);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixPerspectiveOffCenterLH(
    matBaseMATRIX* pOut,
    float l, float r, float b, float t, float zn, float zf)
{
    // TODO: implement iff necessary
    UNREF(l);
    UNREF(r);
    UNREF(b);
    UNREF(t);
    UNREF(zn);
    UNREF(zf);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixReflect( matBaseMATRIX* pOut, const matBasePLANE* pPlane )
{
    matBasePLANE  p;
    matBasePlaneNormalize(&p,pPlane);

    pOut->_11 = -2.0f * p.a * p.a + 1.0f;
    pOut->_12 = -2.0f * p.b * p.a;
    pOut->_13 = -2.0f * p.c * p.a;
    pOut->_14 =  0.0f;
    pOut->_21 = -2.0f * p.a * p.b;
    pOut->_22 = -2.0f * p.b * p.b + 1.0f; 
    pOut->_23 = -2.0f * p.c * p.b;     
    pOut->_24 =  0.0f;
    pOut->_31 = -2.0f * p.a * p.c;   
    pOut->_32 = -2.0f * p.b * p.c;  
    pOut->_33 = -2.0f * p.c * p.c + 1.0f;  
    pOut->_34 =  0.0f;
    pOut->_41 = -2.0f * p.a * p.d;    
    pOut->_42 = -2.0f * p.b * p.d;   
    pOut->_43 = -2.0f * p.c * p.d;   
    pOut->_44 =  1.0f;

    return pOut;
}


matBaseMATRIX* matBaseMatrixRotationX(
    matBaseMATRIX* pOut,
    float angle)
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    matBaseMatrixIdentity(pOut);
    pOut->_22 = c;
    pOut->_32 = -s;
    pOut->_23 = s;
    pOut->_33 = c;
    return pOut;
}

matBaseMATRIX* matBaseMatrixRotationY(
    matBaseMATRIX* pOut,
    float angle)
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    //  c  0 -s  0
    //  0  1  0  0
    //  s  0  c  0
    //  0  0  0  1
    matBaseMatrixIdentity(pOut);
    pOut->_11 = c;
    pOut->_13 = -s;
    pOut->_31 = s;
    pOut->_33 = c;
    return pOut;
}

matBaseMATRIX* matBaseMatrixRotationZ(
    matBaseMATRIX* pOut,
    float angle)
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    //  c  s  0  0
    // -s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    matBaseMatrixIdentity(pOut);
    pOut->_11 = c;
    pOut->_12 = s;
    pOut->_21 = -s;
    pOut->_22 = c;
    return pOut;
}

matBaseMATRIX* matBaseMatrixRotationAxis(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pV,
    float angle)
{
    // hack?  perhaps.
    // compute the quaternion
    matBaseQUATERNION q;
    matBaseQuaternionRotationAxis(&q, pV, angle);
    // convert to matrix
    return matBaseMatrixRotationQuaternion(pOut, &q);
}

matBaseMATRIX* matBaseMatrixRotationQuaternion(
    matBaseMATRIX* pOut,
    const matBaseQUATERNION* pQ)
{
    // implementation from gamasutra.com, with a transpose due to DX

    // normalize input (necessary?)
    matBaseQUATERNION q(*pQ);
    matBaseQuaternionNormalize(&q, &q);

    // calculate coefficients
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx = q.x * x2;
    float xy = q.x * y2;
    float xz = q.x * z2;
    float yy = q.y * y2;
    float yz = q.y * z2;
    float zz = q.z * z2;
    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    // fill in matrix
    pOut->_11 = 1.0f - (yy + zz);
    pOut->_12 = xy + wz;
    pOut->_13 = xz - wy;
    pOut->_14 = 0.0f;

    pOut->_21 = xy - wz;
    pOut->_22 = 1.0f - (xx + zz);
    pOut->_23 = yz + wx;
    pOut->_24 = 0.0f;

    pOut->_31 = xz + wy;
    pOut->_32 = yz - wx;
    pOut->_33 = 1.0f - (xx + yy);
    pOut->_34 = 0.0f;

    pOut->_41 = 0.0f;
    pOut->_42 = 0.0f;
    pOut->_43 = 0.0f;
    pOut->_44 = 1.0f;

    return pOut;
}

matBaseMATRIX* matBaseMatrixRotationYawPitchRoll(
    matBaseMATRIX* pOut,
    float yaw,
    float pitch,
    float roll)
{
    // hack?  perhaps.
    // compute the quaternion
    matBaseQUATERNION q;
    matBaseQuaternionRotationYawPitchRoll(&q, yaw, pitch, roll);
    // convert to matrix
    return matBaseMatrixRotationQuaternion(pOut, &q);
}

matBaseMATRIX* matBaseMatrixShadow(
    matBaseMATRIX* pOut,
    const matBaseVECTOR4* pLight,
    const matBasePLANE* pPlane)
{
    // TODO: implement iff necessary
    UNREF(pLight);
    UNREF(pPlane);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseMATRIX* matBaseMatrixTransformation(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pScalingCenter,
    const matBaseQUATERNION* pScalingRotation,
    const matBaseVECTOR3* pScaling,
    const matBaseVECTOR3* pRotationCenter,
    const matBaseQUATERNION* pRotation,
    const matBaseVECTOR3* pTranslation)
{
    // Note: this is FAR from optimal...it needs to be reworked like
    // matBaseMatrixAffineTransformation()

    // build scaling center matrices
    matBaseMATRIX Msc, MscI;
    matBaseMATRIX* pMsc = NULL;
    matBaseMATRIX* pMscI = NULL;
    if (pScalingCenter != NULL)
    {
        matBaseMatrixTranslation(&Msc, pScalingCenter->x, pScalingCenter->y, pScalingCenter->z);
        matBaseMatrixTranslation(&MscI, -pScalingCenter->x, -pScalingCenter->y, -pScalingCenter->z);
        pMsc = &Msc;
        pMscI = &MscI;
    }

    // build scaling rotation matrices
    matBaseMATRIX Msr, MsrI;
    matBaseMATRIX* pMsr = NULL;
    matBaseMATRIX* pMsrI = NULL;
    if (pScalingRotation != NULL)
    {
        matBaseMatrixRotationQuaternion(&Msr, pScalingRotation);
        // invert the quaternion - faster than invert the matrix, and (hopefully) the same
        matBaseQUATERNION qsrI(0.f, 0.f, 0.f, 1.f);
        matBaseQuaternionInverse(&qsrI, pScalingRotation);
        matBaseMatrixRotationQuaternion(&MsrI, &qsrI);
        pMsr = &Msr;
        pMsrI = &MsrI;
    }

    // build scaling matrix
    matBaseMATRIX Ms;
    matBaseMATRIX* pMs = NULL;
    if (pScaling != NULL)
    {
        matBaseMatrixScaling(&Ms, pScaling->x, pScaling->y, pScaling->z);
        pMs = &Ms;
    }

    // build rotation center matrices
    matBaseMATRIX Mrc, MrcI;
    matBaseMATRIX* pMrc = NULL;
    matBaseMATRIX* pMrcI = NULL;
    if (pRotationCenter != NULL)
    {
        matBaseMatrixTranslation(&Mrc, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
        matBaseMatrixTranslation(&MrcI, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
        pMrc = &Mrc;
        pMrcI = &MrcI;
    }

    // build rotation matrix
    matBaseMATRIX Mr;
    matBaseMATRIX* pMr = NULL;
    if (pRotation != NULL)
    {
        matBaseMatrixRotationQuaternion(&Mr, pRotation);
        pMr = &Mr;
    }

    // build translation matrix
    matBaseMATRIX Mt;
    matBaseMATRIX* pMt = NULL;
    if (pTranslation != NULL)
    {
        matBaseMatrixTranslation(&Mt, pTranslation->x, pTranslation->y, pTranslation->z);
        pMt = &Mt;
    }

    // associate on right side
    // Msc-1 * (Msr-1 * (Ms * (Msr * (Msc * (Mrc-1 * (Mr * (Mrc * Mt)))))))

    // initialize output
    if (pMt != NULL)
    {
        *pOut = *pMt;
    }
    else
    {
        matBaseMatrixIdentity(pOut);
    }

    // multiply
    if (pMrc  != NULL) matBaseMatrixMultiply(pOut, pMrc, pOut);
    if (pMr   != NULL) matBaseMatrixMultiply(pOut, pMr, pOut);
    if (pMrcI != NULL) matBaseMatrixMultiply(pOut, pMrcI, pOut);
    if (pMsc  != NULL) matBaseMatrixMultiply(pOut, pMsc, pOut);
    if (pMsr  != NULL) matBaseMatrixMultiply(pOut, pMsr, pOut);
    if (pMs   != NULL) matBaseMatrixMultiply(pOut, pMs, pOut);
    if (pMsrI != NULL) matBaseMatrixMultiply(pOut, pMsrI, pOut);
    if (pMscI != NULL) matBaseMatrixMultiply(pOut, pMscI, pOut);

    // done
    return pOut;
}

matBaseMATRIX* matBaseMatrixTransformation(
    matBaseMATRIX* pOut,
    const matBaseVECTOR3* pScaling,
    const matBaseQUATERNION* pRotation,
    const matBaseVECTOR3* pTranslation)
{
    // Logically:
    //   Ms = scaling matrix
    //   Mr = rotation
    //   Mt = translation
    //
    // Out = Ms * (Mr * Mt)

    // Initialize the last column
    pOut->_14 = 0.0f;
    pOut->_24 = 0.0f;
    pOut->_34 = 0.0f;
    pOut->_44 = 1.0f;

    // translation
    if (pTranslation != NULL)
    {
        pOut->_41 = pTranslation->x;
        pOut->_42 = pTranslation->y;
        pOut->_43 = pTranslation->z;
    }
    else
    {
        // reset translation
        pOut->_41 = 0.0f;
        pOut->_42 = 0.0f;
        pOut->_43 = 0.0f;
    }

    // build rotation matrix
    if (pRotation != NULL && !matBaseQuaternionIsIdentity(pRotation))
    {
        matBaseMATRIX Mr;
        matBaseMatrixRotationQuaternion(&Mr, pRotation);
        // since we can only be a rotation matrix, no need to do the
        // full multiply, just copy the rotational part
        pOut->_11 = Mr._11; pOut->_12 = Mr._12; pOut->_13 = Mr._13;
        pOut->_21 = Mr._21; pOut->_22 = Mr._22; pOut->_23 = Mr._23;
        pOut->_31 = Mr._31; pOut->_32 = Mr._32; pOut->_33 = Mr._33;
    }
    else
    {
        // reset orientation to identity
        pOut->_11 = 1.0f; pOut->_12 = 0.0f; pOut->_13 = 0.0f;
        pOut->_21 = 0.0f; pOut->_22 = 1.0f; pOut->_23 = 0.0f;
        pOut->_31 = 0.0f; pOut->_32 = 0.0f; pOut->_33 = 1.0f;
    }


    // build scaling matrix
    if (pScaling != NULL)
    {
        pOut->_11 *= pScaling->x;
        pOut->_12 *= pScaling->x;
        pOut->_13 *= pScaling->x;

        pOut->_21 *= pScaling->y;
        pOut->_22 *= pScaling->y;
        pOut->_23 *= pScaling->y;

        pOut->_31 *= pScaling->z;
        pOut->_32 *= pScaling->z;
        pOut->_33 *= pScaling->z;
    }

    // done
    return pOut;
}


matBaseMATRIX* matBaseMatrixTranspose(
    matBaseMATRIX* pOut,
    const matBaseMATRIX* pM)
{
    matBaseMATRIX out; // temporary, because of possible aliasing
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            out.m[row][col] = pM->m[col][row];
        }
    }
    *pOut = out;
    return pOut;
}

// QUATERNION

matBaseQUATERNION* matBaseQuaternionRotationMatrix(
    matBaseQUATERNION* pOut,
    const matBaseMATRIX* pM)
{
    // implementation from www.magic-software.com
    float qx, qy, qz, qw;
    float fTrace = pM->m[0][0] + pM->m[1][1] + pM->m[2][2];
    if (fTrace > 0.0f)
    {
        // |w| > 1/2, may as well choose w > 1/2
        float fRoot = std::sqrt(fTrace + 1.0f); // 2w
        qw = 0.5f * fRoot;
        fRoot = 0.5f / fRoot; // 1/(4w)
        qx = (pM->m[2][1] - pM->m[1][2]) * fRoot;
        qy = (pM->m[0][2] - pM->m[2][0]) * fRoot;
        qz = (pM->m[1][0] - pM->m[0][1]) * fRoot;
    }
    else
    {
        // |w| <= 1/2
        static int s_iNext[3] = { 1, 2, 0 };
        int i = 0;
        if (pM->m[1][1] > pM->m[0][0])
        {
            i = 1;
        }
        if (pM->m[2][2] > pM->m[i][i])
        {
            i = 2;
        }
        int j = s_iNext[i];
        int k = s_iNext[j];
        float fRoot = std::sqrt(pM->m[i][i] - pM->m[j][j] - pM->m[k][k] + 1.0f);
        float* apkQuat[3] = { &qx, &qy, &qz };
        *apkQuat[i] = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;
        qw = (pM->m[k][j] - pM->m[j][k]) * fRoot;
        *apkQuat[j] = (pM->m[j][i] + pM->m[i][j]) * fRoot;
        *apkQuat[k] = (pM->m[k][i] + pM->m[i][k]) * fRoot;
    }
    // JB HACK: negate xyz, for some reason (LH vs RH? dunno)
    pOut->x = -qx;
    pOut->y = -qy;
    pOut->z = -qz;
    pOut->w = qw;
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionSlerp(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ1,
    const matBaseQUATERNION* pQ2,
    float t)
{
    // note: aliasing OK
    // implementation from gamasutra.com
    // calc cosine
    float cosom = (pQ1->x * pQ2->x) + (pQ1->y * pQ2->y) + (pQ1->z * pQ2->z) + (pQ1->w * pQ2->w);
    // adjust signs (if necessary)
    float to1[4];
    if (cosom < 0.0f)
    {
        cosom = -cosom;
        to1[0] = -pQ2->x;
        to1[1] = -pQ2->y;
        to1[2] = -pQ2->z;
        to1[3] = -pQ2->w;
    }
    else
    {
        to1[0] = pQ2->x;
        to1[1] = pQ2->y;
        to1[2] = pQ2->z;
        to1[3] = pQ2->w;
    }
    // calculate coefficients
    float scale0, scale1;
    if (!IsEqual(cosom, 1.0f))
    {
        // standard case (slerp)
        float omega = std::acos(cosom);
        float sinom = std::sin(omega);
        scale0 = std::sin((1.0f - t) * omega) / sinom;
        scale1 = std::sin(t * omega) / sinom;
    }
    else
    {        
        // "from" and "to" quaternions are very close 
        //  ... so we can do a linear interpolation
        scale0 = 1.0f - t;
        scale1 = t;
    }
    // calculate final values
    pOut->x = (scale0 * pQ1->x) + (scale1 * to1[0]);
    pOut->y = (scale0 * pQ1->y) + (scale1 * to1[1]);
    pOut->z = (scale0 * pQ1->z) + (scale1 * to1[2]);
    pOut->w = (scale0 * pQ1->w) + (scale1 * to1[3]);
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionMultiply(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ1,
    const matBaseQUATERNION* pQ2)
{
    // note: aliasing OK
    if (matBaseQuaternionIsIdentity(pQ1))
    {
        *pOut = *pQ2;
    }
    else if (matBaseQuaternionIsIdentity(pQ2))
    {
        *pOut = *pQ1;
    }
    else
    {
        // implementation from www.magic-software.com (swapping left/right)
        float x = (pQ2->w * pQ1->x) + (pQ2->x * pQ1->w) + (pQ2->y * pQ1->z) - (pQ2->z * pQ1->y);
        float y = (pQ2->w * pQ1->y) + (pQ2->y * pQ1->w) + (pQ2->z * pQ1->x) - (pQ2->x * pQ1->z);
        float z = (pQ2->w * pQ1->z) + (pQ2->z * pQ1->w) + (pQ2->x * pQ1->y) - (pQ2->y * pQ1->x);
        float w = (pQ2->w * pQ1->w) - (pQ2->x * pQ1->x) - (pQ2->y * pQ1->y) - (pQ2->z * pQ1->z);
        pOut->x = x;
        pOut->y = y;
        pOut->z = z;
        pOut->w = w;
    }
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionInverse(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ)
{
    // note: aliasing OK
    float qNorm = matBaseQuaternionLengthSq(pQ);
    if (IsEqual_ZERO(qNorm))
    {
        return pOut;
    }
    matBaseQuaternionConjugate(pOut, pQ);
    *pOut *= (1.0f / qNorm);
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionNormalize(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ)
{
    // note: aliasing OK
    float lenSq = matBaseQuaternionLengthSq(pQ);
    if (IsEqual_ZERO(lenSq))
    {
        return pOut;
    }
    // already unit?
    if (IsEqual(lenSq, 1.0f))
    {
        if (pOut != pQ)
        {
            *pOut = *pQ;
        }
    }
    else
    {
        float lenInv = 1.0f / std::sqrt(lenSq);
        pOut->x *= lenInv;
        pOut->y *= lenInv;
        pOut->z *= lenInv;
        pOut->w *= lenInv;
    }
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionBaryCentric(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ1,
    const matBaseQUATERNION* pQ2,
    const matBaseQUATERNION* pQ3,
    float f,
    float g)
{
    // TODO: implement iff necessary
    UNREF(pQ1);
    UNREF(pQ2);
    UNREF(pQ3);
    UNREF(f);
    UNREF(g);
    ERR_UNIMPLEMENTED();
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionExp(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ)
{
    // note: aliasing OK
    // implementation from www.magic-software.com
    float fAngle = std::sqrt(pQ->x*pQ->x + pQ->y*pQ->y + pQ->z*pQ->z);
    float fSin = std::sin(fAngle);
    pOut->w = std::cos(fAngle);
    if (!IsEqual_ZERO(fSin))
    {
        float fCoeff = fSin / fAngle;
        pOut->x = fCoeff * pQ->x;
        pOut->y = fCoeff * pQ->y;
        pOut->z = fCoeff * pQ->z;
    }
    else
    {
        pOut->x = pQ->x;
        pOut->y = pQ->y;
        pOut->z = pQ->z;
    }
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionLn(
    matBaseQUATERNION* pOut,
    const matBaseQUATERNION* pQ)
{
    // note: aliasing OK
    // implementation from www.magic-software.com
    if (std::fabs(pQ->w) < 1.0f)
    {
        float fAngle = std::acos(pQ->w);
        float fSin = std::sin(fAngle);
        if (!IsEqual_ZERO(fSin))
        {
            float fCoeff = fAngle / fSin;
            pOut->w = 0.0f;
            pOut->x = fCoeff * pQ->x;
            pOut->y = fCoeff * pQ->y;
            pOut->z = fCoeff * pQ->z;
            return pOut;
        }
    }
    // else
    pOut->w = 0.0f;
    pOut->x = pQ->x;
    pOut->y = pQ->y;
    pOut->z = pQ->z;
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionRotationAxis(
    matBaseQUATERNION* pOut,
    const matBaseVECTOR3* pV,
    float Angle)
{
    matBaseVECTOR3 v(0.f,0.f,0.f);
    matBaseVec3Normalize(&v, pV);
    // implementation from www.magic-software.com
    float halfAngle = 0.5f * Angle;
    float fSin = std::sin(halfAngle);
    pOut->x = fSin * v.x;
    pOut->y = fSin * v.y;
    pOut->z = fSin * v.z;
    pOut->w = std::cos(halfAngle);
    return pOut;
}

matBaseQUATERNION* matBaseQuaternionRotationYawPitchRoll(
    matBaseQUATERNION* pOut,
    float Yaw,
    float Pitch,
    float Roll)
{
    // implementation from gamasutra.com, modifying final output to
    // match DX's coordinate system (i.e. xyz -> yzx)
    //
    // The code makes user of so many temporary variables in
    // order to try and prevent optimization bugs in release builds
    float halfY = Yaw * 0.5f;
    float halfP = Pitch * 0.5f;
    float halfR = Roll * 0.5f;
    float cy = std::cos(halfY);
    float cp = std::cos(halfP);
    float cr = std::cos(halfR);
    float sy = std::sin(halfY);
    float sp = std::sin(halfP);
    float sr = std::sin(halfR);
    float cpcy = cp * cy;
    float spsy = sp * sy;
    float spcy = sp * cy;
    float cpsy = cp * sy;
    float crspcy = cr * spcy;
    float srcpsy = sr * cpsy;
    float crcpsy = cr * cpsy;
    float srspcy = sr * spcy;
    float srcpcy = sr * cpcy;
    float crspsy = cr * spsy;
    float crcpcy = cr * cpcy;
    float srspsy = sr * spsy;

    pOut->x = crspcy + srcpsy;
    pOut->y = crcpsy - srspcy;
    pOut->z = srcpcy - crspsy;
    pOut->w = crcpcy + srspsy;
    
    return pOut;
}

void matBaseQuaternionToAxisAngle(
    const matBaseQUATERNION* pQ,
    matBaseVECTOR3* pAxis,
    float* pAngle)
{
    // implementation from www.magic-software.com
    float lenSq = matBaseQuaternionLengthSq(pQ);
    if (lenSq > 0.0f)
    {
        *pAngle = 2.0f * std::acos(pQ->w);
        float len = std::sqrt(lenSq);
        pAxis->x = pQ->x * len;
        pAxis->y = pQ->y * len;
        pAxis->z = pQ->z * len;
    }
    else
    {
        // angle is 0 (mod 2*pi), so any axis will do
        *pAngle = 0.0f;
        pAxis->x = 1.0f;
        pAxis->y = 0.0f;
        pAxis->z = 0.0f;
    }
}

// PLANE    

matBasePLANE* matBasePlaneFromPointNormal(
    matBasePLANE* pOut,
    const matBaseVECTOR3* pPoint,
    const matBaseVECTOR3* pNormal)
{
    pOut->a = pNormal->x;
    pOut->b = pNormal->y;
    pOut->c = pNormal->z;
    pOut->d = -matBaseVec3Dot(pNormal, pPoint);
    return pOut;
}

matBasePLANE* matBasePlaneFromPoints(
    matBasePLANE* pOut,
    const matBaseVECTOR3* pV1,
    const matBaseVECTOR3* pV2,
    const matBaseVECTOR3* pV3)
{
    matBaseVECTOR3 tmp1 = *pV2 - *pV1;
    matBaseVECTOR3 tmp2 = *pV3 - *pV1;
    matBaseVECTOR3 normal;
    matBaseVec3Cross(&normal, &tmp1, &tmp2);
    return matBasePlaneFromPointNormal(pOut, pV1, &normal);
}

matBasePLANE* matBasePlaneNormalize(
    matBasePLANE* pOut, 
    const matBasePLANE* pP )
{
    matBaseVec3Normalize((matBaseVECTOR3*)pOut, (matBaseVECTOR3*)pP);
    pOut->d = pP->d;

    return pOut;
}

#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

class matD3DXMathEmuTests : public tstUnit
{
private:
    matBaseVECTOR3 vx;
    matBaseVECTOR3 vy;
    matBaseVECTOR3 vz;
    matBaseVECTOR3 v000;
    matBaseVECTOR3 v111;
    matBaseVECTOR3 v123;

public:
    matD3DXMathEmuTests() :
      vx(1.0f, 0.0f, 0.0f),
      vy(0.0f, 1.0f, 0.0f),
      vz(0.0f, 0.0f, 1.0f),
      v000(0.0f, 0.0f, 0.0f),
      v111(1.0f, 1.0f, 1.0f),
      v123(1.0f, 2.0f, 3.0f)
	{
	}

	virtual void Register()
	{
		SetName("matD3DXMathEmu");
        AddDependency("matUtils");

        AddTestCase("matBaseVECTOR3 construction", &matD3DXMathEmuTests::TestVECTOR3Construction);
        AddTestCase("matBaseVECTOR3::operator+-*/()", &matD3DXMathEmuTests::TestVECTOR3Operators);
        AddTestCase("matBaseVec3Lerp()", &matD3DXMathEmuTests::TestVec3Lerp);
        AddTestCase("matBaseVec3Maximize()", &matD3DXMathEmuTests::TestVec3Maximize);
        AddTestCase("matBaseVec3Minimize()", &matD3DXMathEmuTests::TestVec3Minimize);
        AddTestCase("matBaseVec3Scale()", &matD3DXMathEmuTests::TestVec3Scale);
        AddTestCase("matBaseVec3LengthSq()", &matD3DXMathEmuTests::TestVec3LengthSq);
        AddTestCase("matBaseVec3Length()", &matD3DXMathEmuTests::TestVec3Length);
        AddTestCase("matBaseVec3Normalize()", &matD3DXMathEmuTests::TestVec3Normalize);
        AddTestCase("matBaseVec3Dot()", &matD3DXMathEmuTests::TestVec3Dot);
        AddTestCase("matBaseVec3Cross()", &matD3DXMathEmuTests::TestVec3Cross);
        AddTestCase("matBasePLANE operators", &matD3DXMathEmuTests::TestPlaneOperators);
        AddTestCase("matBasePlaneFromPointNormal()", &matD3DXMathEmuTests::TestPlaneFromPointNormal);
        AddTestCase("matBasePlaneFromPoints()", &matD3DXMathEmuTests::TestPlaneFromPoints);
	};

	virtual void UnitTestSetup()
	{
		// set up code
	}

	virtual void UnitTestTearDown()
	{
		// tear down code
	}

    void TestVECTOR3Construction() const
    {
        matBaseVECTOR3 v0(v000);
        TESTASSERT(v0 == v000);

        matBaseVECTOR3 v1(v111);
        TESTASSERT(v1 == v111);
    }

    void TestVECTOR3Operators() const
    {
        matBaseVECTOR3 v(v000);
        v += vx;
        v += vy;
        v += vz;
        TESTASSERT(v == v111);
        v -= vx;
        v -= vy;
        v -= vz;
        TESTASSERT(v == v000);

        v = v123;
        v *= 2.0f;
        TESTASSERT(v == matBaseVECTOR3(2.0f,4.0f,6.0f));

        v = v000;
        v += 1 * vx;
        v += 2 * vy;
        v += 3 * vz;
        TESTASSERT(v == v123);

        v = v123;
        v -= vy;
        v -= vz;
        v -= vz;
        TESTASSERT(v == v111);

        v = v000;
        v = -v;
        TESTASSERT(v == v000);

        v = matBaseVECTOR3(-1.0f, -2.0f, -3.0f);
        v = -v;
        TESTASSERT(v == v123);
        TESTASSERT(+v == v123);
    }

    void TestVec3Lerp() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Lerp(&v, &v000, &v111, 0.0f);
        TESTASSERT(v == v000);

        matBaseVec3Lerp(&v, &v000, &v111, 1.0f);
        TESTASSERT(v == v111);

        matBaseVec3Lerp(&v, &v111, &v123, 0.5f);
        TESTASSERT(v == matBaseVECTOR3(1.0f, 1.5f, 2.0f));

        // test aliasing
        v = v111;
        matBaseVec3Lerp(&v, &v, &v, 0.0f);
        TESTASSERT(v == v111);

        v = v111;
        matBaseVec3Lerp(&v, &v, &v000, 1.0f);
        TESTASSERT(v == v000);

        v = v111;
        matBaseVec3Lerp(&v, &v000, &v, 0.0f);
        TESTASSERT(v == v000);
    }

    void TestVec3Maximize() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Maximize(&v, &v000, &v123);
        TESTASSERT(v == v123);

        matBaseVec3Maximize(&v, &v123, &v000);
        TESTASSERT(v == v123);

        matBaseVECTOR3 va(-123.456f, +234.567f, -345.678f);
        matBaseVECTOR3 vb(+987.654f, -876.543f, +765.432f);
        matBaseVECTOR3 vc(+987.654f, +234.567f, +765.432f);
        matBaseVec3Maximize(&v, &va, &vb);
        TESTASSERT(v == vc);
        matBaseVec3Maximize(&v, &vb, &va);
        TESTASSERT(v == vc);
    }

    void TestVec3Minimize() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Minimize(&v, &v000, &v123);
        TESTASSERT(v == v000);

        matBaseVec3Minimize(&v, &v123, &v000);
        TESTASSERT(v == v000);

        matBaseVECTOR3 va(-123.456f, +234.567f, -345.678f);
        matBaseVECTOR3 vb(+987.654f, -876.543f, +765.432f);
        matBaseVECTOR3 vc(-123.456f, -876.543f, -345.678f);
        matBaseVec3Minimize(&v, &va, &vb);
        TESTASSERT(v == vc);
        matBaseVec3Minimize(&v, &vb, &va);
        TESTASSERT(v == vc);
    }

    void TestVec3Scale() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Scale(&v, &v123, 0.0f);
        TESTASSERT(v == v000);

        matBaseVec3Scale(&v, &v123, 1.0f);
        TESTASSERT(v == v123);

        matBaseVec3Scale(&v, &v123, -1.0f);
        TESTASSERT(-v == v123);

        matBaseVec3Scale(&v, &v123, 10.0f);
        TESTASSERT(v == matBaseVECTOR3(10.0f, 20.0f, 30.0f));
    }

    void TestVec3LengthSq() const
    {
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&v000), 0.0f));
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&vx), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&vy), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&vz), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&v111), 3.0f));
        TESTASSERT(IsEqual(matBaseVec3LengthSq(&v123), 14.0f));
    }

    void TestVec3Length() const
    {
        TESTASSERT(IsEqual(matBaseVec3Length(&v000), 0.0f));
        TESTASSERT(IsEqual(matBaseVec3Length(&vx), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Length(&vy), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Length(&vz), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Length(&v111), 1.7320508f));
        TESTASSERT(IsEqual(matBaseVec3Length(&v123), 3.7416574f));
    }

    void TestVec3Normalize() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Normalize(&v, &vx);
        TESTASSERT(v == vx);
        matBaseVec3Normalize(&v, &vy);
        TESTASSERT(v == vy);
        matBaseVec3Normalize(&v, &vz);
        TESTASSERT(v == vz);

        v = v111; // doing this prevents optimization bugs in release builds
        matBaseVec3Normalize( &v, &v );
        TESTASSERT(IsEqual(matBaseVec3Length(&v), 1.0f));

        v = v123; // doing this prevents optimization bugs in release builds
        matBaseVec3Normalize( &v, &v );
        TESTASSERT(IsEqual(matBaseVec3Length(&v), 1.0f));

        // test aliasing
        v = -v111; // doing this prevents optimization bugs in release builds
        matBaseVec3Normalize(&v, &v);
        TESTASSERT(IsEqual(matBaseVec3Length(&v), 1.0f));

        v = -v123; // doing this prevents optimization bugs in release builds
        matBaseVec3Normalize(&v, &v);
        TESTASSERT(IsEqual(matBaseVec3Length(&v), 1.0f));
    }

    void TestVec3Dot() const
    {
        TESTASSERT(IsEqual(matBaseVec3Dot(&vx, &vy), 0.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vx, &vz), 0.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vy, &vz), 0.0f));

        TESTASSERT(IsEqual(matBaseVec3Dot(&vx, &vx), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vy, &vy), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vz, &vz), 1.0f));

        TESTASSERT(IsEqual(matBaseVec3Dot(&vx, &v111), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vy, &v111), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vz, &v111), 1.0f));

        TESTASSERT(IsEqual(matBaseVec3Dot(&vx, &v123), 1.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vy, &v123), 2.0f));
        TESTASSERT(IsEqual(matBaseVec3Dot(&vz, &v123), 3.0f));
    }

    void TestVec3Cross() const
    {
        matBaseVECTOR3 v;
        matBaseVec3Cross(&v, &vx, &vy);
        TESTASSERT(v == vz);
        matBaseVec3Cross(&v, &vy, &vz);
        TESTASSERT(v == vx);
        matBaseVec3Cross(&v, &vz, &vx);
        TESTASSERT(v == vy);

        matBaseVec3Cross(&v, &vy, &vx);
        TESTASSERT(v == -vz);
        matBaseVec3Cross(&v, &vz, &vy);
        TESTASSERT(v == -vx);
        matBaseVec3Cross(&v, &vx, &vz);
        TESTASSERT(v == -vy);
    }

    void TestPlaneOperators() const
    {
        matBasePLANE plane0(0.0f, 0.6f, 0.8f, 10.0f);
        matBasePLANE plane1;
        plane1.a = 0.0f;
        plane1.b = 0.6f;
        plane1.c = 0.8f;
        plane1.d = 10.0f;

        TESTASSERT(plane0 == plane1);
        TESTASSERT(plane1 == plane0);

        plane1 = -plane0;
        TESTASSERT(plane0 != plane1);
        TESTASSERT(plane1 != plane0);
    }

    void TestPlaneFromPointNormal() const
    {
        matBasePLANE plane;
        matBaseVECTOR3 planePoint(1.0f, 2.0f, 3.0f);
        matBaseVECTOR3 planeNormal(0.0f, 0.6f, 0.8f);

        matBasePlaneFromPointNormal(&plane, &planePoint, &planeNormal);
        TESTASSERT(plane.a == planeNormal.x);
        TESTASSERT(plane.b == planeNormal.y);
        TESTASSERT(plane.c == planeNormal.z);

        float fDistToPlane = matBaseVec3Dot(&planeNormal, &planePoint) + plane.d;
        TESTASSERT(IsEqual_ZERO(fDistToPlane));
    }

    void TestPlaneFromPoints() const
    {
        matBaseVECTOR3 p0(+1.0f, +2.0f, +3.0f);
        matBaseVECTOR3 p1(-3.0f, +4.0f, +5.0f);
        matBaseVECTOR3 p2(+7.0f, -8.0f, +9.0f);

        matBasePLANE plane;
        matBasePlaneFromPoints(&plane, &p0, &p1, &p2);
        matBaseVECTOR3 planeNormal;
        planeNormal.x = plane.a;
        planeNormal.y = plane.b;
        planeNormal.z = plane.c;

        float fDistToPlane;
        fDistToPlane = matBaseVec3Dot(&planeNormal, &p0) + plane.d;
        TESTASSERT(IsEqual_ZERO(fDistToPlane));
        fDistToPlane = matBaseVec3Dot(&planeNormal, &p1) + plane.d;
        TESTASSERT(IsEqual_ZERO(fDistToPlane));
        fDistToPlane = matBaseVec3Dot(&planeNormal, &p2) + plane.d;
        TESTASSERT(IsEqual_ZERO(fDistToPlane));
    }
};

EXPORTUNITTESTOBJECT(matD3DXMathEmuTests);

#endif // BUILD_UNIT_TESTS
