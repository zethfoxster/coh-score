#include "uiAuctionHouseIncludeInternal.h"
#include "uiAuctionHouse.h"
#include "attrib_names.h"
#include "messagestore.h"
#include "trayCommon.h"
#include "uiSlider.h"
#include "uiContextMenu.h"
#include "uiInfo.h"
#include "uiReticle.h"

// This is used to set the initial size of the memory pool and EArray for
// the lookup list we build at startup to figure out the strict ordering.
#define AUCTION_STRING_LOOKUP_CHUNK_SIZE 2000

// This is used to figure out the strict ordering of the actual auction
// house item names once at startup so that populating the list is fast
// while playing.
typedef struct AuctionStringLookup
{
	U32 ordinal;
	U32 level;
	const char *displayName;
	const char *sortName;
} AuctionStringLookup;

// This is where we'll store all the lookups while we gather them.
MP_DEFINE(AuctionStringLookup);

// This stores an EArray of lookups which is populated as we populate the
// main tree for the UI at startup time. Once it's full we sort it and
// generate the ordinals.
static AuctionStringLookup** auctionStringLookupArray;

static TextAttribs gTextAttr =
{
	/* piBold        */  (int *)0,
	/* piItalic      */  (int *)0,
	/* piColor       */  (int *)0xffffffff,
	/* piColor2      */  (int *)0,
	/* piColorHover  */  (int *)0xffffffff,
	/* piColorSelectBG*/ (int *)0,
	/* piColorSelect */  (int *)0xff0000ff,
	/* piScale       */  (int *)(int)(SMF_FONT_SCALE),
	/* piFace        */  (int *)&game_12,
	/* piFont        */  (int *)0,
	/* piAnchor      */  (int *)0,
	/* piLink        */  (int *)0,
	/* piLinkBG      */  (int *)0,
	/* piLinkHover   */  (int *)0,
	/* piLinkHoverBG */  (int *)0,
	/* piLinkSelect  */  (int *)0,
	/* piLinkSelectBG*/  (int *)0x666666ff,
	/* piOutline     */  (int *)0,
	/* piShadow      */  (int *)0,
};

static int resetSearchResults = 0;
int curSelection = -1;

static void auctionAddLookupEntry(char* sortName, char* displayName, int level);

///////////////////////////////////////////////////////////////////////////////////////////
// C O N T E X T   M E N U
///////////////////////////////////////////////////////////////////////////////////////////
ContextMenu * s_AuctionSalvageContext = 0;
ContextMenu * s_AuctionInspirationContext = 0;
ContextMenu * s_AuctionRecipeContext = 0;
ContextMenu * s_AuctionEnhancementContext = 0;

///////////////////////////////////
// S A L V A G E

static char *cmsalvage_Name( void * data )
{
	SalvageItem *sal = (SalvageItem *) data;
	return sal ? sal->ui.pchDisplayName : "Unknown";
}

static char *cmsalvage_Desc( void * data )
{
	SalvageItem *sal = (SalvageItem *) data;
	return sal ? sal->ui.pchDisplayShortHelp : "Unknown";
}

static void cmsalvage_Info( void * data )
{
	SalvageItem *sal = (SalvageItem *) data;
	if(sal)
		info_Salvage(sal);
}

///////////////////////////////////
// R E C I P E

static char *cmrecipe_Name( void * data )
{
	DetailRecipe *pRec = (DetailRecipe *) data;
	return pRec ? pRec->ui.pchDisplayName : "Unknown";
}

static char *cmrecipe_Desc( void * data )
{
	DetailRecipe *pRec = (DetailRecipe *) data;
	return pRec ? pRec->ui.pchDisplayShortHelp : "Unknown";
}

static void cmrecipe_Info( void * data )
{
	DetailRecipe *pRec = (DetailRecipe *) data;
	if(pRec)
		info_Recipes( pRec );
}

///////////////////////////////////
// E N H A N C E M E N T
static char *cmenhancement_Name( void * data )
{
	AuctionItem *pItem = (AuctionItem *) data;
	BasePower *pPow = NULL;

	if (pItem)
		pPow = pItem->enhancement;

	return pPow ? pPow->pchDisplayName : "Unknown";
}

static char *cmenhancement_Desc( void * data )
{
	AuctionItem *pItem = (AuctionItem *) data;
	BasePower *pPow = NULL;

	if (pItem)
		pPow = pItem->enhancement;

	return pPow ? pPow->pchDisplayShortHelp : "Unknown";
}

static void cmenhancement_Info( void * data )
{
	AuctionItem *pItem = (AuctionItem *) data;
	BasePower *pPow = NULL;

	if (pItem)
		pPow = pItem->enhancement;

	if (pPow)
		info_CombineSpec(pPow, 1, pItem->lvl, 0 );
}

///////////////////////////////////
// I N S P I R A T I O N
static char *cminspiration_Name( void * data )
{
	BasePower *pPow = (BasePower *) data;
	return pPow ? pPow->pchDisplayName : "Unknown";
}

static char *cminspiration_Desc( void * data )
{
	BasePower *pPow = (BasePower *) data;
	return pPow ? pPow->pchDisplayName : "Unknown";
}

static void cminspiration_Info( void * data )
{
	BasePower *pPow = (BasePower *) data;
	if (pPow)
		info_Power(pPow, 0);

}

static void initContext()
{
	s_AuctionSalvageContext = contextMenu_Create(NULL);
	s_AuctionInspirationContext = contextMenu_Create(NULL);
	s_AuctionRecipeContext = contextMenu_Create(NULL);
	s_AuctionEnhancementContext = contextMenu_Create(NULL);

	// salvage
	contextMenu_addVariableTitle( s_AuctionSalvageContext, cmsalvage_Name, 0);
	contextMenu_addVariableText( s_AuctionSalvageContext, cmsalvage_Desc, 0);
	contextMenu_addCode( s_AuctionSalvageContext, alwaysAvailable, 0, cmsalvage_Info, 0, "CMInfoString", 0  );

	// inspiration
	contextMenu_addVariableTitle( s_AuctionInspirationContext, cminspiration_Name, 0);
	contextMenu_addVariableText( s_AuctionInspirationContext, cminspiration_Desc, 0);
	contextMenu_addCode( s_AuctionInspirationContext, alwaysAvailable, 0, cminspiration_Info, 0, "CMInfoString", 0  );

	// recipe
	contextMenu_addVariableTitle( s_AuctionRecipeContext, cmrecipe_Name, 0);
	contextMenu_addCode( s_AuctionRecipeContext, alwaysAvailable, 0, cmrecipe_Info, 0, "CMInfoString", 0  );

	// enhancement
	contextMenu_addVariableTitle( s_AuctionEnhancementContext, cmenhancement_Name, 0);
	contextMenu_addVariableText( s_AuctionEnhancementContext, cmenhancement_Desc, 0);
	contextMenu_addCode( s_AuctionEnhancementContext, alwaysAvailable, 0, cmenhancement_Info, 0, "CMInfoString", 0  );
}

void AuctionDoContextMenu(AuctionItem *pItem )
{
	int mx, my;

	if(!s_AuctionSalvageContext)
		initContext();

	rightClickCoords(&mx,&my);

	switch (pItem->type)
	{
		case kTrayItemType_Inspiration:
			contextMenu_setAlign( s_AuctionInspirationContext, mx, my, CM_LEFT, CM_TOP, pItem->inspiration);
			break;
		case kTrayItemType_Salvage:
			contextMenu_setAlign( s_AuctionSalvageContext, mx, my, CM_LEFT, CM_TOP, pItem->salvage);
			break;
		case kTrayItemType_Recipe:
			contextMenu_setAlign( s_AuctionRecipeContext, mx, my, CM_LEFT, CM_TOP, pItem->recipe);
			break;
		case kTrayItemType_SpecializationInventory:
			contextMenu_setAlign( s_AuctionEnhancementContext, mx, my, CM_LEFT, CM_TOP, pItem);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static int cmpAuctionItems(const AuctionItem** a, const AuctionItem** b)
{
	//int cmp;

	//PERFINFO_AUTO_START("cmpAuctionItems", 1);

	//cmp = (*a)->displayNameIdx - (*b)->displayNameIdx;

	//if ( cmp == 0 &&
	//	( ((AuctionItem*)*a)->type == kTrayItemType_SpecializationInventory || ((AuctionItem*)*a)->type == kTrayItemType_Recipe ) &&
	//	((AuctionItem*)*a)->type == ((AuctionItem*)*b)->type
	//	)
	//{
	//	if ( ((AuctionItem*)*a)->lvl != ((AuctionItem*)*b)->lvl )
	//	{
	//		PERFINFO_AUTO_STOP();
	//		return ((AuctionItem*)*a)->lvl < ((AuctionItem*)*b)->lvl ? -1 : ((AuctionItem*)*a)->lvl > ((AuctionItem*)*b)->lvl;
	//	}
	//	else
	//	{
	//		PERFINFO_AUTO_STOP();
	//		return (int)((AuctionItem*)*a)->pItem < (int)((AuctionItem*)*b)->pItem ? -1 : (int)((AuctionItem*)*a)->pItem > (int)((AuctionItem*)*b)->pItem;
	//	}
	//}
	//else
	//{
	//	PERFINFO_AUTO_STOP();
	//	return cmp;
	//}
	return 0;
}

void sortSearchResults()
{
	static AuctionItem **workingList = NULL;
	int i = 0;

	if ( eaSize(&g_auc.searchResults) == 0)
		return;

	PERFINFO_AUTO_START("sortSearchResults", 1);

	eaQSort(g_auc.searchResults, cmpAuctionItems);

	// remove duplicates from the list
	if ( !workingList )
		eaSetCapacity(&workingList, eaSize(&g_auc.searchResults));
	eaSetSize(&workingList, 0);
	eaPush(&workingList, g_auc.searchResults[0]);

	g_auc.highestID = g_auc.lowestID = g_auc.searchResults[0]->id;

	for ( i = 1; i < eaSize(&g_auc.searchResults); ++i )
	{
		if ( g_auc.searchResults[i]->type == kTrayItemType_SpecializationInventory &&
			g_auc.searchResults[i]->enhancement == workingList[eaSize(&workingList)-1]->enhancement &&
			g_auc.searchResults[i]->lvl == workingList[eaSize(&workingList)-1]->lvl)
		{
			freeAuctionItem(g_auc.searchResults[i]);
			continue;
		}
		if ( g_auc.lowestID == -1 || g_auc.lowestID > g_auc.searchResults[i]->id )
			g_auc.lowestID = g_auc.searchResults[i]->id;
		if ( g_auc.highestID == -1 || g_auc.highestID < g_auc.searchResults[i]->id )
			g_auc.highestID = g_auc.searchResults[i]->id;
		eaPush(&workingList, g_auc.searchResults[i]);
	}
	eaSetSize(&g_auc.searchResults, 0);
	for ( i = 0; i < eaSize(&workingList); ++i )
	{
		eaPush(&g_auc.searchResults, workingList[i]);

	}

	PERFINFO_AUTO_STOP();
}

#define ITEM_MATCHES_FORSALEANDBUY_FILTER(item) ((item->numForSale <= 0 && searchFilter.forSale && !searchFilter.forBuy) || \
				(item->numForBuy <= 0 && searchFilter.forBuy && !searchFilter.forSale) || \
				(searchFilter.forSale && searchFilter.forBuy && item->numForSale <= 0 && item->numForBuy <= 0))

static int s_AllRarirtyMask, s_AllOriginMask;

int itemMatchesSearchFilter(AuctionItem *item, int index)
{
	//char *displayName = NULL;

	//if ( !searchFilter.dirty || index < searchFilter.totalProcessed ||
	//	searchFilter.processedThisFrame >= SEARCHRESULTS_MAX_TO_PROCESS_PER_FRAME )
	//	return item->cached_matchesSearchFilter;

	//item->cached_matchesSearchFilter = 1;

	//// bad item or none for sale
	//if ( !item->pItem ||
	//	(searchFilter.type != kTrayItemType_None && item->type != searchFilter.type))
	//{
	//	item->cached_matchesSearchFilter = 0;
	//}

	//if ( item->cached_matchesSearchFilter &&
	//	(item->type == kTrayItemType_SpecializationInventory ||
	//	(item->type == kTrayItemType_Recipe && item->recipe->pchEnhancementReward)) )
	//{
	//	int cmpLevel = item->type != kTrayItemType_Recipe ? item->lvl : item->lvl-1;
	//	// levels don't match
	//	if ( cmpLevel < searchFilter.minlvl-1 || cmpLevel > searchFilter.maxlvl-1 )
	//		item->cached_matchesSearchFilter = 0;
	//}

	//if( item->cached_matchesSearchFilter && searchFilter.bitmask )
	//{
	//	// check things with rarity
	//	int rarity = -1;
	//	if ( (searchFilter.bitmask&s_AllRarirtyMask) && (item->type == kTrayItemType_Salvage || item->type == kTrayItemType_Recipe) )
	//	{
	//		if(item->type == kTrayItemType_Salvage && item->salvage )
	//			rarity =item->salvage->rarity;
	//		else if ( item->type == kTrayItemType_Recipe && item->recipe )
	//			rarity = item->recipe->Rarity;

	//		if( rarity<0 || !(searchFilter.bitmask&(1<<rarity) ))
	//			item->cached_matchesSearchFilter = 0;
	//	}
	//	// ok now check for things with origin restriction
	//	else if( (searchFilter.bitmask&s_AllOriginMask) && item->type == kTrayItemType_SpecializationInventory)
	//	{
	//		int i;
	//		for( i = EArrayGetSize(&g_CharacterOrigins.ppOrigins)-1; i >= 0; i-- )
	//		{
	//			if( (searchFilter.bitmask&(1<<(kSalvageRarity_Count+i))) && !strstri(item->enhancement->pchFullName, textStd(g_CharacterOrigins.ppOrigins[i]->pchDisplayName)) )
	//				item->cached_matchesSearchFilter = 0;
	//		}
	//	}
	//	else
	//		item->cached_matchesSearchFilter = 0;
	//}

	//// check name
	//displayName = textStd(item->displayName);
	//if ( item->cached_matchesSearchFilter && (!displayName || !strstri( displayName, searchFilter.name )) )
	//	item->cached_matchesSearchFilter = 0;

	//searchFilter.processedThisFrame++;
	//return item->cached_matchesSearchFilter;
	return 0;
}

static void s_addSearchResultItem(AuctionItem *item)
{
	//AuctionItem *tmp = allocAuctionItem();
	//copyAuctionItem(item,tmp);
	//tmp->origItem = item;
	//eaPush(&g_auc.searchResults, tmp);
}

void addSearchResult(uiTreeNode *node)
{
	//AuctionItem *item = node->pData;
	//Entity *e = playerPtr();
	//TrayItemIdentifier *ptii = g_AuctionItemDict.items[item->id];

	//if ( !ptii || !item->pItem ||
	//	(searchFilter.type != kTrayItemType_None && item->type != searchFilter.type) ||
	//	(ptii->playerType != 0 && ptii->playerType-1 != e->pl->playerType)
	//	)
	//	return;

	//if ( item->type == kTrayItemType_SpecializationInventory )
	//{
	//	int crafted = strstri(item->enhancement->pchName,"crafted")?1:0;
	//	int max = MIN(53, searchFilter.maxlvl-1);
	//	int min = 0;
	//	min = MAX(min, searchFilter.minlvl-1);

	//	// look for origin filter
	//	{
	//		uiTreeNode *cur = node->pParent;
	//		if(searchFilter.bitmask)
	//		{
	//			int i;
	//			for( i = EArrayGetSize(&g_CharacterOrigins.ppOrigins)-1; i >= 0; i-- )
	//			{
	//				if( (searchFilter.bitmask&(1<<(kSalvageRarity_Count+i))) && !strstri(item->enhancement->pchFullName, textStd(g_CharacterOrigins.ppOrigins[i]->pchDisplayName)) )
	//					return;
	//			}
	//		}
	//	}

	//	if ( item->lvl >= min || item->lvl <= max )
	//		s_addSearchResultItem(item);
	//}
	//else
	//{
	//	if ( item->type == kTrayItemType_Recipe )
	//		item->lvl = item->recipe->level;

	//	if ( item->type != kTrayItemType_Recipe ||
	//		(item->lvl <= searchFilter.maxlvl && item->lvl >= searchFilter.minlvl) )
	//	{
	//		s_addSearchResultItem(item);
	//	}
	//}
}

void updateSearchBitmask()
{
	int tmp = searchFilter.bitmask;

	searchFilter.bitmask = 0;

	if( !searchByCombo.elements[0]->selected )
	{
		int i;
		for( i = EArrayGetSize(&searchByCombo.elements)-1; i > 0; i-- )
		{
			if( searchByCombo.elements[i]->selected )
				searchFilter.bitmask |= searchByCombo.elements[i]->id;
		}
	}

	if(tmp != searchFilter.bitmask)
		searchFilter.dirty = 1;
}

void getSearchFilters()
{
	char *str;
	str = uiEditGetUTF8Text(nameEdit);
	if ( str )
		strcpy_s(SAFESTR(searchFilter.name), str);
	else
		searchFilter.name[0] = 0;
	estrDestroy(&str);

	searchFilter.minlvl = atoi(minLevelCombo.strings[minLevelCombo.currChoice]);
	searchFilter.maxlvl = atoi(maxLevelCombo.strings[maxLevelCombo.currChoice]);
	updateSearchBitmask();

	searchFilter.dirty = 1;
	searchFilter.totalProcessed = 0;
}

void freeAllSearchResults(int cache)
{
	int i;
	for ( i = 0; i < eaSize(&g_auc.searchResults); ++i )
	{
		if ( cache )
			freeAuctionItem(g_auc.searchResults[i]);
		else
			freeAuctionItem_noCache(g_auc.searchResults[i]);
	}
	eaSetSize(&g_auc.searchResults, 0);
	resetSearchResults = 1;
	g_auc.curStatusRequestIndex = 0;
}

// add all children from a current node to the search results
void treeAddAll( uiTreeNode *node )
{
	int i;

	if ( node->children )
	{
		for ( i = 0; i < eaSize(&node->children); ++i )
		{
			treeAddAll(node->children[i]);
		}
	}
	else
	{
		addSearchResult(node);
	}
}

// fill in g_auc.searchResults with the results
void treeFind_recurse( char *str, uiTreeNode *node, int findParent )
{
	int i;

	if ( node->children )
	{
		if ( findParent )
		{
			char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
			if ( displayName && strstri( displayName, str ) )
				addSearchResult(node);
		}

		for ( i = 0; i < eaSize(&node->children); ++i )
		{
			treeFind_recurse(str, node->children[i], findParent);
		}
	}
	else if ( !findParent )
	{
		char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
		if ( displayName && strstri( displayName, str ) )
			addSearchResult(node);
	}
}

// find the first node by str and return the AuctionItem for it
AuctionItem *treeFindFirst_recurse( char *str, uiTreeNode *node, int findParent )
{
	int i;

	if ( node->children )
	{
		if ( findParent )
		{
			char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
			if ( displayName && strstri( displayName, str ) )
				return ((AuctionItem*)node->pData);
		}
		for ( i = 0; i < eaSize(&node->children); ++i )
		{
			AuctionItem *res = NULL;
			if ( res = treeFindFirst_recurse(str, node->children[i], findParent) )
				return res;
		}
	}
	else if ( !findParent )
	{
		char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
		if ( displayName && strstri( displayName, str ) )
			return ((AuctionItem*)node->pData);
	}

	return NULL;
}

// find the first node by str and return the uiTreeNode for it
uiTreeNode *treeFindFirstNode_recurse( char *str, uiTreeNode *node, int findParent )
{
	int i;

	if ( node->children )
	{
		if ( findParent )
		{
			char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
			if ( displayName && strstri( displayName, str ) )
				return node;
		}
		for ( i = 0; i < eaSize(&node->children); ++i )
		{
			uiTreeNode *res = NULL;
			if ( res = treeFindFirstNode_recurse(str, node->children[i], findParent) )
				return res;
		}
	}
	else if ( !findParent )
	{
		char *displayName = textStd(((AuctionItem*)node->pData)->displayName);
		if ( displayName && strstri( displayName, str ) )
			return node;
	}

	return NULL;
}

Queue breadthFirstQueue = NULL;

// find the first node by str with a breadth first search and return the uiTreeNode for it
uiTreeNode *treeFindFirstNodeBreadthFirst_recurse( char *str, uiTreeNode *node, int findParent, int translate )
{
	int i;

	if ( !node )
		return NULL;

	if ( !breadthFirstQueue )
		breadthFirstQueue = createQueue();

	if ( node->children )
	{
		if ( findParent )
		{
			char *displayName = translate ? textStd(((AuctionItem*)node->pData)->displayName) : ((AuctionItem*)node->pData)->displayName;
			if ( displayName && strstri( displayName, str ) )
				return node;
		}
		qEnqueue(breadthFirstQueue, node);
	}
	else if ( !findParent )
	{
		char *displayName = translate ? textStd(((AuctionItem*)node->pData)->displayName) : ((AuctionItem*)node->pData)->displayName;
		if ( displayName && strstri( displayName, str ) )
			return node;
	}

	while ( node = qDequeue(breadthFirstQueue) )
	{
		for ( i = 0; i < eaSize(&node->children); ++i )
		{
			uiTreeNode *res = NULL;
			if ( res = treeFindFirstNodeBreadthFirst_recurse(str, node->children[i], findParent, translate) )
				return res;
		}
	}


	return NULL;
}

uiTreeNode *rootSearchNode = NULL;

void searchForItemInTree()
{
	freeAllSearchResults(1);
	treeFind_recurse(searchFilter.name, rootSearchNode ? rootSearchNode : searchTreeRoot, 0);
}

StashTable htParentNodeNames = NULL;

// find a node with the given name that has children
uiTreeNode *findParentNodeEx(char *name, uiTreeNode *rootNode, int translate)
{
	//uiTreeNode *ret = NULL;
	//static char *nodeHashName = NULL;
	//static uiTreeNode **nodeStack = NULL;

	//eaSetSize(&nodeStack, 0);
	//estrClear(&nodeHashName);

	//ret = rootNode;
	//while( ret->pParent )
	//{
	//	eaPush(&nodeStack, ret);
	//	ret = ret->pParent;
	//}

	//while ( ret = eaPop(&nodeStack) )
	//{
	//	estrAppend2(&nodeHashName, ((AuctionItem*)ret->pData)->name);
	//	estrConcatChar(&nodeHashName, '.');
	//}
	//estrAppend2(&nodeHashName, name);

	//if ( !htParentNodeNames )
	//	htParentNodeNames = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);
	//if ( !stashFindPointer(htParentNodeNames, nodeHashName, &ret) )
	//{
	//	ret = treeFindFirstNodeBreadthFirst_recurse(name, rootNode, 1, translate);
	//	if ( ret )
	//		stashAddPointer(htParentNodeNames, nodeHashName, ret, 0);
	//}
	return 0;
}

void freeSearchTreeNode(uiTreeNode *pNode)
{
	freeAuctionItem_noCache(pNode->pData);
}

#define findParentNode(name, rootnode) findParentNodeEx(name, rootnode, 1)
#define findParentNode_noTrans(name, rootnode) findParentNodeEx(name, rootnode, 0)

void clearPriceHistory()
{
	viewingPriceHistory = 0;
	freeAuctionHistoryItem(searchMenuHistory);
	searchMenuHistory = NULL;
}

int colorFromRarity(int rarity)
{
	int color = CLR_WHITE;

	if ( rarity == 0 )
		color = CLR_WHITE;
	else if ( rarity == 1 )
		color = CLR_WHITE;
	else if ( rarity == 2 )
		color = CLR_YELLOW;
	else if ( rarity == 3 )
		color = 0xff5500ff;
	else
		// This colour comes from Ken and is intended to be a more readable
		// and pleasing purple.
		color = 0xa082dcff;
	return color;
}



void initBuyMenuEditBoxes(float z)
{
	int i;
	char **eaOrigins = NULL;
	char **eaLevels = NULL;
	char **eaForSaleStrings = NULL;

	INIT_EDIT_BOX(nameEdit,ARRAYSIZE(searchFilter.name)-1 );
	nameEdit->respectWhiteSpace = true;

	INIT_EDIT_BOX(searchResultEdit,13);
	searchResultEdit->respectWhiteSpace = false;
	searchResultEdit->rightAlign = true;
	searchResultEdit->noClip = true;
	searchResultEdit->inputHandler = uiEditCommaNumericInputHandler;

	INIT_EDIT_BOX(displayItemNameEdit, -1);
	displayItemNameEdit->canEdit = 0;

	combocheckbox_init( &searchByCombo, SEARCHBYTYPE_X, SEARCHBYTYPE_Y, AUCTION_Z, SEARCHBYTYPE_W, SEARCHBYTYPE_W+20, SEARCHBYTYPE_H, (SEARCHBYTYPE_H * 7), textStd("SearchBy"), WDW_AUCTIONHOUSE, 0, 1 );

	for( i = 1; i < kSalvageRarity_Count; i++ )
	{
		comboboxTitle_add( &searchByCombo, 0, 0, StaticDefineIntRevLookup(RarityEnum, i), NULL, (1<<i), colorFromRarity(i), 0,0  );
		s_AllRarirtyMask |= (1<<i);
	}
	for( i = EArrayGetSize(&g_CharacterOrigins.ppOrigins)-1; i >= 0; i-- )
	{
		combocheckbox_add( &searchByCombo, 0, uiGetOriginTypeIconByName( g_CharacterOrigins.ppOrigins[i]->pchName), textStd(g_CharacterOrigins.ppOrigins[i]->pchDisplayName), 1<<(kSalvageRarity_Count+i) );
		s_AllOriginMask |= (1<<(kSalvageRarity_Count+i));
	}
	
	searchByCombo.currChoice = 0;

	for ( i = 1; i <= 53; ++i )
	{
		char levelStr[8];
		eaPush(&eaLevels, strdup(itoa(i, levelStr, 10)));
	}

	combobox_init(&minLevelCombo, MINLVL_X, MINLVL_Y, AUCTION_Z, MINLVL_W, STD_EDIT_H, (STD_EDIT_H * 4), eaLevels, WDW_AUCTIONHOUSE);
	minLevelCombo.currChoice = 0;
	combobox_init(&maxLevelCombo, MAXLVL_X, MAXLVL_Y, AUCTION_Z, MAXLVL_W, STD_EDIT_H, (STD_EDIT_H * 4), eaLevels, WDW_AUCTIONHOUSE);
	maxLevelCombo.currChoice = 52;

	eaPush(&eaForSaleStrings, "AllString");
	eaPush(&eaForSaleStrings, "ForSaleOnly");
	eaPush(&eaForSaleStrings, "BiddingOnly");
	eaPush(&eaForSaleStrings, "ForSaleAndBidding");
	combobox_init(&forSaleComboBox, FORSALE_X, FORSALE_Y, AUCTION_Z, FORSALE_W, STD_EDIT_H, (STD_EDIT_H * 4), eaForSaleStrings, WDW_AUCTIONHOUSE);
	forSaleComboBox.currChoice = 3;
}



int searchFiltersMatch(AuctionSearchFilter *f1, AuctionSearchFilter *f2)
{
	if ( f1->type != f2->type 
			|| f1->minlvl != f2->minlvl 
			|| f1->maxlvl != f2->maxlvl 
			|| f1->forSale != f2->forSale 
			|| f1->bitmask != f2->bitmask 
			|| stricmp(f1->name, f2->name) )
	{
		return 0;
	}
	return 1;
}

static void handleBuyMenuSearchBoxes(float x, float y, float z, float sc, int clr, int bkclr)
{
	UIBox bounds;
	char *comboboxChoice = NULL;
	char *comboSelection = NULL;
	// the bounding boxes for the text entry fields
	CBox nameBox = {SEARCHBOX_NAME_X*sc, SEARCHBOX_NAME_Y*sc,
		(SEARCHBOX_NAME_X + SEARCHBOX_NAME_W)*sc,
		(SEARCHBOX_NAME_Y + SEARCHBOX_EDIT_H)*sc };
	int requestedSearch = 0;
	static AuctionSearchFilter oldFilter = DEFAULT_SEARCH_FILTER;

	PERFINFO_AUTO_START("handleBuyMenuSearchBoxes",1);

	{
		CBox newBox = { nameBox.left, nameBox.top, nameBox.right, nameBox.bottom };
		int color = clr, colorBack = bkclr;

		int inputOffset = 70;

		newBox.left += x;
		newBox.right += x;
		newBox.top += y;
		newBox.bottom += y;

		font(&game_12);
		font_color( CLR_WHITE, CLR_WHITE );

		uiBoxFromCBox(&bounds, &newBox);
		uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_LEFT, str_wd(font_get(),sc, sc,textStd("NameColon"))+8*sc);
		uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_TOP, 2*sc);
		uiEditSetBounds(nameEdit, bounds);
		uiEditSetFont(nameEdit, &game_12);
		uiEditSetFontScale(nameEdit, sc);

		if( mouseCollision( &newBox ) )
			color = CLR_WHITE;

		if( mouseDownHit( &newBox, MS_LEFT ) )
			uiEditTakeFocus(nameEdit);

		drawFrame( PIX3, R10, newBox.lx, newBox.ly, AUCTION_Z,newBox.hx-newBox.lx, newBox.hy-newBox.ly, sc, color, colorBack );
		uiEditProcess(nameEdit);
		prnt(x + (nameBox.left + 5)*sc, newBox.top + 17*sc, AUCTION_Z, sc, sc, "NameColon");

		{
			char *str;
			str = uiEditGetUTF8Text(nameEdit);
			if ( str )
				strcpy_s(SAFESTR(searchFilter.name), str);
			else
				searchFilter.name[0] = 0;
			estrDestroy(&str);
		}
	}

	font(&game_9);
	font_color( CLR_WHITE, CLR_WHITE );
	prnt(x + 10*sc, y + (MINLVL_Y+18)*sc, AUCTION_Z, sc, sc, "LevelRangeString");
	prnt(x + (MAXLVL_X-17)*sc, y + (MINLVL_Y+18)*sc, AUCTION_Z, sc, sc, "ToString");

	combobox_setLoc(&searchByCombo, SEARCHBYTYPE_X, SEARCHBYTYPE_Y, 1);
	combobox_display(&searchByCombo);
	if ( searchByCombo.changed )
		updateSearchBitmask();

	combobox_setLoc(&minLevelCombo, MINLVL_X, MINLVL_Y, 1);
	comboboxChoice = combobox_display(&minLevelCombo);
	if ( comboboxChoice )
		searchFilter.minlvl = atoi(comboboxChoice);

	combobox_setLoc(&maxLevelCombo, MAXLVL_X, MAXLVL_Y, 1);
	comboboxChoice = combobox_display(&maxLevelCombo);
	if ( comboboxChoice )
		searchFilter.maxlvl = atoi(comboboxChoice);

	if ( searchFilter.maxlvl < searchFilter.minlvl )
	{
		searchFilter.maxlvl = searchFilter.minlvl;
		maxLevelCombo.currChoice = searchFilter.maxlvl-1;
	}

	{
		combobox_setLoc(&forSaleComboBox, FORSALE_X, FORSALE_Y, 1);
		comboboxChoice = combobox_display(&forSaleComboBox);
		if ( comboboxChoice )
		{
			if ( forSaleComboBox.currChoice == 0 )
				searchFilter.forBuy = searchFilter.forSale = 0;
			else if ( forSaleComboBox.currChoice == 1 )
			{
				searchFilter.forSale = 1;
				searchFilter.forBuy = 0;
			}
			else if ( forSaleComboBox.currChoice == 2 )
			{
				searchFilter.forSale = 0;
				searchFilter.forBuy = 1;
			}
			else
				searchFilter.forBuy = searchFilter.forSale = 1;
		}
	}

	if ( !searchFiltersMatch(&oldFilter, &searchFilter) )
	{
		memcpy(&oldFilter, &searchFilter, sizeof(AuctionSearchFilter));
		searchFilter.dirty = 1;
		searchFilter.totalProcessed = 0;
	}

	PERFINFO_AUTO_STOP();
}

void doBatchItemStatusRequests()
{
	//if ( !g_auc.requestingItemStatus && timerElapsed(g_auc.requestTimer) >= BATCH_ITEMSTATUS_REQ_WAIT_TIME )
	//{
	//	g_auc.requestingItemStatus = 1;
	//	g_auc.curStatusRequestIndex = MAX(g_auc.lowestID, 0);
	//	auction_batchRequestItemStatus(g_auc.curStatusRequestIndex, BATCH_ITEMSTATUS_REQ_SIZE);
	//	// update the items they are looking at independently
	//	if ( EAINRANGE(curSelection,g_auc.searchResults) )
	//		auction_updateHistory(g_auc.searchResults[curSelection]->type, g_auc.searchResults[curSelection]->name, g_auc.searchResults[curSelection]->lvl);
	//	if ( displayingExtraInfo && currentInvItem != -1 )
	//		auction_updateHistory(localInventory[currentInvItem]->type, localInventory[currentInvItem]->name, localInventory[currentInvItem]->lvl);
	//	g_auc.curStatusRequestIndex += BATCH_ITEMSTATUS_REQ_SIZE;
	//	if ( g_auc.curStatusRequestIndex > g_auc.highestID )
	//	{
	//		g_auc.requestingItemStatus = 0;
	//		g_auc.curStatusRequestIndex = 0;
	//	}
	//	timerStart(g_auc.requestTimer);
	//}
	//else if ( g_auc.requestingItemStatus && timerElapsed(g_auc.requestTimer) >= BATCH_ITEMSTATUS_REQ_UPDATE_TIME )
	//{
	//	auction_batchRequestItemStatus(g_auc.curStatusRequestIndex, BATCH_ITEMSTATUS_REQ_SIZE);
	//	g_auc.curStatusRequestIndex += BATCH_ITEMSTATUS_REQ_SIZE;
	//	if ( g_auc.curStatusRequestIndex > g_auc.highestID )
	//	{
	//		g_auc.requestingItemStatus = 0;
	//		g_auc.curStatusRequestIndex = 0;
	//	}
	//	timerStart(g_auc.requestTimer);
	//}
}

void handleBuyMenuSearchResults(Entity *e, float x, float y, float z, float wd, float ht, float sc, int clr, int bkclr)
{
	//static int curPage = 0;
	//int i, firstItem = curPage * RESULT_ITEMS_PER_PAGE;
	//float yPos = y + (DISPLAYITEM_Y*sc);
	//static ScrollBar sb = { -1, 0 };
	//CBox framebox = {x+FRAME2_X*sc, y+FRAME2_Y*sc, x+(FRAME2_X+FRAME_W)*sc, y+(FRAME2_Y+FRAME_H)*sc};
	//int numItems = 0, numItemsBeforeCurSelThisFrame = 0, selectedItemThisFrame = 0;
	//static int numItemsBeforeCurSel = 0;
	//static int numForSaleIndexToUpdateThisFrame = 0;
	//const int NumForSaleUpdatesPerFrame = 50;
	//int numForSaleUpdatedThisFrame = 0;
	//int atLeastOneItemFound = 0;

	//PERFINFO_AUTO_START("handleBuyMenuSearchResults",1);

	//if ( resetSearchResults )
	//{
	//	curPage = 0;
	//	curSelection = -1;
	//	resetSearchResults = 0;
	//}

	//if ( firstItem >= eaSize(&g_auc.searchResults) )
	//	curPage = 0;

	//PERFINFO_AUTO_START("BATCH_ITEMSTATUS_REQ",1);
	//// update items to see what items are up for sale
	//doBatchItemStatusRequests();
	//PERFINFO_AUTO_STOP();

	//PERFINFO_AUTO_START("check_for_clicks",1);
	//// go through once to figure out if they changed the currently selected item
	//// (this is to prevent it from looking bad when they select an item above the
	//// currently selected item, becuase it causes two items to be drawn as the
	//// currently selected item for one frame)
	//for ( i = 0; i < eaSize(&g_auc.searchResults); ++i )
	//{
	//	CBox itemFrameBox;
	//	int displayItemH = curSelection == i ? DISPLAYITEMSELECTED_H : DISPLAYITEM_H;

	//	g_auc.searchResults[i]->numForSale = getNumSelling(g_auc.searchResults[i]);
	//	g_auc.searchResults[i]->numForBuy = getNumBuying(g_auc.searchResults[i]);

	//	if ( curSelection != i && (!itemMatchesSearchFilter(g_auc.searchResults[i], i) ||
	//		ITEM_MATCHES_FORSALEANDBUY_FILTER(g_auc.searchResults[i])) )
	//	{
	//		continue;
	//	}
	//	atLeastOneItemFound = 1;
	//	++numItems;
	//	if ( i < curSelection )
	//		++numItemsBeforeCurSelThisFrame;

	//	itemFrameBox.left = x + (FRAME2_X+5)*sc;
	//	itemFrameBox.top = yPos - 8*sc;
	//	itemFrameBox.right = itemFrameBox.left + (FRAME_W-10) * sc;
	//	itemFrameBox.bottom = itemFrameBox.top + displayItemH * sc;
	//	itemFrameBox.top -= sb.offset;
	//	itemFrameBox.bottom -= sb.offset;

	//	if ( itemFrameBox.top + displayItemH*sc < framebox.top ||
	//		itemFrameBox.bottom - displayItemH*sc > framebox.bottom)
	//	{
	//		yPos += displayItemH * sc;
	//		continue;
	//	}

	//	if( curSelection != i && mouseClickHit( &itemFrameBox, MS_LEFT ) )
	//	{
	//		AuctionItem *search_res = g_auc.searchResults[i];
	//		curSelection = i;
	//		
	//		uiEditTakeFocus(searchResultEdit);
	//		clearPriceHistory();
	//		auction_updateHistory(search_res->type, search_res->name, search_res->lvl);
	//		mouseClear(); // caused a buttons that appeared underneath to get hit when mouse wasnt cleared
	//		selectedItemThisFrame = 1;
	//	}
	//	else if ( curSelection == i && mouseDownHitNoCol(MS_LEFT) && !mouseClickHit( &itemFrameBox, MS_LEFT ) )
	//	{
	//		uiEditReturnFocus(searchResultEdit);
	//	}
	//	else if ( mouseClickHit( &itemFrameBox, MS_RIGHT ) )
	//	{
	//		AuctionDoContextMenu(g_auc.searchResults[i]);
	//	}


	//	yPos += displayItemH * sc;
	//}
	//PERFINFO_AUTO_STOP();

	//yPos = y + (DISPLAYITEM_Y*sc);

	//// keep the scroll bar centered on the currently selected item
	//if ( !selectedItemThisFrame && searchResultEdit && uiEditHasFocus(searchResultEdit) && numItemsBeforeCurSelThisFrame > numItemsBeforeCurSel )
	//{
	//	sb.offset += (numItemsBeforeCurSelThisFrame - numItemsBeforeCurSel) * (DISPLAYITEM_H);
	//}
	//numItemsBeforeCurSel = numItemsBeforeCurSelThisFrame;

	//numItems = 0;

	//PERFINFO_AUTO_START("handle_search_results",1);
	//if ( atLeastOneItemFound )
	//{
	//	for ( i = 0; i < eaSize(&g_auc.searchResults); ++i )
	//	{
	//		CBox box;
	//		CBox itemFrameBox;
	//		UIBox bounds;
	//		int color = clr, colorBack = bkclr;
	//		int displayItemH = curSelection == i ? DISPLAYITEMSELECTED_H : DISPLAYITEM_H;
	//		int buyPrice = 0;

	//		if ( curSelection != i && (!itemMatchesSearchFilter(g_auc.searchResults[i], i) ||
	//		    ITEM_MATCHES_FORSALEANDBUY_FILTER(g_auc.searchResults[i])) )
	//		{
	//			continue;
	//		}
	//		numItems++;
	//		if ( i < curSelection )
	//			++numItemsBeforeCurSelThisFrame;

	//		itemFrameBox.left = x + (FRAME2_X+5)*sc;
	//		itemFrameBox.top = yPos - 8*sc;
	//		itemFrameBox.right = itemFrameBox.left + (FRAME_W-10) * sc;
	//		itemFrameBox.bottom = itemFrameBox.top + displayItemH * sc;
	//		itemFrameBox.top -= sb.offset;
	//		itemFrameBox.bottom -= sb.offset;

	//		if ( itemFrameBox.top + displayItemH*sc < framebox.top ||
	//			itemFrameBox.bottom - displayItemH*sc > framebox.bottom)
	//		{
	//			yPos += displayItemH * sc;
	//			continue;
	//		}

	//		// figure out the name if it doesnt yet exist
	//		if ( !g_auc.searchResults[i]->name )
	//		{
	//			auctionItemMakeName(g_auc.searchResults[i]);
	//		}

	//		drawFrameBox(PIX3, R10, &itemFrameBox, AUCTION_Z,
	//			(g_auc.searchResults[i]->numForSale > 0 || g_auc.searchResults[i]->numForBuy > 0? 0xFFFFFFFF : 0x99999999), 0x55555500 | (g_auc.searchResults[i]->numForSale > 0 || g_auc.searchResults[i]->numForBuy > 0 ? 0xAA : 0x55) );

	//		if ( curSelection == i )
	//		{
	//			if (g_auc.searchResults[i]->type == kTrayItemType_Recipe)
	//				drawAuctionItemIcon(g_auc.searchResults[i], x+(DISPLAYITEM_X-5)*sc, itemFrameBox.top+35*sc, AUCTION_Z, sc*0.75f, sc*0.75f, 0.0f);
	//			else
	//				drawAuctionItemIcon(g_auc.searchResults[i], x+(DISPLAYITEM_X-5)*sc, itemFrameBox.top+25*sc, AUCTION_Z, sc, sc, 0.0f);
	//		}
	//		else
	//			drawAuctionItemIcon(g_auc.searchResults[i], x+DISPLAYITEM_X*sc, itemFrameBox.top+3*sc, AUCTION_Z, sc*0.58f, sc*0.9f, 0.0f);

	//		{
	//			font(&game_12);
	//			font_color( CLR_WHITE, CLR_WHITE );

	//			gTextAttr.piScale = (int *)(int)((sc*1.1)*SMF_FONT_SCALE);

	//			if ( g_auc.searchResults[i]->type == kTrayItemType_Salvage )
	//				gTextAttr.piColor = (int*)colorFromRarity(g_auc.searchResults[i]->salvage->rarity);
	//			else if ( g_auc.searchResults[i]->type == kTrayItemType_Recipe )
	//				gTextAttr.piColor = (int*)colorFromRarity(g_auc.searchResults[i]->recipe->Rarity);
	//			else
	//				gTextAttr.piColor = (int*)CLR_WHITE;

	//			smf_ParseAndDisplay(g_auc.searchResults[i]->smf, textStd(g_auc.searchResults[i]->displayName), 
	//								x + (DISPLAYITEM_INFO_X-10)*sc, itemFrameBox.top, AUCTION_Z,
	//								((FRAME_W + FRAME2_X) - DISPLAYITEM_INFO_X)*sc, DISPLAYITEM_H*sc, 1, 1, 
	//								&gTextAttr, NULL, 0, true);
	//			if ( curSelection != i )
	//			{
	//				yPos += displayItemH * sc;
	//				continue;
	//			}
	//		}

	//		curSearchMenuItemName = g_auc.searchResults[i]->name;

	//		if ( INRANGE0(g_auc.searchResults[i]->id,eaSize(&g_AuctionItemDict.items)) &&
	//			g_AuctionItemDict.items[g_auc.searchResults[i]->id]->buyPrice > 0 )
	//		{
	//			buyPrice = g_AuctionItemDict.items[g_auc.searchResults[i]->id]->buyPrice;
	//		}

	//		box.left = x + DISPLAYITEM_EDIT_X*sc;
	//		box.top = itemFrameBox.top+(DISPLAYITEM_EDIT_Y/*+DISPLAYITEM_EDIT_H*/)*sc;
	//		box.right = box.left + DISPLAYITEM_EDIT_W*sc;
	//		box.bottom = box.top + DISPLAYITEM_EDIT_H*sc;

	//		if ( buyPrice )
	//		{
	//			prnt(box.left+10*sc, box.top+10*sc, z, sc, sc, "%s", textStd("ValueInf", buyPrice));
	//			if ( drawStdButtonTopLeft(box.left, box.top + DISPLAYITEM_EDIT_H*sc,
	//				AUCTION_Z, box.hx-box.lx, box.hy-box.ly, clr, "BuyString", sc,
	//				buyPrice > e->pchar->iInfluencePoints || buyPrice <= 0 || g_auc.uiDisabled ? BUTTON_LOCKED : 0) == D_MOUSEHIT )
	//			{
	//				char *newName = NULL;
	//				// we need to strip the level from the name
	//				switch ( g_auc.searchResults[i]->type )
	//				{
	//					case kTrayItemType_Salvage:
	//						newName = g_auc.searchResults[i]->salvage->pchName;
	//					xcase kTrayItemType_Inspiration:
	//						newName = g_auc.searchResults[i]->recipe->pchName;
	//					xcase kTrayItemType_Recipe:
	//						newName = g_auc.searchResults[i]->inspiration->pchFullName;
	//					xcase kTrayItemType_SpecializationInventory:
	//						newName = g_auc.searchResults[i]->enhancement->pchFullName;
 //						xcase kTrayItemType_Power:
 //							newName = g_auc.searchResults[i]->temp_power->pchFullName;
 //						xdefault:
 //							Errorf("Invalid TrayItemType for handleBuyMenuSearchResults: %s",TrayItemType_Str(g_auc.searchResults[i]->type));
	//				}
	//				auctionHouseAddItem(g_auc.searchResults[i]->type, newName, g_auc.searchResults[i]->lvl, 0, 1,
	//					buyPrice, AuctionInvItemStatus_Bought, g_auc.searchResults[i]->id);
	//			}
	//			prnt(box.left-15*sc, box.bottom + (DISPLAYITEM_EDIT_H * 1.8f) *sc, AUCTION_Z, sc, sc, "%s", textStd("AlwaysAvailable") );
	//		}
	//		else
	//		{
	//			int amtToBid = 1;

	//			box.left = x + DISPLAYITEM_EDIT_X*sc;
	//			box.top = itemFrameBox.top+(DISPLAYITEM_EDIT_Y/*+DISPLAYITEM_EDIT_H*/)*sc;
	//			box.right = box.left + DISPLAYITEM_EDIT_W*sc;
	//			box.bottom = box.top + DISPLAYITEM_EDIT_H*sc;
	//			uiBoxFromCBox(&bounds, &box);
	//			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_LEFT, R10*sc);
	//			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_RIGHT, R10*sc);
	//			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_TOP, PIX3*sc);
	//			uiEditSetBounds(searchResultEdit, bounds);
	//			uiEditSetFontScale(searchResultEdit, sc);
	//			uiEditSetFont(searchResultEdit, &game_12);


	//			if( mouseDownHit( &box, MS_LEFT ) )
	//			{
	//				uiEditTakeFocus(searchResultEdit);
	//			}

	//			drawFrameBox( PIX3, R10, &box, AUCTION_Z, mouseCollision(&box) ? CLR_WHITE : clr, colorBack );
	//			uiEditProcess(searchResultEdit);

	//			// stack amount input and slider
	//			if ( g_auc.searchResults[i]->type == kTrayItemType_Salvage || g_auc.searchResults[i]->type == kTrayItemType_Recipe )
	//			{
	//				static UIEdit *amountEdit = NULL;
	//				static float curSliderSel = -1.0f;
	//				float newCurSel = 0.0f;
	//				char *editText = NULL;
	//				static char text[3] = {0};
	//				CBox amountBox;
	//				UIBox amountBounds;

	//				#define INT_AMOUNT_FROM_SLIDER ((int)(((curSliderSel + 1.f) / 2.f) * (MAX_AUCTION_STACK_SIZE-1) + 0.5f) + 1)

	//				if ( !amountEdit )
	//				{
	//					amountEdit = uiEditCreate();
	//					amountEdit->textColor = CLR_WHITE;
	//					uiEditSetFont(amountEdit, &game_12);
	//					amountEdit->limitType = UIECL_UTF8_ESCAPED;
	//					amountEdit->limitCount = 2;
	//					amountEdit->disallowReturn = 1;
	//					newCurSel = INT_AMOUNT_FROM_SLIDER;
	//					if ( INRANGE((int)newCurSel, 0, 99) )
	//					{
	//						itoa((int)newCurSel, text, 10);
	//						uiEditSetUTF8Text(amountEdit, text);
	//					}
	//				}

	//				amountBox.left = box.lx + DISPLAYITEM_SLIDER_W*sc;
	//				amountBox.top = box.ly + DISPLAYITEM_EDIT_H*sc;
	//				amountBox.right = amountBox.left + DISPLAYITEM_SLIDER_EDIT_W*sc;
	//				amountBox.bottom = amountBox.top + DISPLAYITEM_EDIT_H*sc;
	//				curSliderSel = doSlider(
	//					box.lx + ((DISPLAYITEM_SLIDER_W+1)/2)*sc, box.ly + (DISPLAYITEM_EDIT_H+DISPLAYITEM_EDIT_H/2)*sc, z,
	//					DISPLAYITEM_SLIDER_W, sc, sc, curSliderSel, 0, clr, 0);

	//				if ( isDown(MS_LEFT) )
	//				{
	//					newCurSel = INT_AMOUNT_FROM_SLIDER;
	//					if ( INRANGE((int)newCurSel, 0, 99) )
	//					{
	//						itoa((int)newCurSel, text, 10);
	//						uiEditSetUTF8Text(amountEdit, text);
	//					}

	//					if( mouseCollision( &amountBox ) )
	//						uiEditTakeFocus(amountEdit);
	//				}

	//				uiBoxFromCBox(&amountBounds, &amountBox);
	//				uiBoxAlter(&amountBounds, UIBAT_SHRINK, UIBAD_TOP, PIX3*sc);
	//				if(amountEdit->contentCharacterCount < 2)
	//				{
	//					uiBoxAlter(&amountBounds, UIBAT_SHRINK, UIBAD_LEFT, 13*sc);
	//				}
	//				else
	//				{
	//					uiBoxAlter(&amountBounds, UIBAT_SHRINK, UIBAD_LEFT, 9*sc);
	//				}
	//				uiEditSetBounds(amountEdit, amountBounds);
	//				amountEdit->z = z;
	//				uiEditSetFontScale(amountEdit, sc);
	//				uiEditSetFont(searchResultEdit, &game_12);

	//				drawFrameBox(PIX3, R10, &amountBox, z, mouseCollision(&amountBox) ? CLR_WHITE : clr, colorBack);
	//				uiEditProcess(amountEdit);
	//				editText = uiEditGetUTF8Text(amountEdit);
	//				if ( editText )
	//				{
	//					if ( !strIsNumeric(editText) )
	//						newCurSel = MAX_AUCTION_STACK_SIZE;
	//					else
	//					{
	//						newCurSel = atoi(editText);
	//						if ( newCurSel > MAX_AUCTION_STACK_SIZE )
	//							newCurSel = MAX_AUCTION_STACK_SIZE;
	//						else if ( newCurSel < 1 )
	//							newCurSel = 1;
	//					}
	//					itoa((int)newCurSel, text, 10);
	//					uiEditSetUTF8Text(amountEdit, text);
	//				}

	//				curSliderSel = ((newCurSel-1)/(float)(MAX_AUCTION_STACK_SIZE-1))*2.0f - 1.0f;
	//				amtToBid = INT_AMOUNT_FROM_SLIDER;
	//			}

	//			font(&game_9);
	//			font_color( 0xffffffff, 0xffffffff );
	//			if ( g_auc.searchResults[i]->numForSale != -1 )
	//				prnt(itemFrameBox.left + 8*sc, box.bottom + (DISPLAYITEM_EDIT_H * 2.1) *sc, z, sc, sc, "%d %s", g_auc.searchResults[i]->numForSale, textStd("ForSale") );
	//			if ( g_auc.searchResults[i]->numForSale != -1 )
	//				prnt(itemFrameBox.left + 8*sc, box.bottom + (DISPLAYITEM_EDIT_H * 1.6) *sc, z, sc, sc, "%d %s", g_auc.searchResults[i]->numForBuy, textStd("BiddingString") );

	//			{
	//				char *bid = uiEditGetUTF8Text(searchResultEdit);
	//				int bidAmt = bid ? atoi_ignore_nonnumeric(bid) : 0;
	//				INT64 totalBid;

	//				if ( bidAmt > MAX_INFLUENCE )
	//				{
	//					char infstr[MAX_INF_LENGTH_WITH_COMMAS];

	//					bidAmt = MAX_INFLUENCE;
	//					uiEditSetUTF8Text(searchResultEdit, itoa_with_commas(bidAmt, infstr));
	//				}

	//				if ( !bid )
	//				{
	//					font(&game_12);
	//					font_color( 0xffffff77, 0xffffff77 );
	//					prnt(bounds.x, bounds.y + ((DISPLAYITEM_EDIT_H/2+PIX3)*sc), z, sc, sc, "EnterBid");
	//				}

	//				totalBid = (INT64)amtToBid*(INT64)bidAmt;

	//				if ( drawStdButtonTopLeft(box.lx,
	//					box.ly + (g_auc.searchResults[i]->type == kTrayItemType_Salvage ||
	//						g_auc.searchResults[i]->type == kTrayItemType_Recipe ? DISPLAYITEM_EDIT_H*2 : DISPLAYITEM_EDIT_H)*sc,
	//					z, box.hx-box.lx, box.hy-box.ly, clr, "MakeOffer", sc,
	//					totalBid > (INT64)e->pchar->iInfluencePoints || totalBid <= 0 || g_auc.uiDisabled ? BUTTON_LOCKED : 0) == D_MOUSEHIT ||
	//					(uiEditHasFocus(searchResultEdit/*[i-firstItem]*/) && inpEdge(INP_RETURN)) )
	//				{
	//					if ( bid )
	//					{
	//						char *newName = NULL;
	//						switch ( g_auc.searchResults[i]->type )
	//						{
	//							case kTrayItemType_Salvage:
	//								newName = g_auc.searchResults[i]->salvage->pchName;
	//							xcase kTrayItemType_Inspiration:
	//								newName = g_auc.searchResults[i]->recipe->pchName;
	//							xcase kTrayItemType_Recipe:
	//								newName = g_auc.searchResults[i]->inspiration->pchFullName;
	//							xcase kTrayItemType_SpecializationInventory:
	//								newName = g_auc.searchResults[i]->enhancement->pchFullName;
	//							xcase kTrayItemType_Power:
	//								newName = g_auc.searchResults[i]->temp_power->pchFullName;
	//							xdefault:
	//								Errorf("Invalid TrayItemType for handleBuyMenuSearchResults: %s",TrayItemType_Str(g_auc.searchResults[i]->type));
	//						}
	//						if (g_auc.searchResults[i]->type == kTrayItemType_Salvage || g_auc.searchResults[i]->type == kTrayItemType_Recipe)
	//						{
	//							auctionHouseAddItem(g_auc.searchResults[i]->type, newName, g_auc.searchResults[i]->lvl, 0, amtToBid,
	//								bidAmt, AuctionInvItemStatus_Bidding, g_auc.searchResults[i]->id);
	//						}
	//						else
	//						{
	//							auctionHouseAddItem(g_auc.searchResults[i]->type, newName, g_auc.searchResults[i]->lvl, 0, 1,
	//								bidAmt, AuctionInvItemStatus_Bidding, g_auc.searchResults[i]->id);
	//						}
	//						uiEditClear(searchResultEdit);
	//						estrDestroy(&bid);
	//					}
	//				}
	//			}
	//		}

	//		// draw the item history
	//		{
	//			UIBox historyBox = {x + DISPLAYITEM_HISTORY_X*sc, itemFrameBox.top+(DISPLAYITEM_EDIT_Y)*sc, DISPLAYITEM_HISTORY_W*sc, DISPLAYITEM_HISTORY_H*sc };
	//			int j = 0;
	//			int histItemHt = 13;
	//			TrayItemIdentifier *tii = searchMenuHistory ? TrayItemIdentifier_Struct(searchMenuHistory->pchIdentifier) : NULL;

	//			drawFlatFrame(PIX3, R10, historyBox.x, historyBox.y, AUCTION_Z, historyBox.width+3*sc, historyBox.height+5*sc, 1.0f, 0x99, 0x99);

	//			if ( INRANGE0(g_auc.searchResults[i]->id,eaSize(&g_AuctionItemDict.items)) &&
	//				g_AuctionItemDict.items[g_auc.searchResults[i]->id]->buyPrice > 0 )
	//			{
	//				cprnt( historyBox.x + (historyBox.width/2), historyBox.y+20*sc, AUCTION_Z, sc, sc, "%s", textStd("ItemPriceFixed") );
	//			}
	//			else
	//			{
	//				historyBox.y += 2*sc;
	//				clipperPushRestrict(&historyBox);
	//				historyBox.y -= 2*sc;

	//				font(&game_9);
	//				font_color( 0xffffffff, 0xffffffff );

	//				if ( tii && g_auc.searchResults[i] && g_auc.searchResults[i]->name && stricmp(tii->name, g_auc.searchResults[i]->name) == 0 )
	//				{
	//					if ( eaSize(&searchMenuHistory->histories) <= 0 )
	//					{
	//						font_color( 0xffffff77, 0xffffff77 );
	//						cprnt( historyBox.x+(DISPLAYITEM_HISTORY_W/2)*sc, historyBox.y+(DISPLAYITEM_HISTORY_H/2)*sc, AUCTION_Z, sc, sc, "%s", textStd("NoHistory") );
	//					}
	//					else
	//					{
	//						for ( j = 0; j < eaSize(&searchMenuHistory->histories); ++j )
	//						{
	//							// remove time
	//							char buffer[80];
	//							char *c;
	//							timerMakeDateStringFromSecondsSince2000(buffer,searchMenuHistory->histories[j]->date);
	//							if( (c = strrchr(buffer,' ')) != NULL )
	//								*c = '\0';

	//							prnt( historyBox.x+5*sc, historyBox.y + (12+j*histItemHt)*sc, AUCTION_Z, sc, sc,
	//								"%s", textStd("HistoryItemStringSmall", searchMenuHistory->histories[j]->price, buffer) );
	//						}
	//					}
	//				}
	//				else
	//				{
	//					font_color( 0xffffff77, 0xffffff77 );
	//					cprnt( historyBox.x+(DISPLAYITEM_HISTORY_W/2)*sc, historyBox.y+(DISPLAYITEM_HISTORY_H/2)*sc, AUCTION_Z, sc, sc, "%s", textStd("WaitingForHistory") );
	//				}

	//				clipperPop();
	//			}
	//		}

	//		yPos += displayItemH * sc;
	//	}
	//}
	//else
	//{
	//	if ( !searchFilter.forBuy && !searchFilter.forSale )
	//	{
	//		font(&game_14);
	//		font_color( 0xaaaaaaaa, 0xaaaaaaaa );
	//		cprnt( x+(FRAME2_X+FRAME_W/2)*sc, y+(FRAME2_Y+50)*sc, AUCTION_Z, sc, sc, "NoResults");
	//	}
	//	else
	//	{
	//		font(&game_14);
	//		font_color( 0xffffffff, 0xffffffff );
	//		cprnt( x+(FRAME2_X+FRAME_W/2)*sc, y+(FRAME2_Y+50)*sc, AUCTION_Z, sc, sc, "Searching");
	//	}
	//}
	//PERFINFO_AUTO_STOP();

	//if ( selectedItemThisFrame )
	//	numItemsBeforeCurSel = numItemsBeforeCurSelThisFrame;

	//sb.use_color = 1;
	//sb.color = clr;
	//doScrollBar(&sb, (dividerOffset-32)*sc, (numItems * (DISPLAYITEM_H*sc*1.00065) + DISPLAYITEMSELECTED_H*sc),
	//	x + (FRAME2_X + FRAME_W)*sc, y + (FRAME2_Y + R12/2 + 15)*sc, AUCTION_Z, &framebox, 0);

	//searchFilter.totalProcessed += searchFilter.processedThisFrame;
	//searchFilter.processedThisFrame = 0;
	//if ( searchFilter.totalProcessed >= eaSize(&g_auc.searchResults) )
	//	searchFilter.dirty = 0;

	//PERFINFO_AUTO_STOP();
}

int allNodeChildrenAreHidden(uiTreeNode *pNode)
{
	if ( !pNode->children )
		return 0;
	else
	{
		int i = 0;

		for ( i = 0; i < eaSize(&pNode->children); ++i )
		{
			if ( !(pNode->children[i]->state&UITREENODE_HIDDEN) )
				return 0;
		}
	}
	return 1;
}

float drawSearchTreeNode(uiTreeNode *pNode, float x, float y, float z, float wd, float sc, int display)
{
	static int dosearch = 0;
	CBox box;
	int color = CLR_BLUE;

	// we decided, last frame, that we want to do an item search on the currently selected node
	if ( dosearch && rootSearchNode )
	{
		if ( eaSize(&rootSearchNode->children) )
		{
			freeAllSearchResults(1);
			getSearchFilters();
			treeAddAll(rootSearchNode);
			sortSearchResults();
		}
		else if ( ((AuctionItem*)rootSearchNode->pData)->pItem )
		{
			if ( eaSize(&g_auc.searchResults) )
				freeAllSearchResults(1);

			getSearchFilters();
			addSearchResult(rootSearchNode);
		}
		dosearch = 0;
	}

	if(pNode->state&UITREENODE_HIDDEN)
		return 0;

	wd += 10*sc;

	BuildCBox( &box, x-10*sc, y-((SEARCHTREE_NODE_HT-4)*sc), wd, SEARCHTREE_NODE_HT*sc);

	if ( pNode == rootSearchNode && display)
		drawFlatFrameBox(PIX2, R10, &box, AUCTION_Z, CLR_WHITE, 0x55aa22ff);

	if ( mouseCollision(&box) )
	{
		color = CLR_WHITE;
	}
	if( mouseClickHit( &box, MS_LEFT ) )
	{
		if ( eaSize(&pNode->children) )
		{
			pNode->state ^= UITREENODE_EXPANDED;
			uiTreeRecalculateNodeSize(pNode, wd, sc);
			if ( rootSearchNode != pNode )
			{
				rootSearchNode = pNode;
				dosearch = 1;
			}
		}
		else
		{
			if ( ((AuctionItem*)pNode->pData)->pItem)
			{
				if ( rootSearchNode != pNode )
				{
					rootSearchNode = pNode;
					dosearch = 1;
				}
			}
		}
	}

	if ( display )
	{
		int rarity = 0;

		if ( ((AuctionItem*)pNode->pData)->type == kTrayItemType_Salvage && ((AuctionItem*)pNode->pData)->salvage )
			rarity = ((AuctionItem*)pNode->pData)->salvage->rarity;
		else if ( ((AuctionItem*)pNode->pData)->type == kTrayItemType_Recipe && ((AuctionItem*)pNode->pData)->recipe )
			rarity = ((AuctionItem*)pNode->pData)->recipe->Rarity;

		color = colorFromRarity(rarity);

		font(&game_14);
		font_color( color, color );

		if ( pNode->children && !allNodeChildrenAreHidden(pNode)  )
		{
			if (pNode->state&UITREENODE_EXPANDED)
			{
				AtlasTex * leftArrow  = atlasLoadTexture( "Jelly_arrow_down.tga");

				display_sprite( leftArrow, x-1.0*sc, y-10.0*sc, z, 0.5*sc, sc, CLR_WHITE);
			} else {
				AtlasTex * leftArrow  = atlasLoadTexture( "teamup_arrow_expand.tga");	// uses same arrows as uiTray.c

				display_sprite( leftArrow, x, y-12.0*sc, z, sc, 0.4*sc, CLR_WHITE);
			}
		}

		prnt( x+12.0*sc, y, z, sc, sc, "%s", textStd(((AuctionItem*)pNode->pData)->displayName) );
	}

	pNode->heightThisNode = SEARCHTREE_NODE_HT*sc;
	return pNode->heightThisNode;
}

void *uiTreeAddFunc(char *nameParent, void *parentNode, char *name, TrayItemIdentifier *tii, void *itemdata, int id, int findParent, int hide)
{
//	uiTreeNode *node = NULL, *parent = NULL;
//	AuctionItem *data;
//
//#define INIT_NODE_COMMON(node_name, inv_type) \
//	node = uiTreeNewNode();\
//	node->fpDraw = drawSearchTreeNode;\
//	node->fpFree = freeSearchTreeNode;\
//	data = allocAuctionItem();\
//	data->name = (node_name);\
//	data->type = (inv_type);\
//	node->pData = data;
//
//	if ( !nameParent && !parentNode )
//	{
//		INIT_NODE_COMMON(name, tii?tii->type:0);
//		data->pItem = itemdata;
//		data->displayName = data->name;
//		devassert(!searchTreeRoot);
//		searchTreeRoot = node;
//		return node;
//	}
//
//	parent = parentNode ? (uiTreeNode*)parentNode : findParentNodeEx(nameParent, searchTreeRoot, 0);
//
//	if ( !parent )
//	{
//		INIT_NODE_COMMON(name, tii?tii->type:0);
//		parent = node;
//		data->displayName = data->name;
//		parent->pParent = searchTreeRoot;
//		eaPush(&searchTreeRoot->children, parent);
//	}
//
//	if ( findParent )
//		node = findParentNodeEx(name, parent, 0);
//
//	if ( !node )
//	{
//		INIT_NODE_COMMON(name, tii?tii->type:0);
//		data->pItem = itemdata;
//		data->displayName = data->name;
//		data->lvl = tii?tii->level:0;
//		data->id = id;
//		node->pParent = parent;
//		eaPush(&parent->children, node);
//	}
//
//	switch ( ((AuctionItem*)node->pData)->type )
//	{
//	case kTrayItemType_Salvage:
//		((AuctionItem*)node->pData)->displayName = ((AuctionItem*)node->pData)->salvage->ui.pchDisplayName;
//		break;
//	case kTrayItemType_Recipe:
//		((AuctionItem*)node->pData)->displayName = ((AuctionItem*)node->pData)->recipe->ui.pchDisplayName;
//		break;
//	case kTrayItemType_Inspiration:
//		((AuctionItem*)node->pData)->displayName = ((AuctionItem*)node->pData)->inspiration->pchDisplayName;
//		break;
//	case kTrayItemType_SpecializationInventory:
//		((AuctionItem*)node->pData)->displayName = ((AuctionItem*)node->pData)->enhancement->pchDisplayName;
//		break;
//	case kTrayItemType_Power:
//		((AuctionItem*)node->pData)->displayName = ((AuctionItem*)node->pData)->temp_power->pchDisplayName;
//		break;
//	case kTrayItemType_None:
//		break;
//	default:
//		Errorf("Invalid TrayItemType for uiTreeAddFunc: %s",TrayItemType_Str(((AuctionItem*)node->pData)->type));
//		break;
//	}
//
//	// Every item needs to have its name looked up and (once we've found
//	// them all) its place found so we can assign it an ordinal number.
//	auctionAddLookupEntry(name, ((AuctionItem*)(node->pData))->displayName, ((AuctionItem*)(node->pData))->lvl);
//
//	if(hide)
//		node->state |= UITREENODE_HIDDEN;
//
	return 0;
//
//#undef INIT_NODE_COMMON
}

static void auctionAddLookupEntry(char* sortName, char* displayName, int level)
{
	AuctionStringLookup* new_lookup;

	devassert(displayName);

	if (displayName)
	{
		new_lookup = MP_ALLOC(AuctionStringLookup);

		new_lookup->sortName = sortName;
		new_lookup->displayName = msGetUnformattedMessageConst(
			menuMessages, displayName);
		new_lookup->level = level;

		// Use the translated text of the display name in cases where no
		// sort name is supplied.
		if (!sortName || sortName[0] == '\0')
		{
			new_lookup->sortName = new_lookup->displayName;
		}

		eaPush(&auctionStringLookupArray, new_lookup);
	}
}

static int cmpAuctionLookupSortNames(const AuctionStringLookup** lookup1,
	const AuctionStringLookup** lookup2)
{
	int retval;

	devassert(lookup1);
	devassert(*lookup1);
	devassert(lookup2);
	devassert(*lookup2);

	retval = 0;

	if (lookup1 && *lookup1 && lookup2 && *lookup2)
	{
		retval = stricmp((*lookup1)->sortName, (*lookup2)->sortName);
	}

	return retval;
}

static int cmpAuctionLookupDisplayNames(const AuctionStringLookup** lookup1,
	const AuctionStringLookup** lookup2)
{
	int retval;

	devassert(lookup1);
	devassert(*lookup1);
	devassert(lookup2);
	devassert(*lookup2);

	retval = 0;

	if (lookup1 && *lookup1 && lookup2 && *lookup2)
	{
		retval = stricmp((*lookup1)->displayName, (*lookup2)->displayName);

		if (!retval)
		{
			retval = (*lookup1)->level - (*lookup2)->level;
		}
	}

	return retval;
}

static int searchAuctionLookupSortName(
	const char** sort_name, const AuctionStringLookup** lookup)
{
	int retval;

	devassert(lookup);
	devassert(*lookup);
	devassert(sort_name);
	devassert(*sort_name);

	retval = 0;

	if (lookup && *lookup && sort_name && *sort_name)
	{
		retval = stricmp((*sort_name), (*lookup)->sortName);
	}

	return retval;
}

static void auctionSetTreeNodeNameInfo(uiTreeNode* tree_node)
{
	//AuctionStringLookup** lookup;
	//AuctionItem* item;
	//int count;

	//devassert(tree_node);

	//if (tree_node)
	//{
	//	item = (AuctionItem*)(tree_node->pData);
	//	devassert(item);

	//	// Turn our own display name into its sequence number in the
	//	// master list, assuming we have one.
	//	if (item && item->name)
	//	{
	//		lookup = eaBSearch(auctionStringLookupArray,
	//			searchAuctionLookupSortName, item->name);
	//		item->displayNameIdx = (*lookup)->ordinal;
	//	}

	//	// If we've got any children, recurse down through them all so
	//	// everybody gets their number figured out.
	//	if (tree_node->children)
	//	{
	//		for (count = 0; count < eaSize(&(tree_node->children)); count++)
	//		{
	//			auctionSetTreeNodeNameInfo(eaGet(&(tree_node->children),
	//				count));
	//		}
	//	}
	//}
}

void auctionInitializeData(void)
{
//	int count;
//	AuctionStringLookup* asl;
//
//	if ( !g_AuctionItemDict.items )
//	{
//		MP_CREATE(AuctionStringLookup, AUCTION_STRING_LOOKUP_CHUNK_SIZE);
//		eaSetCapacity(&auctionStringLookupArray,
//			AUCTION_STRING_LOOKUP_CHUNK_SIZE);
//
//		auctiondata_Init(uiTreeAddFunc);
//
//		// Sort the pool by the translated, visible names of all the
//		// items with level as a secondary qualifier. We want every kind
//		// of similar enhancement, recipe etc to be grouped together.
//		eaQSort(auctionStringLookupArray, cmpAuctionLookupDisplayNames);
//
//		for (count = 0; count < eaSize(&auctionStringLookupArray); count++)
//		{
//			asl = eaGet(&auctionStringLookupArray, count);
//			asl->ordinal = count;
//		}
//
//		// In order to assign the numbers we have to match the unique
//		// names of the items, so we resort the list by those names.
//		eaQSort(auctionStringLookupArray, cmpAuctionLookupSortNames);
//
//		// Walk the tree from searchTreeRoot and set ordinal from
//		// sortName pointer (the actual name of each tree node). Use
//		// binary chop to make it fast.
//		auctionSetTreeNodeNameInfo(searchTreeRoot);
//
//		// Names are all copied, numbers are all assigned, we can ditch
//		// the lookup list we used to generate that numbering and names.
//		eaDestroy(&auctionStringLookupArray);
//		MP_DESTROY(AuctionStringLookup);
//	}
}

void initializeBuyMenuSearchTree()
{
	int i;

	PERFINFO_AUTO_START("initializeBuyMenuSearchTree",1);

	auctionInitializeData();
	searchTreeRoot->state = UITREENODE_EXPANDED|UITREENODE_HIDDEN;
	rootSearchNode = searchTreeRoot;
	for(i = eaSize(&searchTreeRoot->children)-1; i >= 0; --i)
		searchTreeRoot->children[i]->state = UITREENODE_EXPANDED;
	getSearchFilters();
	treeAddAll(searchTreeRoot);
	sortSearchResults();

	PERFINFO_AUTO_STOP();
}

void freeBuyMenuSearchTree()
{
	//uiTreeNode *root = searchTreeRoot, *lowestParent = NULL;
	//int i = 0, foundParent = 0;

	//while ( !lowestParent )
	//{
	//	for ( i = 0; i < eaSize(&root->children); ++i)
	//	{
	//		if ( root->children[i] && root->children[i]->children )
	//		{
	//			root = root->children[i];
	//			foundParent = 1;
	//			break;
	//		}
	//		else if ( root->children[i] && !root->children[i]->children )
	//		{
	//			lowestParent = root;
	//			foundParent = 1;
	//			break;
	//		}
	//	}
	//	if ( !foundParent )
	//		lowestParent = searchTreeRoot;
	//}
	//uiTreeFree(lowestParent);
	//if ( !foundParent )
	//	searchTreeRoot = NULL;
}

static void handleBuyMenuSearchTree(float x, float y, float z, float sc, int clr, int bkclr)
{
	//float yorig, tree_ht, height, docHt,
	//	tree_wd = (FRAME_W - 40)*sc;
	//static ScrollBar sb = { -1, 0 };
	//static int initialized = 0;
	//CBox framebox = {x+FRAME1_X*sc, y+FRAME1_Y*sc, x+(FRAME1_X+FRAME_W-5)*sc, y+(FRAME1_Y+FRAME_H)*sc};
	//UIBox uibox;

	//PERFINFO_AUTO_START("handleBuyMenuSearchTree",1);

	//if ( !initialized || eaSize(&g_auc.searchResults) == 0 )
	//{
	//	if ( htParentNodeNames )
	//	{
	//		stashTableDestroy( htParentNodeNames );
	//		htParentNodeNames = NULL;
	//	}
	//	initializeBuyMenuSearchTree();
	//	initialized = 1;
	//}

	//uiBoxFromCBox(&uibox, &framebox);

	//tree_ht = ((dividerOffset-50) - SEARCHTREE_TOP)*sc;
	//height = tree_ht;
	//y += (SEARCHTREE_TOP + 15)*sc;
	//yorig = y - sb.offset;
	//clipperPushRestrict(&uibox);
	//docHt = uiTreeDisplay(searchTreeRoot, 0, x + (FRAME1_X + 30)*sc, &yorig, y, AUCTION_Z, tree_wd, &height, sc, true);
	//clipperPop();

	//if ( tree_ht > 0.0f )
	//{
	//	sb.use_color = 1;
	//	sb.color = clr;
	//	doScrollBar(&sb, tree_ht, docHt, x + (3+FRAME1_X + FRAME_W)*sc, y, AUCTION_Z, 0, &uibox );
	//}

	//PERFINFO_AUTO_STOP();
}

void auctionHouseBuyMenu(Entity *e, float x, float y, float z, float wd, float ht, float sc, int clr, int bkclr)
{
	//static int buyMenuInitialized = 0;

	//PERFINFO_AUTO_START("auctionHouseBuyMenu",1);
	//// FRAME 1
	//if ( !buyMenuInitialized )
	//{
	//	initBuyMenuEditBoxes(z);
	//	uiEditTakeFocus(nameEdit);
	//	buyMenuInitialized = 1;
	//}
	//handleBuyMenuSearchBoxes(x, y, z, sc, clr, bkclr);
	//handleBuyMenuSearchTree(x, y, z, sc, clr, bkclr);

	//// FRAME 2
	//{
	//	font(&game_12);
	//	font_color( CLR_WHITE, CLR_WHITE );
	//	handleBuyMenuSearchResults(e, x, y, z, wd, ht, sc, clr, bkclr);
	//}
	//PERFINFO_AUTO_STOP();
}
