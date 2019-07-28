
#ifndef _MEGAGRIDPRIVATE_H
#define _MEGAGRIDPRIVATE_H

//#define MEGA_GRID_DEBUG 1

typedef struct MegaGrid						MegaGrid;
typedef struct MegaGridNode					MegaGridNode;
typedef struct MegaGridNodeEntry			MegaGridNodeEntry;
typedef struct MegaGridCell					MegaGridCell;
typedef struct MegaGridCellEntryArray		MegaGridCellEntryArray;
typedef struct MegaGridCellEntryArraySlot	MegaGridCellEntryArraySlot;

typedef struct MegaGridNodeEntry {
	MegaGridCellEntryArraySlot*		slot;
	MegaGridCell*					cell;
} MegaGridNodeEntry;

typedef struct MegaGridNode {
	MegaGrid*			grid;
	void*				owner;
	Vec3				pos;
	F32					radius;
	S32					entries_count;
	S32					radius_hi_bits;
	S32					radius_lo;
	S32					use_small_cells;
	S32					grid_pos[3];
	MegaGridNodeEntry	entries[100];
} MegaGridNode;

typedef struct MegaGridCellEntryArraySlot {
	void*							owner;
	MegaGridNodeEntry*				entry;
	
	#if MEGA_GRID_DEBUG
		MegaGridNode*				node;
	#endif
} MegaGridCellEntryArraySlot;

typedef struct MegaGridCellEntryArray {
	MegaGridCellEntryArray*			next;
	
	S32								count;
	MegaGridCellEntryArraySlot		slot[10];
} MegaGridCellEntryArray;

typedef struct MegaGridCell {
	S32 mid[3];

	struct {
		MegaGridCell* xyz[2][2][2];
		MegaGridCell* xy[2];
		MegaGridCell* yz[2];
		MegaGridCell* xz[2];
	} children;
	
	S32 cellSize;

	MegaGridCellEntryArray* entries;
} MegaGridCell;

typedef struct MegaGrid {
	MegaGridCell	rootCell;
	S32				nodeCount;
	S32				nodeEntryCount;
} MegaGrid;

extern MegaGrid megaGrid[];

#if MEGA_GRID_DEBUG
	typedef void(*MegaGridVerifyCallback)(void* owner, MegaGridNode* node);

	void mgVerifyGrid(int gridIndex, MegaGridVerifyCallback callback);
#endif

#endif
