//******************************************************************************
/**
 *   Created: 2005/08/31
 * Copyright: 2005, NCSoft. All Rights Reserved
 *    Author: Tom C Gambill
 *
 *  @par Last modified
 *      $DateTime: 2005/11/17 10:07:52 $
 *      $Author: cthurow $
 *      $Change: 178508 $
 *  @file
 *
 */
//******************************************************************************

#ifndef __utlResourceHeap
#define __utlResourceHeap
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//******************************************************************************
/**
 * Template for a single block/resource in a resource heap cache
 *
 * Template Parameters:
 *      ResourceElementType - Class manged by the block.(VB Impl, Tex Impl, etc.)
 *
 *
 * @ingroup Core Utility Classes
 * @brief Interface for a single resource cache heap block
 */
//******************************************************************************
template <typename ResourceElementType>
class utlResourceBlock
{
public:
    errResult   Initialize();
    void        Destroy();
    void        Empty();
    bool        RequestEmpty(int iEfficiencyRequirement);
    bool        IsEmpty();

    errResult   AddResource(ResourceElementType resource, int iBlockIndex);
    errResult   RemoveResource(ResourceElementType resource);

static int      GetBlockIndex(ResourceElementType resource);

protected:
private:
};


// Define parameters here so that the function definitions below are easier to read
#define RESOURCEHEAP_PARAMDEFS template <typename blockType, int INITIAL_BLOCKS, int MAX_BLOCKS, int FRAMES_PER_OPTIMIZE, int FRAMES_PER_EMPTYREQUEST, int EFFICIENCY_REQUIREMENT,typename ResourceElementType>
#define RESOURCEHEAP_PARAMS    utlResourceHeap<blockType,INITIAL_BLOCKS,MAX_BLOCKS,FRAMES_PER_OPTIMIZE,FRAMES_PER_EMPTYREQUEST,EFFICIENCY_REQUIREMENT,ResourceElementType>

//******************************************************************************
/**
 * Template for managing a heap of managed resources like vertex buffers, 
 * textures, etc. Many smaller resources can be combined into larger resources
 * in special memory pools including VRAM/AGP/PCI-E/mem-mapped-files/etc.
 *
 * Template Parameters:
 *      blockType      - Class specialized from utlResourceBlock to manage.
 *      INITIAL_BLOCKS - Number of blocks to allocated on Initialize().
 *      MAX_BLOCKS     - Maximum number of blocks that can be allocated and 
 *                       managed by this heap.
 *      FRAMES_PER_OPTIMIZE     - Number of frames between calls to Optimize(),
 *                                0 means never.
 *      FRAMES_PER_EMPTYREQUEST - Number of frames between sweeping 
 *                                EmptyRequest() calls, 0 means never.
 *      EFFICIENCY_REQUIREMENT  - 0-100 used by Optimize() when requesting VBs
 *                                dump contents. 40 is a good default.
 *      ResourceElementType     - Class manged by the blocks.
 *
 * @ingroup Core Utility Classes
 * @brief Interface for a generic managed resource heap cache
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS class utlResourceHeap
{
public:
    utlResourceHeap() : m_iNumAllocatedBlocks(0), 
                        m_iCurrentlyAllocatingBlock(-1),
                        m_iNextFreeBlockIndex(-1),
                        m_iLastBlockTested(-1),
                        m_iFramesSinceLastAllocation(0) {};
    ~utlResourceHeap() {Destroy();}

    errResult Initialize();
    void      Destroy();

    void	  BeginNewFrame();
    void      Optimize();
    void      Flush();

    void      PreResetDevice();
    errResult PostResetDevice();

    errResult AddResource(ResourceElementType resource);
    void      RemoveResource(ResourceElementType resource);

    // For Perf/Stats Access
    int              GetLastTestedBlock() const      { return m_iLastBlockTested; }
    int              GetAllocatingBlock() const      { return m_iCurrentlyAllocatingBlock; }
    int              GetNumAllocatedBlocks() const   { return m_iNumAllocatedBlocks; }
    const blockType& GetBlock(int iBlockIndex) const { return m_blocks[iBlockIndex]; }

protected:
private:
    errResult GoToNextBlock();
    errResult FreeOneBlock();
    bool      RequestEmptyBlock(int iBlock,
                                int iEfficiencyRequirement);

    blockType   m_blocks[MAX_BLOCKS];
    int         m_iNumAllocatedBlocks;
    int         m_iCurrentlyAllocatingBlock;

    int         m_iLastBlockTested;

    int         m_freeBlocks[MAX_BLOCKS];
    int         m_iNextFreeBlockIndex;

    int         m_iFramesSinceLastAllocation;
};


//******************************************************************************
/**
 * Initialize the heap and allocate the initial blocks
 * 
 * @return ER_Success if successful, ER_Failure if not
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
errResult RESOURCEHEAP_PARAMS::Initialize()
{
    // Allocate some initial cache blocks
    m_iNumAllocatedBlocks = INITIAL_BLOCKS;
    errResult er = PostResetDevice();
    if (ISOK(er))
        return ER_Success; 

    // Failed, no blocks allocated
    ERR_REPORT(ES_Error, "Failed to Initialize Resource Heap");
    m_iNumAllocatedBlocks = 0;
    return er;
}


//******************************************************************************
/**
 * Shutdown the resource heap
 * 
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS
void RESOURCEHEAP_PARAMS::Destroy()
{
    // Destroy all the blocks
    PreResetDevice();
    m_iNumAllocatedBlocks = 0;
}


//******************************************************************************
/**
 * Call after rendering one frame and before starting the next. Ideally right
 * after calling D3D::Present().
 * 
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
void RESOURCEHEAP_PARAMS::BeginNewFrame()
{
    ++m_iFramesSinceLastAllocation;

    // If someone put something in the currently active block, start a new one
    // for this frame so we don't (re)lock a buffer that's still being used by 
    // the GPU to render the last frame.
    if( !m_blocks[m_iCurrentlyAllocatingBlock].IsEmpty() )
    {
        GoToNextBlock();
    }
    else if (FRAMES_PER_OPTIMIZE!=0 && (m_iFramesSinceLastAllocation%FRAMES_PER_OPTIMIZE)==0)
    {
        Optimize();
    }
    else if (FRAMES_PER_EMPTYREQUEST!=0 && (m_iFramesSinceLastAllocation%FRAMES_PER_EMPTYREQUEST)==0)
    {
        if (++m_iLastBlockTested==m_iCurrentlyAllocatingBlock)
            ++m_iLastBlockTested; // Skip the currently allocating block
        if (m_iLastBlockTested>=m_iNumAllocatedBlocks)
            m_iLastBlockTested=0; // Wrap to 0

        // Every few frames that we don't add something, check another allocated
        // block to see if it has expired and should be dumped.
        RequestEmptyBlock(m_iLastBlockTested,0);
    }
}


//******************************************************************************
/**
 * Flush all cached resources, forcing used data to be reloaded next frame.
 * This should be called when loading a new level or teleporting when you know
 * most of the scene will change before the next frame.
 * 
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
void RESOURCEHEAP_PARAMS::Flush()
{
    // Iterate through the blocks and free them all
    for (int iBlock=0; iBlock<m_iNumAllocatedBlocks; ++iBlock)
        m_blocks[iBlock].Empty();

    m_iCurrentlyAllocatingBlock = 0;

    // Reset the free block list
    const int iNumFreeBlocks = m_iNumAllocatedBlocks-1;
    for (int iFreeBlock=0; iFreeBlock<iNumFreeBlocks; ++iFreeBlock)
        m_freeBlocks[iFreeBlock] = iNumFreeBlocks - iFreeBlock;
    m_iNextFreeBlockIndex = iNumFreeBlocks-1;
}


//******************************************************************************
/**
 * This is a minor flush that only frees all buffers that don't meet the 
 * efficiency criteria. This will be called by the frame-counted trigger in
 * BeginNewFrame() if enabled.
 * 
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
void RESOURCEHEAP_PARAMS::Optimize()
{
    // If there's only one block being used, we can't improve on that
    if (m_iNextFreeBlockIndex>=(m_iNumAllocatedBlocks-3))
        return;

    // Iterate through the blocks and free the ones that don't meet the 
    // threshold.
    for (int iBlock=0; iBlock<m_iNumAllocatedBlocks; ++iBlock)
    {
        if (iBlock != m_iCurrentlyAllocatingBlock)
        {
            blockType& rBlock = m_blocks[iBlock];
            if ( !rBlock.IsEmpty() && rBlock.RequestEmpty(40))
            {
                m_freeBlocks[++m_iNextFreeBlockIndex] = iBlock;
            }
        }
    }
}


//******************************************************************************
/**
 * Called before the hardware device is reset when we lost our rendering 
 * surfaces (window resize, resolution change, user logout of Windows.
 * 
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
void RESOURCEHEAP_PARAMS::PreResetDevice()
{
    // Iterate through the blocks and release any device objects
    for (int iBlock=0; iBlock<m_iNumAllocatedBlocks; ++iBlock)
        m_blocks[iBlock].Destroy();

    m_iCurrentlyAllocatingBlock = -1;
}


//******************************************************************************
/**
 * Called after the device is successfully reset to re-create the HW surfaces.
 * 
 * @return ER_Success if successful, ER_Failure if not
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
errResult RESOURCEHEAP_PARAMS::PostResetDevice()
{
    // Iterate through the blocks and (re)create device objects
    for (int iBlock=0; iBlock<m_iNumAllocatedBlocks; ++iBlock)
    {
        errResult er = m_blocks[iBlock].Initialize();
        if (ISOK(er)); else return er;
    }
    m_iCurrentlyAllocatingBlock = 0;

    // Reset the free block list
    const int iNumFreeBlocks = m_iNumAllocatedBlocks-1;
    for (int iFreeBlock=0; iFreeBlock<iNumFreeBlocks; ++iFreeBlock)
        m_freeBlocks[iFreeBlock] = iNumFreeBlocks - iFreeBlock;
    m_iNextFreeBlockIndex = iNumFreeBlocks-1;

    return ER_Success;
}


//******************************************************************************
/**
 * Add a new resource element to a heap block.
 * 
 * @param resource - Resource to be added
 *
 * @return ER_Success if successful, ER_Failure if not
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
errResult RESOURCEHEAP_PARAMS::AddResource(ResourceElementType resource)
{
    errResult er;

    m_iFramesSinceLastAllocation = 0;

    // First try to allocate from the currently active block
    er = m_blocks[m_iCurrentlyAllocatingBlock].AddResource(resource, m_iCurrentlyAllocatingBlock);
    if (ISOK(er)) return ER_Success;

    // That block is full, need a new one
    er = GoToNextBlock();
    if (ISOK(er))
    {
        er = m_blocks[m_iCurrentlyAllocatingBlock].AddResource(resource, m_iCurrentlyAllocatingBlock);
        if (ISOK(er)) return ER_Success;
    }

    ERR_REPORT(ES_Error, "Add Resource Failed");
    return er;
}


//******************************************************************************
/**
 * Remove an allocated resource element from a heap block
 * 
 * @param resource - Resource to be removed
 *
 * @return void
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
void RESOURCEHEAP_PARAMS::RemoveResource(ResourceElementType resource)
{
    int iBlockIndex = blockType::GetBlockIndex(resource);
    errAssertV(iBlockIndex<m_iNumAllocatedBlocks, ("BlockIndex out of range"));
    m_blocks[iBlockIndex].RemoveResource(resource);
}


//******************************************************************************
/**
 * When one resource block is full or done being filled for one frame. Find the
 * next free block, free one, or allocate a new one.
 * 
 * @return ER_Success if successful, ER_Failure if not
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
errResult RESOURCEHEAP_PARAMS::GoToNextBlock()
{
    if (m_iNextFreeBlockIndex>=0)
    {
        // There are free blocks, so use the next available 
        m_iCurrentlyAllocatingBlock = m_freeBlocks[m_iNextFreeBlockIndex--];
        return ER_Success;
    }

    // try to create a new free block
    errResult er = FreeOneBlock();
    if (ISOK(er))
    {
        m_iCurrentlyAllocatingBlock = m_freeBlocks[m_iNextFreeBlockIndex--];
        return er;
    }

    // Couldn't free any, can we allocate another?
    if (m_iNumAllocatedBlocks<MAX_BLOCKS)
    { 
        // No more free blocks, must allocate a new one
        m_iCurrentlyAllocatingBlock = m_iNumAllocatedBlocks;
        errResult er = m_blocks[m_iCurrentlyAllocatingBlock].Initialize();
        if (ISOK(er))
            m_iNumAllocatedBlocks++;

        return er;
    }

    ERR_REPORT(ES_Error, "Resource Heap is full and cannot allocate more blocks,"
        " need to increase MAX_BLOCKS template parameter.");
    return ER_Failure;
}


//******************************************************************************
/**
 * First try to find a block that will free itself using normal rules. If that
 * doesn't work, try calling Optimize() to do a more forceful release.
 * 
 * @return ER_Success if successful, ER_Failure if not
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
errResult RESOURCEHEAP_PARAMS::FreeOneBlock()
{
    // Find the next block to test
    int iTestingBlock = m_iLastBlockTested+1;
    for (;iTestingBlock<m_iNumAllocatedBlocks; ++iTestingBlock)
    {
        if (RequestEmptyBlock(iTestingBlock,0))
            return ER_Success;
    }
    for (iTestingBlock=0;iTestingBlock<m_iLastBlockTested; ++iTestingBlock)
    {
        if (RequestEmptyBlock(iTestingBlock,0))
            return ER_Success;
    }

    // No free blocks, let's try an Optimize() this may free up some partially
    // used buffers
    Optimize();

    // Did it work?
    if (m_iNextFreeBlockIndex>=0)
        return ER_Success;

    // We did not find a block that could be freed
    return ER_Failure;
}


//******************************************************************************
/**
 * Ask a block if it can be freed with the provided efficiency.
 * 
 * @param iBlock - Block to request
 * @param iEfficiencyRequirement - 0-100% requirement.
 *
 * @return true if freed, false if not.
 */
//******************************************************************************
RESOURCEHEAP_PARAMDEFS 
bool RESOURCEHEAP_PARAMS::RequestEmptyBlock(int iBlock,
                                            int iEfficiencyRequirement)
{
    m_iLastBlockTested = iBlock;

    // Make sure it's not already empty
    blockType& rBlock = m_blocks[iBlock];
    if (rBlock.IsEmpty())
        return true;

    // If it's not, see if it can be freed.
    if (rBlock.RequestEmpty(iEfficiencyRequirement))
    {
        m_freeBlocks[++m_iNextFreeBlockIndex] = iBlock;
        return true;
    }

    return false;
}


#endif /* __utlResourceHeap */



