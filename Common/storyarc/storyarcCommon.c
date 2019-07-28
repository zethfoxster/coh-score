#include "storyarc/storyarcCommon.h"
#include "structDefines.h"
#include "superassert.h"
#include "malloc.h"

// approximate room placement
StaticDefineInt ParseMissionPlaceEnum[] =
{
	DEFINE_INT
	{"(Undefined)",			MISSION_NONE},
	{"Front",				MISSION_FRONT},
	{"Back",				MISSION_BACK},
	{"Middle",				MISSION_MIDDLE},
	{"Objective",			MISSION_OBJECTIVE},
	{"Any",					MISSION_ANY},
	{"ScriptControlled",	MISSION_SCRIPTCONTROLLED},
	{"ReplaceGeneric",		MISSION_REPLACEGENERIC},
	{"Lobby",			MISSION_LOBBY},
	DEFINE_END
};


StoryClue* StoryClueCreate()
{
	return calloc(1, sizeof(StoryClue));
}

void StoryClueDestroy(StoryClue* clue)
{
	if(!clue)
		return;

	if(clue->name)
		free(clue->name);
	if(clue->displayname)
		free(clue->displayname);
	if(clue->detailtext)
		free(clue->detailtext);
	if(clue->iconfile)
		free(clue->iconfile);
	
	free(clue);
}

SouvenirClue* scCreate()
{
	return calloc(1, sizeof(SouvenirClue));
}