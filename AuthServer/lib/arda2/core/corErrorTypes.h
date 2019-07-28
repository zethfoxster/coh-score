/**
	copyright:	(c) 2003, NCsoft. All Rights Reserved
	author(s):	Peter Freese
	purpose:	
	modified:	$DateTime: 2005/11/17 10:07:52 $
				$Author: cthurow $
				$Change: 178508 $
				@file
**/

#ifndef   INCLUDED_corErrorTypes_h
#define   INCLUDED_corErrorTypes_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum errSeverity
{
  ES_Debug,             // use for debug and trace messages  
  ES_Info,              // information messages  
  ES_Warning,           //   
  ES_Error,             //  
  ES_Assertion,         // assert triggered  
  ES_Fatal              // abandon ship
};

// error code type, >=0 is success, <0 is failure
typedef int errResult;

// basic error codes
const errResult ER_Failure = -1;
const errResult ER_Success = 0;

// app error code TODO - move to app specific code
const errResult ER_Emulate = 1;


enum errHandlerResult
{
  EH_Ignored,
  EH_Logged,
  EH_Break,
  EH_Abort
};


#endif // INCLUDED_corErrorTypes_h
