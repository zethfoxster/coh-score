#include "statgroupstruct.h"
#include "container/dbcontainerpack.h"
#include "teamCommon.h"

LineDesc levelingpact_line_desc[] =
{
	{{ PACKTYPE_INT,	SIZE_INT32,	"Count",				OFFSET(LevelingPact, count) },			"Number of members in the pact" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Experience",			OFFSET(LevelingPact, experience) },		"Total experience gained by the pact" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence1",			OFFSET(LevelingPact, influence[0]) },	"Influence waiting for member 1" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence2",			OFFSET(LevelingPact, influence[1]) },	"Influence waiting for member 2" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence3",			OFFSET(LevelingPact, influence[2]) },	"Influence waiting for member 3" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence4",			OFFSET(LevelingPact, influence[3]) },	"Influence waiting for member 4" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence5",			OFFSET(LevelingPact, influence[4]) },	"Influence waiting for member 5" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence6",			OFFSET(LevelingPact, influence[5]) },	"Influence waiting for member 6" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence7",			OFFSET(LevelingPact, influence[6]) },	"Influence waiting for member 7" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"Influence8",			OFFSET(LevelingPact, influence[7]) },	"Influence waiting for member 8" },
	{{ PACKTYPE_INT,	SIZE_INT32,	"timeLogged",			OFFSET(LevelingPact, timeLogged) },		"Total time put into the pact by both members." },
	{{ PACKTYPE_INT,	SIZE_INT32,	"version",				OFFSET(LevelingPact, version) },		"The leveling pact version.  As the structure changes and needs updates, this number will increase." },
	{ 0 },
};
STATIC_ASSERT(ARRAY_SIZE(((LevelingPact*)NULL)->influence) == 8); // update fields above

StructDesc levelingpact_desc[] =
{
	sizeof(LevelingPact),
	{AT_NOT_ARRAY,{0}},
	levelingpact_line_desc,
	"The list of leveling pacts."
};

LineDesc league_teamleaders_line_desc[] =
{
	// Note there's no offset, because we find the base address of the
	// array elements and they're just numbers so there's no offsetting
	// within a larger structure.
	{{PACKTYPE_INT, SIZE_INT32, "TeamLeadId", 0},
	"This members teamLeader Id"},
	{0}
};

StructDesc league_teamleaders_desc[] =
{
	//	array of team leaders
	//	this is meant to store the team leaders while there is a member update occuring
	sizeof(U32),
	{AT_STRUCT_ARRAY,{INDIRECTION(League, teamLeaderIDs, 0)}},
	league_teamleaders_line_desc,

	"League team leader ids\n"
};

LineDesc league_teamlock_line_desc[] =
{
	// Note there's no offset, because we find the base address of the
	// array elements and they're just numbers so there's no offsetting
	// within a larger structure.
	{{PACKTYPE_INT, SIZE_INT32, "TeamLockStatus", 0},
	"This members team lock status"},
	{0}
};
StructDesc league_teamlock_desc[] =
{
	//	array of team locks
	//	this is meant to store the team locks while there is a member update occuring
	sizeof(U32),
	{AT_STRUCT_ARRAY,{INDIRECTION(League, lockStatus, 0)}},
	league_teamlock_line_desc,

	"League team lock ids\n"
};

LineDesc league_line_desc[] =
{
	{{ PACKTYPE_INT,			SIZE_INT32,						"LeaderId",		OFFSET2(League, members, TeamMembers, leader  ) },
				"DB ID - ContainerID of the leader of the team"},

	{{ PACKTYPE_INT,	SIZE_INT32,	"version",				OFFSET(League, revision) },		
				"The league revision. When members get added/quit too fast, the db can lag behind the statserver" },
	{ PACKTYPE_SUB, MAX_LEAGUE_MEMBERS,	"TeamLeaderIds",		(int)league_teamleaders_desc		},

	{ PACKTYPE_SUB, MAX_LEAGUE_MEMBERS,	"TeamLockStatus",		(int)league_teamlock_desc		},

	{ 0 },
};

StructDesc league_desc[] =
{
	sizeof(League),
	{AT_NOT_ARRAY,{0}},
	league_line_desc,

	"Describe a league that consists of teams."	
};