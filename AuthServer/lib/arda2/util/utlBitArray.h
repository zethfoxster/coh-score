/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved, except where noted.
	author(s):	Andy Mitchell, except where noted.
	
	purpose:	General purpose bit array, from Andrew Kirmse/Game Programming Gems
*****************************************************************************/

// A one-dimensional array of bits, similar to STL bitset.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef INCLUDED_utlBitArray
#define INCLUDED_utlBitArray

#include <memory.h>
#include <assert.h>

class utlBitArray;

/**
 * this was an internal class within corBitArray that was moved
 * to enable the wrapper in arda2(1) to continue to function.
 * 
 * TODO: this entire file should be removed.
**/
class utlBitProxyHelper
{
public:
    utlBitProxyHelper(utlBitArray &array, unsigned pos);
    utlBitProxyHelper &operator=(bool value);
    utlBitProxyHelper &operator=(const utlBitProxyHelper &that);
    operator bool() const;
    bool Flip();

private:
    utlBitArray &mArray;
    unsigned  mPos;
};

class utlBitArray
{
public:

  //
  // Constructors and destructor
  //

  explicit utlBitArray(unsigned size)
  {
    Init(size);

    // Clear last bits
    Trim();
  }

  utlBitArray(const utlBitArray &that)
  {
    mpStore = 0;
    *this = that;
  }

  virtual ~utlBitArray()
  {
    if (mLength > 1)
      delete [] mpStore;
  }

  //
  // Operators
  //

  typedef utlBitProxyHelper utlBitProxy;
  typedef utlBitProxyHelper corBitProxy;

  utlBitArray &operator=(const utlBitArray &that)
  {
    if (this != &that)
    {
      if (mLength > 1)
        delete [] mpStore;

      Init(that.mNumBits);

      memcpy(mpStore, that.mpStore, mLength * sizeof(store_type));
    }
    return *this;
  }

  utlBitProxy operator[](unsigned pos)
  {
    assert(pos < mNumBits);
    return utlBitProxy(*this, pos);
  }

  const utlBitProxy operator[](unsigned pos) const
  {
    assert(pos < mNumBits);
    return utlBitProxy(const_cast<utlBitArray &>(*this), pos);
  }

  bool operator==(const utlBitArray &that) const
  {
    if (mNumBits != that.mNumBits)
      return false;

    for (unsigned i = 0; i < mLength; i++)
      if (mpStore[i] != that.mpStore[i])
        return false;
    return true;
  }

  bool operator!=(const utlBitArray &that) const
  {
    return !(*this == that);
  }

  utlBitArray &operator&=(const utlBitArray &that)
  {
    assert(mNumBits == that.mNumBits);
    for (unsigned i = 0; i < mLength; i++)
      mpStore[i] &= that.mpStore[i];
    return *this;
  }

  utlBitArray operator|=(const utlBitArray &that)
  {
    assert(mNumBits == that.mNumBits);
    for (unsigned i = 0; i < mLength; i++)
      mpStore[i] |= that.mpStore[i];
    return *this;
  }

  utlBitArray operator^=(const utlBitArray &that)
  {
    assert(mNumBits == that.mNumBits);
    for (unsigned i = 0; i < mLength; i++)
      mpStore[i] ^= that.mpStore[i];
    return *this;
  }

  utlBitArray operator~() const
  {
    return utlBitArray(*this).FlipAllBits();
  }

  friend utlBitArray operator&(const utlBitArray &a1, const utlBitArray &a2)
  {
    return utlBitArray(a1) &= a2;
  }

  friend utlBitArray operator|(const utlBitArray &a1, const utlBitArray &a2)
  {
    return utlBitArray(a1) |= a2;
  }

  friend utlBitArray operator^(const utlBitArray &a1, const utlBitArray &a2)
  {
    return utlBitArray(a1) ^= a2;
  }

  //
  // Plain English interface
  //

  // Set all bits to false.
  void Clear()
  {
    memset(mpStore, 0, mLength * sizeof(store_type));
  }

  // Set the bit at position pos to true.
  void SetBit(unsigned pos)
  {
    assert(pos < mNumBits);
    mpStore[GetIndex(pos)] |= 1 << GetOffset(pos); 
  }

  // Set the bit at position pos to false.
  void ClearBit(unsigned pos)
  { 
    assert(pos < mNumBits);
    mpStore[GetIndex(pos)] &= ~(1 << GetOffset(pos)); 
  }

  // Toggle the bit at position pos.
  void FlipBit(unsigned pos) 
  { 
    assert(pos < mNumBits);
    mpStore[GetIndex(pos)] ^= 1 << GetOffset(pos); 
  }

  // Set the bit at position pos to the given value.
  void Set(unsigned pos, bool val)
  { 
    val ? SetBit(pos) : ClearBit(pos);
  }

  // Returns true iff the bit at position pos is true.
  bool IsBitSet(unsigned pos) const
  {
    assert(pos < mNumBits);
    return (mpStore[GetIndex(pos)] & (1 << GetOffset(pos))) != 0;
  }

  // Returns true iff all bits are false.
  bool AllBitsFalse() const
  {
    for (unsigned i=0; i < mLength; i++)
      if (mpStore[i] != 0)
        return false;
    return true;
  }

  // Change value of all bits
  utlBitArray &FlipAllBits()
  {
    for (unsigned i=0; i < mLength; i++)
      mpStore[i] = ~mpStore[i];

    Trim();
    return *this;
  }

  //
  // Bit proxy (for operator[])
  //

  friend class utlBitProxyHelper;

private:

  typedef unsigned long store_type;
  enum
  {
    bits_per_byte = 8,
    cell_size     = sizeof(store_type) * bits_per_byte
  };

  store_type        *mpStore;  
  store_type         mSingleWord; // Use this buffer when mLength is 1
  unsigned           mLength;     // Length of mpStore in units of store_type
  unsigned           mNumBits;

  // Get the index and bit offset for a given bit number.
  static unsigned GetIndex(unsigned bit_num)
  {
    return bit_num / cell_size;
  }

  static unsigned GetOffset(unsigned bit_num)
  {
    return bit_num % cell_size;
  }

  void Init(unsigned size)
  {
    mNumBits = size;

    if (size == 0)
      mLength = 0;
    else
      mLength = 1 + GetIndex(size - 1);

    // Avoid allocation if length is 1 (common case)
    if (mLength <= 1)
      mpStore = &mSingleWord;      
    else
      mpStore = new store_type[mLength];
  }

  // Force overhang bits at the end to 0
  inline void Trim()
  {
    unsigned extra_bits = mNumBits % cell_size;
    if (mLength > 0 && extra_bits != 0)
      mpStore[mLength - 1] &= ~((~(store_type) 0) << extra_bits);
  }
};

#endif // INCLUDED_utlBitArray

