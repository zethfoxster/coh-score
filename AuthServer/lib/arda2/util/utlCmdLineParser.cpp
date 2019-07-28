/*****************************************************************************
	created:	2003/01/29
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Sean Riley
	
	purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/util/utlCmdLineParser.h"

using namespace std;

string utlCmdLineParser::kNoCommand = string("*");

/**
 *  Parses the command line into a map of (string,string)
 *
 *  commands must be separated by spaces from their args. for example:
 *    "-foo xx" is valid, but not "-fooxx"
 *
 *  "lone" args must be specified before GetArgs() is called. for example:
 *
 *    in "-bg -pid my.pid", the arg "bg" is a lone arg.
 *
 *  @return the map of commands
**/
cmdArgSet utlCmdLineParser::GetArgs()
{
	cmdArgSet argMap;

	int i=1;

	while (i < m_argc)
	{
		if (m_argv[i][0] == '-')
		{
            // this is a command
            char *cmd = &m_argv[i][1];

            // look for matching lone command
            bool found = false;
            for (list<string>::const_iterator it=m_loneArgs.begin(); it!=m_loneArgs.end(); it++)
            {
                if (*it == cmd)
                    found = true;
            }
            if (found)
            {
                argMap.insert( cmdArgSet::value_type(string(cmd),string("")) );
                i++;
                continue;
            }

            // associate with a value
            if (i+1 >= m_argc)
            {
                // no value for command
                ERR_REPORTV(ES_Warning, ("No value for command <%s>", cmd));
                break;
            }
            argMap.insert( cmdArgSet::value_type(string(cmd),string( m_argv[i+1])) );
            i+=2;
        }
        else
        {
            // not a command - single arg.
            argMap.insert( cmdArgSet::value_type(kNoCommand,string( m_argv[i])) );
            i++;
        }
	}

	return argMap;
}

/**
 *  Parses the command line into an array of (string,string) pairs.
 *  Flags in the array (first parameter) are not unique.
 *
 *  commands must be separated by spaces from their args. for example:
 *    "-foo xx" is valid, but not "-fooxx"
 *
 *  "lone" args must be specified before GetArgs() is called. for example:
 *
 *    in "-bg -pid my.pid", the arg "bg" is a lone arg.
 *
 *  @return the map of commands
**/
cmdArgArray utlCmdLineParser::GetArgsArray()
{
	cmdArgArray argMap;

	int i=1;

	while (i < m_argc)
	{
		if (m_argv[i][0] == '-')
		{
            // this is a command
            char *cmd = &m_argv[i][1];

            // look for matching lone command
            bool found = false;
            for (list<string>::const_iterator it=m_loneArgs.begin(); it!=m_loneArgs.end(); it++)
            {
                if (*it == cmd)
                    found = true;
            }
            if (found)
            {
                argMap.insert( cmdArgSet::value_type(string(cmd),string("")) );
                i++;
                continue;
            }

            // associate with a value
            if (i+1 >= m_argc)
            {
                // no value for command
                ERR_REPORTV(ES_Warning, ("No value for command <%s>", cmd));
                break;
            }
            argMap.insert( cmdArgSet::value_type(string(cmd),string( m_argv[i+1])) );
            i+=2;
        }
        else
        {
            // not a command - single arg.
            argMap.insert( cmdArgSet::value_type(kNoCommand,string( m_argv[i])) );
            i++;
        }
	}

	return argMap;
}

/**
 *  format the parsed command line into an output string
 *
 *  @return string with formatted cmds in it
**/
string utlCmdLineParser::GetInfo()
{
	string out;
	char buf[256];

	cmdArgSet argMap = GetArgs();

	cmdArgSet::iterator it = argMap.begin();
	cmdArgSet::iterator end = argMap.end();
	for (; it != end; it++)
	{
		const string &cmd = it->first;
		 string &value= it->second;
		 snprintf(buf, 255, " Flag: '%s' Value: '%s'\n", cmd.c_str(), value.c_str());
		out += buf;
	}
	return out;
}


