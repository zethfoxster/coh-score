/*****************************************************************************
	created:	2002/04/22
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	furnish file finding capabilities
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/core/corStlVector.h"

typedef std::vector< std::string >::const_iterator FindFileIt;

#if			CORE_SYSTEM_WINAPI
#define		FIND_DATA				WIN32_FIND_DATA
#define		FIND_HANDLE				HANDLE
#else	//	CORE_SYSTEM_WINAPI
#include	<io.h>
#define		FIND_DATA				struct _finddata_t
#define		FIND_HANDLE				intptr_t
#define		INVALID_HANDLE_VALUE	-1
#define		FindFirstFile			_findfirst
#define		FindNextFile			_findnext
#define		FindClose				_findclose
#endif	//	CORE_SYSTEM_WINAPI

class stoFindFile
{
public:
	
	stoFindFile(){};
	stoFindFile(const std::string & str, int numfiles=INT_MAX) { InitFind(str, numfiles); }
	~stoFindFile(){};

	// for c style interfacing
	size_t GetNumFiles(){ return m_Files.size(); }
	std::string GetFile(int fileindex){ return m_Files[fileindex]; }

	// for cpp style interfacing
	FindFileIt GetFileIt(){ return m_Files.begin(); }
	FindFileIt GetEndIt() { return m_Files.end(); }
	
	void InitFind( const std::string & str, int MaxNumToFind = INT_MAX)
	{
		FIND_DATA fd;
		FIND_HANDLE hFind = FindFirstFile(str.c_str(), &fd);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			int NumFound=1;
			do{
				if(NumFound>MaxNumToFind)
					break;
				else
					++NumFound;
				
#if			CORE_SYSTEM_WINAPI
				m_Files.push_back(fd.cFileName);
#else	//	CORE_SYSTEM_WINAPI
				m_Files.push_back(fd.name);
#endif	//	CORE_SYSTEM_WINAPI
			}while(FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
protected:
	
private:
	std::vector< std::string >	m_Files;
};
