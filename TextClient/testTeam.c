#include "testTeam.h"
#include "textClientInclude.h"
#include "utils.h"
#include "StashTable.h"
#include "wininclude.h"
#include "components/ArrayOld.h"
#include "entity.h"
#include "clientcomm.h"
#include "player.h"
#include "gamedata/randomName.h"
#include "gamedata/randomCharCreate.h"
#include "earray.h"
#include "uiNet.h"
#include "netio.h"
#include "mathutil.h"
#include "Supergroup.h"
#include "testUtil.h"
#include "cmdcommon.h"
#include "uiturnstile.h"

#define commAddInput(s) printf("commAddInput(\"%s\");\n", s); commAddInput(s);


int wantToInvite(StashTable offer_type, Entity* e, const char* invite_type, const char* invitee_name, bool useHashTable, bool useSgMembers)
{
	int i;
	char buf[1024];
	bool invite = false;

	// maybe just get rid of useSgMembers entirely?
	if(useSgMembers)
	{
		bool found = false;
		for(i = 0; i < eaSize(&e->sgStats);i++)
		{
			if(!stricmp(invitee_name, e->sgStats[i]->name))
			{
				found = true;
				break;
			}
		}
		if(!found)
			invite = true;
	}
	else if ((!useHashTable && rand()%10==0) || (useHashTable && !stashFindInt(offer_type, invitee_name, NULL))) {
		invite = true;

		if (useHashTable) {
			stashAddInt(offer_type, invitee_name, 1, false);
		}
	}

	if(invite)
	{
		sprintf(buf, "%s %s", invite_type, invitee_name);
		commAddInput(buf);
	}

	return invite;
}

static StashTable htHts = 0;

void clearInviteHashTable(char* invite_type)
{
	StashTable htSentOffer;
	if (stashFindPointer(htHts, invite_type, &htSentOffer)) {
		stashTableClear(htSentOffer);
	}
}

typedef struct InviteData {
	Entity* e;
	char* invite_type;
	StashTable htSentOffer;
}InviteData;

int stashCheckInvite(void* data, StashElement element)
{
	InviteData* invite_data = data;
	const char* inviting = stashElementGetStringKey(element);
	wantToInvite(invite_data->htSentOffer, invite_data->e, invite_data->invite_type,
		stashElementGetStringKey(element), !!invite_data->htSentOffer, false);
	return 1;
}