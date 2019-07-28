/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef PROFANITY_H__
#define PROFANITY_H__

void replace(char *pch, char *set, char ch);
int IsProfanity(const char *pch, int iLen);
char * IsAnyWordProfane(const char *pch);
char * IsAnyWordProfaneOrCopyright(const char *pch);
void LoadProfanity(void);
int ReplaceAnyWordProfane(char *pch);

typedef struct Packet Packet;
void receiveProfanity(Packet * pak);
void sendProfanity(Packet * pak);

#endif /* #ifndef PROFANITY_H__ */

/* End of File */

