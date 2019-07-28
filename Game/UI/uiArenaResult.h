#ifndef UIARENARESULT_H
#define UIARENARESULT_H

#define ARENA_RESULT_RANK				"RankString"
#define ARENA_RESULT_RATING				"RatingString"
#define ARENA_RESULT_PTS				"PointsString"
#define ARENA_RESULT_TIME				"TimePerMatchString"
#define ARENA_RESULT_PLAYED				"PlayedString"
#define ARENA_RESULT_OPP_AVG_PTS		"OppAvgPtsString"
#define ARENA_RESULT_OPP_OPP_AVG_PTS	"OppOppAvgPtsString"
#define ARENA_RESULT_SIDE				"SideString"
#define ARENA_RESULT_KILLS				"KillsString"
#define ARENA_RESULT_SE_PTS				"SEPtsString"
#define ARENA_RESULT_PLAYERNAME			"NameString"
#define ARENA_RESULT_DEATHTIME			"DeathTime"

typedef struct ArenaRankingTable ArenaRankingTable;

int arenaResultWindow();
void arenaRebuildResultView(ArenaRankingTable * ranking);
#endif