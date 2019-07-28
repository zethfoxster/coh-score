/**
 *  copyright:  (c) 2005, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    Math utility functions
 *  modified:   $DateTime: 2006/06/05 17:48:40 $
 *              $Author: cthurow $
 *              $Change: 254334 $
 *              @file
 */


#include "arda2/math/matFirst.h"
#include "arda2/math/matUtils.h"


/*
*  UNIT TESTS
*/
#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

class matUtilsTests : public tstUnit
{
private:

public:
    matUtilsTests()
    {
    }

    virtual void Register()
    {
        SetName("matUtils");

        AddTestCase("matSquare()", &matUtilsTests::TestSquare);
        AddTestCase("matClamp()", &matUtilsTests::TestClamp);
        AddTestCase("matLerp()", &matUtilsTests::TestLerp);
        AddTestCase("matClampedLerp()", &matUtilsTests::TestClampedLerp);
        AddTestCase("matSmoothStep()", &matUtilsTests::TestSmoothStep);
        AddTestCase("matBilerp()", &matUtilsTests::TestBilerp);
        AddTestCase("matDegreesToRadians()", &matUtilsTests::TestDegreesToRadians);
        AddTestCase("matRadiansToDegrees()", &matUtilsTests::TestRadiansToDegrees);
        AddTestCase("matAngleInterp()", &matUtilsTests::TestAngleInterp);
        AddTestCase("matIsNegative()", &matUtilsTests::TestIsNegative);
        AddTestCase("matFloatToInt()", &matUtilsTests::TestFloatToInt);
        AddTestCase("matFloatToIntRound()", &matUtilsTests::TestFloatToIntRound);
        AddTestCase("matFloatToIntFloor()", &matUtilsTests::TestFloatToIntFloor);
        AddTestCase("matFloatToIntCeil()", &matUtilsTests::TestFloatToIntCeil);
    };

    virtual void UnitTestSetup()
    {
    }

    virtual void UnitTestTearDown()
    {
    }

    void TestSquare() const
    {
        // NOTE: these tests only exercise POD types. matSquare is a template, and therefore
        // other data types should include their own versions of these tests

        // integer tests
        TESTASSERT( matSquare(0) == 0 );
        TESTASSERT( matSquare(1) == 1 );
        TESTASSERT( matSquare(-1) == 1 );
        TESTASSERT( matSquare(0x8000) == 0x40000000 );

        // float tests
        TESTASSERT( IsEqual( matSquare(0.0f), 0.0f) );
        TESTASSERT( IsEqual( matSquare(1.0f), 1.0f) );
        TESTASSERT( IsEqual( matSquare(-1.0f), 1.0f) );
        TESTASSERT( IsEqual( matSquare(-1.0f), 1.0f) );
        TESTASSERT( IsEqual( matSquare(1.0e16f), 1.0e32f) );
    }

    void TestClamp() const
    {
        // NOTE: these tests only exercise POD types. matClamp is a template, and therefore
        // other data types should include their own versions of these tests

        // integer tests
        TESTASSERT( matClamp(0, -1, +1) == 0 );
        TESTASSERT( matClamp(-1, -1, +1) == -1 );
        TESTASSERT( matClamp(+1, -1, +1) == +1 );
        TESTASSERT( matClamp(-2, -1, +1) == -1 );
        TESTASSERT( matClamp(+2, -1, +1) == +1 );
        TESTASSERT( matClamp(0, -1, -1) == -1 );
        TESTASSERT( matClamp(0, +1, +1) == +1 );

        // float tests
        TESTASSERT( matClamp(0.0f, -1.0f, +1.0f) == 0.0f );
        TESTASSERT( matClamp(-1.0f, -1.0f, +1.0f) == -1.0f );
        TESTASSERT( matClamp(+1.0f, -1.0f, +1.0f) == +1.0f );
        TESTASSERT( matClamp(-2.0f, -1.0f, +1.0f) == -1.0f );
        TESTASSERT( matClamp(+2.0f, -1.0f, +1.0f) == +1.0f );
        TESTASSERT( matClamp(0.0f, -1.0f, -1.0f) == -1.0f );
        TESTASSERT( matClamp(0.0f, +1.0f, +1.0f) == +1.0f );
    }

    void TestLerp() const
    {
        // NOTE: these tests only exercise POD types. matLerp is a template, and therefore
        // other data types should include their own versions of these tests

        // float tests
        TESTASSERT( IsEqual(matLerp(0.0f, 10.0f, 0.0f), 0.0f) );
        TESTASSERT( IsEqual(matLerp(0.0f, 10.0f, 0.5f), 5.0f) );
        TESTASSERT( IsEqual(matLerp(0.0f, 10.0f, 1.0f), 10.0f) );

        // out of range values
        TESTASSERT( IsEqual(matLerp(0.0f, 10.0f, -1.0f), -10.0f) );
        TESTASSERT( IsEqual(matLerp(0.0f, 10.0f, 2.0f), 20.0f) );
    }

    void TestClampedLerp() const
    {
        // NOTE: these tests only exercise POD types. matClampedLerp is a template, and therefore
        // other data types should include their own versions of these tests

        // float tests
        TESTASSERT( IsEqual(matClampedLerp(0.0f, 10.0f, 0.0f), 0.0f) );
        TESTASSERT( IsEqual(matClampedLerp(0.0f, 10.0f, 0.5f), 5.0f) );
        TESTASSERT( IsEqual(matClampedLerp(0.0f, 10.0f, 1.0f), 10.0f) );

        // out of range values
        TESTASSERT( IsEqual(matClampedLerp(0.0f, 10.0f, -1.0f), 0.0f) );
        TESTASSERT( IsEqual(matClampedLerp(0.0f, 10.0f, 2.0f), 10.0f) );
    }

    void TestSmoothStep() const
    {
        TESTASSERT( IsEqual(matSmoothStep(0.0f, 10.0f, 0.0f), 0.0f) );
        TESTASSERT( IsEqual(matSmoothStep(0.0f, 10.0f, 0.5f), 5.0f) );
        TESTASSERT( IsEqual(matSmoothStep(0.0f, 10.0f, 1.0f), 10.0f) );

        // out of range values
        TESTASSERT( IsEqual(matSmoothStep(0.0f, 10.0f, -1.0f), 0.0f) );
        TESTASSERT( IsEqual(matSmoothStep(0.0f, 10.0f, 2.0f), 10.0f) );
    }

    void TestBilerp() const
    {
        // NOTE: these tests only exercise POD types. matBilerp is a template, and therefore
        // other data types should include their own versions of these tests

        const float kTL = -1234.0f;
        const float kTR = +5678.0f;
        const float kBL = +0.9876f;
        const float kBR = -0.5432f;

        // float tests
        //
        // please do not re-order these tests.  They have been placed in this
        // specific order in so that release build unit tests do not crash in VC7.
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.5f, 1.0f), (kBL + kBR) / 2) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.5f, 0.0f), (kTL + kTR) / 2) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.0f, 0.5f), (kTL + kBL) / 2) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 1.0f, 0.5f), (kTR + kBR) / 2) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.5f, 0.5f), (kTL + kTR + kBL + kBR) / 4) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.0f, 0.0f), kTL) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 1.0f, 0.0f), kTR) );
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 0.0f, 1.0f), kBL) ); // this fails?
        TESTASSERT( IsEqual(matBilerp(kTL, kTR, kBL, kBR, 1.0f, 1.0f), kBR) ); // this fails?
    }

    void TestDegreesToRadians() const
    {
        TESTASSERT( IsEqual(matDegreesToRadians(0.0f), 0.0f) );
        TESTASSERT( IsEqual(matDegreesToRadians(360.0f), mat2Pi) );
        TESTASSERT( IsEqual(matDegreesToRadians(180.0f), matPi) );
        TESTASSERT( IsEqual(matDegreesToRadians(90.0f), matPiDiv2) );
        TESTASSERT( IsEqual(matDegreesToRadians(45.0f), matPiDiv4) );
        TESTASSERT( IsEqual(matDegreesToRadians(36.0f), mat2Pi / 10.0f) );
        TESTASSERT( IsEqual(matDegreesToRadians(3.6f), mat2Pi / 100.0f) );
        TESTASSERT( IsEqual(matDegreesToRadians(3600.0f), mat2Pi * 10) );
        TESTASSERT( IsEqual(matDegreesToRadians(360000000.0f), mat2Pi * 1000000) );
    }

    void TestRadiansToDegrees() const
    {
        TESTASSERT( IsEqual(matRadiansToDegrees(0.0f), 0.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(mat2Pi), 360.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(matPi), 180.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(matPiDiv2), 90.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(matPiDiv4), 45.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(mat2Pi / 10.0f), 36.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(mat2Pi / 100.0f), 3.6f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(mat2Pi * 10), 3600.0f) );
        TESTASSERT( IsEqual(matRadiansToDegrees(mat2Pi * 1000000), 360000000.0f) );
    }

    void TestAngleInterp() const
    {
        const float k45 = matDegreesToRadians(45.0f);
        const float k90 = matDegreesToRadians(90.0f);
        const float k135 = matDegreesToRadians(135.0f);
        const float k180 = matDegreesToRadians(180.0f);
        const float k360 = matDegreesToRadians(360.0f);

        // 0 to 90 degrees
        TESTASSERT( IsEqual(matAngleInterp(0.0f, k90, 0.0f), 0.0f) );
        TESTASSERT( IsEqual(matAngleInterp(0.0f, k90, 0.5f), k45) );
        TESTASSERT( IsEqual(matAngleInterp(0.0f, k90, 1.0f), k90) );

        // -45 to +45 degrees
        TESTASSERT( IsEqual(matAngleInterp(-k45, +k45, 0.0f), -k45) );
        TESTASSERT( IsEqual(matAngleInterp(-k45, +k45, 0.5f), 0) );
        TESTASSERT( IsEqual(matAngleInterp(-k45, +k45, 1.0f), +k45) );

        // +45 to -45 degrees
        TESTASSERT( IsEqual(matAngleInterp(+k45, -k45, 0.0f), +k45) );
        TESTASSERT( IsEqual(matAngleInterp(+k45, -k45, 0.5f), 0) );
        TESTASSERT( IsEqual(matAngleInterp(+k45, -k45, 1.0f), -k45) );

        // -360 to +360 degrees
        TESTASSERT( IsEqual(matAngleInterp(-k360, +k360, 0.0f), 0.0f) );
        TESTASSERT( IsEqual(matAngleInterp(-k360, +k360, 0.5f), 0.0f) );
        TESTASSERT( IsEqual(matAngleInterp(-k360, +k360, 1.0f), 0.0f) );

        // 90 to 180 degrees
        TESTASSERT( IsEqual(matAngleInterp(k90, k180, 0.0f), k90) );
        TESTASSERT( IsEqual(matAngleInterp(k90, k180, 0.5f), k135) );
        TESTASSERT( IsEqual(matAngleInterp(k90, k180, 1.0f), k180) );

        // 180 to 90 degrees
        TESTASSERT( IsEqual(matAngleInterp(k180, k90, 0.0f), k180) );
        TESTASSERT( IsEqual(matAngleInterp(k180, k90, 0.5f), k135) );
        TESTASSERT( IsEqual(matAngleInterp(k180, k90, 1.0f), k90) );

        // 90 to 180 offset by large k; larger epsilon necessary
        const float kOffset1 = mat2Pi * +33.0f;
        const float kOffset2 = mat2Pi * -77.0f;
        const float kEpsilon = 0.001f;
        TESTASSERT( IsEqual(matAngleInterp(k90+kOffset1, k180+kOffset1, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k90+kOffset2, k180+kOffset2, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k90+kOffset1, k180+kOffset2, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k90+kOffset2, k180+kOffset1, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k180+kOffset1, k90+kOffset1, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k180+kOffset2, k90+kOffset2, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k180+kOffset1, k90+kOffset2, 0.5f), k135, kEpsilon) );
        TESTASSERT( IsEqual(matAngleInterp(k180+kOffset2, k90+kOffset1, 0.5f), k135, kEpsilon) );
    }

    void TestIsNegative() const
    {
        TESTASSERT( !matIsNegative(0.0f) );
        for ( int i = 0; i < FLT_MAX_EXP; ++i )
        {
            float fValue = ldexpf(1.0f, i);
            TESTASSERT( matIsNegative(-fValue) );
            TESTASSERT( !matIsNegative(+fValue) );
        }
    }

    void TestFloatToInt() const
    {
        TESTASSERT( matFloatToInt(+0.0f) == +0 );
        TESTASSERT( matFloatToInt(+0.1f) == +0 );
        TESTASSERT( matFloatToInt(+0.5f) == +0 );
        TESTASSERT( matFloatToInt(+0.9f) == +0 );
        TESTASSERT( matFloatToInt(+1.0f) == +1 );
        TESTASSERT( matFloatToInt(+1.1f) == +1 );
        TESTASSERT( matFloatToInt(+1.5f) == +1 );
        TESTASSERT( matFloatToInt(+1.9f) == +1 );

        TESTASSERT( matFloatToInt(-0.0f) == -0 );
        TESTASSERT( matFloatToInt(-0.1f) == -0 );
        TESTASSERT( matFloatToInt(-0.5f) == -0 );
        TESTASSERT( matFloatToInt(-0.9f) == -0 );
        TESTASSERT( matFloatToInt(-1.0f) == -1 );
        TESTASSERT( matFloatToInt(-1.1f) == -1 );
        TESTASSERT( matFloatToInt(-1.5f) == -1 );
        TESTASSERT( matFloatToInt(-1.9f) == -1 );

        // 4194304 = 0x400000 = 2^22; test with values that still have one fractional mantissa bit
        TESTASSERT( matFloatToInt(+4194304.0f) == +4194304 );
        TESTASSERT( matFloatToInt(+4194304.6f) == +4194304 );
        TESTASSERT( matFloatToInt(-4194304.0f) == -4194304 );
        TESTASSERT( matFloatToInt(-4194304.6f) == -4194304 );

        // 8388608 = 0x800000 = 2^23; all mantissa bits used
        TESTASSERT( matFloatToInt(+8388608.0f) == +8388608 );
        TESTASSERT( matFloatToInt(-8388608.0f) == -8388608 );
    }

    void TestFloatToIntRound() const
    {
        TESTASSERT( matFloatToIntRound(+0.0f) == +0 );
        TESTASSERT( matFloatToIntRound(+0.1f) == +0 );
        TESTASSERT( matFloatToIntRound(+0.9f) == +1 );
        TESTASSERT( matFloatToIntRound(+1.0f) == +1 );
        TESTASSERT( matFloatToIntRound(+1.1f) == +1 );
        TESTASSERT( matFloatToIntRound(+1.9f) == +2 );

        TESTASSERT( matFloatToIntRound(-0.0f) == -0 );
        TESTASSERT( matFloatToIntRound(-0.1f) == -0 );
        TESTASSERT( matFloatToIntRound(-0.9f) == -1 );
        TESTASSERT( matFloatToIntRound(-1.0f) == -1 );
        TESTASSERT( matFloatToIntRound(-1.1f) == -1 );
        TESTASSERT( matFloatToIntRound(-1.9f) == -2 );

        // 2097152 = 0x200000 = 2^21; test with values that still have two fractional mantissa bits
        TESTASSERT( matFloatToIntRound(+2097152.0f) == +2097152 );
        TESTASSERT( matFloatToIntRound(+2097152.8f) == +2097153 );
        TESTASSERT( matFloatToIntRound(-2097152.0f) == -2097152 );
        TESTASSERT( matFloatToIntRound(-2097152.8f) == -2097153 );

        // 8388608 = 0x800000 = 2^23; all mantissa bits used
        TESTASSERT( matFloatToIntRound(+8388608.0f) == +8388608 );
        TESTASSERT( matFloatToIntRound(-8388608.0f) == -8388608 );
    }

    void TestFloatToIntFloor() const
    {
        TESTASSERT( matFloatToIntFloor(+0.0f) == +0 );
        TESTASSERT( matFloatToIntFloor(+0.1f) == +0 );
        TESTASSERT( matFloatToIntFloor(+0.5f) == +0 );
        TESTASSERT( matFloatToIntFloor(+0.9f) == +0 );
        TESTASSERT( matFloatToIntFloor(+1.0f) == +1 );
        TESTASSERT( matFloatToIntFloor(+1.1f) == +1 );
        TESTASSERT( matFloatToIntFloor(+1.5f) == +1 );
        TESTASSERT( matFloatToIntFloor(+1.9f) == +1 );

        TESTASSERT( matFloatToIntFloor(-0.0f) == -0 );
        TESTASSERT( matFloatToIntFloor(-0.1f) == -1 );
        TESTASSERT( matFloatToIntFloor(-0.5f) == -1 );
        TESTASSERT( matFloatToIntFloor(-0.9f) == -1 );
        TESTASSERT( matFloatToIntFloor(-1.0f) == -1 );
        TESTASSERT( matFloatToIntFloor(-1.1f) == -2 );
        TESTASSERT( matFloatToIntFloor(-1.5f) == -2 );
        TESTASSERT( matFloatToIntFloor(-1.9f) == -2 );

        // 4194304 = 0x400000 = 2^22; test with values that still have one fractional mantissa bit
        TESTASSERT( matFloatToIntFloor(+4194304.0f) == +4194304 );
        TESTASSERT( matFloatToIntFloor(+4194304.6f) == +4194304 );
        TESTASSERT( matFloatToIntFloor(-4194304.0f) == -4194304 );
        TESTASSERT( matFloatToIntFloor(-4194304.6f) == -4194305 );

        // 8388608 = 0x800000 = 2^23; all mantissa bits used
        TESTASSERT( matFloatToIntFloor(+8388608.0f) == +8388608 );
        TESTASSERT( matFloatToIntFloor(-8388608.0f) == -8388608 );
    }

    void TestFloatToIntCeil() const
    {
        TESTASSERT( matFloatToIntCeil(+0.0f) == +0 );
        TESTASSERT( matFloatToIntCeil(+0.1f) == +1 );
        TESTASSERT( matFloatToIntCeil(+0.5f) == +1 );
        TESTASSERT( matFloatToIntCeil(+0.9f) == +1 );
        TESTASSERT( matFloatToIntCeil(+1.0f) == +1 );
        TESTASSERT( matFloatToIntCeil(+1.1f) == +2 );
        TESTASSERT( matFloatToIntCeil(+1.5f) == +2 );
        TESTASSERT( matFloatToIntCeil(+1.9f) == +2 );

        TESTASSERT( matFloatToIntCeil(-0.0f) == -0 );
        TESTASSERT( matFloatToIntCeil(-0.1f) == -0 );
        TESTASSERT( matFloatToIntCeil(-0.5f) == -0 );
        TESTASSERT( matFloatToIntCeil(-0.9f) == -0 );
        TESTASSERT( matFloatToIntCeil(-1.0f) == -1 );
        TESTASSERT( matFloatToIntCeil(-1.1f) == -1 );
        TESTASSERT( matFloatToIntCeil(-1.5f) == -1 );
        TESTASSERT( matFloatToIntCeil(-1.9f) == -1 );

        // 4194304 = 0x400000 = 2^22; test with values that still have one fractional mantissa bit
        TESTASSERT( matFloatToIntCeil(+4194304.0f) == +4194304 );
        TESTASSERT( matFloatToIntCeil(+4194304.6f) == +4194305 );
        TESTASSERT( matFloatToIntCeil(-4194304.0f) == -4194304 );
        TESTASSERT( matFloatToIntCeil(-4194304.6f) == -4194304 );

        // 8388608 = 0x800000 = 2^23; all mantissa bits used
        TESTASSERT( matFloatToIntCeil(+8388608.0f) == +8388608 );
        TESTASSERT( matFloatToIntCeil(-8388608.0f) == -8388608 );
    }


};

EXPORTUNITTESTOBJECT(matUtilsTests);


#endif


