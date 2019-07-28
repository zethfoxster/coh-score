/* sha512Impl.cpp: Implements the heart and soul of the SHA-512 algorithm.
* 
* @author: Chris Cowden, ccowden@ncsoft.com
*
* created: 8/23/2006
*
* @file
*
* Functions:
*/

#include "cryptLib/sha512Impl.h"
#include <cassert>

using namespace cryptLib;

sha512Impl::sha512Impl() : 
w_bits(w_64_bits) // default to 64-bit, SHA-512
{
	_LoadK();
}

sha512Impl::~sha512Impl()
{

}

// This function initializes K, since I can't simply initialize K statically,
// due to type restrictions by the compiler (currently Bloodshed Dev-C++)
void sha512Impl::_LoadK()
{
	const uint64 k[160] = {
		0x428A2F98, 0xD728AE22, 0x71374491, 0x23EF65CD,
			0xB5C0FBCF, 0xEC4D3B2F, 0xE9B5DBA5, 0x8189DBBC,
			0x3956C25B, 0xF348B538, 0x59F111F1, 0xB605D019, 
			0x923F82A4, 0xAF194F9B, 0xAB1C5ED5, 0xDA6D8118,
			0xD807AA98, 0xA3030242, 0x12835B01, 0x45706FBE, 
			0x243185BE, 0x4EE4B28C, 0x550C7DC3, 0xD5FFB4E2,
			0x72BE5D74, 0xF27B896F, 0x80DEB1FE, 0x3B1696B1, 
			0x9BDC06A7, 0x25C71235, 0xC19BF174, 0xCF692694,
			0xE49B69C1, 0x9EF14AD2, 0xEFBE4786, 0x384F25E3, 
			0x0FC19DC6, 0x8B8CD5B5, 0x240CA1CC, 0x77AC9C65,
			0x2DE92C6F, 0x592B0275, 0x4A7484AA, 0x6EA6E483, 
			0x5CB0A9DC, 0xBD41FBD4, 0x76F988DA, 0x831153B5,
			0x983E5152, 0xEE66DFAB, 0xA831C66D, 0x2DB43210, 
			0xB00327C8, 0x98FB213F, 0xBF597FC7, 0xBEEF0EE4,
			0xC6E00BF3, 0x3DA88FC2, 0xD5A79147, 0x930AA725, 
			0x06CA6351, 0xE003826F, 0x14292967, 0x0A0E6E70,
			0x27B70A85, 0x46D22FFC, 0x2E1B2138, 0x5C26C926, 
			0x4D2C6DFC, 0x5AC42AED, 0x53380D13, 0x9D95B3DF,
			0x650A7354, 0x8BAF63DE, 0x766A0ABB, 0x3C77B2A8, 
			0x81C2C92E, 0x47EDAEE6, 0x92722C85, 0x1482353B,
			0xA2BFE8A1, 0x4CF10364, 0xA81A664B, 0xBC423001, 
			0xC24B8B70, 0xD0F89791, 0xC76C51A3, 0x0654BE30,
			0xD192E819, 0xD6EF5218, 0xD6990624, 0x5565A910, 
			0xF40E3585, 0x5771202A, 0x106AA070, 0x32BBD1B8,
			0x19A4C116, 0xB8D2D0C8, 0x1E376C08, 0x5141AB53, 
			0x2748774C, 0xDF8EEB99, 0x34B0BCB5, 0xE19B48A8,
			0x391C0CB3, 0xC5C95A63, 0x4ED8AA4A, 0xE3418ACB, 
			0x5B9CCA4F, 0x7763E373, 0x682E6FF3, 0xD6B2B8A3,
			0x748F82EE, 0x5DEFB2FC, 0x78A5636F, 0x43172F60, 
			0x84C87814, 0xA1F0AB72, 0x8CC70208, 0x1A6439EC,
			0x90BEFFFA, 0x23631E28, 0xA4506CEB, 0xDE82BDE9, 
			0xBEF9A3F7, 0xB2C67915, 0xC67178F2, 0xE372532B,
			0xCA273ECE, 0xEA26619C, 0xD186B8C7, 0x21C0C207, 
			0xEADA7DD6, 0xCDE0EB1E, 0xF57D4F7F, 0xEE6ED178,
			0x06F067AA, 0x72176FBA, 0x0A637DC5, 0xA2C898A6, 
			0x113F9804, 0xBEF90DAE, 0x1B710B35, 0x131C471B,
			0x28DB77F5, 0x23047D84, 0x32CAAB7B, 0x40C72493, 
			0x3C9EBE0A, 0x15C9BEBC, 0x431D67C4, 0x9C100D4C,
			0x4CC5D4BE, 0xCB3E42B6, 0x597F299C, 0xFC657E2A, 
			0x5FCB6FAB, 0x3AD6FAEC, 0x6C44198C, 0x4A475817
	};
	for (uint32 i = 0,j = 0; i < 80; ++i,j+=2)
	{
		K[i] = (static_cast<uint64>(k[j]) << 32) + static_cast<uint64>(k[j+1]);
	}
}

void sha512Impl::_PackMBlock(MessageBlock& m, const string& s, uint32 index)
{
	for (uint32 i = 0; i < 128; i+=8)
	{
		m._[i/8] = 
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 0))) << 56) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 1))) << 48) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 2))) << 40) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 3))) << 32) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 4))) << 24) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 5))) << 16) +
			(static_cast<uint64>(static_cast<uint8>(s.at(index + i + 6))) << 8) +
			static_cast<uint64>(static_cast<uint8>(s.at(index + i + 7)));
	}
}

void sha512Impl::GetMessageDigest(digest512& hash, string& message)
{
	// Step 1: Pad the message
	uint64 len = static_cast<uint64>(message.length());
	len <<= 3; // multiply by 8 to get number of bits
	// Pad to 896 % 1024
	uint32 remainder = static_cast<uint32>(len % 1024); // yes, truncate, for now
	uint32 distance = 0;
	if (remainder >= 896)
	{
		// The "distance" to pad is the remaining bits to make 1024 plus
		// another 896 bits (save 1) of 0's to pad. We also subtract 7 
		// because we are dealing with whole bytes, not a bit stream. We
		// know that the message will require a minimum of 1 byte of padding
		// and since we pad with a "1" bit, we also will pad with at least
		// 7 "0"'s.
		distance = 1024 - remainder + 896 - 1 - 7;
	}
	else
	{
		distance = 896 - remainder - 1 - 7;
	}
	// Pad the "1" and 7 "0"'s
	uint8 c0 = 1 << 7;
	message += c0;

	// Pad the distance == the remaining 0's
	c0 = 0;
	uint32 distInBytes = distance >> 3;
	for (uint32 i = 0; i < distInBytes; ++i)
		message += c0;

	// Pad the length in 128 bits
	c0 = 0;
	for (uint32 i = 0; i < 8; ++i) // 64-bits of 0's
		message += c0;
	uint64 l1 = len;
	for (int i = 7; i >= 0; --i) // 64-bits of the actual length
	{
		l1 >>= (i*8);
		c0 = static_cast<char>( l1 & 0xFF );
		message += c0;
		l1 = len;
	}
	//
	// assert(the length of the message should now be a multiple of 1024 bits);
	//
	len = message.length();
	assert( (len%128) == 0 ); // 128 bytes is 1024 bits

	// Step 2: Parse the message
	// Instead of allocating memory for MessageBlock's we will parse this in-place

	// Step 3: Set the initial hash value
	hash._[0] = (static_cast<uint64>(0x6A09E667) << 32) + static_cast<uint64>(0xF3BCC908);
	hash._[1] = (static_cast<uint64>(0xBB67AE85) << 32) + static_cast<uint64>(0x84CAA73B);
	hash._[2] = (static_cast<uint64>(0x3C6EF372) << 32) + static_cast<uint64>(0xFE94F82B);
	hash._[3] = (static_cast<uint64>(0xA54FF53A) << 32) + static_cast<uint64>(0x5F1D36F1);
	hash._[4] = (static_cast<uint64>(0x510E527F) << 32) + static_cast<uint64>(0xADE682D1);
	hash._[5] = (static_cast<uint64>(0x9B05688C) << 32) + static_cast<uint64>(0x2B3E6C1F);
	hash._[6] = (static_cast<uint64>(0x1F83D9AB) << 32) + static_cast<uint64>(0xFB41BD6B);
	hash._[7] = (static_cast<uint64>(0x5BE0CD19) << 32) + static_cast<uint64>(0x137E2179);

	// Step 4: Get the hash
	len >>= 7; // divide by 128 to get # of MessageBlocks (128 == 1024 bits)
	MessageBlock m;
	MessageSchedule W;
	uint64 a,b,c,d,e,f,g,h; // "working variables"
	uint64 t1,t2; // "temporaries"
	for (uint32 i = 0; i < len; ++i)
	{
		// Get the ith MessageBlock
		_PackMBlock(m,message,i<<7);
		// Prepare the message schedule for the ith block
		for (uint32 t = 0; t < 16; ++t)
		{
			W._[t] = m._[t];
		}
		for (uint32 t = 16; t < 80; ++t)
		{
			W._[t] = _LowercaseSigma_1(W._[t-2]) + 
				W._[t-7] +
				_LowercaseSigma_0(W._[t-15]) +
				W._[t-16];
		}
		// Initialize the working variables
		a = hash._[0];
		b = hash._[1];
		c = hash._[2];
		d = hash._[3];
		e = hash._[4];
		f = hash._[5];
		g = hash._[6];
		h = hash._[7];
		// Process message schedule
		for (uint32 t = 0; t < 80; ++t)
		{
			t1 = h + _UppercaseSigma_1(e) + _Ch(e,f,g) + K[t] + W._[t];
			t2 = _UppercaseSigma_0(a) + _Maj(a,b,c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
		// Compute the i+1th intermediate hash value
		hash._[0] += a;
		hash._[1] += b;
		hash._[2] += c;
		hash._[3] += d;
		hash._[4] += e;
		hash._[5] += f;
		hash._[6] += g;
		hash._[7] += h;
	}
}
