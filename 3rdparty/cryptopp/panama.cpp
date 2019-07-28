// panama.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "panama.h"

NAMESPACE_BEGIN(CryptoPP)

static const unsigned int STAGES = 32;

Panama::Panama()
	: m_state(17*2 + STAGES*sizeof(Stage))
{
	Reset();
}

void Panama::Reset()
{
	m_bstart = 0;
	memset(m_state, 0, m_state.size*4);
}

void Panama::Iterate(unsigned int count, const word32 *p, word32 *z, const word32 *y)
{
	unsigned int bstart = m_bstart;
	word32 *const a = m_state;
#define c (a+17)
#define b ((Stage *)(a+34))

// output
#define OA(i) z[i] = a[i+9]
#define OX(i) z[i] = y[i] ^ a[i+9]
// buffer update
#define US(i) {word32 t=b0[i]; b0[i]=p[i]^t; b25[(i+6)%8]^=t;}
#define UL(i) {word32 t=b0[i]; b0[i]=a[i+1]^t; b25[(i+6)%8]^=t;}
// gamma and pi
#define GP(i) c[5*i%17] = rotlFixed(a[i] ^ (a[(i+1)%17] | ~a[(i+2)%17]), ((5*i%17)*((5*i%17)+1)/2)%32)
// theta and sigma
#define T(i,x) a[i] = c[i] ^ c[(i+1)%17] ^ c[(i+4)%17] ^ x
#define TS1S(i) T(i+1, p[i])
#define TS1L(i) T(i+1, b4[i])
#define TS2(i) T(i+9, b16[i])

	while (count--)
	{
		if (z)
		{
			if (y)
			{
				OX(0); OX(1); OX(2); OX(3); OX(4); OX(5); OX(6); OX(7);
				y += 8;
			}
			else
			{
				OA(0); OA(1); OA(2); OA(3); OA(4); OA(5); OA(6); OA(7);
			}
			z += 8;
		}

		word32 *const b16 = b[(bstart+16) % STAGES];
		word32 *const b4 = b[(bstart+4) % STAGES];
		bstart = (bstart + STAGES - 1) % STAGES;
		word32 *const b0 = b[bstart];
		word32 *const b25 = b[(bstart+25) % STAGES];


		if (p)
		{
			US(0); US(1); US(2); US(3); US(4); US(5); US(6); US(7);
		}
		else
		{
			UL(0); UL(1); UL(2); UL(3); UL(4); UL(5); UL(6); UL(7);
		}

		GP(0); GP(1); GP(2); GP(3); GP(4); GP(5); GP(6); GP(7);
		GP(8); GP(9); GP(10); GP(11); GP(12); GP(13); GP(14); GP(15); GP(16);

		T(0,1);

		if (p)
		{
			TS1S(0); TS1S(1); TS1S(2); TS1S(3); TS1S(4); TS1S(5); TS1S(6); TS1S(7);
			p += 8;
		}
		else
		{
			TS1L(0); TS1L(1); TS1L(2); TS1L(3); TS1L(4); TS1L(5); TS1L(6); TS1L(7);
		}

		TS2(0); TS2(1); TS2(2); TS2(3); TS2(4); TS2(5); TS2(6); TS2(7);
	}
	m_bstart = bstart;
}

template <bool H>
unsigned int PanamaHash<H>::HashMultipleBlocks(const word32 *input, unsigned int length)
{
	if (CheckEndianess(HIGHFIRST))
	{
		Iterate(length / BLOCKSIZE, input);
		return length % BLOCKSIZE;
	}
	else
		return IteratedHashBase<word32>::HashMultipleBlocks(input, length);
}

template <bool H>
void PanamaHash<H>::TruncatedFinal(byte *hash, unsigned int size)
{
	assert(size <= DIGESTSIZE);

	PadLastBlock(BLOCKSIZE, 0x01);
	CorrectEndianess(data, data, BLOCKSIZE);
	
	vTransform(data);

	Iterate(32);	// pull

	CorrectEndianess(m_state+9, m_state+9, DIGESTSIZE);
	memcpy(hash, m_state+9, size);

	Reinit();		// reinit for next use
}

template <bool H>
PanamaCipher<H>::PanamaCipher(const byte *key, const byte *iv)
	: m_buf(8), m_leftOver(0)
{
	memcpy(m_buf, key, 32);
	CorrectEndianess(m_buf, m_buf, 32);
	Iterate(1, m_buf);
	if (iv)
	{
		memcpy(m_buf, iv, 32);
		CorrectEndianess(m_buf, m_buf, 32);
	}
	else
		memset(m_buf, 0, 32);
	Iterate(1, m_buf);

	Iterate(32);
}

template <bool H>
void PanamaCipher<H>::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	if (m_leftOver > 0)
	{
		unsigned int len = STDMIN(m_leftOver, length);
		xorbuf(outString, inString, (byte *)(m_buf+m_buf.size)-m_leftOver, len);
		length -= len;
		m_leftOver -= len;
		inString += len;
		outString += len;
	}

	if (CheckEndianess(HIGHFIRST) && IsAligned<word32>(outString))
	{
		if (!IsAligned<word32>(inString))
		{
			memcpy(outString, inString, length);
			inString = outString;
		}
		Iterate(length / 32, NULL, (word32 *)outString, (const word32 *)inString);
		inString += length - length % 32;
		outString += length - length % 32;
		length %= 32;
	}

	while (length)
	{
		Iterate(1, NULL, m_buf);
		CorrectEndianess(m_buf, m_buf, 32);
		unsigned int len = STDMIN(32U, length);
		xorbuf(outString, inString, (byte *)m_buf.ptr, len);
		length -= len;
		m_leftOver = 32 - len;
		inString += len;
		outString += len;
	}
}

template class PanamaHash<true>;
template class PanamaHash<false>;

template class PanamaCipher<true>;
template class PanamaCipher<false>;

NAMESPACE_END
