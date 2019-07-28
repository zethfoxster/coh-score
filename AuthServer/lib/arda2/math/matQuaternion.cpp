/*****************************************************************************
	created:	2002/10/02
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	Math Quaternion functions
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matQuaternion.h"
#include "arda2/math/matMatrix4x4.h"

const matQuaternion matQuaternion::Identity(matIdentity);

void matQuaternion::StoreLookDir( const	matVector3& vForward, const matVector3& vGoalUp )
{
	matMatrix4x4 mLook;
	mLook.StoreLookDir(vForward, vGoalUp);
	StoreFromMatrix(mLook);
}


#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

class matQuaternionTests : public tstUnit
{
private:
    matQuaternion   m_q0;
    matQuaternion   m_q1;
    matQuaternion   m_q2;
public:
    matQuaternionTests() :
        m_q0(1.2f, -0.5f, 0.33f, 1.1f),
        m_q1(-1.1f, 0.6f, 0.34f, -0.99f),
        m_q2(2.03f, 1.1f, 0.654f, -0.983f)
	{
	}

	virtual void Register()
	{
		SetName("matQuaternion");

        AddDependency("matUtils");
        AddDependency("matVector3");

        AddTestCase("matQuaternion(matIdentity)",&matQuaternionTests::TestIdentity );
        AddTestCase("matQuaternion::Length()",   &matQuaternionTests::TestLength );
        AddTestCase("matQuaternion::LengthSq()", &matQuaternionTests::TestLengthSq);
        AddTestCase("matQuaternion::Normalize",  &matQuaternionTests::TestNormalize);
        AddTestCase("matQuaternion::Exp",        &matQuaternionTests::TestExp);
        AddTestCase("matQuaternion::Ln",         &matQuaternionTests::TestLn);
        AddTestCase("matQuaternion::Conjugate",  &matQuaternionTests::TestConjugate);
        AddTestCase("matQuaternion::Inverse",    &matQuaternionTests::TestInverse);
        AddTestCase("matQuaternion::Dot",        &matQuaternionTests::TestDot);
        AddTestCase("matQuaternion::Multiply",   &matQuaternionTests::TestMultiply);
        AddTestCase("matQuaternion::Slerp",      &matQuaternionTests::TestSlerp);
        AddTestCase("matQuaternion::FromYPR",    &matQuaternionTests::TestFromYPR);
        AddTestCase("matQuaternion::FromMatrix", &matQuaternionTests::TestFromMatrix);
        AddTestCase("matQuaternion::AxisAngle",  &matQuaternionTests::TestAxisAngle);
	};

	virtual void UnitTestSetup()
	{
		// set up code
	}

	virtual void UnitTestTearDown()
	{
		// tear down code
	}

    void TestIdentity() const
    {
        matQuaternion qI0(matIdentity);
        TESTASSERT(qI0.IsIdentity());
        TESTASSERT(IsEqual(qI0, matQuaternion::Identity));

        TESTASSERT(IsEqual(qI0.w, 1.0f));
        TESTASSERT(IsEqual(qI0.x, 0.0f));
        TESTASSERT(IsEqual(qI0.y, 0.0f));
        TESTASSERT(IsEqual(qI0.z, 0.0f));

        matQuaternion qI1(0.0f, 0.0f, 0.0f, 1.0f);
        TESTASSERT(qI1.IsIdentity());
        TESTASSERT(IsEqual(qI1, matQuaternion::Identity));
        TESTASSERT(IsEqual(qI1, qI0));
    }

    void TestLength() const
    {
        TESTASSERT( IsEqual(matQuaternion::Identity.Length(), 1.0f));
        TESTASSERT( IsEqual(m_q0.Length(), 1.734618f) );
        TESTASSERT( IsEqual(m_q1.Length(), 1.632697f) );
        TESTASSERT( IsEqual(m_q2.Length(), 2.593242f) );
    }

    void TestLengthSq() const
    {
        TESTASSERT( IsEqual(matQuaternion::Identity.LengthSq(), 1.0f));
        TESTASSERT( IsEqual(m_q0.LengthSq(), 3.008900f) );
        TESTASSERT( IsEqual(m_q1.LengthSq(), 2.665700f) );
        TESTASSERT( IsEqual(m_q2.LengthSq(), 6.724905f) );
    }

    void TestNormalize() const
    {
        const matQuaternion q0_norm(0.6917959f, -0.2882479f, 0.1902436f, 0.6341454f);

        matQuaternion q0(m_q0);
        matQuaternion q1(m_q1);
        matQuaternion q2(m_q2);
        q0.Normalize();
        q1.Normalize();
        q2.Normalize();
        TESTASSERT( IsEqual(q0.LengthSq(), 1.0f));
        TESTASSERT( IsEqual(q1.LengthSq(), 1.0f));
        TESTASSERT( IsEqual(q2.LengthSq(), 1.0f));
        TESTASSERT( q0.IsNormal() );
        TESTASSERT( q1.IsNormal() );
        TESTASSERT( q2.IsNormal() );
        TESTASSERT( IsEqual(q0, q0_norm) );
    }

    void TestExp() const
    {
        matQuaternion q_exp(0.8712285f, -0.3630118f, 0.2395878f, 0.2275544f);

        matQuaternion q(m_q0);
        q.StoreExponential();
        TESTASSERT( IsEqual(q, q_exp) );
    }

    void TestLn() const
    {
        matQuaternion q_ln(1.2000000f, -0.5000000f, 0.3300000f, 0.0000000f);
        matQuaternion q(m_q0);
        q.StoreNaturalLog();
        TESTASSERT( IsEqual(q, q_ln) );
    }

    void TestConjugate() const
    {
        matQuaternion q_conj(-1.2000000f, 0.5000000f, -0.3300000f, 1.1000000f);
        matQuaternion q(m_q0);
        q.StoreConjugate();
        TESTASSERT( IsEqual(q, q_conj) );
    }

    void TestInverse() const
    {
        matQuaternion q_inv(-0.3988168f, 0.1661737f, -0.1096746f, 0.3655821f);
        matQuaternion q;
        q.StoreInverse(m_q0);
        TESTASSERT( IsEqual(q, q_inv) );
    }

    void TestDot() const
    {
        TESTASSERT( IsEqual(m_q1.Dot(m_q2), -0.37747f) );
    }

    void TestMultiply() const
    {
        const matQuaternion q_mul(-0.9468000f, -3.0883999f, 1.4463201f, 2.3238101f);

        matQuaternion q;
        q.StoreMultiply(m_q1,m_q2);
        TESTASSERT( IsEqual(q, q_mul) );
    }

    void TestSlerp() const
    {
        const matQuaternion q_slerp1(-1.8857677f, -0.3012409f, -0.1891792f, -0.0042174f);
        const matQuaternion q_slerp2(0.0599866f, 1.2971215f, 0.7535115f, -1.6389856f);
        const matQuaternion q_slerp3(-2.0583560f, -0.9628299f, -0.5745794f, 0.8025436f);

        matQuaternion q;
        q.StoreSlerp(m_q1, m_q2, 0.0f);
        TESTASSERT( IsEqual(q, m_q1) );

        q.StoreSlerp(m_q1, m_q2, 0.5f);
        TESTASSERT( IsEqual(q, q_slerp1) );

        q.StoreSlerp(m_q1, m_q2, 1.0f);
        TESTASSERT( IsEqual(q, m_q2) );

        q.StoreSlerp(m_q1, m_q2, -0.5f);
        TESTASSERT( IsEqual(q, q_slerp2) );

        q.StoreSlerp(m_q1, m_q2, 0.9f);
        TESTASSERT( IsEqual(q, q_slerp3) );
    }

    void TestFromYPR() const
    {
        const matQuaternion q_ypr(-0.3038092f, 0.7045271f, -0.3782389f, -0.5179546f);

        matQuaternion q;
        q.StoreEulerAngles(1.23f, 2.13f, -3.21f);
        TESTASSERT( IsEqual(q, q_ypr) );
    }

    void TestFromMatrix() const
    {
        static const matMatrix4x4 mat(
            0.749581f,    -0.321058f,    -0.578835f,     0.000000f, 
            0.433737f,    -0.422332f,     0.795933f,     0.000000f, 
            -0.500001f,    -0.847678f,    -0.177317f,     0.000000f, 
            0.000000f,     0.000000f,     0.000000f,     1.000000f
            );

        static const matQuaternion q_mat(0.7663598f, 0.0367576f, -0.3519353f, 0.5361745f);

        matQuaternion q;
        q.StoreFromMatrix(mat);
        TESTASSERT( IsEqual(q, q_mat) );
    }

    void TestAxisAngle() const
    {
        const matQuaternion q_aa(0.0157636f, 0.3765250f, -0.4368770f, 0.8167734f);
        const matVector3 v_axis(0.0157636f, 0.376525f, -0.436877f);
        const float fAngle = 1.23f;

        // from AA
        matVector3 v(1.4f, 33.44f, -38.8f);
        matQuaternion q;
        q.StoreRotationAxis(v, 1.23f);
        TESTASSERT( IsEqual(q, q_aa) );

        // to AA
        matVector3 v1(0.0f, 0.0f, 0.0f);
        float f1 = 0.0f;
        q.ToAxisAngle(v1, f1);
        TESTASSERT( IsEqual(v1, v_axis) && IsEqual(f1, fAngle) );
    }
};

EXPORTUNITTESTOBJECT(matQuaternionTests);

#endif // BUILD_UNIT_TESTS
