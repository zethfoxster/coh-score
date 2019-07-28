#ifndef CREPORTCONDUIT_H
#define CREPORTCONDUIT_H

#include "stdafx.h"
#include <atlmisc.h>

class CReportConduit
{
public:
	virtual int send(LPCSTR filename) = 0;
	virtual ~CReportConduit(){};
};

class CFtpConduit : public CReportConduit
{
public:
	CFtpConduit(LPCSTR hostname, LPCSTR username, LPCSTR password);
	~CFtpConduit();
	int send(LPCSTR filename);

private:
	CString hostname;
	CString username;
	CString password;

	int sendInternal(LPCSTR filename);
};

#endif