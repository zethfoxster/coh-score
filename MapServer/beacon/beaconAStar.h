
#ifndef BEACONASTAR_H
#define BEACONASTAR_H

#include "ArrayOld.h"
#include "stdtypes.h"

//-------------------------------------------------------------------
// AStarInfo
//-------------------------------------------------------------------

typedef struct AStarInfo{
	void*				ownerNode;			// The node that this info refers to.
	struct AStarInfo*	parentInfo;			// Info for node that I was reached from.
	struct AStarInfo*	next;				// The next in the list of AStarInfos for this search.
	int					queueIndex;			// The index in the priority queue, used for percolating up when the cost changes.
	int					costSoFar;			// The "g" value seen in AI books/articles.
	int					totalCost;			// The "f" value.
	void*				connFromPrevNode;	// The connection object from the previous node.

	//union {
	//	int				userInt;
	//	float			userFloat;
	//	void*			userPointer;
	//} userData[2];

	int					closed;				// Which set is this node in?
} AStarInfo;


//-------------------------------------------------------------------
// NavSearchFunctions
//-------------------------------------------------------------------

typedef struct AStarSearchData AStarSearchData;

typedef int			(* NavSearchCostToTargetFunction)	(	AStarSearchData* data,
															void* nodeParent,
															void* node,
															void* connectionToNode);

typedef int			(* NavSearchCostFunction)			(	AStarSearchData* data,
															void* sourceNode,
															void* connection);

typedef int			(* NavSearchGetConnectionsFunction)	(	AStarSearchData* data,
															void* node,
															void*** connBuffer,
															void*** nodeBuffer,
															int* position,
															int* count);

typedef void		(* NavSearchOutputPath)				(	AStarSearchData* data,
															AStarInfo* tailInfo);

typedef void		(* NavSearchCloseNode)				(	void* parentNode,
															void* connection,
															void* closedNode);

typedef const char*	(* NavSearchGetNodeInfo)			(	void* node,
															void* parentNode,
															void* connection);

typedef void		(* NavSearchReportClosedInfo)		(	AStarInfo* nodeInfo,
															int checkedConnectionCount);

typedef struct NavSearchFunctions{
	NavSearchCostToTargetFunction				costToTarget;			// Required!!!
	NavSearchCostFunction						cost;					// Required!!!
	NavSearchGetConnectionsFunction				getConnections;			// Required!!!
	NavSearchOutputPath							outputPath;				// NOT Required.
	NavSearchCloseNode							closeNode;				// NOT Required.
	NavSearchGetNodeInfo						getNodeInfo;			// NOT Required.
	NavSearchReportClosedInfo					reportClosedInfo;		// NOT Required.
} NavSearchFunctions;

//-------------------------------------------------------------------
// AStarSearchData
//-------------------------------------------------------------------

typedef struct AStarOpenSet {
	AStarInfo**			nodes;
	int					size;
	int					maxSize;
} AStarOpenSet;

typedef struct AStarSearchData{
	AStarOpenSet		openSet;
	U32					exploredNodeCount;
	U32					checkedConnectionCount;
	void*				sourceNode;
	void*				targetNode;
	S32					steps;
	S32					nodeAStarInfoOffset;
	void*				userData;
	U32					pathWasOutput		: 1;	// Set if the outputPath callback was called.
	U32					searching			: 1;	// Set if a search has been started but not completed.
	U32					findClosestOnFail	: 1;	// Set if it is okay to return the closest node if dest is unreachable.
} AStarSearchData;

AStarSearchData* createAStarSearchData(void);
void initAStarSearchData(AStarSearchData* search);
void clearAStarSearchDataTempInfo(AStarSearchData* search);
void destroyAStarSearchData(AStarSearchData* search);

//-------------------------------------------------------------------
// AStar
//-------------------------------------------------------------------

void AStarSearch(AStarSearchData* search, NavSearchFunctions* functions);

#endif
