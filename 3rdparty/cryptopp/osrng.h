#ifndef CRYPTOPP_OSRNG_H
#define CRYPTOPP_OSRNG_H

#include "config.h"

#if !defined(NO_OS_DEPENDENCE)

#include "randpool.h"

NAMESPACE_BEGIN(CryptoPP)

//! Exception class for Random Number Generator.
class OS_RNG_Err : public Exception
{
public:
	OS_RNG_Err(const std::string &s) : Exception(s) {}
};

#if ((defined(_WIN32) && defined(USE_MS_CRYPTOAPI)) || defined(__FreeBSD__) || defined(__linux__))

#define NONBLOCKING_RNG_AVAILABLE

//! encapsulate CryptoAPI's CryptGenRandom or /dev/urandom
class NonblockingRng : public RandomNumberGenerator
{
public:
	NonblockingRng();
	~NonblockingRng();
	byte GenerateByte();
	void GenerateBlock(byte *output, unsigned int size);

protected:
#if defined(_WIN64)
	unsigned __int64 m_hProvider;	// type HCRYPTPROV, avoid #include <wininclude.h>
#elif defined(_WIN32)
	unsigned long m_hProvider;
#else
	int m_fd;
#endif
};

#endif

#if (defined(__FreeBSD__) || defined(__linux__))

#define BLOCKING_RNG_AVAILABLE

//! encapsulate /dev/random
class BlockingRng : public RandomNumberGenerator
{
public:
	BlockingRng();
	~BlockingRng();
	byte GenerateByte();
	void GenerateBlock(byte *output, unsigned int size);

protected:
	int m_fd;
};

#endif

#if defined(NONBLOCKING_RNG_AVAILABLE) || defined(BLOCKING_RNG_AVAILABLE)

#define AUTO_SEEDED_RANDOM_POOL_AVAILABLE

//! Automaticly Seeded Randomness Pool
/*! This class seeds itself using an
operating system provided RNG. */
class AutoSeededRandomPool : public RandomPool
{
public:
	//! blocking will be ignored if the prefered RNG isn't available
	explicit AutoSeededRandomPool(bool blocking = false, unsigned int seedSize = 16)
		{Reseed(blocking, seedSize);}
	void Reseed(bool blocking = false, unsigned int seedSize = 16);
};

#endif

NAMESPACE_END

#endif	// #if !defined(NO_OS_DEPENDENCE)

#endif
