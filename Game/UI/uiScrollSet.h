

// Note, this is the pristine ScrollSet Widget before all the custom crap has
// been added to make it work.  uiMissionMakerScrollSet.c/h uses a hacked up version
// and also too focus for any improvements.  This is now a nostalgia/rough sketch file
// In case we ever need a widget like this again.
// Hooray for copy/paste.

AUTO_STRUCT;
typedef struct SSElement
{
	char *pchName;
	//---Don't rearrange above this---
}SSElement;

AUTO_STRUCT;
typedef struct SSElementList
{
	char *pchName;
	SSElement **ppElement;
	int current_element;
	//---Don't rearrange above this---
}SSElementList;

AUTO_STRUCT;
typedef struct SSRegionButton
{
	char *pchName;
	SSElementList **ppElementList;
	//---Don't rearrange above this---

	int isOpen;
	F32 wd;
	F32 ht;

}SSRegionButton;

AUTO_STRUCT;
typedef struct SSRegionSet  
{
	char *pchName;
	SSRegionButton ** ppButton;
	//---Don't rearrange above this---
}SSRegionSet;

typedef struct ScrollSetRegion ScrollSetRegion;
typedef struct ScrollSet ScrollSet;

AUTO_STRUCT;
typedef struct ScrollSetRegion
{
	char *pchName;
	int current_RegionSet;
	SSRegionSet **ppRegionSet;
	//---Don't rearrange above this---


	F32 outer_ht;
	F32 inner_ht;
	F32 inner_wd;
	int isOpen;
	ScrollSetRegion **ppSubRegion;

	// Back Pointers
	ScrollSet *pParentSet;
	ScrollSetRegion *pParentRegion;

}ScrollSetRegion;

AUTO_STRUCT;
typedef struct ScrollSet
{
	F32 compressSc; 
	F32 wd;
	F32 ht;

	int current_region;
	int textColor1;
	int textColor2;
	int buttonColor;
	int frameBackColor1;
	int frameBackColor2;

	ScrollSetRegion ** ppRegion;

}ScrollSet;


void scrollSet_Test( F32 x, F32 y, F32 z, F32 wd, F32 ht, F32 sc );