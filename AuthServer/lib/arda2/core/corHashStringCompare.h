/*****************************************************************************
	created:	2002/05/14
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Hash traits class for std::string and std::wstring
*****************************************************************************/

#ifndef   INCLUDED_corHashStringCompare_h
#define   INCLUDED_corHashStringCompare_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class corHashStringCompare
{
public:
	enum
	{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8,	// min_buckets = 2 ^^ N, 0 < N
	};

	// hash string to size_t value (based on elfhash)
	size_t operator()( const std::string &s ) const
	{
		size_t h = 0, g;
		for ( std::string::const_iterator i = s.begin(); i != s.end(); ++i )
		{
			h = ( h << 4 ) | *i;
			g = h & 0xF0000000;
			if ( g )
				h ^= g >> 24;
			h &= ~g;
		}
		return h;
	}

	// test if key1 ordered before key2
	bool operator()(const std::string &key1, const std::string &key2) const
	{
		return key1 < key2;
	}
};

class corHashWStringCompare
{
public:
  enum
  {	// parameters for hash table
    bucket_size = 4,	// 0 < bucket_size
    min_buckets = 8,	// min_buckets = 2 ^^ N, 0 < N
  };

  // hash string to size_t value (based on elfhash)
  size_t operator()( const std::wstring &s ) const
  {
    size_t h = 0, g;
    for ( std::wstring::const_iterator i = s.begin(); i != s.end(); ++i )
    {
      h = ( h << 4 ) | *i;
      g = h & 0xF0000000;
      if ( g )
        h ^= g >> 24;
      h &= ~g;
    }
    return h;
  }

  // test if key1 ordered before key2
  bool operator()(const std::wstring &key1, const std::wstring &key2) const
  {
    return key1 < key2;
  }
};

#endif // INCLUDED_corHashStringCompare_h

