#define DEF3(a, b) a ## b
#define DEF2(a, b) DEF3(a, b)
#define DEF(a) DEF2(a, DIM)

#define VECTYPE DEF(Vec)
#define MATTYPE DEF(Mat)

typedef float DEF(Vec)[DIM];
typedef DEF(Vec) DEF(Mat)[DIM];


//////////////////////////////////////////////////////////////////////////


static INLINEDBG void DEF(zeroVec) (VECTYPE v)
{
	memset(v, 0, sizeof(float) * DIM);
}

// r = a + b
static INLINEDBG void DEF(addVec) (const VECTYPE a, const VECTYPE b, VECTYPE r)
{
	int i;
	for (i = 0; i < DIM; i++)
		r[i] = a[i] + b[i];
}

// a += b
static INLINEDBG void DEF(addToVec) (const VECTYPE v, VECTYPE r)
{
	int i;
	for (i = 0; i < DIM; i++)
		r[i] += v[i];
}

// r = a - b
static INLINEDBG void DEF(subVec) (const VECTYPE a, const VECTYPE b, VECTYPE r)
{
	int i;
	for (i = 0; i < DIM; i++)
		r[i] = a[i] - b[i];
}

// a -= b
static INLINEDBG void DEF(subFromVec) (const VECTYPE v, VECTYPE r)
{
	int i;
	for (i = 0; i < DIM; i++)
		r[i] -= v[i];
}

// r = v * scale
static INLINEDBG void DEF(scaleVec) (const VECTYPE v, float scale, VECTYPE r)
{
	int i;
	for (i = 0; i < DIM; i++)
		r[i] = v[i] * scale;
}

// v *= scale
static INLINEDBG void DEF(scaleByVec) (VECTYPE v, float scale)
{
	int i;
	for (i = 0; i < DIM; i++)
		v[i] *= scale;
}

static INLINEDBG void DEF(copyVec) (const VECTYPE v, VECTYPE r)
{
	memcpy(r, v, sizeof(float) * DIM);
}


//////////////////////////////////////////////////////////////////////////


static INLINEDBG void DEF(zeroMat) (MATTYPE m)
{
	memset(m, 0, sizeof(float) * DIM * DIM);
}

static INLINEDBG void DEF(identityMat) (MATTYPE m)
{
	int i;
	DEF(zeroMat)(m);
	for (i = 0; i < DIM; i++)
		m[i][i] = 1.f;
}

// r = a + b
static INLINEDBG void DEF(addMat) (const MATTYPE a, const MATTYPE b, MATTYPE r)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			r[i][j] = a[i][j] + b[i][j];
}

// a += b
static INLINEDBG void DEF(addToMat) (const MATTYPE m, MATTYPE r)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			r[i][j] += m[i][j];
}

// r = a - b
static INLINEDBG void DEF(subMat) (const MATTYPE a, const MATTYPE b, MATTYPE r)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			r[i][j] = a[i][j] - b[i][j];
}

// a -= b
static INLINEDBG void DEF(subFromMat) (const MATTYPE m, MATTYPE r)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			r[i][j] -= m[i][j];
}

// r = m * scale
static INLINEDBG void DEF(scaleMat) (const MATTYPE m, MATTYPE r, float scale)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			r[i][j] = m[i][j] * scale;
}

// m *= scale
static INLINEDBG void DEF(scaleByMat) (MATTYPE m, float scale)
{
	int i, j;
	for (i = 0; i < DIM; i++)
		for (j = 0; j < DIM; j++)
			m[i][j] *= scale;
}

static INLINEDBG void DEF(copyMat) (const MATTYPE m, MATTYPE r)
{
	memcpy(r, m, sizeof(float) * DIM * DIM);
}

static INLINEDBG void DEF(mulVecMat) (const VECTYPE v, const MATTYPE m, VECTYPE r)
{
	int i, j;
	for (j = 0; j < DIM; j++)
	{
		r[j] = m[0][j] * v[0];
		for (i = 1; i < DIM; i++)
			r[j] += m[i][j] * v[i];
	}
}


//////////////////////////////////////////////////////////////////////////


// compute the inner product of two vectors
static INLINEDBG float DEF(dotVec) (const VECTYPE a, const VECTYPE b)
{
	int i;
	float res = a[0] * b[0];
	for (i = 1; i < DIM; i++)
		res += a[i] * b[i];
	return res;
}

// compute the outer product of two vectors
static INLINEDBG void DEF(outerProductVec) (const VECTYPE a, const VECTYPE b, MATTYPE r)
{
	int x,y;

	for (y = 0; y < DIM; y++)
	{
		for (x = 0; x < DIM; x++)
			r[x][y] = a[y] * b[x];
	}
}

static INLINEDBG void DEF(normalVec) (VECTYPE v)
{
	float mag2 = DEF(dotVec)(v, v);
	float mag = fsqrt(mag2);
	if (mag > 0)
		DEF(scaleVec)(v, 1.f/mag, v);
}


//////////////////////////////////////////////////////////////////////////


static INLINEDBG int DEF(invertMat) (const MATTYPE m, MATTYPE r)
{
	float max, t, det, pivot, oneoverpivot;
	int i, j, k;
	MATTYPE A;
	DEF(copyMat)(m, A);


	/*---------- forward elimination ----------*/

	DEF(identityMat)(r);

	det = 1.0f;
	for (i = 0; i < DIM; i++)
	{
		/* eliminate in column i, below diag */
		max = -1.f;
		for (k = i; k < DIM; k++)
		{
			/* find pivot for column i */
			t = A[i][k];
			if (fabs(t) > max)
			{
				max = fabs(t);
				j = k;
			}
		}

		if (max <= 0.f)
			return 0;         /* if no nonzero pivot, PUNT */

		if (j != i)
		{
			/* swap rows i and j */
			for (k = i; k < DIM; k++)
			{
				t = A[k][i];
				A[k][i] = A[k][j];
				A[k][j] = t;
			}
			for (k = 0; k < DIM; k++)
			{
				t = r[k][i];
				r[k][i] = r[k][j];
				r[k][j] = t;
			}
			det = -det;
		}
		pivot = A[i][i];
		oneoverpivot = 1.f / pivot;
		det *= pivot;
		for (k = i + 1; k < DIM; k++)           /* only do elems to right of pivot */
			A[k][i] *= oneoverpivot;
		for (k = 0; k < DIM; k++)
			r[k][i] *= oneoverpivot;

		/* we know that A(i, i) will be set to 1, so don't bother to do it */

		for (j = i + 1; j < DIM; j++)
		{
			/* eliminate in rows below i */
			t = A[i][j];                /* we're gonna zero this guy */
			for (k = i + 1; k < DIM; k++)       /* subtract scaled row i from row j */
				A[k][j] -= A[k][i] * t;   /* (ignore k<=i, we know they're 0) */
			for (k = 0; k < DIM; k++)
				r[k][j] -= r[k][i] * t;   /* (ignore k<=i, we know they're 0) */
		}
	}

	/*---------- backward elimination ----------*/

	for (i = DIM - 1; i > 0; i--)
	{
		/* eliminate in column i, above diag */
		for (j = 0; j < i; j++)
		{
			/* eliminate in rows above i */
			t = A[i][j];                /* we're gonna zero this guy */
			for (k = 0; k < DIM; k++)         /* subtract scaled row i from row j */
				r[k][j] -= r[k][i] * t;   /* (ignore k<=i, we know they're 0) */
		}
	}

	if (det < 1e-8 && det > -1e-8)
		return 0;

	return 1;
}


//////////////////////////////////////////////////////////////////////////


#undef DEF3
#undef DEF2
#undef DEF

#undef VECTYPE
#undef MATTYPE

#undef DIM





