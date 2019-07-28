/**
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    purpose:    Configuration file reader/writer
    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#ifndef   INCLUDED_stoConfigFile
#define   INCLUDED_stoConfigFile
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corError.h"
#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlList.h"
#include "arda2/math/matVector3.h"

class stoFile;

/**
 * an interior section of a configuration file.
**/
class stoConfigFileSection 
{
    friend class stoConfigFile;

public:
    stoConfigFileSection() : m_blankLines(1) {};

    bool KeyExists( IN const std::string &sKey );
    void RemoveKey( IN const std::string &sKey );

    // set the value of an existing key (or add a dupe key with another value)
    template <typename T>
        void SetValue( const std::string &sKey, const T &value, bool dupeKey = false, bool dupeValue = true )
    {
        std::string s = ConvertFromType(value);
        if (dupeKey)
        {
            AddValueString(sKey, s, dupeValue);
        }
        else
        {
            SetValueString(sKey, s);
        }
    }

    // get the value of a key in the section (will get the first for dupes)
    template <typename T>
        bool GetValue( IN const std::string &sKey, OUT T &value )
    {
        LineVector::iterator it = FindKey(sKey);
        if ( it == m_lines.end() ) 
            return false;

        return ConvertToType(it->value, value);
    }

    // these versions will add the key with the default value if it doesn't already exist
    template <typename T>
        T GetValueWithDefault( const std::string &sKey, const T &defaultValue )
    {
        LineVector::iterator it = FindKey(sKey);
        if ( it == m_lines.end() )
        {
            AddValueString(sKey, ConvertFromType(defaultValue));
            return defaultValue;
        }

        T value;
        if ( ConvertToType(it->value, value) )
            return value;

        it->value = ConvertFromType(defaultValue);
        return defaultValue;
    }

    // overload for char *
    std::string GetValueWithDefault( const std::string &sKey, const char* defaultValue )
    {
        return GetValueWithDefault<std::string>(sKey, std::string(defaultValue));
    }

    // get all of the values that exist for a particular key
    void GetMultiValues( IN const std::string &sKey, OUT std::vector<std::string> &values );

    struct Line
    {
        std::string  key;
        std::string  value;
        std::string  comment;
    };
    typedef std::vector<Line> LineVector;

    const LineVector &GetLines();

protected:
  
    errResult Save( IN stoFile& file );

    LineVector::iterator FindKey( IN const std::string& sKey );
    void SetValueString( IN const std::string &sKey, IN const std::string &sValue);
    void AddValueString( IN const std::string &sKey, IN const std::string &sValue, IN const bool dupeValue = true );

    // helper conversion functions to simplify Get/Set methods
    std::string ConvertFromType( IN const char* szValue );
    std::string ConvertFromType( IN const std::string &sValue );
    std::string ConvertFromType( IN const int &iValue );
    std::string ConvertFromType( IN const uint &iValue );
    std::string ConvertFromType( IN const float &fValue );
    std::string ConvertFromType( IN const bool &bValue );
    std::string ConvertFromType( IN const matVector3& inValue );

    bool ConvertToType( IN const std::string &sIn, OUT std::string &sValue );
    bool ConvertToType( IN const std::string &sIn, OUT int &iValue );
    bool ConvertToType( IN const std::string &sIn, OUT uint &iValue );
    bool ConvertToType( IN const std::string &sIn, OUT float &fValue );
    bool ConvertToType( IN const std::string &sIn, OUT bool &bValue );
    bool ConvertToType( IN const std::string &sIn, OUT matVector3& outValue );

private:
    std::string          m_name;
    LineVector      m_lines;
    int             m_blankLines;
};


/**
 * configuration file. uses config sections.
**/
class stoConfigFile
{
public:
    stoConfigFile();
	stoConfigFile( const std::string& filename );
	~stoConfigFile( void );

    errResult Load( const std::string &filename );
    errResult Load( stoFile &file );
    errResult Save();
    errResult Save( stoFile &file );

    bool SectionExists( IN const std::string &sSection );
    bool KeyExists( IN const std::string &sSection, IN const std::string &sKey );

    void RemoveKey( IN const std::string &sSection, IN const std::string &sKey );
    void RemoveSection( IN const std::string &sSection );

    // finds the section. returns NULL if it doesn't exist
    stoConfigFileSection* FindSection( IN const std::string &sSection );

    // finds the section. creates it if it doesn't exist
    stoConfigFileSection* GetSection( IN const std::string &sSection );

    // add a new section. doesn't check if it already exists
    stoConfigFileSection* AddNewSection( IN const std::string &sSection );

    // set an existing value (or add another value with the same key - dupe)
    template <typename T>
	void SetValue( IN const std::string &sSection, IN const std::string &sKey, IN T value, bool dupe = false )
    {
        stoConfigFileSection *pSection = GetSection(sSection);
        pSection->SetValue(sKey, value, dupe);
    }

    // get the value of a single key (will get the first for dupes)
    template <typename T>
    bool GetValue( IN const std::string &sSection, IN const std::string &sKey, OUT T &value )
    {
        stoConfigFileSection *pSection = FindSection(sSection);
        if ( !pSection )
            return false;

        return pSection->GetValue(sKey, value);
    }

    // this version will add the key with the default value if it doesn't already exist
    template <typename T>
    T GetValueWithDefault( IN const std::string &sSection, IN const std::string &sKey, IN const T &defaultValue )
    {
        stoConfigFileSection *pSection = GetSection(sSection);
        return pSection->GetValueWithDefault(sKey, defaultValue);
    }

    void GetMultiValues( IN const std::string &sSection, IN const std::string &sKey, OUT std::vector<std::string> &values )
    {
        stoConfigFileSection *pSection = FindSection(sSection);
        if ( pSection )
            pSection->GetMultiValues(sKey, values);
    }

    void GetSectionNames(OUT std::vector<std::string> &values )
    {
        std::string  name;

        for ( SectionList::iterator it = m_sections.begin(); it != m_sections.end(); ++it )
        {
            stoConfigFileSection &section = *it;
            name = section.m_name;

            if(name.size())
            {
                values.push_back(name.substr(1,name.size() - 2));
            }
        }
    }

    const errResult GetStatus() const { return m_status; }
protected:
    void Initialize();
    errResult Load();

private:
    typedef std::list<stoConfigFileSection> SectionList;
    SectionList     m_sections;
    std::string          m_filename;
    errResult       m_status;
};


#endif // INCLUDED_stoConfigFile

