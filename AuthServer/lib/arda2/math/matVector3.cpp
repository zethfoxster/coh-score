/*****************************************************************************
	created:	2001/10/31
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matVector3.h"

/*
	NOTE[pmf]: Most of this file can be inlined, as it will either call the D3DX
	functions or our own math layer. For the sake of reduced compile dependency,
	these functions are not inlined at present.
 */

const matVector3 matVector3::Zero(matZero);
const matVector3 matVector3::Max(matMaximum);
const matVector3 matVector3::Min(matMinimum);

const matVector3 matVector3::Up(0.0f, 1.0f, 0.0f);
const matVector3 matVector3::Right(1.0f, 0.0f, 0.0f);
const matVector3 matVector3::Forward(0.0f, 0.0f, 1.0f);


void matVector3::StoreComponentClamp(	const matVector3& min,
                                     const matVector3& max)
{
    // componentwise clamp	
    x = matClamp(x, min.x, max.x);
    y = matClamp(y, min.y, max.y);
    z = matClamp(z, min.z, max.z);
}


/*
 *  UNIT TESTS
 */
#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

class matVector3Tests : public tstUnit
{
private:
    matVector3 vx, vy, vz, v1, v2;

public:
    matVector3Tests()
    {
    }

    virtual void Register()
    {
        SetName("matVector3");
        AddDependency("matUtils");

        AddTestCase("matVector3::Set()", &matVector3Tests::testSet);
        AddTestCase("operator+()", &matVector3Tests::testAddition);
        AddTestCase("operator-()", &matVector3Tests::testSubtraction);
        AddTestCase("operator*()", &matVector3Tests::testMultiplication);
        AddTestCase("CrossProduct()", &matVector3Tests::testCrossProduct);
        AddTestCase("DotProduct()", &matVector3Tests::testDotProduct);
        AddTestCase("matVector3::Normalize()", &matVector3Tests::testNormalize);
        AddTestCase("matVector3::Magnitude()", &matVector3Tests::testMagnitude);
        AddTestCase("matVector3::MagnitudeSquared()", &matVector3Tests::testMagnitudeSquared);
    };

    virtual void UnitTestSetup()
    {
        vx = matVector3(1.0f, 0.0f, 0.0f);
        vy = matVector3(0.0f, 1.0f, 0.0f);
        vz = matVector3(0.0f, 0.0f, 1.0f);
        v1 = matVector3(1.0f, 1.0f, 1.0f);
        v2 = matVector3(-1.0f, 1.0f, 1.0f);
    }

    virtual void UnitTestTearDown()
    {
    }

    void testSet() const
    {
        matVector3 vec;
        vec.Set(1.0f, 1.0f, 1.0f);
        TESTASSERT( v1.IsEqual(vec));
    }

    void testAddition() const
    {
        TESTASSERT( (vx + vy + vz).IsEqual(v1) );
    }

    void testSubtraction() const
    {
        TESTASSERT( (v1 - vx - vy - vz).IsEqual(matVector3::Zero) );
        TESTASSERT( (vx - vx).IsEqual(matVector3::Zero));
    }

    void testMultiplication() const
    {
        TESTASSERT( (v1 * 2.0f).IsEqual(matVector3(2.0f, 2.0f, 2.0f)) );
    }

    void testCrossProduct() const
    {
        TESTASSERT( CrossProduct(vx, vy).IsEqual(vz) );
        TESTASSERT( CrossProduct(vy, vz).IsEqual(vx) );
        TESTASSERT( CrossProduct(vz, vx).IsEqual(vy) );
        TESTASSERT( CrossProduct(vy, vx).IsEqual(-vz) );
    }

    void testDotProduct() const
    {
        TESTASSERT( IsEqual(DotProduct(v1, v2), 1.0f) );
        TESTASSERT( IsEqual(DotProduct(vx, vy), 0.0f) );
    }

    void testNormalize() const
    {
        // this can also fail because Magnitude is bad!!
        matVector3 vec = v1;
        vec.Normalize();
        TESTASSERT( IsEqual( vec.Magnitude(), 1.0f) );
    }

    void testMagnitude() const
    {
        TESTASSERT( IsEqual(vx.Magnitude(), 1.0f) );
        TESTASSERT( IsEqual(v1.Magnitude(), sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)) );
    }

    void testMagnitudeSquared() const
    {
        TESTASSERT( IsEqual(vx.MagnitudeSquared(), 1.0f) );
        TESTASSERT( IsEqual(v1.MagnitudeSquared(), (v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)) );
    }

};

EXPORTUNITTESTOBJECT(matVector3Tests);


#endif
