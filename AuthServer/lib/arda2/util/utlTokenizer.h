/*****************************************************************************
created:	2002-9-30
copyright:	2002, NCSoft. All Rights Reserved
author(s):	Ryan Prescott

purpose:	
*****************************************************************************/
#ifndef   INCLUDED_utltokenizer_h
#define   INCLUDED_utltokenizer_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

// TODO: make this unicode?

class utlTokenizer
{
public:
	utlTokenizer():m_NextToken(0){}
	~utlTokenizer(){ }

	void			Tokenize( const std::string& text );
	void			SetKeptDelimiters(const std::string& delims);
	void			SetDroppedDelimiters( const std::string& delims);
	void			SetWhitespaceChars( const std::string& wschars);
	void			DeleteCurrentToken();
	const std::string*	GetNextToken();
	void			ClearTokenizer();
	void			ResetToTopToken();

	size_t          GetTokenCount()					{ return m_Tokens.size(); }
	uint			IncrementNextToken(uint Distance);
	uint			DecrementNextToken(uint Distance);

	uint			GetNextTokenIndex()				{ return m_NextToken; }
	void			SetNextToken( uint Offset );

protected:

private:
    std::vector<std::string>	m_Tokens;
	std::string			m_Whitespace;
	std::string			m_KeptDelimiters;
	std::string			m_DroppedDelimiters;
	uint			m_NextToken;
};

inline void utlTokenizer::SetNextToken( uint Offset )
{ 
	if(m_NextToken < m_Tokens.size()) 
		m_NextToken = Offset;
}

#endif // INCLUDED_utltokenizer_h


