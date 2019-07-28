#include "container.h"
#include "dbmission.h"
#include "dbinit.h"
#include "comm_backend.h"
#include "dbmsg.h"
#include "error.h"
#include <stdio.h>
#include "timing.h"
#include "utils.h"
#include "log.h"

void teamCheckLeftMission(int map_id, int team_id, char *sWhy)
{
	int			i;
	GroupCon	*team;
	Packet		*pak_out;
	MapCon		*map;

	if (!map_id)
		return;

	for(i=0;i<teamups_list->num_alloced;i++)
	{
		team = (GroupCon*)teamups_list->containers[i];
		if (team->id != team_id && team->map_id == map_id) // some other team is linked to this map, don't kill it!
			return;
	}

	map = containerPtr(map_list,map_id);

	if (!map)
		return;

	// don't use this method for shutting down SG bases
	if( map->map_name && strstri( map->map_name, "Supergroupid=" ) )
	{
		GroupCon *pTeam = NULL;

		for(i=0;i<teamups_list->num_alloced;i++)
		{
			if (((GroupCon*)teamups_list->containers[i])->id == team_id)
			{
				pTeam = (GroupCon*)teamups_list->containers[i];
				break;
			}
		}

		LOG_OLD_ERR( "missiondoor dbserver refusing shutdown of SG base map %i from team %i (new map %i) (%s)", map_id, team_id, pTeam == NULL ? -1 : pTeam->map_id, sWhy);
		return;
	}

	// Arena maps are shut down using the mission timer function in ArenaMapProcess().
	if ( map->map_name && strstri( map->map_name, "Arena_" ) )
	{
		LOG_OLD_ERR( "missiondoor dbserver refusing shutdown of arena map %i from team %i (%s)", map_id, team_id, sWhy);
		return;
	}

	//	Endgame raids are handled differently from standard missions since they are owned by multiple teams
	if (map->mission_info && map->mission_info[0] == 'E')
	{
		LOG(LOG_DEBUG_LOG, LOG_LEVEL_IMPORTANT, LOG_DEFAULT, 
			"missiondoor dbserver refusing shutdown of endgame raid map %i. from team %i/%i (%s)", 
			map_id, team_id, teamups_list->num_alloced, sWhy);
		return;
	}

	if (!map->link) {
		map->shutdown = 1;
		LOG_OLD_ERR( "missiondoor dbserver delayed requesting shutdown of map %i (no teams linked to it) (%s)", map_id, sWhy);
		return;
	}
	pak_out = pktCreateEx(map->link, DBSERVER_TEAM_LEFT_MISSION);
	pktSend(&pak_out,map->link);
	LOG_OLD( "missiondoor dbserver requesting shutdown of map %i (no teams linked to it) (%s)", map_id, sWhy);
}

void dbClientReadyForPlayers(Packet *pak,NetLink *link)
{
	int			map_id;
	MapCon		*container=0;

	map_id		= pktGetBitsPack(pak,1);
	container	= containerPtr(map_list,map_id);
	if (!container)
		return;

	container->starting	= 0;
	container->lastRecvTime = timerCpuSeconds();
	container->seconds_since_update = 0;
	if (container->shutdown)
	{
		Packet *pak_out = pktCreateEx(container->link,DBSERVER_TEAM_LEFT_MISSION);
		pktSend(&pak_out,container->link);
		LOG_OLD("dbserver finishing delayed requesting shutdown of map %i (no teams linked to it)", container->id);
	}
}

