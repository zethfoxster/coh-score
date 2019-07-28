// osrng.cpp - written and placed in the public domain by Wei Dai

// Thanks to Leonard Janke for the suggestion for AutoSeededRandomPool.

#include "pch.h"
#include "osrng.h"

#if !defined(NO_OS_DEPENDENCE)

#if (defined(_WIN32) && defined(USE_MS_CRYPTOAPI))
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <wincrypt.h>
#elif defined(__FreeBSD__) || defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#endif

NAMESPACE_BEGIN(CryptoPP)

#ifdef NONBLOCKING_RNG_AVAILABLE

NonblockingRng::NonblockingRng()
{
#ifdef _WIN32
	if(!CryptAcquireContext(&m_hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		throw OS_RNG_Err("NonblockingRng: CryptAcquireContext failed");
#else
	m_fd = open("/dev/urandom",O_RDONLY);
	if (m_fd == -1)
		throw OS_RNG_Err("NonblockingRng: could not open /dev/urandom");
#endif
}

NonblockingRng::~NonblockingRng()
{
#ifdef _WIN32
	CryptReleaseContext(m_hProvider, 0);
#else
	close(m_fd);
#endif
}

byte NonblockingRng::GenerateByte()
{
	byte b;
	GenerateBlock(&b, 1);
	return b;
}

void NonblockingRng::GenerateBlock(byte *output, unsigned int size)
{
#ifdef _WIN32
	if (!CryptGenRandom(m_hProvider, size, output))
		throw OS_RNG_Err("NonblockingRng: CryptGenRandom failed");
#else
	if (read(m_fd, output, size) != size)
		throw OS_RNG_Err("NonblockingRng: error reading from /dev/urandom");
#endif
}

#endif

// *************************************************************

#ifdef BLOCKING_RNG_AVAILABLE

BlockingRng::BlockingRng()
{
	m_fd = open("/dev/random",O_RDONLY);
	if (m_fd == -1)
		throw OS_RNG_Err("BlockingRng: could not open /dev/random");
}

BlockingRng::~BlockingRng()
{
	close(m_fd);
}

byte BlockingRng::GenerateByte()
{
	byte b;
	GenerateBlock(&b, 1);
	return b;
}

void BlockingRng::GenerateBlock(byte *output, unsigned int size)
{
	while (size)
	{
		// on some systems /dev/random will block until all bytes
		// are available, on others it will returns immediately
		int len = read(m_fd, output, STDMIN(size, (unsigned int)INT_MAX));
		if (len == -1)
			throw OS_RNG_Err("BlockingRng: error reading from /dev/random");
		size -= len;
		output += len;
		if (size)
			sleep(1);
	}
}

#endif

// *************************************************************

#ifdef AUTO_SEEDED_RANDOM_POOL_AVAILABLE

void AutoSeededRandomPool::Reseed(bool blocking, unsigned int seedSize)
{
#ifdef NONBLOCKING_RNG_AVAILABLE
	if (blocking)
#endif
	{
#ifdef BLOCKING_RNG_AVAILABLE
		BlockingRng rng;
		SecByteBlock seed(seedSize);
		rng.GenerateBlock(seed, seedSize);
		Put(seed, seedSize);
#endif
	}

#ifdef BLOCKING_RNG_AVAILABLE
	if (!blocking)
#endif
	{
#ifdef NONBLOCKING_RNG_AVAILABLE
		NonblockingRng rng;
		SecByteBlock seed(seedSize);
		rng.GenerateBlock(seed, seedSize);
		Put(seed, seedSize);
#endif
	}
}

#endif

NAMESPACE_END

#endif