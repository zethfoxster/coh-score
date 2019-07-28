// logParser3.h : main header file for the PROJECT_NAME application
//

#pragma once
#include "LogSearch.h"
#include "LogConfig.h"
#include "LogTime.h"
#include <string>

class LogFileSearch
{
private:

	/**
	* concurrentSearches represents the number of threads actively searching 
	* at a given time.
	*/
	volatile long concurrentSearchers;
	/**
	* All of the configuration options, usually specified via the command line,
	* for a search.
	*/
	LpConfig config;
	/**
	* A Search structure representing all of the filenames which will be included
	* by the search and the pattern matching info for identifying those filenames.
	*/
	Search filesToSearch;
	/**
	* A Search structure representing the actual log search and results.
	*/
	Search s;

	/**
	* Populates Search with filenames matching filePattern from given directory.
	*
	* This function looks at all filenames within a provided directory and
	* attempts to match them against filePattern.  FilePattern is formatted using
	* the conventions described in LogPattern.h, including wildcards.  If a
	* beginning and ending time are specified, the file must also contain a date
	* within the specified range plus or minus the provided threshold.  Any
	* matching filenames will be placed into the provided Search, fileSearch.
	*
	* @param $dirName
	*   dirName is a null terminated C string representing the directory whose
	*   contents you wish to search.  The dirName MUST be an absolute path.
	* @param $filePattern
	*   In order to be a successful match, a file's name must match the specified
	*   filePattern.  This function uses LogPattern.h to interpret filePattern,
	*   so please see the details there for how to interpret it.
	* @see LogPattern
	* @param $fileSearch
	*   fileSearch is the Structure that the successfully matches filenames will
	*   be placed in.
	* @param $start
	*   (optional) if set, a filename will only succeed if it contains a date
	*   within the provided range.  The range goes from start - threshold to 
	*   end + threshold.  The default value of 0 implies no time constaints.
	* @param $end
	*   (optional) see start parameter
	* @param $threshold
	*   (optional) see start parameter
	*
	* @return
	*   "@return" true if the filename search was successful.  False if not.
	*/
	bool getFilesByPattern(char *dirName, const char *filePattern, Search &outFileSearch, log_time start=0, log_time end=0, log_time threshold=0);

	/**
	* Opens, reads and searches the provided gzipped file for the provided term.
	*
	* This function opens the provided filename and reads it in in chunks.  These
	* chunks are specified by the buffer size specified in the LogConfig.  Once
	* read, each chunk is searched until the last complete logline in the file.
	* When a new chunk is loaded, it starts at the end of the last partial logline
	* so that only compelte log lines are considered for the search.
	*
	* @param $fileStr
	*   The absolute path of the file to read and search.
	* @param $config
	*   The Search configuration, containing both the time parameters and info
	*   about how large the search buffer should be.
	* @param $s
	*   The Search structure containing the specifics about the search to be
	*   performed.  matching log lines will be stored here is awell.
	* @param $buffer
	*   pre-allocated memory to store the read-in file from.
	*
	* @return
	*   "@return" true if the search was successful.  False if not.
	*/
	bool searchFile(std::string fileStr, const LpConfig &config, Search &s, char *myText);

	/**
	* Updates the console with the percentage of files that have been successfully searched.
	*
	* As files are read by search threads, the LogFileSearch updates the console with the
	* percentage of the search that is left to complete.
	*
	* @param $nFiles
	*   total number of files.
	*
	* @param $nFilesRemaining
	*   total number of files that have not yet been read.
	*
	* @param $nFilesRemainingOld
	*   the number of files that were read in the last update.  Used to identify whether we
	*   need to update the percent.

	*/
	void updateSearchConsole(unsigned int nFiles,
		unsigned int &nFilesRemaining,
		unsigned int &nFilesRemainingOld);
public:
	LogFileSearch(): concurrentSearchers(0){};
	~LogFileSearch(){};
	/**
	* Initializes the LogFileSearch with the provided command line argments.
	*
	* Please see LogConfig for how the log file search is configured.  Once
	* configured, this function will use the configuration to prepare the list
	* of files the logSearch will go through.
	*
	* @param $lpParam
	*   The LogFileSearch structure must be passed here.
	*
	* @return
	*   "@return" true if initialization succeeded.  False if not.

	*/
	bool init(int argc, char **args);

	/**
	* The work done by a search thread to search files until all files are complete
	*
	* This is the work done by a single search thread to continuously search files
	* as long as there are files to search.
	*
	* While searchWork is actively searching files, it will be represented in the
	* number of concurrentSearchers in the LogFileSearch.
	*
	* @return
	*   "@return" true if the search was successful.  False if not.
	*/
	int searchWork();

	/**
	* Manages search across multiple threads.  
	*
	* Call this after initializing the LogFileSearch to actually retrieve search results.
	*
	* @return
	*   "@return" true if search succeeded.  False if not.

	*/
	bool performSearch();
};