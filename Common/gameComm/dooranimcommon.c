#include "dooranimcommon.h"
#include "entity.h"
#include "cmdcommon.h"
#include "seqstate.h"
#include "seq.h"
#include "assert.h"
#include "motion.h"
#include "ctri.h"
#include "group.h"

#ifdef SERVER
#include "svr_base.h"
#include "entaivars.h"
#endif


#define MAX_ANGLE_DIRECT_TO_TARGET 10.0	// if player is within this angle of heading directly to target, he will ignore the entry point
#define MAX_ANGLE_INSIDE_TARGET 45.0 // if player is past entry point and within this angle of straight in, he will ignore the entry point
#define ORIENT_TURN_RATE 0.15 // radians per tick
#define FINALORIENT_TURN_RATE 0.30 // radians per tick
#define RUN_TURN_RATE 0.4 // radians per tick
#define PLAYER_ACCEL_RATE 0.1 // feet per tick per tick

// do one tick of movement with current anim, returns 0 when done
int DoorAnimMove(Entity* player, DoorAnimState* anim)  
{
	switch (anim->state)
	{
	case DOORANIM_BEGIN: // calculate points we want to go through
		{
			Vec3 dir;

			// decide if running through entry point, animations always go to entry point
			if (!anim->animation && !nearSameVec3(anim->entrypos, anim->targetpos))
			{
				Vec3 v1, v2;
				F32 angle;
				subVec3(anim->targetpos, anim->entrypos, v1);
				subVec3(anim->entrypos, ENTPOS(player), v2);
				normalVec3(v1);
				normalVec3(v2);
				angle = DEG(fixAngle(acosf(dotVec3(v1,v2))));

				// if close to straight in, don't worry about entry point
				if (angle <= MAX_ANGLE_DIRECT_TO_TARGET && angle >= -MAX_ANGLE_DIRECT_TO_TARGET) 
				{
					copyVec3(anim->targetpos, anim->entrypos); // make entry same as target
				}
				// if past point and pretty close to door, don't worry about it
				if (angle < -180 + MAX_ANGLE_INSIDE_TARGET || angle > 180 - MAX_ANGLE_INSIDE_TARGET)
				{
					copyVec3(anim->targetpos, anim->entrypos); // make entry same as target
				}
			}

			
			// calc final point, along line from entry to target or player to target
			if (nearSameVec3(anim->entrypos, anim->targetpos))
				subVec3(anim->targetpos, ENTPOS(player), dir);		
			else
				subVec3(anim->targetpos, anim->entrypos, dir);
			dir[1] = 0.0;	// don't continue climbing and falling
			normalVec3(dir);
			scaleVec3(dir, 3.0, dir); // 3 feet beyond target
			addVec3(anim->targetpos, dir, anim->finalpos);

			//If you start out acceptably close to the entry point, go right into the runin/animation
			//if( 0 && nearSameVec3Tol(anim->entrypos, player->mat[3], MAX( 0.1, anim->entryPointRadius) ) ) 
			//{
			//	if (anim->animation)
			//		anim->state = DOORANIM_ORIENTTOFINAL;
			//	else
			//		anim->state = DOORANIM_TOFINAL;
			//	break;
			//}
			//else
				anim->state = DOORANIM_ORIENT;
		}
		// FALL THROUGH

	case DOORANIM_ORIENT: //Orienting to the entry point -- 
		{
			Vec3 dir, pyr;
			F32 targetyaw, currentyaw, yawdist, turn, dontcare;

			// orient player to entry point
			subVec3(anim->entrypos, ENTPOS(player), dir);
			getVec3YP(dir, &targetyaw, &dontcare);
			getMat3YPR(ENTMAT(player), pyr);
			currentyaw = pyr[1];

			// if not already there
#ifdef SERVER
			if (subAngle(targetyaw, currentyaw) > 0.001f && (!player->client || !player->client->controls.nocoll))
#else
			if (subAngle(targetyaw, currentyaw) > 0.001f)
#endif
			{
				// decide how much to turn this tick
				Mat3 newmat;
				yawdist = subAngle(targetyaw, currentyaw);
				if (yawdist >= 0.0)
					turn = MIN(ORIENT_TURN_RATE * TIMESTEP, yawdist);
				else
					turn = MAX(-ORIENT_TURN_RATE * TIMESTEP, yawdist);
				pyr[1] = addAngle(pyr[1], turn);

				// set player up
				createMat3YPR(newmat, pyr);
				entSetMat3(player, newmat);
#ifdef SERVER
				copyVec3(pyr, ENTAI(player)->pyr);
#endif
				break;
			}
			anim->state = DOORANIM_TOENTRY;
		}
		// FALL THROUGH

	case DOORANIM_TOENTRY:
		{
			Vec3 dir, vel;
			F32 dist;

			// check if past threshold
			if (!anim->pastthreshold && !anim->animation)
			{
				Vec3 finaldir, curdir;
				subVec3(anim->targetpos, anim->finalpos, finaldir);
				subVec3(anim->targetpos, ENTPOS(player), curdir);
				if (dotVec3(finaldir, curdir) >= 0.0)
					anim->pastthreshold = 1;
			}

			// if not already there
			if (!nearSameVec3(anim->entrypos, ENTPOS(player)))
			{
				Vec3 newpos;
				// ramp speed if needed
				if (anim->speed < BASE_PLAYER_FORWARDS_SPEED * TIMESTEP)
				{
					anim->speed += PLAYER_ACCEL_RATE * TIMESTEP;
					anim->speed = MIN(anim->speed, BASE_PLAYER_FORWARDS_SPEED * TIMESTEP);
				}

				// run player to target
				subVec3(anim->entrypos, ENTPOS(player), dir);
				dist = lengthVec3(dir);
				normalVec3(dir);
				scaleVec3(dir, MIN(anim->speed, dist), vel);
				addVec3(ENTPOS(player), vel, newpos);
				entUpdatePosInterpolated(player, newpos);
				SETB(player->seq->state, STATE_FORWARD);
				break;
			}

			// animations take a different branch here
			if (anim->animation)
			{
				anim->state = DOORANIM_ORIENTTOFINAL;
				break;
			}
			anim->state = DOORANIM_TOFINAL;
		}
		// FALL THROUGH

	case DOORANIM_TOFINAL:
		{
			Vec3 dir, vel, pyr;
			F32 dist, targetyaw, currentyaw, dontcare, yawdist, turn;

			// check if past threshold
			if (!anim->pastthreshold)
			{
				Vec3 finaldir, curdir;
				subVec3(anim->targetpos, anim->finalpos, finaldir);
				subVec3(anim->targetpos, ENTPOS(player), curdir);
				if (dotVec3(finaldir, curdir) >= 0.0)
					anim->pastthreshold = 1;
			}

			// if not already there
			if (!nearSameVec3(anim->finalpos, ENTPOS(player)))
			{
				Vec3 newpos;

				// change yaw if needed
				subVec3(anim->finalpos, ENTPOS(player), dir);
				getVec3YP(dir, &targetyaw, &dontcare);
				getMat3YPR(ENTMAT(player), pyr);
				currentyaw = pyr[1];
				if (subAngle(targetyaw, currentyaw) > 0.001f)
				{
					Mat3 newmat;
					// decide how much to turn this tick
					yawdist = subAngle(targetyaw, currentyaw);
					if (yawdist >= 0.0)
						turn = MIN(RUN_TURN_RATE * TIMESTEP, yawdist);
					else
						turn = MAX(-RUN_TURN_RATE * TIMESTEP, yawdist);
					pyr[1] = addAngle(pyr[1], turn);

					// set player up
					createMat3YPR(newmat, pyr);
					entSetMat3(player, newmat);
#ifdef SERVER
					copyVec3(pyr, ENTAI(player)->pyr);
#endif
				}

				// ramp speed if needed
				if (anim->speed < BASE_PLAYER_FORWARDS_SPEED)
				{
					anim->speed += PLAYER_ACCEL_RATE * TIMESTEP;
					anim->speed = MIN(anim->speed, BASE_PLAYER_FORWARDS_SPEED);
				}

				// run player to target
				dist = lengthVec3(dir);
				normalVec3(dir);
				scaleVec3(dir, MIN(anim->speed, dist), vel);
				addVec3(ENTPOS(player), vel, newpos);
				entUpdatePosInterpolated(player, newpos);
				SETB(player->seq->state, STATE_FORWARD);
				break;
			}
			anim->state = DOORANIM_DONE;
		}
		break;

	case DOORANIM_ORIENTTOFINAL:
		{
			Vec3 dir, pyr;
			F32 targetyaw, currentyaw, yawdist, turn, dontcare;

			// orient player to final point
			subVec3(anim->finalpos, ENTPOS(player), dir);
			getVec3YP(dir, &targetyaw, &dontcare);
			getMat3YPR(ENTMAT(player), pyr);
			currentyaw = pyr[1];

			// if not already there
			if (subAngle(targetyaw, currentyaw) > 0.001f)
			{
				Mat3 newmat;
				// decide how much to turn this tick
				yawdist = subAngle(targetyaw, currentyaw);
				if (yawdist >= 0.0)
					turn = MIN(FINALORIENT_TURN_RATE * TIMESTEP, yawdist);
				else
					turn = MAX(-FINALORIENT_TURN_RATE * TIMESTEP, yawdist);
				pyr[1] = addAngle(pyr[1], turn);

				// set player up
				createMat3YPR(newmat, pyr);
				entSetMat3(player, newmat);
#ifdef SERVER
				copyVec3(pyr, ENTAI(player)->pyr);
#endif
				break;
			}

			// entering animation now

			anim->state = DOORANIM_ANIMATION;
		}
		//break;
		// can't break through here.. need to wait until sequencer sets up anim

	case DOORANIM_ANIMATION:
		// set the bits you need,
		{
			SETB(player->seq->state, STATE_RUNIN);
			seqSetStateFromString(player->seq->state, anim->animation);
			anim->state = DOORANIM_ANIMATIONRUNNNING;
		}
		break;

	case DOORANIM_ANIMATIONRUNNNING:
		// just wait for animation to complete, needs own state becuase sequencer needs one cycle to get to right move before 
		//we start checking
		{
			if (getMoveFlags(player->seq, SEQMOVE_RUNIN))
				break;  
			anim->state = DOORANIM_DONE;
			anim->pastthreshold = 1;
		}
		break;

	case DOORANIM_DONE:
	case DOORANIM_ACK:
		return 0; // movement is complete
	}
	return 1;
}

//Put this here even though it's for Mission objectives, not Door animation, because INTERACT_DISTANCE and SERVER_INTERACT_DISTACNE
//are defined in this file, and this should go with those defines
bool closeEnoughToInteract( Entity * player, Entity * target_ent, F32 interactDist )
{
	bool closeEnoughtToInteract = false;

	if( !player || !target_ent )
		return false;

	//Hand selection for mission interacts now does a proper line line distance if mission item capsule is valid
	if( target_ent->seq->type->selection == SEQ_SELECTION_CAPSULE || target_ent->seq->type->collisionType == SEQ_ENTCOLL_CAPSULE )
	{
		EntityCapsule plrCap;
		Mat4 plrCapMat;
		EntityCapsule tgtCap;
		Mat4 tgtCapMat;
		F32 dist;

		plrCap = player->motion->capsule; 
		positionCapsule( ENTMAT(player), &plrCap, plrCapMat );

		tgtCap = target_ent->motion->capsule; 
		positionCapsule( ENTMAT(target_ent), &tgtCap, tgtCapMat );

#ifdef TEST_CLIENT //This is because I didn't want to figure out why LineLineDist seems to be both already defined and an unresolved external in testclient (has something to do with forceInline? ) 
		//(Why in the world is something from ctris.c not in TestClient anyway?
		dist = 0;
#else
		{
			Vec3 plrIntersectPt, tgtIntersectPt;
			dist = coh_LineLineDist(plrCapMat[3], ENTMAT(player)[plrCap.dir],		plrCap.length, plrIntersectPt,
									tgtCapMat[3], ENTMAT(target_ent)[tgtCap.dir],	tgtCap.length, tgtIntersectPt );
		}
#endif

		if ( dist < SQR( interactDist + tgtCap.radius ) ) //Removed player radius, though it really should be here, because it's already baked into InteractDistance everywhere else
			closeEnoughtToInteract = true;

	}
	else //Otherwise just use the entity position like before
	{
		F32 extraDist = target_ent->seq->worldGroupPtr ? target_ent->seq->worldGroupPtr->radius : 0.0;

		if( distance3Squared( ENTPOS(player), ENTPOS(target_ent) ) < SQR( interactDist + extraDist ) )
			closeEnoughtToInteract = true;
	}

	return closeEnoughtToInteract;
}

