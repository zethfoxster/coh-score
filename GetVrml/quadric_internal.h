#define DEF3(a, b) a ## b
#define DEF2(a, b) DEF3(a, b)
#define DEF(a) DEF2(a, DIM)

#define VECTYPE DEF(Vec)
#define MATTYPE DEF(Mat)

typedef struct DEF(_Quadric)
{
	MATTYPE A;
	VECTYPE b;
	float c;
} DEF(Quadric);

#define QTYPE DEF(Quadric)


//////////////////////////////////////////////////////////////////////////


_inline void DEF(zeroQ)(QTYPE *q)
{
	DEF(zeroMat)(q->A);
	DEF(zeroVec)(q->b);
	q->c = 0;
}

_inline void DEF(initQ)(QTYPE *q, const VECTYPE e1, const VECTYPE e2, const VECTYPE p)
{
	MATTYPE outer;
	VECTYPE temp;
	float tempF;

	// A = I - outer(e1,e1) - outer(e2,e2)
	DEF(identityMat)(q->A);
	DEF(outerProductVec)(e1, e1, outer);
	DEF(subFromMat)(outer, q->A);
	DEF(outerProductVec)(e2, e2, outer);
	DEF(subFromMat)(outer, q->A);

	// b = (p*e1)e1 + (p*e2)e2 - p
	DEF(scaleVec)(e1, DEF(dotVec)(p, e1), q->b);
	DEF(scaleVec)(e2, DEF(dotVec)(p, e2), temp);
	DEF(addToVec)(temp, q->b);
	DEF(subFromVec)(p, q->b);

	// c = p*p - (p*e1)^2 - (p*e2)^2
	tempF = DEF(dotVec)(p, e1);
	q->c = DEF(dotVec)(p, p) - SQR(tempF);
	tempF = DEF(dotVec)(p, e2);
	q->c -= SQR(tempF);
}

_inline void DEF(addQ)(const QTYPE *q1, const QTYPE *q2, QTYPE *res)
{
	DEF(addMat)(q1->A, q2->A, res->A);
	DEF(addVec)(q1->b, q2->b, res->b);
	res->c = q1->c + q2->c;
}

// q1 += q2
_inline void DEF(addToQ)(const QTYPE *q, QTYPE *res)
{
	DEF(addToMat)(q->A, res->A);
	DEF(addToVec)(q->b, res->b);
	res->c += q->c;
}

_inline void DEF(subQ)(const QTYPE *q1, const QTYPE *q2, QTYPE *res)
{
	DEF(subMat)(q1->A, q2->A, res->A);
	DEF(subVec)(q1->b, q2->b, res->b);
	res->c = q1->c - q2->c;
}

// q1 -= q2
_inline void DEF(subFromQ)(const QTYPE *q, QTYPE *res)
{
	DEF(subFromMat)(q->A, res->A);
	DEF(subFromVec)(q->b, res->b);
	res->c -= q->c;
}

_inline void DEF(scaleQ)(const QTYPE *q, QTYPE *res, float scale)
{
	DEF(scaleMat)(q->A, res->A, scale);
	DEF(scaleVec)(q->b, scale, res->b);
	res->c = q->c * scale;
}

// q *= scale
_inline void DEF(scaleByQ)(QTYPE *q, float scale)
{
	DEF(scaleByMat)(q->A, scale);
	DEF(scaleByVec)(q->b, scale);
	q->c *= scale;
}

_inline float DEF(evaluateQ)(const QTYPE *q, const VECTYPE v)
{
	VECTYPE temp;
	DEF(mulVecMat)(v, q->A, temp);
	return DEF(dotVec)(v, temp) + 2.f * DEF(dotVec)(q->b, v) + q->c;
}

_inline int DEF(optimizeQ)(const QTYPE *q, VECTYPE out)
{
	MATTYPE inv;

	if (!DEF(invertMat)(q->A, inv))
		return 0;

	DEF(mulVecMat)(q->b, inv, out);
	DEF(scaleByVec)(out, -1.f);

	return 1;
}

_inline void DEF(initFromTriQ)(QTYPE *q, const VECTYPE v1, const VECTYPE v2, const VECTYPE v3)
{
	VECTYPE e1;
	VECTYPE e2;
	VECTYPE temp;

	DEF(subVec)(v2, v1, e1);
	DEF(normalVec)(e1);

	DEF(subVec)(v3, v1, e2);
	DEF(scaleVec)(e1, DEF(dotVec)(e1, e2), temp);
	DEF(subVec)(e2, temp, e2);
	DEF(normalVec)(e2);

	DEF(initQ)(q, e1, e2, v1);
}


//////////////////////////////////////////////////////////////////////////


#undef DEF3
#undef DEF2
#undef DEF

#undef VECTYPE
#undef MATTYPE
#undef QTYPE

#undef DIM


