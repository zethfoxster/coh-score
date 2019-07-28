// IOObject.h: interface for the CIOObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOOBJECT_H__974B82E9_EDDA_4F22_89F8_01355BF004CD__INCLUDED_)
#define AFX_IOOBJECT_H__974B82E9_EDDA_4F22_89F8_01355BF004CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Memory.h"

class CIOObject  
: public MemoryObject
{
private:
	long refcount;

public:
	CIOObject();
	CIOObject(const CIOObject &other) { refcount = 1; }
	virtual ~CIOObject();

	int AddRef( void );
	void ReleaseRef( void );
	int GetRef( void ){ return refcount; };
	virtual void OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped);

	virtual void OnTimerCallback(void);
	virtual BOOL RegisterTimer( UINT waitTime, BOOL setEvent = TRUE);

	virtual void OnEventCallback();
	virtual bool RegisterEvent(HANDLE hEvent);
	
	BOOL PostObject( int dwDataSize, HANDLE completionPort);
};

#endif // !defined(AFX_IOOBJECT_H__974B82E9_EDDA_4F22_89F8_01355BF004CD__INCLUDED_)
