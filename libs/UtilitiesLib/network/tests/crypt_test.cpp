#include "crypt.h"
#include "blowfish.h"
#include "UnitTest++.h"

#define NUM_LOOPS 32

static const char * test = "this is a test";
static const U8 test_md5[16] = {0x54, 0xb0, 0xc5, 0x8c, 0x7c, 0xe9, 0xf2, 0xa8, 0xb5, 0x51, 0x35, 0x11, 0x02, 0xee, 0x09, 0x38};
static const U8 test_adler32[4] = {0x26, 0x33, 0x05, 0x16};

static const U8 test_key[32] = {0x4f, 0xd8, 0x98, 0x7, 0xed, 0x2d, 0x40, 0xef, 0xfc, 0x88, 0xb1, 0xa4, 0x98, 0xd2, 0xea, 0xe1,
								0xed, 0xe7, 0xab, 0xaf, 0x6c, 0x77, 0x95, 0x20, 0x6b, 0xb1, 0xc9, 0xbb, 0x82, 0xf3, 0x49, 0x7d};

TEST(MD5) {
	U32 hash[4];

	for (unsigned i=0; i<NUM_LOOPS; i++) {
		cryptMD5Init();
		cryptMD5Update((U8*)test, (int)strlen(test));
		cryptMD5Final(hash);
		CHECK_ARRAY_EQUAL(((U32*)test_md5), hash, 4);
	}
}

TEST(Adler32) {
	for (unsigned i=0; i<NUM_LOOPS; i++) {
		cryptAdler32Init();
		cryptAdler32Update((U8*)test, (int)strlen(test));
		CHECK_EQUAL(*(U32*)test_adler32, cryptAdler32Final());
	}
}

TEST(CryptStore) {
	cryptInit();

	const int testSize = 16;
	U8 test[testSize+1] = "0123456789ABCDEF";

	U8 encA[testSize];
	cryptStore(encA, test, testSize);

	U8 decA[testSize];
	cryptRetrieve(decA, encA, testSize);
	CHECK_ARRAY_EQUAL(test, decA, testSize);

	U8 encB[testSize];
	cryptStore(encB, decA, testSize);
	CHECK_ARRAY_EQUAL(encA, encB, testSize);

	U8 decB[testSize];
	cryptRetrieve(decB, encB, testSize);
	CHECK_ARRAY_EQUAL(test, decB, testSize);
}

TEST(SharedSecret) {
	cryptInit();

	for (unsigned i=0; i<NUM_LOOPS; i++) {
		U32 private_key_buf[2][16];
		U32 public_key_buf[2][16];

		cryptMakeKeyPair(private_key_buf[0], public_key_buf[0]);
		cryptMakeKeyPair(private_key_buf[1], public_key_buf[1]);

		U32 shared_secret_buf[2][16];

		CHECK(cryptMakeSharedSecret(shared_secret_buf[0], private_key_buf[0], public_key_buf[1]));
		CHECK(cryptMakeSharedSecret(shared_secret_buf[1], private_key_buf[1], public_key_buf[0]));
		
		CHECK_ARRAY_EQUAL(shared_secret_buf[0], shared_secret_buf[1], 16);
	}
}

TEST(Blowfish) {
	cryptInit();

	for (unsigned i=0; i<NUM_LOOPS; i++) {
		size_t length = (strlen(test) + 8) & ~7;
		char * buf = (char*)calloc(length, 1);
		memcpy(buf, test, strlen(test));

		BLOWFISH_CTX ctx[2];
		cryptBlowfishInit(&ctx[0], (U8*)test_key, sizeof(test_key));
		cryptBlowfishInit(&ctx[1], (U8*)test_key, sizeof(test_key));

		cryptBlowfishEncrypt(&ctx[0], (U8*)buf, (int)length);
		CHECK(test[0] != buf[0]);

		cryptBlowfishDecrypt(&ctx[1], (U8*)buf, (int)length);
		CHECK_ARRAY_EQUAL(test, buf, (int)strlen(test));

		free(buf);
	}
}
