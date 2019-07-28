/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef BIGNUM_H
#define BIGNUM_H

typedef struct BigNum
{
    U32 *data;
    int msd;
    int data_len;
} BigNum;

void BigNum_Destroy( BigNum **r );

int BigNum_Test();
bool BigNum_RsaEncrypt(BigNum **res, BigNum *m, BigNum *e, BigNum *n);
bool BigNum_RsaDecrypt(BigNum **res, BigNum *c, BigNum *d, BigNum *n);

void BigNum_FromBytes(BigNum **r, U8 const *bytes, int n_bytes );
void BigNum_ToBytes(U8 **res, int *res_len, BigNum const *a);
void BigNum_FromBigEndBytes(BigNum **res,U8 const *bytes, U8 bytes_len);
void BigNum_ToBigEndBytes(U8 **res, int *res_len, BigNum const *a);
U32 BigNum_NumBytes(BigNum const *a);


#endif //BIGNUM_H
