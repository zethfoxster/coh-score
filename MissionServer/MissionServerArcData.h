#pragma once

#define MISSIONSERVER_CLOSE_HOGGS	0	// slow, but won't keep file handles open
#define MISSIONSERVER_LOADALL_HOGGS	1	// more load time, but faster loading if close_hoggs is set
#define MISSIONSERVER_BACKUP_HOGGS	0	// slow
STATIC_ASSERT(MISSIONSERVER_CLOSE_HOGGS || !MISSIONSERVER_BACKUP_HOGGS);

typedef struct MissionServerArc MissionServerArc;

void missionserver_LoadAllArcData(MissionServerArc **arcs, int count);
void missionserver_FlushAllArcData(void);

int missionserver_FindArcData(MissionServerArc *arc); // checks existance only
U8* missionserver_GetArcData(MissionServerArc *arc);
void missionserver_RestoreArcData(MissionServerArc *arc); // like GetArcData, but won't assert on missing data and LOADALL

void missionserver_UpdateArcData(MissionServerArc *arc, U8 *data);
// takes ownership of data, which should be malloc'd
// arc->size and arc->zsize must be set to the values for the new data _before_ calling

void missionserver_ArchiveArcData(MissionServerArc *arc);
void missionserver_CloseArcDataArchive(void);

void missionserver_SetForceLoadDb(int force);
void missionserver_getLoadMismatches(MissionServerArc ***removed, MissionServerArc ***changed);
void missionserver_clearLoadMismatches();

////////////////////////////////////////////////////////////////////////////////
