#ifndef _UTIL_H_
#define _UTIL_H_

#include "GlobalAuth.h"

#define RAND_MAX 0x7fff
#define PRIVATE_KEY 0x87546CA100000000

int LNStrNCpy(char *dst, const char *src, int len);
int Assemble(char* buf, int bufLen, const char* format, va_list ap);
int Assemble(char *buf, int bufLen, const char *format, ...);
//void EncPwd( char *password );
void EncPwdL2( char *password );
void EncPwdShalo( char *password );
void EncPwdDev( char *password );
void EncPwdSha512( char *password, unsigned int salt );
void Encrypt(unsigned char *buf, __int64 &key, int &length);
bool Decrypt(unsigned char *buf, __int64 &key, int length);
void WriteAction( char *action, char *account, in_addr ip, int ssn, char gender, int worldid, int stat);
void WriteLogD( int LOG_ID, char *account, in_addr ip, int stat, int age, int gender, int zipcode, int param1, int uid );
void StdAccount( char *account );
void DumpPacket( const unsigned char *packet, int packetlen );
int AnsiToUnicode(char* ansistr, int maxansilen, WCHAR* unistr, int maxunilen);
bool CheckAccount( char *account );

void BlowFishEncryptPacket(unsigned char *buf, __int64 &key, int &len);
bool BlowFishDecryptPacket(unsigned char *buf, __int64 &key, int len);


time_t ConvertSQLToTome( SQL_TIMESTAMP_STRUCT &sqlDate, struct tm *tm);

extern BOOL SendSocket( in_addr ina, const char *format, ...);
HFONT GetAFont(HDC hdc);

inline int GetIntFromPacket(const unsigned char *&packet)
{
	int result = ((int *)packet)[0];
    packet += 4;
    return result;
}

inline in_addr GetAddrFromPacket(const unsigned char *&packet)
{
	in_addr result;
	*((int *)&result) = ((int *)packet)[0];
    packet += 4;
    return result;
}

inline short GetShortFromPacket(const unsigned char *&packet)
{
	short result = ((short *)packet)[0];
    packet += 2;
    return result;
}

inline char GetCharFromPacket(const unsigned char *&packet)
{
    char result = packet[0];
    packet++;
    return result;
}

inline void GetStrFromPacket(const unsigned char *&packet, int dstLen, char *dst)
{
    packet += LNStrNCpy(dst, (char *)packet, dstLen);
}

#endif
