#ifndef _BASEPARSE_H
#define _BASEPARSE_H

typedef struct Base Base;
typedef struct BaseRoom BaseRoom;
typedef struct ParseLink ParseLink;
typedef struct RoomDetail RoomDetail;
typedef struct ParseTable ParseTable;
#define TokenizerParseInfo ParseTable

// --------------------
// globals

extern TokenizerParseInfo parse_base[];
extern TokenizerParseInfo parse_roomdetail[];
extern TokenizerParseInfo parse_detailswap[];
extern ParseLink g_base_roomInfoLink;
extern ParseLink g_base_detailInfoLink;
extern ParseLink g_base_BasePlotLink;

int baseFromStr(char *str,Base *base);
int baseToStr(char **estr,Base *base);
int baseFromBin(char *bin,U32 num_bytes,Base *base);
int baseToBin(char **ebin,Base *base);

int baseToFile(char *fname,Base *base);
int baseFromFile(char *fname,Base *base);

void baseReset(Base *base);
void baseResetIds(Base *base);
void baseResetIdsForBaseRaid(Base *base, int initial_id);

void rebuildBaseFromStr( Base * base );

void destroyRoomDetail( RoomDetail * detail );
void destroyRoom( BaseRoom * room );

#endif
