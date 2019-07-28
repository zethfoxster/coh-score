/**
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    purpose:    Configuration file reader/writer
    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoConfigFile.h"
#include "arda2/storage/stoFileOSFile.h"
#include "arda2/storage/stoFileBuffer.h"

#include <ctype.h>

using namespace std;

// ********************************************************************
// stoConfigFileSection
// 
// Helper class for managing config file sections
// ********************************************************************


/**
Search for a key name within a config file section.

@param sKey:    key name to find    
@param pLine:   optional OUT parameter for line number in section

@return         pointer to first char of value, or NULL if not found
**/
stoConfigFileSection::LineVector::iterator stoConfigFileSection::FindKey( IN const string& sKey )
{
    LineVector::iterator it;
    for ( it = m_lines.begin(); it != m_lines.end(); ++it )
    {
        if ( _stricmp(it->key.c_str(), sKey.c_str()) == 0 )
            break;
    }
    return it;
}


errResult stoConfigFileSection::Save( IN stoFile &file )
{
    // write out section header
    if ( !m_name.empty() )
        file.WriteLine(m_name);

    for ( LineVector::iterator it = m_lines.begin(); it != m_lines.end(); ++it )
    {
        string sLine = "";

        if ( !it->key.empty() )
            sLine += it->key + "=";

        sLine += it->value + it->comment;
        file.WriteLine(sLine);
    }

    // write out blank lines that we trimmed on load (except on empty anonymous section)
    if ( !(m_name.empty() && m_lines.empty()) )
    for ( int i = 0; i < m_blankLines; ++i )
        file.WriteLine("");

    return ER_Success;
}


bool stoConfigFileSection::KeyExists( IN const string &sKey )
{
    return FindKey(sKey) != m_lines.end();
}


void stoConfigFileSection::RemoveKey( IN const string &sKey )
{
    LineVector::iterator it = FindKey(sKey);
    m_lines.erase(it);
}


void stoConfigFileSection::SetValueString( IN const string &sKey, IN const string &sValue)
{
    LineVector::iterator it = FindKey(sKey);
    if ( it != m_lines.end() )
        it->value = sValue;
    else
        AddValueString(sKey, sValue);
}

/**
 * add a new key/value pair
 * 
 * @param sKey name of key
 * @param sValue value
 * @param dupeValue flag to specify if duplicate values for a single key are allowed
 */
void stoConfigFileSection::AddValueString( 
    IN const string &sKey, 
    IN const string &sValue, 
    IN const bool dupeValue /*= true*/ )
{
    if (!dupeValue)
    {
        // check for duplicate values for this key
        for ( LineVector::iterator it = m_lines.begin(); it != m_lines.end(); ++it )
        {
            if ( _stricmp( it->key.c_str(), sKey.c_str()) == 0 )
            {
                if (_stricmp( it->value.c_str(), sValue.c_str()) == 0)
                {
                    // found a dupe, do not add this line
                    return;
                }
            }
        }
    }
    Line line;
    line.key = sKey;
    line.value = sValue;
    m_lines.push_back(line); 
}

void stoConfigFileSection::GetMultiValues( IN const string &sKey, OUT vector<string> &values )
{
    for ( LineVector::iterator it = m_lines.begin(); it != m_lines.end(); ++it )
    {
        if ( _stricmp( it->key.c_str(), sKey.c_str()) == 0 )
            values.push_back( it->value );
    }
}

const stoConfigFileSection::LineVector &stoConfigFileSection::GetLines()
{
    return m_lines;
}



// helper methods for getting/setting key values by specific type below
string stoConfigFileSection::ConvertFromType(IN const char* szValue )
{
    return string(szValue);
}

string stoConfigFileSection::ConvertFromType(IN const string &sValue )
{
    return sValue;
}
bool stoConfigFileSection::ConvertToType(IN const string &sIn, OUT string &sValue )
{
    sValue = sIn;
    return true;
}

string stoConfigFileSection::ConvertFromType(IN const int &iValue )
{
    char buffer[16];
    sprintf(buffer, "%i", iValue);
    return string(buffer);
}
bool stoConfigFileSection::ConvertToType(IN const string &sIn, OUT int &iValue )
{
    return sscanf(sIn.c_str(), "%i", &iValue) == 1;
}

string stoConfigFileSection::ConvertFromType(IN const uint &iValue )
{
    char buffer[16];
    sprintf(buffer, "%i", iValue);
    return string(buffer);
}
bool stoConfigFileSection::ConvertToType(IN const string &sIn, OUT uint &iValue )
{
    return sscanf(sIn.c_str(), "%i", &iValue) == 1;
}

string stoConfigFileSection::ConvertFromType(IN const float &fValue )
{
    char buffer[16];
    sprintf(buffer, "%#f", fValue);
    return string(buffer);
}
bool stoConfigFileSection::ConvertToType(IN const string &sIn, OUT float &fValue )
{
    return sscanf(sIn.c_str(), "%f", &fValue) == 1;
}


string stoConfigFileSection::ConvertFromType(IN const bool &bValue )
{
    return string(bValue ? "1" : "0");
}
bool stoConfigFileSection::ConvertToType(IN const string &sIn, OUT bool &bValue )
{
    int iValue;
    if ( sscanf(sIn.c_str(), "%i", &iValue) == 1)
    {
        bValue = iValue != 0;
        return true;
    }
    return false;
}

string stoConfigFileSection::ConvertFromType( IN const matVector3& inValue )
{
    char buffer[ 257 ] = { 0 };
    sprintf( buffer, "%f, %f, %f", inValue.x, inValue.y, inValue.z );

    return string( buffer );
}

inline void SkipWhiteSpace( const char*& inString )
{
    while ( inString && *inString && ( *inString == ' ' ||
        *inString == '\t' || *inString == '\n' ) )
    {
        ++inString;
    }
}
inline void SkipNonWhiteSpace( const char*& inString )
{
    while ( inString && *inString && *inString != ' ' &&
        *inString != '\t' && *inString != '\n' )
    {
        ++inString;
    }
}

bool stoConfigFileSection::ConvertToType( IN const string &sIn, OUT matVector3& outValue )
{
    // parse the string directly
    const char* myString = sIn.c_str();
    if ( !myString || !*myString ) return false;

    SkipWhiteSpace( myString );
    if ( !myString || !*myString ) return false;
    sscanf( myString, "%f", &outValue.x );
    SkipNonWhiteSpace( myString );
    if ( !myString || !*myString ) return false;
    SkipWhiteSpace( myString );
    if ( !myString || !*myString ) return false;
    sscanf( myString, "%f", &outValue.y );
    SkipNonWhiteSpace( myString );
    if ( !myString || !*myString ) return false;
    SkipWhiteSpace( myString );
    if ( !myString || !*myString ) return false;
    sscanf( myString, "%f", &outValue.z );

    return true;
}



stoConfigFile::stoConfigFile() : m_status(ER_Failure)
{
    Initialize();
}


stoConfigFile::stoConfigFile( const string &filename ) :
    m_filename(filename),
    m_status(ER_Failure)
{
    Initialize();
	Load();
}


stoConfigFile::~stoConfigFile()
{
}


void stoConfigFile::Initialize()
{
    m_sections.resize(0);

    // add implicit (anonymous) section
    stoConfigFileSection anonymous;
    anonymous.m_name = "";
    m_sections.push_back(anonymous);
}


errResult stoConfigFile::Load( const string &filename )
{
    m_filename = filename;
    return Load();
}


errResult stoConfigFile::Load()
{
    stoFileOSFile osFile;
    errResult er = osFile.Open(m_filename.c_str(), stoFileOSFile::kAccessRead);
    
    if (ISOK(er))
    {
        er = Load(osFile);
        osFile.Close();
    } else {
        ERR_REPORTV(ES_Info, ("Unable to open configuration file <%s>", m_filename.c_str()) );
    }
    return er;
}


errResult stoConfigFile::Load( IN stoFile& file )
{
    stoConfigFileSection *pSection = &m_sections.back();
    errAssert(pSection);

    stoFileBuffer buffer(file);

	while ( !buffer.Eof() )
	{
        string sLine;
        buffer.ReadLine(sLine);
		const char *pChar = sLine.c_str();

		// strip out leading whitespace
		while ( isspace(*pChar) )
			pChar++;

        if ( *pChar == '[' )
        {
            if ( strchr(pChar, ']') == NULL )
            {
                ERR_REPORTV(ES_Error, ("Error in INI section header: \"%s\"\n", sLine.c_str()));

                // ignore this line
                continue;
            }

            // add section
            m_sections.resize(m_sections.size() + 1);
            pSection = &m_sections.back();
            pSection->m_name = pChar;
            continue;
        }

        // should be line of format [[key =] value] [comment]
        const char *pKey = pChar;   // save start position
        const char *pEqual = NULL;
        bool bInString = false;
        char cStringChar = '?';    // must initialize to prevent compiler warning

        // parse the string, bailing out when we hit the end of line or a comment char
        for ( ; *pChar != '\0'; ++pChar )
        {
            if ( bInString )
            {
                if ( *pChar == cStringChar )
                    bInString = false;

                continue;
            }

            // comment char?
            if ( *pChar == ';' || *pChar == '#' )
                break;

            if ( *pChar == '\"' || *pChar == '\'' )
            {
                bInString = true;
                cStringChar = *pChar;
                continue;
            }

            if ( *pChar == '=' && pEqual == NULL )
            {
                pEqual = pChar;
                continue;
            }
        }

        // at this point, we have the following pointers
        // pKey = first non-whitespace char
        // pEqual = first '=' if found
        // pChar = comment char or EOL

        // find end of key
        const char *pKeyEnd;
        if ( pEqual )
        {
            pKeyEnd = pEqual;
            while ( pKeyEnd > pKey  && isspace(pKeyEnd[-1]) )
                --pKeyEnd;
        }
        else
            pKeyEnd = pKey;

        // find beginning of comment
        const char *pCommentStart = pChar;
        while ( pCommentStart > pKeyEnd && isspace(pCommentStart[-1]) )
            --pCommentStart;

        // find beginning of value
        const char *pValueStart = pEqual ? &pEqual[1] : pKey;
        while ( pValueStart < pCommentStart && isspace(*pValueStart) )
            ++pValueStart;

        stoConfigFileSection::Line line;
        line.key.assign(pKey, pKeyEnd);

        // assign value and remove leading/trailing quotes
        char *valueBegin = (char*)pValueStart;
        char *valueEnd = (char*)(pCommentStart);
        if (valueBegin != valueEnd)
        {
            if ( (*valueBegin == '"' && *(valueEnd-1) == '"') || 
                 (*valueBegin == '\'' && *(valueEnd-1) == '\'') )
            {
                valueBegin++;
                valueEnd--;
            }
        }
        line.value.assign(valueBegin, valueEnd);
        line.comment.assign(pCommentStart);

        pSection->m_lines.push_back(line);
    }

    // trim blank lines from the ends of sections. they're frequently put there
    // to keep things pretty, and we don't want to add new key values after them
    // we can write them back out when we save
    for ( SectionList::iterator it = m_sections.begin(); it != m_sections.end(); ++it )
    {
        stoConfigFileSection &section = *it;
        section.m_blankLines = 0;
        int nLine = (int)section.m_lines.size() - 1;

        while ( nLine >= 0 )
        {
            stoConfigFileSection::Line line = section.m_lines[nLine];
            if ( !line.key.empty() || !line.value.empty() || !line.comment.empty() )
                break;

            section.m_lines.resize(nLine);
            ++section.m_blankLines;
            --nLine;
        }
    }
    m_status = ER_Success;
    return ER_Success;
}


errResult stoConfigFile::Save()
{
    stoFileOSFile osFile;
    errResult er = osFile.Open(m_filename.c_str(), stoFileOSFile::kAccessCreate);
    ERR_TESTV(er, ("Could not open file %s", m_filename.c_str()));

    er = Save(osFile);
    osFile.Close();
    return er;
}


errResult stoConfigFile::Save( IN stoFile &file )
{
    errResult er = ER_Success;
    for ( SectionList::iterator it = m_sections.begin(); it != m_sections.end(); ++it )
    {
        if ( !ISOK(it->Save(file)) )
            er = ER_Failure;
    }
    return er;
}


stoConfigFileSection* stoConfigFile::FindSection( IN const string &sSection )
{
    // special case for anonymous section
    if ( sSection.empty() )
        return &(*m_sections.begin());

    string search = "[" + sSection + "]";

    for ( SectionList::iterator i = m_sections.begin(); i != m_sections.end(); ++i )
    {
        if ( _strnicmp(i->m_name.c_str(), search.c_str(), search.length()) == 0 )
            return &(*i);
    }

    return NULL;
}


stoConfigFileSection* stoConfigFile::GetSection( IN const string &sSection )
{
    stoConfigFileSection *pSection = FindSection(sSection);
    if ( pSection )
        return pSection;
    else
        return AddNewSection(sSection);
}


stoConfigFileSection* stoConfigFile::AddNewSection( IN const string &sSection )
{
    stoConfigFileSection newSection;
    newSection.m_name = "[" + sSection + "]";
    m_sections.push_back(newSection);
    return &m_sections.back();
}


bool stoConfigFile::SectionExists( IN const string &sSection )
{
    return FindSection(sSection) != NULL;
}


bool stoConfigFile::KeyExists( IN const string &sSection, IN const string &sKey )
{
    stoConfigFileSection *pSection = FindSection(sSection);
    if ( !pSection )
        return false;

    return pSection->KeyExists(sKey);
}


/***********************************************************************
* RemoveSection()
**********************************************************************/
void stoConfigFile::RemoveSection( IN const string &sSection )
{
    stoConfigFileSection *pSection = FindSection(sSection);
    if ( pSection )
    {
        for (SectionList::iterator it = m_sections.begin(); it != m_sections.end(); ++it )
        {
            if ( pSection == &(*it) )
            {
                m_sections.erase(it);
                break;
            }
        }
    }
}


/***********************************************************************
* RemoveKey()
**********************************************************************/
void stoConfigFile::RemoveKey( IN const string &sSection, IN const string &sKey )
{
    stoConfigFileSection *pSection = FindSection(sSection);
    if ( pSection )
        pSection->RemoveKey(sKey);
}


