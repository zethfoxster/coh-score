
#ifndef FRIENDCOMMON_H
#define FRIENDCOMMON_H

#define MAX_FRIENDS	25

typedef struct StaticMapInfo StaticMapInfo;

typedef struct Friend
{
	int	dbid;
	int online;
	int map_id;
	int send;
	StaticMapInfo* static_info;

	char*	name;
	char*	mapname;

	const char*	origin;
	const char*	arch;
	char	description[64];

}Friend;

/* Not used?
typedef enum ClassName
{
	CLASS_BLASTER = 1,
	CLASS_CONTROLLER,
	CLASS_DEFENDER,
	CLASS_SCRAPPER,
	CLASS_TANKER,
}ClassName;

typedef enum OriginName
{
	ORIGIN_SCIENCE = 1,
	ORIGIN_MUTATION,
	ORIGIN_MAGIC,
	ORIGIN_TECHNOLOGY,
	ORIGIN_NATURAL,
}OriginName;
*/

void friendDestroy(Friend *f);

#endif