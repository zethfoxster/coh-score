#include "chatter.h"
#include "clientcomm.h"
#include "stdio.h"
#include "dooranimclient.h"
#include "PipeClient.h"
#include "testClientInclude.h"

TCStats stats;

int evilchat = 0;

int shakespeare = 1;

int randInt2(int max);

int wpm=60; // Words per minute typed
#define CPS (wpm*5/60)
char sendstyle[20] = "s"; // not "far"
char evilsendstyle[20] = "sgyaf";
char notevilsendstyle[20] = "sg";


char *wordlist[] = {
#include "wordlist.h"
};

char *sentencelist[] = {
#include "HenryVII.txt"
};

/*
float bigramsdata[27][26] = {
#include "bigrams.h"
};
char randomLetterFromBigrams(char prev) { // Expects 'a'-'z' or ' ' for pure random
	int i;
	float r = rand()/(float)RAND_MAX;
	int precedingLetter = prev-'a';
	if (prev==' ') precedingLetter=26;
	for (i=0; i<26; i++) {
		if (r < bigramsdata[precedingLetter][i]) {
			return i + 'a';
		}
		r -= bigramsdata[precedingLetter][i];
	}
	return ' ';
}
const char *randomWordFromBigrams() {
	static char word[32];
	int i;
	word[0]=randomLetterFromBigrams(' ');
	for (i=1; i<randInt2(20) && strlen(word) < 20; i++) {
		word[i] = randomLetterFromBigrams(word[i-1]);
	}
	word[i]=0;
	return word;
}
*/

const char *randomWord() {
	return wordlist[randInt2(999)];
}

const char *randomChatString() {
	static char str[1024];
	int i;

	if (shakespeare) {
		return sentencelist[randInt2(ARRAY_SIZE(sentencelist))];
	}
	switch (randInt2(2)) {
	case 0: // Info message
		switch (randInt2(4)) {
		case 0:
			sprintf(str, "I've killed %d bad guys!\n", stats.defeatcount);
			break;
		case 1:
			sprintf(str, "I've died %d frickin' times!\n", stats.deathcount);
			break;
		case 2:
			sprintf(str, "I've got %d XP, beat that!\n", stats.xp);
			break;
		case 3:
			sprintf(str, "W00T!  %d Influence!\n", stats.influence);
			break;
		}
		break;
	case 1: // Random chat
		str[0]=0;
		for (i=0; (i<randInt2(100) || i<3) && strlen(str)<1000; i++) {
			const char *word = randomWord();
			if (word) {
				strcat(str, randomWord());
				strcat(str, " ");
			}
		}
		break;
	}
	return str;
}

void setEvilChat(int evil)
{
	evilchat = evil;
	if (evil) {
		strcpy(sendstyle, evilsendstyle);
		wpm = 360;
	} else {
		strcpy(sendstyle, notevilsendstyle);
		wpm = 60;
	}
}

void chatterLoop(void) {
	static int lastSend = 0;
	static char nextstring[1024]={0};
	static char lastString[1025]={0};
	int timeToType = strlen(lastString)*60/(wpm*5);
	int elapsedTime = (timeGetTime() - lastSend) / 1000;
	if (!lastSend) lastSend = timeGetTime();
	if (elapsedTime > timeToType) {
		// Send it and make a new string
		if (nextstring[0]) {
			commAddInput(nextstring);
		}
		lastSend = timeGetTime();
		strcpy(lastString, nextstring);
		sprintf(nextstring, "%c (randomchat) %s", sendstyle[randInt2(strlen(sendstyle))], randomChatString());
	}
}


#include "entity.h"
#include "netio.h"
#include "player.h"
#include "entVarUpdate.h"
#include "uiCursor.h"

void CodeChatNPC(int svr_idx)
{
	START_INPUT_PACKET(pak, CLIENTINP_TOUCH_NPC );
	pktSendBitsPack(pak, PKT_BITS_TO_REP_SVR_IDX, svr_idx);
	END_INPUT_PACKET
}

int testChatNPC(void)
{
	int		i;
	Entity	*e;
	float	dist;
	int		ret=0;

	for( i=1; i<=entities_max; i++)
	{
		if (!(entity_state[i] & ENTITY_IN_USE ) )
			continue;

		e = entities[i];

		dist = distance3Squared( ENTPOS(playerPtr()), ENTPOS(e) );
		if ( dist < 15*15 ) 
		{
			CodeChatNPC(e->svr_idx);
			ret++;
			if( e && (ENTTYPE(e) == ENTTYPE_DOOR) )
			{
				enterDoorClient( ENTPOS(e), 0 );
			}
		}
	}
	return ret;
}

/*void enterDoor( Vec3 pos )
{
	setWaitForDoorMode();	
	pktSendBitsPack( input_pak, 1, CLIENTINP_ENTER_DOOR );
	pktSendF32( input_pak, pos[0] );
	pktSendF32( input_pak, pos[1] );
	pktSendF32( input_pak, pos[2] );
}
*/

int g_sendChatToLauncher=1;

void sendChatToLauncher(const char *fmt, ...) {
	char str[20000] = {0};
	va_list ap;

	if (!g_sendChatToLauncher)
		return;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	PipeClientSendMessage(pc,"ChatText:%s",str);
}
