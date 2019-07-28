#include "arda2/core/corFirst.h"
#include "arda2/core/corStlHashFunc.h"

#include <cctype>
#include <cwctype> // wide characters

using namespace std;

//
// Strings
//

static inline size_t HashIt(const char* pch, size_t len)
{
  // use 'djb2' algorithm
  unsigned long h = 5381;
  for (unsigned i = 0; i < len; ++i, ++pch)
  {
    const char ch = *pch;
    h = ((h << 5) + h) + ch; /* h*33 + ch */
    //h = 5*h + ch; // OLD
  }
  return size_t(h);
}

size_t corStringHashFunc::Hash(const char* s)
{
    return HashIt(s, strlen(s));
}

size_t corStringHashFunc::Hash(const string& s)
{
    return HashIt(s.data(), s.size());
}



//
// Wide Strings
//

static inline size_t HashIt(const wchar_t* pch, size_t len)
{
  // use 'djb2' algorithm
  unsigned long h = 5381;
  for (unsigned i = 0; i < len; ++i, ++pch)
  {
    const wchar_t ch = *pch;
    h = ((h << 5) + h) + ch; /* h*33 + ch */
    //h = 5*h + ch; // OLD
  }
  return size_t(h);
}


size_t corWStringHashFunc::Hash(const wchar_t* s)
{
  return HashIt(s, wcslen(s));
}

size_t corWStringHashFunc::Hash(const wstring& s)
{
  return HashIt(s.data(), s.size());
}




