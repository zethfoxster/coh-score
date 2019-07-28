/*****************************************************************************
	created:	2003/01/29
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Sean Riley
	
	purpose:	convert cmd line into a map of args

*****************************************************************************/

#ifndef   INCLUDED_utlCmdLineParser
#define   INCLUDED_utlCmdLineParser
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "arda2/core/corFirst.h"
#include "arda2/core/corStlMap.h"
#include "arda2/core/corStlList.h"
#include "arda2/core/corStdString.h"

typedef std::map<std::string, std::string> cmdArgSet;
typedef std::multimap< std::string, std::string > cmdArgArray;

/***
*  command line parser class
*  
*  this parser returns a map of <command line flags, strings>.
*  There is a special flag kNoCommand (which is "*") that holds
*  the argument with no associated flag. eg.
*
*     program -h stuff foo -o xxx
*
* would return a map of ( 'h':stuff, '*':foo, 'o':xxx )
*
***/
class utlCmdLineParser
{
public:
	utlCmdLineParser(int32 argc, char **argv) : m_argc(argc), m_argv(argv) {};
	~utlCmdLineParser() {};
	void AddLoneArg(const std::string &arg) { m_loneArgs.push_back(arg); };

	cmdArgSet GetArgs();
	cmdArgArray GetArgsArray();

    std::string GetInfo();

	static std::string kNoCommand;

private:
	int32 m_argc;
	char **m_argv;
    std::list<std::string> m_loneArgs;

};

#endif  // end INCLUDED_utlCmdLineParser
