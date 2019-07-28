/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef RSA_H
#define RSA_H

U8 *rsaEncrypt(U8 **res, int *res_len, char *msg, U8 *pubexp, int pubexp_len, U8 *mod, int mod_len);
U8 *rsaDecrypt(U8 **res, int *res_len, U8 *ciphertext,int ciphertext_len, U8 *privexp, int privexp_len, U8 *mod, int mod_len );
char *rsaEncryptToBase64(char *msg, U8 *mod, int mod_len);
int rsaTest(void);

#endif //RSA_H
