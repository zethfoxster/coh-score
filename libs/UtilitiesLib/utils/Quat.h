
#pragma once

extern  Quat unitquat;


#define quatXidx 0
#define quatYidx 1
#define quatZidx 2
#define quatWidx 3
#define quatX(q) ((q)[quatXidx])
#define quatY(q) ((q)[quatYidx])
#define quatZ(q) ((q)[quatZidx])
#define quatW(q) ((q)[quatWidx])
#define quatParamsXYZW(q) quatX(q), quatY(q), quatZ(q), quatW(q)
#define copyQuat(src,dst)			copyVec4(src,dst)
#define zeroQuat(q)					zeroVec4(q)
#define quatIsZero(q)				vec4IsZero(q)
#define unitQuat(q)					((q)[0] = 0.0f, (q)[1] = 0.0f, (q)[2] = 0.0f, (q)[3] = -1.0f)
#define printQuat(quat)				printVec4(quat)


//
// Quat creation
//
void mat3ToQuat(const Mat3 R, Quat q);
void PYRToQuat(const Vec3 pyr, Quat q);
bool axisAngleToQuat(const Vec3 axis, F32 angle, Quat quat);
void yawQuat(F32 yaw, Quat q);
void pitchQuat(F32 yaw, Quat q);
void rollQuat(F32 yaw, Quat q);



//
// Quat conversion
//
void quatToMat(const Quat q,Mat3 R);
void quatToMat3_0(const Quat q, Vec3 vec);
void quatToMat3_1(const Quat q, Vec3 vec);
void quatToMat3_2(const Quat q, Vec3 vec);
void quatToPYR(const Quat q, Vec3 pyr);
void quatToAxisAngle(const Quat quat, Vec3 axis, F32* angle);
F32 quatGetAngle(const Quat quat);


//
// Quat utils
//
void quatForceWPositive(Quat q);
void quatCalcWFromXYZ(Quat q);
void quatNormalize(Quat q);
bool quatIsValid(const Quat q);

//
// Quat multiplication
//
F32 quatInterp(F32 alpha, const Quat a, const Quat b, Quat q);
void quatInverse(const Quat q, Quat qi);
void quatRotateVec3(const Quat q, const Vec3 vecIn, Vec3 vecOut);
void quatRotateVec3ZOnly(const Quat q, const Vec3 vecIn, Vec3 vecOut);
void quatMultiply(const Quat q1, const Quat q2, Quat qOut); // qOut = q2 * q1 (note the inverted order)
