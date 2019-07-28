// ServerList.h: interface for the CServerList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERLIST_H__0CE7C769_E592_440D_A478_E7EF7D94A8A0__INCLUDED_)
#define AFX_SERVERLIST_H__0CE7C769_E592_440D_A478_E7EF7D94A8A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AuthType.h"

/**
 * A class to maintain the list of servers.
 *
 * The serverlist is (optionally) loaded from the database.  When servers connect,
 * the matching entry in the list is updated with a pointer to their connection.
 */
class CServerList  
{
public:
	CServerList();
	~CServerList();

	void Load();

	void SetServerStatus(ServerId id, ServerStatus status);
	void SetServerVIPStatus(ServerId id, int isVIP);

	ServerId SetServerSocketByAddress(in_addr address, CSocketServer *socket);
	bool SetServerSocketById(ServerId id, CSocketServer *socket, in_addr actualAddress, short int actualPort);
	void RemoveSocket(CSocketServer *socket);

	in_addr GetInternalAddress(ServerId id) const;

	bool IsServerUp(ServerId id) const;
	bool IsServerVIPonly(ServerId id) const;

	void RequestUserCounts() const;
	void SetServerUserCount( ServerId id, short userCount, short userLimit);
	void SetServerQueueSize( ServerId id, int queueLevel, int queueSize, int queueTime);

	void MakeServerListPacket(std::vector<char> & buffer, ServerId lastServer, int regions[MAX_REGIONS]) const;
	void MakeQueueSizePacket(std::vector<char> & buffer) const;

	void UpdateDB() const;

private:
	typedef std::map<ServerId, WorldServer> ServerListType;
	typedef std::map<ServerId, std::vector<std::pair<int, int> > > QueueSizesType;
	ServerListType m_serverList;
	QueueSizesType m_queueSizes;
	HANDLE m_timer;
	mutable CRWLock mylock;

	ServerId GetFreeServerId() const;
};

extern CServerList g_ServerList;

#endif // !defined(AFX_SERVERLIST_H__0CE7C769_E592_440D_A478_E7EF7D94A8A0__INCLUDED_)
