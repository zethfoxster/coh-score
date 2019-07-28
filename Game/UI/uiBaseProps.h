/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIBASEPROPS_H__
#define UIBASEPROPS_H__

typedef struct RoomDetail RoomDetail;
typedef struct Detail Detail;

int basepropsWindow();
void openPermissions(RoomDetail *detail);
int basepermissionsWindow();
char * getBaseDetailDescriptionString( const Detail * detail, const RoomDetail * room_detail,int width );

#endif /* #ifndef UIBASEPROPS_H__ */

/* End of File */

