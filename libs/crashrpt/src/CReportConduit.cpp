#include "stdafx.h"
#include "ftplib-3.1-1/ftplib.h"
#include "CReportConduit.h"
#include "Utility.h"

#include <objbase.h>
#include <sys/types.h> 
#include <sys/stat.h>

CFtpConduit::CFtpConduit(LPCSTR hostnameIn, LPCSTR usernameIn, LPCSTR passwordIn)
{
	this->hostname = hostnameIn;
	this->username = usernameIn;
	this->password = passwordIn;
}

CFtpConduit::~CFtpConduit()
{

}

int CFtpConduit::send(LPCSTR filename)
{
	int result;

	// Make sure the specified file exists.
	struct stat status;
	if(stat(filename, &status) == -1)
		return 0;

	if(!FtpInit())
		return 0;

	result = sendInternal(filename);
	FtpShutdown();

	return result;
}

int CFtpConduit::sendInternal(LPCSTR filename)
{
#define RETURN(x)			\
	{						\
		if(conn)			\
			FtpQuit(conn);	\
		return x;			\
	}

	// Open a connection to the ftp server.
	netbuf* conn = NULL;

	if(!FtpConnect(hostname, &conn))
		RETURN(0);

	if(!FtpLogin(username, password, conn))
		RETURN(0);

	CString appName;
	appName = CUtility::getAppName();
	if(appName.IsEmpty())
		RETURN(0);

	// Try to move to a directory with that matches the name.
	if(!FtpChdir(appName, conn))
	{
		if(!(FtpMkdir(appName, conn) && FtpChdir(appName, conn)))
			RETURN(0);
	}

	// Try to send the specified file.
	//	Generate a unique filename.

	GUID guid;
	CoCreateGuid(&guid);

	CString remoteName;
	remoteName.Format(_T("%x-%x-%x-%x.zip"), guid.Data1, guid.Data2, guid.Data3, guid.Data4);

	//	Try to send the file.
	if(FtpPut(filename, remoteName, FTPLIB_IMAGE, conn))
		RETURN(1);

	RETURN(0);
#undef RETURN
}
