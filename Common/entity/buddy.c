/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>
#include <float.h>

#include "mathutil.h"

#include "entity.h"
#include "character_base.h"
#include "character_level.h"
#include "teamCommon.h"
#include "Supergroup.h"


/**********************************************************************func*
* character_IsExemplar
*
*/
bool character_IsExemplar(Character *p, int pending_counts)
{
	Entity *e = NULL;

	assert(p!=NULL);

	e = p->entParent;

	if( !e || !e->teamup )
		return false;

	if( !pending_counts && p->buddy.eType == kBuddyType_Pending )
		return false;

	if( character_CalcExperienceLevel(p) > e->teamup->team_level )
		return true;

	return false;
}

/**********************************************************************func*
* character_IsSidekick
*
*/
bool character_IsSidekick(Character *p, int pending_counts)
{
	Entity *e = NULL;
	assert(p!=NULL);

	e = p->entParent;

	if( !e || !e->teamup )
		return false;

	if( !pending_counts && p->buddy.eType == kBuddyType_Pending )
		return false;

	if( character_CalcExperienceLevel(p) < e->teamup->team_level-1 )
		return true;

	return false;
}

/**********************************************************************func*
* character_IsMentor
*
*/
bool character_IsMentor(Character *p)
{
	Entity *e = NULL;


	assert(p!=NULL);

	e = p->entParent;

	if( !e || !e->teamup )
		return false;

	if( e->teamup->team_mentor == p->entParent->db_id && e->teamup->members.count > 1  )
		return true;

	return false;
}

/**********************************************************************func*
* character_IsMentor
*
*/
bool character_IsPending(Character *p)
{
	Entity *e = NULL;

	assert(p!=NULL);

	e = p->entParent;

	if( !e || !e->teamup )
		return false;

	if( p->buddy.eType == kBuddyType_Pending )
		return true;

	return false;
}


/**********************************************************************func*
* character_TeamLevel
*
*/
int character_TeamLevel(Character *p)
{
	Entity *e = NULL;

	assert(p!=NULL);

	e = p->entParent;

	if( !e || !e->teamup )
		return 0;

	return e->teamup->team_level;
}

/* End of File */
