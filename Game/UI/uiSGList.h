/*\
 *
 *	uiSGList.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Shows a list of supergroups at the registration desk
 *
 */

#ifndef UISGLIST_H
#define UISGLIST_H

int supergroupListWindow(void);
void receiveSupergroupList(Packet *pak);

#endif // UISGLIST_H