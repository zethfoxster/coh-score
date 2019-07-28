#pragma once
#include "LogTime.h"
#include <string>

const unsigned int MAX_CHARS = ((unsigned char)~0)+1;
const unsigned int BITS_LONG = sizeof(unsigned long long)*8;
const unsigned int UPPER_TO_LOWER = 'a' - 'A';
const unsigned int MAX_CHARS_TO_DATE = 100;

class SearchImpl;	//Private Implementation of Search

class Search
{
public:
	Search();
	~Search();
	//you can't copy 
	Search( const Search &rhs );
	Search& operator=( Search );

	/**
	* Prepares a pattern object to efficiently search for the specified pattern
	*
	* This function sets up the search structures for the bitapp algorithm.
	*
	* @param $in
	*   a null terminated C string representing the pattern we will match.
	* @param $caseInsensitive
	*   Should the search ignore case?
	* @param $wildCards
	*   Should the search use # and @ wildcards?
	* @param $startTime
	*   what is the earliest log we care about?
	* @param $endTime
	*   what is the oldest log we care about?
	* @param $abort
	*   what is our tolerance for looking at older or younger logs?
	*
	* @return
	*   "@return" did we successfully initialize?
	*/
	bool init(const char *searchPattern, 
		bool caseInsensitive, 
		bool wildcards, 
		log_time startTime, 
		log_time endTime, 
		log_time abort=0);

	/**
	* Searches a block of text for log lines.
	*
	* This function searches a block of text for all the matching log
	* lines contained within.  These lines are then added to the search
	* results.  Behavior is a little different if this is the last chunk
	* in the file since that line and only that line will not end with
	* the start of another line.
	*
	* @param $text
	*   a block of characters we want to search.
	* @param $maxLine
	*   how many characters should we be willing to look for the start
	*   of the first line?
	* @param $singleLine
	*   is the block to be searched one single line, or a bunch of lines
	* @param $endOfFile
	*   is this the last chunk of text in the file?
	*
	* @return
	*   "@return" the last character covered by the search.  The last line
	*   is not covered by the search since the text chunks may break unevenly
	*   around lines.
	*/
	unsigned int findLines(const char *text, 
		unsigned int maxLine, 
		bool singleLine, 
		bool endOfFile);

	/**
	* How many chunks of search results have we stored?
	*/
	unsigned int nResultsFound();
	/**
	* Sort the stored search results by the first date in the chunk.
	*/
	void sortResults();
	/**
	* get a chunk of log lines.
	*/
	std::string popResult();

private:
	SearchImpl *pimpl_;
};


