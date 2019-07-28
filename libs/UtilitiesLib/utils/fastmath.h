#ifndef _FASTMATH_H_
#define _FASTMATH_H_

//////////////////////////////////////////////////////////////////////////
// This header file defines datatypes and functions that wrap intrinsic
// functions on the XBox360 and work with normal floats on the PC.
// 
// Setting these datatypes and reading from these datatypes is slow, but
// vector and matrix operations are very fast.
//////////////////////////////////////////////////////////////////////////

#include "stdtypes.h"
#include "assert.h"
#include "mathutil.h"

// for testing:
//#define NO_INTRINSICS 1

static INLINEDBG int is16ByteAligned(const void *data)
{
	return !(((size_t)data) & 0x0f);
}

#if defined(_XBOX)

	#include "wininclude.h"

	#ifdef _DEBUG
	#define DEBUGUNDEFED
	#undef _DEBUG
	#endif
		#include "xboxmath.h"
	#ifdef DEBUGUNDEFED
	#define _DEBUG 1
	#undef DEBUGUNDEFED
	#endif
#endif

#if defined(_XBOX) && !defined(NO_INTRINSICS)
	typedef XMMATRIX FastMat4;
	typedef XMMATRIX FastMat4In;
	typedef XMMATRIX * __restrict FastMat4Out;
	typedef XMMATRIX *FastMat4Ptr;

	typedef XMMATRIX FastMat44;
	typedef XMMATRIX FastMat44In;
	typedef XMMATRIX * __restrict FastMat44Out;
	typedef XMMATRIX *FastMat44Ptr;

	typedef XMVECTOR FastVec;
	typedef XMVECTOR *FastVecPtr;
	typedef XMVECTOR FastVecIn;
	typedef XMVECTOR * __restrict FastVecOut;

	extern const __declspec(selectany) FastMat44 unitfastmat44 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	extern const __declspec(selectany) FastMat4 unitfastmat4 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

	#define FASTPTR(var) (&(var))
	#define FASTDEREF(var) (*(var))

#if 1
#include "file.h"
	#define CHECKALIGN16(data) assert(isDevelopmentMode() && is16ByteAligned(FASTPTR(data)))
	#define CHECKALIGN16DEREF(data) assert(isDevelopmentMode() && is16ByteAligned(data))
#else
	#define CHECKALIGN16(data)
	#define CHECKALIGN16DEREF(data)
#endif

	static INLINEDBG void mulFastMat4(FastMat4In fmIn1, FastMat4In fmIn2, FastMat4Out fmOut)
	{
		*fmOut = XMMatrixMultiply(fmIn2, fmIn1);
	}

	static INLINEDBG void mulFastVecMat4(FastVecIn fvIn, FastMat4In fmIn, FastVecOut fvOut)
	{
		*fvOut = XMVector3Transform(fvIn, fmIn);
	}

	static INLINEDBG void rotateFastVecMat4(FastVecIn fvIn, FastMat4In fmIn, FastVecOut fvOut)
	{
		*fvOut = XMVector3TransformNormal(fvIn, fmIn);
	}

	static INLINEDBG void mulFastMat44(FastMat44In fmIn1, FastMat44In fmIn2, FastMat44Out fmOut)
	{
		*fmOut = XMMatrixMultiply(fmIn2, fmIn1);
	}

	static INLINEDBG void mulFastVecMat44(FastVecIn fvIn, FastMat44In fmIn, FastVecOut fvOut)
	{
		*fvOut = XMVector3TransformCoord(fvIn, fmIn);
	}

	static INLINEDBG void rotateFastVecMat44(FastVecIn fvIn, FastMat44In fmIn, FastVecOut fvOut)
	{
		*fvOut = XMVector3TransformNormal(fvIn, fmIn);
	}

	//////////////////////////////////////////////////////////////////////////

	static INLINEDBG void copyFastVec(FastVecOut fvOut, FastVecIn fvIn)
	{
		*fvOut = fvIn;
	}

	// slow!
	static INLINEDBG void setFastVec(FastVecOut fvOut, const Vec3 vIn)
	{
		*fvOut = XMVectorSet(vIn[0], vIn[1], vIn[2], 1);
	}

	// slow!
	static INLINEDBG void setFastVecf(FastVecOut fvOut, F32 f0, F32 f1, F32 f2)
	{
		*fvOut = XMVectorSet(f0, f1, f2, 1);
	}

	//////////////////////////////////////////////////////////////////////////

	static INLINEDBG void copyFastMat4(FastMat4Out fmOut, FastMat4In fmIn)
	{
		*fmOut = fmIn;
	}

	static INLINEDBG void copyFastMat4Rotation(FastMat4Out fmOut, FastMat4In fmIn)
	{
		fmOut->r[0] = fmIn.r[0];
		fmOut->r[1] = fmIn.r[1];
		fmOut->r[2] = fmIn.r[2];
	}

	static INLINEDBG void copyFastMat44toMat4Rotation(FastMat4Out fmOut, FastMat44In fmIn)
	{
		fmOut->r[0] = fmIn.r[0];
		fmOut->r[1] = fmIn.r[1];
		fmOut->r[2] = fmIn.r[2];
	}

	// slow!
	static INLINEDBG void setFastMat4(FastMat4Out fmOut, const Mat4 mIn)
	{
		fmOut->r[0] = XMVectorSet(mIn[0][0], mIn[0][1], mIn[0][2], 0);
		fmOut->r[1] = XMVectorSet(mIn[1][0], mIn[1][1], mIn[1][2], 0);
		fmOut->r[2] = XMVectorSet(mIn[2][0], mIn[2][1], mIn[2][2], 0);
		fmOut->r[3] = XMVectorSet(mIn[3][0], mIn[3][1], mIn[3][2], 1);
	}

	// slow
	static INLINEDBG void setFastMat4Column(FastMat4Out fmOut, int column, FastVecIn fvIn)
	{
		fmOut->r[column] = fvIn;
		fmOut->r[column].v[3] = __fsel(column-3, 1, 0);
	}

	// slow!
	static INLINEDBG void setFastMat4Columnf(FastMat4Out fmOut, int column, F32 f0, F32 f1, F32 f2)
	{
		fmOut->r[column] = XMVectorSet(f0, f1, f2, __fsel(column-3, 1, 0));
	}

	// slow!
	static INLINEDBG void setFastMat4Columnfv(FastMat4Out fmOut, int column, const Vec3 vIn)
	{
		fmOut->r[column] = XMVectorSet(vIn[0], vIn[1], vIn[2], __fsel(column-3, 1, 0));
	}

	static INLINEDBG void copyFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		*fmOut = fmIn;
	}

	// slow!
	static INLINEDBG void setFastMat44(FastMat44Out fmOut, const Mat44 mIn)
	{
		fmOut->r[0] = XMVectorSet(mIn[0][0], mIn[0][1], mIn[0][2], mIn[0][3]);
		fmOut->r[1] = XMVectorSet(mIn[1][0], mIn[1][1], mIn[1][2], mIn[1][3]);
		fmOut->r[2] = XMVectorSet(mIn[2][0], mIn[2][1], mIn[2][2], mIn[2][3]);
		fmOut->r[3] = XMVectorSet(mIn[3][0], mIn[3][1], mIn[3][2], mIn[3][3]);
	}

	static INLINEDBG void setFastMat44FromFastMat4(FastMat44Out fmOut, FastMat4In fmIn)
	{
		*fmOut = fmIn;
	}

	// slow!
	static INLINEDBG void setFastMat44FromMat4(FastMat44Out fmOut, const Mat4 mIn)
	{
		fmOut->r[0] = XMVectorSet(mIn[0][0], mIn[0][1], mIn[0][2], 0);
		fmOut->r[1] = XMVectorSet(mIn[1][0], mIn[1][1], mIn[1][2], 0);
		fmOut->r[2] = XMVectorSet(mIn[2][0], mIn[2][1], mIn[2][2], 0);
		fmOut->r[3] = XMVectorSet(mIn[3][0], mIn[3][1], mIn[3][2], 1);
	}

	// slow
	static INLINEDBG void setFastMat44Column(FastMat44Out fmOut, int column, FastVecIn fvIn)
	{
		fmOut->r[column] = fvIn;
		fmOut->r[column].v[3] = __fsel(column-3, 1, 0);
	}

	// slow!
	static INLINEDBG void setFastMat44Columnf(FastMat44Out fmOut, int column, F32 f0, F32 f1, F32 f2)
	{
		fmOut->r[column] = XMVectorSet(f0, f1, f2, __fsel(column-3, 1, 0));
	}

	// slow!
	static INLINEDBG void setFastMat44Columnfv(FastMat44Out fmOut, int column, const Vec3 vIn)
	{
		fmOut->r[column] = XMVectorSet(vIn[0], vIn[1], vIn[2], __fsel(column-3, 1, 0));
	}

	// slow!
	static INLINEDBG void setFastMat44Column4f(FastMat44Out fmOut, int column, F32 f0, F32 f1, F32 f2, F32 f3)
	{
		fmOut->r[column] = XMVectorSet(f0, f1, f2, f3);
	}

	// slow!
	static INLINEDBG void setFastMat44Column4fv(FastMat44Out fmOut, int column, const Vec4 vIn)
	{
		fmOut->r[column] = XMVectorSet(vIn[0], vIn[1], vIn[2], vIn[3]);
	}

	static INLINEDBG void transposeFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		*fmOut = XMMatrixTranspose(fmIn);
	}

	static INLINEDBG void invertFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		XMVECTOR det;
		*fmOut = XMMatrixInverse(&det, fmIn);
	}

	static INLINEDBG void invertFastMat4Quick(FastMat4Out fmOut, FastMat4In fmIn)
	{
		XMVECTOR dv;
		dv = XMVectorScale(fmIn.r[3], -1);
		fmIn.r[3] = XMVectorSet(0, 0, 0, 1);
		*fmOut = XMMatrixTranspose(fmIn);
		fmOut->r[3] = XMVectorSet(0, 0, 0, 1);
		fmOut->r[3] = XMVector3Transform(dv, *fmOut);
	}

	static INLINEDBG void invertFastMat44Quick(FastMat44Out fmOut, FastMat44In fmIn)
	{
		XMVECTOR dv;
		dv = XMVectorScale(fmIn.r[3], -1);
		fmIn.r[3] = XMVectorSet(0, 0, 0, 1);
		*fmOut = XMMatrixTranspose(fmIn);
		fmOut->r[3] = XMVectorSet(0, 0, 0, 1);
		fmOut->r[3] = XMVector3Transform(dv, *fmOut);
	}

	//////////////////////////////////////////////////////////////////////////

	// slow!
	static INLINEDBG void getFastVec(Vec3 vOut, FastVecIn fvIn)
	{
		copyVec3(fvIn.v, vOut);
	}

	// slow!
	static INLINEDBG void getFastMat4(Mat4 mOut, FastMat4In fmIn)
	{
		copyVec3(fmIn.r[0].v, mOut[0]);
		copyVec3(fmIn.r[1].v, mOut[1]);
		copyVec3(fmIn.r[2].v, mOut[2]);
		copyVec3(fmIn.r[3].v, mOut[3]);
	}

	// slow!
	static INLINEDBG void getFastMat4Column(FastVecOut fvOut, FastMat4In fmIn, int column)
	{
		*fvOut = fmIn.r[column];
	}

	// slow!
	static INLINEDBG void getFastMat4Columnv(Vec3 vOut, FastMat4In fmIn, int column)
	{
		copyVec3(fmIn.r[column].v, vOut);
	}

	// slow!
	static INLINEDBG F32 getFastMat4Element(FastMat4In fmIn, int column, int row)
	{
		return fmIn.r[column].v[row];
	}

	// slow!
	static INLINEDBG void getFastMat44Column(FastVecOut fvOut, FastMat44In fmIn, int column)
	{
		*fvOut = fmIn.r[column];
	}

	// slow!
	static INLINEDBG void getFastMat44Columnv(Vec4 vOut, FastMat44In fmIn, int column)
	{
		copyVec4(fmIn.r[column].v, vOut);
	}

	// slow!
	static INLINEDBG void getFastMat44Rowv(Vec4 vOut, FastMat44In fmIn, int row)
	{
		setVec4(vOut, fmIn.r[0].v[row], fmIn.r[1].v[row], fmIn.r[2].v[row], fmIn.r[3].v[row]);
	}

	// slow!
	static INLINEDBG F32 getFastMat44Element(FastMat44In fmIn, int column, int row)
	{
		return fmIn.r[column].v[row];
	}

	// slow!
	static INLINEDBG void getFastMat4As44(Mat44 mOut, FastMat4In fmIn)
	{
		copyVec3(fmIn.r[0].v, mOut[0]);
		mOut[0][3] = 0;
		copyVec3(fmIn.r[1].v, mOut[1]);
		mOut[1][3] = 0;
		copyVec3(fmIn.r[2].v, mOut[2]);
		mOut[2][3] = 0;
		copyVec3(fmIn.r[3].v, mOut[3]);
		mOut[3][3] = 1;
	}

	// slow!
	static INLINEDBG void getFastMat44As4(Mat4 mOut, FastMat44In fmIn)
	{
		copyVec3(fmIn.r[0].v, mOut[0]);
		copyVec3(fmIn.r[1].v, mOut[1]);
		copyVec3(fmIn.r[2].v, mOut[2]);
		copyVec3(fmIn.r[3].v, mOut[3]);
	}

	// slow!
	static INLINEDBG void getFastMat44(Mat44 mOut, FastMat44In fmIn)
	{
		copyVec4(fmIn.r[0].v, mOut[0]);
		copyVec4(fmIn.r[1].v, mOut[1]);
		copyVec4(fmIn.r[2].v, mOut[2]);
		copyVec4(fmIn.r[3].v, mOut[3]);
	}

	static INLINEDBG F32 *getFastMat44FloatPtr(FastMat44Out fmIn)
	{
		return &fmIn->m[0][0];
	}

	static INLINEDBG void getFastMat4QuickInverse(Mat4 mOut, FastMat4In fmIn)
	{
		Vec3 dv;
		Mat3 mIn;

		copyVec3(fmIn.r[0].v, mIn[0]);
		copyVec3(fmIn.r[1].v, mIn[1]);
		copyVec3(fmIn.r[2].v, mIn[2]);

		transposeMat3Copy(mIn,mOut);
		subVec3(zerovec3,fmIn.r[3].v,dv);
		mulVecMat3(dv,mOut,mOut[3]);
	}
#else //////////////////////////////////////////////////////////////////////////

	typedef Mat4 FastMat4;
	typedef const Mat4 FastMat4In;
	typedef Mat4 FastMat4Out;
	typedef Vec3 *FastMat4Ptr;

	typedef Mat44 FastMat44;
	typedef const Mat44 FastMat44In;
	typedef Mat44 FastMat44Out;
	typedef Vec4 *FastMat44Ptr;

	typedef Vec3 FastVec;
	typedef F32 *FastVecPtr;
	typedef const Vec3 FastVecIn;
	typedef Vec3 FastVecOut;

	extern const __declspec(selectany) FastMat44 unitfastmat44 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	extern const __declspec(selectany) FastMat4 unitfastmat4 = {1,0,0, 0,1,0, 0,0,1, 0,0,0};

	#define FASTPTR(var) (var)
	#define FASTDEREF(var) (var)

	#define CHECKALIGN16(data)
	#define CHECKALIGN16DEREF(data)

	static INLINEDBG void mulFastMat4(FastMat4In fmIn1, FastMat4In fmIn2, FastMat4Out fmOut)
	{
		mulMat4Inline(fmIn1, fmIn2, fmOut);
	}

	static INLINEDBG void mulFastVecMat4(FastVecIn fvIn, FastMat4In fmIn, FastVecOut fvOut)
	{
		mulVecMat4(fvIn, fmIn, fvOut);
	}

	static INLINEDBG void rotateFastVecMat4(FastVecIn fvIn, FastMat4In fmIn, FastVecOut fvOut)
	{
		mulVecMat3(fvIn, fmIn, fvOut);
	}

	static INLINEDBG void mulFastMat44(FastMat44In fmIn1, FastMat44In fmIn2, FastMat44Out fmOut)
	{
		mulMat44Inline(fmIn1, fmIn2, fmOut);
	}

	static INLINEDBG void mulFastVecMat44(FastVecIn fvIn, FastMat44In fmIn, FastVecOut fvOut)
	{
		Vec4 vOut;
		F32 oneOverW;
		mulVecMat44(fvIn, fmIn, vOut);
		oneOverW = 1.f/vOut[3];
		scaleVec3(vOut, oneOverW, fvOut);
	}

	static INLINEDBG void rotateFastVecMat44(FastVecIn fvIn, FastMat44In fmIn, FastVecOut fvOut)
	{
		Mat3 mat;
		copyVec3(fmIn[0], mat[0]);
		copyVec3(fmIn[1], mat[1]);
		copyVec3(fmIn[2], mat[2]);
		mulVecMat3(fvIn, mat, fvOut);
	}

	//////////////////////////////////////////////////////////////////////////

	static INLINEDBG void copyFastVec(FastVecOut fvOut, FastVecIn fvIn)
	{
		copyVec3(fvIn, fvOut);
	}

	// slow!
	static INLINEDBG void setFastVec(FastVecOut fvOut, const Vec3 vIn)
	{
		copyVec3(vIn, fvOut);
	}

	// slow!
	static INLINEDBG void setFastVecf(FastVecOut fvOut, F32 f0, F32 f1, F32 f2)
	{
		setVec3(fvOut, f0, f1, f2);
	}

	//////////////////////////////////////////////////////////////////////////

	static INLINEDBG void copyFastMat4(FastMat4Out fmOut, FastMat4In fmIn)
	{
		copyMat4(fmIn, fmOut);
	}

	static INLINEDBG void copyFastMat4Rotation(FastMat4Out fmOut, FastMat4In fmIn)
	{
		copyMat3(fmIn, fmOut);
	}

	static INLINEDBG void copyFastMat44toMat4Rotation(FastMat4Out fmOut, FastMat44In fmIn)
	{
		copyVec3(fmIn[0], fmOut[0]);
		copyVec3(fmIn[1], fmOut[1]);
		copyVec3(fmIn[2], fmOut[2]);
	}

	// slow!
	static INLINEDBG void setFastMat4(FastMat4Out fmOut, const Mat4 mIn)
	{
		copyMat4(mIn, fmOut);
	}

	// slow
	static INLINEDBG void setFastMat4Column(FastMat4Out fmOut, int column, FastVecIn fvIn)
	{
		copyVec3(fvIn, fmOut[column]);
	}

	// slow!
	static INLINEDBG void setFastMat4Columnf(FastMat4Out fmOut, int column, F32 f0, F32 f1, F32 f2)
	{
		setVec3(fmOut[column], f0, f1, f2);
	}

	// slow!
	static INLINEDBG void setFastMat4Columnfv(FastMat4Out fmOut, int column, const Vec3 vIn)
	{
		copyVec3(vIn, fmOut[column]);
	}

	static INLINEDBG void copyFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		copyMat44(fmIn, fmOut);
	}

	// slow!
	static INLINEDBG void setFastMat44(FastMat44Out fmOut, const Mat44 mIn)
	{
		copyMat44(mIn, fmOut);
	}

	static INLINEDBG void setFastMat44FromFastMat4(FastMat44Out fmOut, FastMat4In fmIn)
	{
		mat43to44(fmIn, fmOut);
	}

	// slow!
	static INLINEDBG void setFastMat44FromMat4(FastMat44Out fmOut, const Mat4 mIn)
	{
		mat43to44(mIn, fmOut);
	}

	// slow
	static INLINEDBG void setFastMat44Column(FastMat44Out fmOut, int column, FastVecIn fvIn)
	{
		copyVec3(fvIn, fmOut[column]);
		fmOut[column][3] = (column==3);
	}

	// slow!
	static INLINEDBG void setFastMat44Columnf(FastMat44Out fmOut, int column, F32 f0, F32 f1, F32 f2)
	{
		setVec3(fmOut[column], f0, f1, f2);
		fmOut[column][3] = (column==3);
	}

	// slow!
	static INLINEDBG void setFastMat44Columnfv(FastMat44Out fmOut, int column, const Vec3 vIn)
	{
		copyVec3(vIn, fmOut[column]);
		fmOut[column][3] = (column==3);
	}

	// slow!
	static INLINEDBG void setFastMat44Column4f(FastMat44Out fmOut, int column, F32 f0, F32 f1, F32 f2, F32 f3)
	{
		setVec4(fmOut[column], f0, f1, f2, f3);
	}

	// slow!
	static INLINEDBG void setFastMat44Column4fv(FastMat44Out fmOut, int column, const Vec4 vIn)
	{
		copyVec4(vIn, fmOut[column]);
	}

	static INLINEDBG void transposeFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		transposeMat44Copy(fmIn, fmOut);
	}

	static INLINEDBG void invertFastMat44(FastMat44Out fmOut, FastMat44In fmIn)
	{
		invertMat44Copy(fmIn, fmOut);
	}

	static INLINEDBG void invertFastMat4Quick(FastMat4Out fmOut, FastMat4In fmIn)
	{
		Vec3 dv;
		transposeMat3Copy(fmIn,fmOut);
		subVec3(zerovec3,fmIn[3],dv);
		mulVecMat3(dv,fmOut,fmOut[3]);
	}

	static INLINEDBG void invertFastMat44Quick(FastMat44Out fmOut, FastMat44In fmIn)
	{
		Vec3 dv;
		Mat4 mat, inv;

		mat44to43(fmIn, mat);
		transposeMat3Copy(mat,inv);
		subVec3(zerovec3,mat[3],dv);
		mulVecMat3(dv,inv,inv[3]);

		mat43to44(inv, fmOut);
	}

	//////////////////////////////////////////////////////////////////////////

	// slow!
	static INLINEDBG void getFastVec(Vec3 vOut, FastVecIn fvIn)
	{
		copyVec3(fvIn, vOut);
	}

	// slow!
	static INLINEDBG void getFastMat4(Mat4 mOut, FastMat4In fmIn)
	{
		copyMat4(fmIn, mOut);
	}

	// slow!
	static INLINEDBG void getFastMat4Column(FastVecOut fvOut, FastMat4In fmIn, int column)
	{
		copyVec3(fmIn[column], fvOut);
	}

	// slow!
	static INLINEDBG void getFastMat4Columnv(Vec3 vOut, FastMat4In fmIn, int column)
	{
		copyVec3(fmIn[column], vOut);
	}

	// slow!
	static INLINEDBG F32 getFastMat4Element(FastMat4In fmIn, int column, int row)
	{
		return fmIn[column][row];
	}

	// slow!
	static INLINEDBG void getFastMat44Column(FastVecOut fvOut, FastMat44In fmIn, int column)
	{
		copyVec3(fmIn[column], fvOut);
	}

	// slow!
	static INLINEDBG void getFastMat44Columnv(Vec4 vOut, FastMat44In fmIn, int column)
	{
		copyVec4(fmIn[column], vOut);
	}

	// slow!
	static INLINEDBG void getFastMat44Rowv(Vec4 vOut, FastMat44In fmIn, int row)
	{
		getMatRow(fmIn, row, vOut);
	}

	// slow!
	static INLINEDBG F32 getFastMat44Element(FastMat44In fmIn, int column, int row)
	{
		return fmIn[column][row];
	}

	// slow!
	static INLINEDBG void getFastMat4As44(Mat44 mOut, FastMat4In fmIn)
	{
		mat43to44(fmIn, mOut);
	}

	// slow!
	static INLINEDBG void getFastMat44As4(Mat4 mOut, FastMat44In fmIn)
	{
		mat44to43(fmIn, mOut);
	}

	// slow!
	static INLINEDBG void getFastMat44(Mat44 mOut, FastMat44In fmIn)
	{
		copyMat44(fmIn, mOut);
	}

	static INLINEDBG F32 *getFastMat44FloatPtr(FastMat44Out fmIn)
	{
		return &fmIn[0][0];
	}

	static INLINEDBG void getFastMat4QuickInverse(Mat4 mOut, FastMat4In fmIn)
	{
		Vec3 dv;
		transposeMat3Copy(fmIn,mOut);
		subVec3(zerovec3,fmIn[3],dv);
		mulVecMat3(dv,mOut,mOut[3]);
	}

#endif


#endif //_FASTMATH_H_
