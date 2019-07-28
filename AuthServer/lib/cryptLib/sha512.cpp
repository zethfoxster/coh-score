/* sha512.cpp: Implements the class representing the SHA-512 algorithm.
* 
* @author: Chris Cowden, ccowden@ncsoft.com
*
* created: 8/24/2006
*
* @file
*
* Functions:
*/

#include "cryptLib/sha512.h"
#include "cryptLib/sha512Impl.h"

using namespace cryptLib;

sha512::sha512()
{
	m_impl = new sha512Impl();
}

sha512::~sha512()
{
	delete m_impl;
}

void sha512::GetMessageDigest(digest512& hash, string message) // Yes, pass message by value so we can modify the copy, not the original (for bit-padding)
{
	m_impl->GetMessageDigest(hash,message);
}
