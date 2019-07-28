#pragma once

#include "MissionSearch.h"						// MissionSearchStatus
#include "storyarc/playerCreatedStoryarc.h"		//max search types
#include "mathutil.h"							//for MAX in MISSIONSEARCH_NUMVOTES_BUCKETS
typedef struct MissionServerArc MissionServerArc;


#define MISSIONSEARCH_NUMVOTES_BUCKETS (MAX(4, MISSIONHONORS_TYPES))
#define MSSEARCH_MAX_SORTBUCKETS (MISSIONRATING_RATINGTYPES*MISSIONSEARCH_NUMVOTES_BUCKETS)
//TODO: eventually we should switch to calculating this automatically.
										/*MAX(MAX(\
											MAX(kMorality_Count,AUTHREGION_COUNT),\
											MAX(MISSIONRATING_RATINGTYPES, MISSIONHONORS_TYPES)),\
										MAX(MISSIONBAN_TYPES, MISSIONSEARCH_MAPLENGTH_COUNT))) */

// this def is here so missionserver.c can easily register it with the persist layer
typedef struct MissionServerSearchNode
{
	struct MissionServerSearchNode *backward;
	struct MissionServerSearchNode *forward;
	S64 size;
	int cacheRefreshId;
	__time64_t timestamp;
} MissionServerSearchNode;

AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerSearch
{
	char *str;				AST(PRIMARY_KEY STRUCTPARAM)
	U32 *arcids;
	U32 hits;
	U32 nextid;				// next id to check
	__time64_t timestamp;			// has the search been updated
} MissionServerSearch;

AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerQuotedSearch
{
	MissionServerSearchNode node;					NO_AST
	char *str;				AST(PRIMARY_KEY STRUCTPARAM)
	U32 *arcids;
	U32 hits;
	U32 nextid;				// next id to check
} MissionServerQuotedSearch;

/*typedef MissionServerSearch MissionServerCSearch;*/
//same as above, but with room for search parameters
AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerCSearch
{
	MissionServerSearchNode node;					NO_AST
	char *str;				AST(PRIMARY_KEY STRUCTPARAM)
	U32 *arcids;
	U32 *arcidsBySort[MSSEARCH_MAX_SORTBUCKETS];	NO_AST
	U32 hits;
	U32 nextid;				// next id to check
	MissionSearchParams params;
} MissionServerCSearch;

//The most number of the same letter we want to track in a single word
//10 is well above what we should care about.  4 repeats is uncommon for most letters.
//however, the vowels do end up with a lot of repitions, and this is fairly cheap to track.
//	pneumonoultramicroscopicsilicovolcanoconiosis (9 i's) is the longest word, and I don't
//care much beyond that.  
//Look out for arcs about the Massachusetts lake "Chargoggagoggmanchauggagoggchaubunagungamaugg" (15 g's).
//http://en.wikipedia.org/wiki/English_words_with_uncommon_properties#Many_repeated_letters
#define MAX_TRACKED_LETTER_OCCURENCES 10

//The largest words we care about.
//20 is the biggest it should be, as the numbers almost totally drop off by then.  
//We can probably drop this to around 10 or 12
//source: (for English)
//http://www.thefreelibrary.com/On+word-length+and+dictionary+size.-a0189832222
#define MAX_TRACKED_WORD_LENGTH 20

//The largest letter combination we track as a single group
//example: we'll search all the single characters, but also:
//Most common two letter combinations - TH, HE, AN, IN, ER, RE, ES, ON, TI, AT
//Most common three letter combinations - THE, AND, HAT, ENT, ION, FOR, TIO, HAS, TIS
//Maybe these:
//Most common two letter combinations that show up as reversals of themselves 
//						- ER RE, ES SE, AN NA, ON NO, TI IT, EN NE, TO OT, ED DE
//http://www.prism.net/user/dcowley/analysub.html
#define MAX_TRACKED_CHARSIZE 4

//AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerTokenList
{
	char letter[MAX_TRACKED_CHARSIZE];				AST(PRIMARY_KEY STRUCTPARAM)
	U32 *tokenids[MAX_TRACKED_LETTER_OCCURENCES][MAX_TRACKED_WORD_LENGTH];
	U32 hits;
	__time64_t timestamp;			// has the search been updated
} MissionServerTokenList;

#define MISSIONSERVER_TOKENLIST_IDS(list, occurences, wordLength) list.tokenids[MIN(occurences,MAX_TRACKED_LETTER_OCCURENCES-1)][MIN(wordLength, MAX_TRACKED_WORD_LENGTH-1)]

AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerSubstringSearch
{
	MissionServerSearchNode node;	NO_AST
	char *substring;				AST(PRIMARY_KEY STRUCTPARAM)
	U32 *arcIds;					NO_AST
	int hits;
} MissionServerSubstringSearch;


typedef enum MissionServerSearchArcAction {
	MSSEARCHACT_DELETE = 0,
	MSSEARCHACT_ADD,
} MissionServerSearchArcAction;

// MissionServerSearch does not manage arc dirtiness, but owns the arc status and rating fields

void missionserver_IndexNewArc(MissionServerArc *arc); // on load and new publishes
void missionserver_UpdateArcRating(MissionServerArc *arc, MissionRating rating); // call to change arc->rating
void missionserver_UpdateArcHonors(MissionServerArc *arc, MissionHonors honors); // call to change arc->permanent
void missionserver_UpdateArcBan(MissionServerArc *arc, MissionBan ban); // call to change arc->ban
void missionserver_UpdateArcKeywords(MissionServerArc *arc, int keywordsBitfield); // call to change arc->ban
void missionserver_UnindexArc(MissionServerArc *arc); // on unpublish

// takes an earrayint handle (to be filled with results) and a serialized MissionSearchParams
// returns success
int missionserver_GetSearchResults(int ***parcids, const char *params_str, int *firstArc);
void missionserversearch_AddRemoveArcToCache(MissionServerArc *arc, MissionServerSearchArcAction action);
void missionserversearch_UpdateArcInCache(MissionServerArc *oldArc, MissionServerArc *newArc);
void missionserversearch_initTokenLists();
//void missionserver_updateSearchCache();
void missionserversearch_PrintStats();
void missionserversearch_InitializeSearchCache();

#include "AutoGen/MissionServerSearch_h_ast.h"
//how many seconds between requesting a search before the search is destroyed.
#define MISSIONSERVER_SEARCHCLEAR 30
//how many seconds between cleaning sweeps.  At this interval, we check to see
//  if searches are older than MISSIONSERVER_SEARCHCLEAR
#define MISSIONSERVER_SEARCHCLEAR_INTERVAL 5
#define MISSIONSEARCH_MAXCOMPOSED 500
#define MISSIONSEARCH_MAXSUBSTR 1000

#define MISSIONSEARCH_CACHE_COMPOSED 1

int MissionServerSearchTest_TestAllArcSearch();
int MissionServerSearchTest_TestAllArcSearchVote();
int MissionServerSearchTest_TestAllArcSearchPublish();
int MissionServerSearchTest_FullRun();
int MissionServerSearchTest_SearchFormatting();
