
#include "megaGrid.h"
#include "megaGridPrivate.h"
#include "assert.h"
#include "entity.h"
#include "entserver.h"

static void mgVerifyGridCell(MegaGridCell* cell){
	MegaGridCellEntryArray* entries;
	int x, y, z;
	
	if(!cell){
		return;
	}
	
	assert(!(cell->cellSize & (cell->cellSize - 1)));

	for(entries = cell->entries; entries; entries = entries->next){
		int i;
		for(i = 0; i < entries->count; i++){
			assert(entries->slot[i].entry->cell == cell);
			assert(entries->slot[i].entry->slot == entries->slot + i);
		}
	}
	
	for(x = 0; x < 2; x++){
		for(y = 0; y < 2; y++){
			for(z = 0; z < 2; z++){
				mgVerifyGridCell(cell->children.xyz[x][y][z]);
			}
		}
	}
	
	for(z = 0; z < 2; z++){
		mgVerifyGridCell(cell->children.xy[z]);
	}
	
	for(y = 0; y < 2; y++){
		mgVerifyGridCell(cell->children.xz[y]);
	}
	
	for(x = 0; x < 2; x++){
		mgVerifyGridCell(cell->children.yz[x]);
	}
}

static void mgVerifyNode(MegaGridNode* node){
	int i;
	
	if(!node){
		return;
	}
	
	for(i = 0; i < node->entries_count; i++){
		MegaGridCellEntryArray* entries;
		
		assert(node->entries[i].slot->entry == node->entries + i);
		assert(node->entries[i].slot->owner == node->owner);
		
		for(entries = node->entries[i].cell->entries; entries; entries = entries->next){
			int j;
			
			for(j = 0; j < entries->count; j++){
				if(entries->slot[j].entry == node->entries + i){
					assert(entries->slot + j == node->entries[i].slot);
					break;
				}
			}
			
			if(j != entries->count){
				break;
			}
		}
		
		assert(entries);
	}
}

void mgVerifyAllGrids(MegaGridNode* excludeNode){
	int i;
	
	for(i = 0; i < 4; i++){
		mgVerifyGridCell(&megaGrid[i].rootCell);
	}
	
	for(i = 1; i < entities_max; i++){
		Entity* e = validEntFromId(i);
		
		if(e){
			if(e->megaGridNode != excludeNode){
				mgVerifyNode(e->megaGridNode);
			}
			
			if(e->megaGridNodeCollision != excludeNode){
				mgVerifyNode(e->megaGridNodeCollision);
			}
			
			if(e->megaGridNodePlayer != excludeNode){
				mgVerifyNode(e->megaGridNodePlayer);
			}
		}
	}
}

#if MEGA_GRID_DEBUG

#include "StashTable.h"

static struct {
	S32 nodeCount;
	S32 nodeEntryCount;
	
	StashTable htFoundNodes;
} verifyData;

void mgVerifyGridCellCallback(MegaGridCell* cell, MegaGridVerifyCallback callback){
	MegaGridCellEntryArray* entries;
	S32 i;
	
	if(!cell){
		return;
	}
	
	entries = cell->entries;
	
	for(; entries; entries = entries->next){
		for(i = 0; i < entries->count; i++){
			MegaGridCellEntryArraySlot* slot = entries->slot + i;

			
			if ( stashAddressAddInt(verifyData.htFoundNodes, slot->node, 1, false))
				verifyData.nodeCount++;
		}
		
		verifyData.nodeEntryCount += entries->count;
	}
	
	for(i = 0; i < 2; i++){
		mgVerifyGridCellCallback(cell->children.xy[i], callback);
		mgVerifyGridCellCallback(cell->children.xz[i], callback);
		mgVerifyGridCellCallback(cell->children.yz[i], callback);
	}

	for(i = 0; i < 8; i++){
		mgVerifyGridCellCallback(cell->children.xyz[i&1][(i&2)>>1][(i&4)>>2], callback);
	}
}

void mgVerifyGrid(int gridIndex, MegaGridVerifyCallback callback){
	MegaGrid* grid = &megaGrid[gridIndex];
	
	if(!verifyData.htFoundNodes){
		stashTableCreateAddress(1000);
	}
	
	stashTableClear(verifyData.htFoundNodes);
	
	verifyData.nodeCount = 0;
	verifyData.nodeEntryCount = 0;

	mgVerifyGridCellCallback(&grid->rootCell, callback);
	
	assert(grid->nodeCount == verifyData.nodeCount);
	assert(grid->nodeEntryCount == verifyData.nodeEntryCount);
}

#endif // MEGA_GRID_DEBUG

