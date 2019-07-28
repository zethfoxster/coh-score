#include "container.h"
#include "statservercomm.h"
#include "dbinit.h"
#include "comm_backend.h"
#include "utils.h"

#define MAX_BROADCAST_LINKS 10000

// src_id represents the 'source' of the broadcast. use this if you are interested
// in being notified of changes to a specific record in a container.
static int getBroadcastList(int list_id,int src_id,int *member_ids,int member_count,NetLink **links)
{
	int			i,j,link_count=0;
	DbContainer	*container;
	NetLink		*link;
	DbList		*list;

	if (list_id == CONTAINER_MAPGROUPS)
	{
		MapCon	*map_con;

		list = map_list;
		for(i=0;i<member_count;i++)
		{
			map_con = (MapCon*)containerPtr(list,member_ids[i]);
			if (map_con && map_con->active && map_con->link && link_count < MAX_BROADCAST_LINKS-1)
				links[link_count++] = map_con->link;
		}
	}
	else
	{
		list = ent_list;
		for(i=0;i<member_count;i++)
		{
			container = containerPtr(list,member_ids[i]);
			if (!container || !container->active)
				continue;
			link = dbLinkFromLockId(container->lock_id);
			if (!link)
				continue;
			for(j=0;j<link_count;j++)
			{
				if (links[j] == link)
					break;
			}
			if (j >= link_count && j < MAX_BROADCAST_LINKS-1)
				links[link_count++] = link;
		} 

		if( list_id == CONTAINER_SUPERGROUPS )
		{
			MapCon *base_con = containerFindBaseBySgId(src_id);
			if(base_con && base_con->link)
			{
				for(i = 0; i < link_count; ++i)
				{
					if(links[i] == base_con->link)
						break;
				}
				if(i >= link_count)
					links[link_count++] = base_con->link;
			}
		}

		if(CONTAINER_IS_STATSERVER_GROUP(list_id))
		{
			if(statLink())
				links[link_count++] = statLink();
		}
	}
	return link_count;
}

void containerBroadcast(int list_id,int id,int *member_ids,int member_count,NetLink *link)
{
	int		i,link_count,seen_link=0;
	NetLink	*links[MAX_BROADCAST_LINKS];
	DbList	*list = dbListPtr(list_id);

	link_count = getBroadcastList(list_id,id,member_ids,member_count,links);
	assert(AINRANGE(link_count, links));
	for(i=0;i<link_count;i++)
	{
		if (link == links[i])
			seen_link = 1;
		containerSendList(list,&id,1,0,links[i],0);
	}
	if (!seen_link && link)
		containerSendList(list,&id,1,0,link,0);
}

void containerBroadcastAll(int list_id, int id)
{
	int i;
	DbList	*list = dbListPtr(list_id);

	for (i = 0; i < map_list->num_alloced; i++)
	{
		MapCon *map = (MapCon *) map_list->containers[i];
		if (map && map->active && map->link)
		{
			containerSendList(list, &id, 1, 0, map->link, 0);
		}
	}
}

void containerBroadcastMsg(int list_id,int msg_type,int senderID,int id,char *msg)
{
	int			i,j,link_count,member_id_count,*member_ids;
	Packet		*pak;
	NetLink		*links[MAX_BROADCAST_LINKS];
	GroupCon	*group_con;
	DbContainer *container;

	if (id == -1)
	{
		DbList		*list = dbListPtr(list_id);

		for(j=0;j<list->num_alloced;j++)
		{
			container = list->containers[j];
			if (container->active)
				containerBroadcastMsg(list_id,msg_type,senderID,container->id,msg);
		}
		return;
	}

	container = containerPtr(dbListPtr(list_id),id);
	if (!container)
		return;
	if (CONTAINER_IS_GROUP(container->type))
	{
		group_con = (GroupCon *)container;
		member_id_count = group_con->member_id_count;
		if (!member_id_count)
			return;
		member_ids = group_con->member_ids;
		link_count = getBroadcastList(list_id,senderID,member_ids,member_id_count,links);
	}
	else if ( container->type == CONTAINER_ENTS )
	{
		member_id_count = 1;
		member_ids = &container->id;
		link_count = getBroadcastList(list_id,senderID,member_ids,member_id_count,links);
	}
	else
	{
		link_count = getBroadcastList(list_id,senderID,&id,1,links);
	}
	assert(AINRANGE(link_count, links)); // Else we just corrupted the stack

	for(i=0;i<link_count;i++)
	{
		pak = pktCreateEx(links[i],DBSERVER_BROADCAST_MSG);
		pktSendBitsPack(pak,1,list_id);
		pktSendBitsPack(pak,1,msg_type);
		pktSendBitsPack(pak,1,senderID);
		pktSendBitsPack(pak,1,id);
		pktSendString(pak,msg);
		pktSendBitsPack(pak,1,member_id_count);
		for(j=0;j<member_id_count;j++)
			pktSendBitsPack(pak,1,member_ids[j]);
		pktSend(&pak,links[i]);
	}
}

void handleSendMsg(Packet *pak)
{
	char		*msg;
	int			list_id,count,i,id,msg_type,senderID;

	list_id		= pktGetBitsPack(pak,1);
	msg_type	= pktGetBitsPack(pak,1);
	senderID	= pktGetBitsPack(pak,1);
	count		= pktGetBitsPack(pak,1);
	msg			= pktGetString(pak);

	for(i=0;i<count;i++)
	{
		id = pktGetBitsPack(pak,1);
		containerBroadcastMsg(list_id, msg_type, senderID, id, msg);
	}
}
