/*****************************************************************************
	created:	2001/04/23
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corErrorHandler
#define   INCLUDED_corErrorHandler
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corError.h"
#include "arda2/core/corStlList.h"


// Define an interface class for custom error handlers, that can be
// used by errError functions.

class corErrorHandler
{
typedef std::list<corErrorHandler *> HandlerList;
typedef HandlerList::iterator HandlerListIterator;

public:
	virtual ~corErrorHandler();

	void Install();
	void Remove();

	void SetFilter( errSeverity nSeverity );
	errSeverity GetFilter() const { return m_filter; }

    // Win32 only
    static void SetDialogBoxMinSeverity( errSeverity nSeverity ) { m_dialogBoxMinSeverity = nSeverity; }
    static errSeverity GetDialogBoxMinSeverity() { return m_dialogBoxMinSeverity; }

    static void SetDefaultFilter( errSeverity nSeverity ) { m_defaultFilter = nSeverity; }
    static errSeverity GetDefaultFilter() { return m_defaultFilter; }

	static errHandlerResult HandleError( const char* szFileName, int iLineNumber, 
	errSeverity nErrorLevel, const char* szDescription);

protected:

	corErrorHandler();
	errSeverity m_filter; 		  // current filter level

	//// error reporting
	static errHandlerResult DefaultReport(const char* szFileName, int iLineNumber, 
	errSeverity nSeverity, const char *szErrorLevel, const char* szDescription);

	virtual errHandlerResult Report(const char* szFileName, int iLineNumber, 
	errSeverity nSeverity, const char *szErrorLevel, const char* szDescription) = 0;

	//// notify derived class of installation/removal
	virtual void OnInstall() {};
	virtual void OnRemove() {};
	virtual void OnSetFilter() {};

private:

	static HandlerList &GetHandlerList();

	//static bool			m_handlerListEmpty;  // unused variable?

    static errSeverity  m_dialogBoxMinSeverity;
    static errSeverity  m_defaultFilter;

	bool				m_installed; 			 // this handler is installed
};



#endif // INCLUDED_corErrorHandler

