
#ifndef UIMISSIONCOMMENT_H
#define UIMISSIONCOMMENT_H

void missioncomment_ClearAdmin(int arcid); // arcid 0 == clear all
void missioncomment_Add(int owned, int arcid, int type, char * name, char *comment);

void loadMissionComments();

int missioncomment_Exists( int arcid );
void missioncomment_Open( int arcid );
int missionCommentWindow();

#endif