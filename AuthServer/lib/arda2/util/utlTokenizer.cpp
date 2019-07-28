/*****************************************************************************
created:	2002-9-30
copyright:	2002, NCSoft. All Rights Reserved
author(s):	Ryan Prescott

purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/util/utlTokenizer.h"

using namespace std;

//	This funct implements an ordering of preference of kept delimiters over 
//		dropped delimiters over whitespace 
//
//		this doesn't amortize the tokenization... should we do so?
void utlTokenizer::Tokenize( const string& text )
{
	string::const_iterator it = text.begin();
	string::const_iterator endit = text.end();
	string curtok;

	for(;it != endit; ++it)
	{
		if(m_KeptDelimiters.find_first_of(*it) != string::npos)
		{
			if(curtok.size() > 0)
			{
				m_Tokens.push_back(curtok);
				curtok.resize(0);
			}
			curtok.push_back(*it);
			m_Tokens.push_back( curtok );
			curtok.resize(0);
			continue;
		}
		else
		{
			if(m_DroppedDelimiters.find_first_of(*it) != string::npos)
			{
				if(curtok.size() > 0)
				{
					m_Tokens.push_back(curtok);
					curtok.resize(0);
				}
				continue;
			}
			else
			{
				if(m_Whitespace.find_first_of(*it) != string::npos)
				{
					if(curtok.size() > 0)
					{
						m_Tokens.push_back(curtok);
						curtok.resize(0);
					}
					continue;
				}
				else
				{
					curtok += *it;
				}
			}
		}
	}
	if(curtok.size())
		m_Tokens.push_back( curtok );
}

void utlTokenizer::SetKeptDelimiters( const string& delims )
{
	m_KeptDelimiters = delims;
}

void utlTokenizer::SetDroppedDelimiters( const string& delims )
{
	m_DroppedDelimiters = delims;
}

void utlTokenizer::SetWhitespaceChars( const string& wschars )
{
	m_Whitespace = wschars;
}

void utlTokenizer::ResetToTopToken()
{
	m_NextToken = 0;
}

const string* utlTokenizer::GetNextToken()
{
	if( m_NextToken < m_Tokens.size() )
		return &( m_Tokens[m_NextToken++] );
	else
		return 0;
}

void utlTokenizer::ClearTokenizer()
{
	m_Tokens.resize(0);
}

void utlTokenizer::DeleteCurrentToken()
{
	vector<string>::iterator it = m_Tokens.begin();
	if(m_NextToken < (m_Tokens.size()+1))
	{
		--m_NextToken;
		advance(it, m_NextToken);
		m_Tokens.erase(it);
		
	}
}

uint utlTokenizer::DecrementNextToken(uint Distance)
{ 
	if((int)(m_NextToken - Distance)<0) 
		return 0; 
	m_NextToken -= Distance; 
	return m_NextToken; 
}

uint utlTokenizer::IncrementNextToken(uint Distance)
{ 
	if(m_Tokens.size()<(m_NextToken + Distance)) 
		return 0; 
	m_NextToken += Distance; 
	return m_NextToken; 
}
