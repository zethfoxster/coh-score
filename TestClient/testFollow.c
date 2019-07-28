#include "testFollow.h"
#include "testClientInclude.h"
#include "entity.h"
#include "player.h"
#include "mover.h"
#include "mathutil.h"
#include "teamCommon.h"
#include "assert.h"

extern Entity *current_target;

Entity	*followme;
FollowMode follow_mode = FM_FREEDOM;
int display_follow_messages=1;

void checkFollow(void) {
	Entity *player = playerPtr();
	float dist;
	float freedomrange = 80.0;
	float madeitbackrange = 15.0;
	static int frame_count=0;

	frame_count++;

	if (!player || !player->pchar || !player->teamup || !(g_testMode & TEST_MOVE))
		return;
	if (!followme) {
		if (player->teamup) {
			followme = entFromDbId(player->teamup->members.leader);
		}
		if (!followme)
			return;
		if (followme==player || followme->db_id == player->db_id) {
			// Playing follow the leader with yourself is no fun.
			return;
		}
		if (display_follow_messages)
			printf("FOLLOWER: Following %s\n", followme->name);
	}
	if (followme->owner==-1 || followme->svr_idx==-1) {
		if (display_follow_messages)
			printf("FOLLOWER: Lost followed entity, roaming free\n");
		followme = NULL;
		current_target = NULL;
		return;
	}
	if (ENTTYPE(followme)!=ENTTYPE_PLAYER) {
		if (display_follow_messages)
			printf("FOLLOWER: Was following a non-player?!  Stopping that nonsense!\n");
		followme = NULL;
		return;
	}
	if (followme==player || followme->db_id == player->db_id) {
		// Playing follow the leader with yourself is no fun.
		return;
	}

	dist = distance3( ENTPOS(player), ENTPOS(followme) );
	if (follow_mode==FM_FREEDOM) {
		//printf("Distance : %1.3f\n", dist);
		if (dist > freedomrange) {
			if (display_follow_messages)
				printf("FOLLOWER: Too far away, reeling back in to my leader\n");
			follow_mode = FM_RETURNING;
			setDestEnt(followme);
			startMoving();
		}
	} else if (follow_mode==FM_RETURNING) {
		//if ((frame_count%3)==0) {
			setDestEnt(followme);
		//}
		if (dist < madeitbackrange) {
			if (display_follow_messages)
				printf("FOLLOWER: Made it home, now roaming on my own again\n");
			follow_mode = FM_FREEDOM;
			current_target = NULL; // Reset the target
			stopMoving();
		}
	}
}
