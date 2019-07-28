

#include "entity.h"
#include "netio.h"
#include "entStrings.h"
#include "utils.h"
#include "MemoryPool.h"

char * breakHTMLtags( char * text )
{
	//	This function is used to break out HTML tags in description and comment
	//	allowBR is a bit of special condition hack, but I don't another good way around it.
	//	The text comes in as plaintext. To be able to handle <br>, we could either convert it back on the playerside or don't convert it on the serverside.
	int i;
	for( i = 0; i < (int)strlen(text); i++ )
	{
		if( text[i] == '<')
			text[i] = '[';
		if( text[i] == '>')
			text[i] = ']';
	}

	return text;
}

void sendEntStrings( Packet *pak, EntStrings * strings )
{
	pktSendString( pak, strings->motto				);
	pktSendString( pak, strings->playerDescription	);
}

void receiveEntStrings( Packet *pak, EntStrings * strings )
{
	strncpyt( strings->motto,				pktGetString( pak ), ARRAY_SIZE(strings->motto));
	strncpyt( strings->playerDescription,	pktGetString( pak ), ARRAY_SIZE(strings->playerDescription));
	breakHTMLtags(strings->playerDescription);
}

void initEntStrings( EntStrings *strings )
{
	strings->motto[0]				= '\0';
	strings->playerDescription[0]	= '\0';
}

MP_DEFINE(EntStrings);
EntStrings *createEntStrings(void)
{
	MP_CREATE(EntStrings,4);
	return MP_ALLOC(EntStrings);
}
void destroyEntStrings(EntStrings *entstrings)
{
	MP_FREE(EntStrings,entstrings);
}
