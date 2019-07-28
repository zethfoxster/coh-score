//****************************************************************************
//*	  Created:  2002/06/10
//*	Copyright:  2002, NCSoft. All Rights Reserved
//*	Author(s):  Tom C Gambill
//*	
//*	  Purpose:  Template for adding fixed size block allocator functionality 
//*				to a class
//*	
//****************************************************************************

#ifndef __utlBlockAllocator
#define __utlBlockAllocator
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlVector.h"

template <class T, size_t blocksPerBatch=100, size_t blockAlignment=4>
class utlBlockAllocator
{
public:
	void* operator new(size_t s)
	{
		UNREF(s); errAssert(s==sizeof(T) && sizeof(T)>=sizeof(byte*));
		return s_Store.AllocateBlock();
	}

	void operator delete(void* pBlock)
	{
		s_Store.ReleaseBlock((T*)pBlock);
	}

	//* These can be called directly, but be warned, the constructors 
	//* and destructors on the blocks will not be called!
static T*	AllocateBlock() 		{ return s_Store.AllocateBlock(); }
static void	ReleaseBlock(T* pBlock)	{ s_Store.ReleaseBlock(pBlock); }

private:

	struct BlockStore
	{
		BlockStore() : ppNextBlock(0) {};
		~BlockStore()
		{
			//* Check for memory leaks...

			// Must clean up these pointers
			size_t iNum = batches.size();
			for (size_t i=0; i<iNum; ++i)
			{
				byte* p = (byte*)batches[i];
				delete [] p;
			}
		}

		T* AllocateBlock()
		{
			//* Is there any room?
			if (!ppNextBlock || !*ppNextBlock)
			{
				// determine the alligned size of the blocks
				static const size_t blockSize = (sizeof(T)+blockAlignment-1)&(~(blockAlignment-1));

				// make a new batch 
				byte *pNewBatch = new byte[blocksPerBatch*blockSize+15];
				batches.push_back(pNewBatch);

				//* Align the block on a 16-byte boundary
				byte* pAlignedPtr =(byte*)((size_t)(pNewBatch+15)&(~15));

				// fill the pointers with the new blocks
				ppNextBlock = (byte**)pAlignedPtr;
				for (size_t i=0; i<blocksPerBatch-1; ++i)
				{
					*((size_t*)(pAlignedPtr + i*blockSize)) = (size_t)(pAlignedPtr + (i+1)*blockSize);
//					byte* pNextBlock = pAlignedPtr + blockSize;
//					*((size_t*)pAlignedPtr) = (size_t)pNextBlock;
//					pAlignedPtr = pNextBlock;
				}
				*((size_t*)(pAlignedPtr + (blocksPerBatch-1)*blockSize)) = (size_t)0;
//				*((size_t*)pAlignedPtr) = (size_t)0;
			}

			byte* pBlock = (byte*)ppNextBlock;
			ppNextBlock = (byte**)*ppNextBlock;
			return (T*)pBlock;
		}

		void ReleaseBlock(T* pBlock)
		{
			if(pBlock)
			{
				*((size_t*)pBlock) = (size_t)ppNextBlock;
				ppNextBlock = (byte**)((byte*)pBlock);
			}
		}

		typedef std::vector<byte*> BatchPtrVector;

		byte**			ppNextBlock;		// Pointer to the next available block pointer
		BatchPtrVector	batches;			// Array of pointers to batches
	};

static BlockStore s_Store;
};

template<class T, size_t blocksPerBatch, size_t blockAlignment>
typename utlBlockAllocator<T, blocksPerBatch, blockAlignment>::BlockStore utlBlockAllocator<T, blocksPerBatch, blockAlignment>::s_Store;


#endif /* __utlBlockAllocator */

