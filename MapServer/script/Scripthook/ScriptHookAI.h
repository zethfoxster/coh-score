/*\
 *
 *	scripthookAI.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to AI
 *
 */

#ifndef SCRIPTHOOKAI_H
#define SCRIPTHOOKAI_H

// AI -- behavior
void AddAIBehavior(TEAM team, STRING behaviorstring);
void RemoveAllBehaviors(TEAM team);
void RemoveAIBehaviorByType(TEAM team, STRING behaviorname);
void AddAIBehaviorFlag(TEAM team, STRING behaviorstring);
void AddAIBehaviorPermFlag(TEAM team, STRING behaviorstring);
NUMBER CurAIBehaviorMatchesName(ENTITY entity, STRING behaviorname);
NUMBER CurAIBehaviorOverridenByCombat(ENTITY entity);

// AI
void SetAIAttackTarget(TEAM attackTeam, TEAM targetTeam, PRIORITY priority);
void SetAIMoveTarget(TEAM team, LOCATION movetarget, PRIORITY priority, NUMBER doNotRun, NUMBER releaseOnFinish, FRACTION variation);
void SetAIMoveTargetTeam(TEAM movingTeam, TEAM targetTeam, PRIORITY priority, NUMBER doNotRun, NUMBER releaseOnFinish, FRACTION variation);
void SetAIStandoffDistance(TEAM team, FRACTION distance);
void SetAIPriorityList(TEAM team, STRING prioritylist);
void SetAIAlly(TEAM team, int allyID);
void SetAIGang(TEAM team, int gangID);
void SetAIGangPerm(TEAM team, int gangID);
void SetAITeamMonster(TEAM team);
void SetAITeamVillain(TEAM team);
void SetAITeamHero(TEAM team);
void SetAITeamVillain2(TEAM team);
void SetAITeamToMatchMissionPlayers(TEAM team); //Sets the ent to Villain if it's a Villain mission, Hero if it's a Hero Mission

void SetFollowers(ENTITY leader, TEAM newFollowerTeam );
void SetFollowersEx(ENTITY leader, TEAM newFollowerTeam, NUMBER addDefaultFollowBehavior);
void SetAsPets(ENTITY owner, TEAM petTeam);
void SetAsControlledPets(ENTITY owner, TEAM petTeam);
void JoinAITeam( ENTITY currAiTeamMember, TEAM joiners );

NUMBER GetAIReachedObjective(ENTITY entity);
void MarkAIReachedObjective(ENTITY entity);
ENTITY GetAIWasHitBy(ENTITY entity);
NUMBER GetAIAttackerCount(ENTITY entity);
NUMBER GetAIHasTarget(ENTITY entity);

void SetChatAudience( ENTITY meStr, ENTITY audienceStr );
void SetChatObjective( ENTITY meStr, ENTITY objectiveStr );
void AddRandomAngryPLAndFight( TEAM team );
void SetAITogglePowerRightNow(TEAM team, STRING power, NUMBER on );
void SetAINeverChooseThisPowerUnlessITellYouTo( TEAM team, STRING power );
void SetAIUsePowerRightNow(TEAM team, ENTITY target, STRING power );
void SetAIUsePowerNow(TEAM team, ENTITY target, STRING power );

ENTITY GetOwner( ENTITY petName );
ENTITY GetTrueOwner( ENTITY petName );
void MinionSays( TEAM team, STRING says );

void FollowerFollows( ENTITY followerName, FRACTION controlRadius, NUMBER noRecapture, NUMBER seeStealth );
void FollowerFollowsLeader( ENTITY followerName, FRACTION controlRadius );

#endif