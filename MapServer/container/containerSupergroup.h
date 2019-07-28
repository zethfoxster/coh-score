/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CONTAINERSUPERGROUP_H
#define CONTAINERSUPERGROUP_H

#include "stdtypes.h"

typedef struct Entity Entity;
typedef struct Supergroup Supergroup;
typedef struct SpecialDetail SpecialDetail;
typedef struct Detail Detail;
typedef int TaskHandle;

SpecialDetail *CreateSpecialDetail(void);
void DestroySpecialDetail(SpecialDetail *p);

char *superGroupTemplate(void);
char *superGroupSchema(void);
char *packageSuperGroup(Supergroup *sg);
void unpackSupergroup(Entity *e,char *mem,int send_to_client);
void sgroup_DbContainerUnpack(Supergroup *sg, char *mem);

//Raidserver uses these functions to help it update a Supergroup's SpecialDetails without the Raidserver having to really load all the Details. 
#if RAIDSERVER
void MakeDummyDetailMP( void );
void DestroyDummyDetailMP( void );
Detail * getDummyDetail( const char * pch );
#endif

const char* sgTaskFileName(intptr_t context);

#endif //CONTAINERSUPERGROUP_H
