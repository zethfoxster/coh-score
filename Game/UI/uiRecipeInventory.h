/***************************************************************************
*     Copyright (c) 2005-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
***************************************************************************/
#ifndef UIRECIPEINVENTORY_H
#define UIRECIPEINVENTORY_H

typedef struct DetailRecipe DetailRecipe;
typedef struct StuffBuff StuffBuff;
typedef struct SalvageInventoryItem SalvageInventoryItem;
typedef struct ToolTip ToolTip;
typedef struct SMFBlock SMFBlock;
typedef struct AtlasTex AtlasTex;
typedef struct uiEnhancement uiEnhancement;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// R E C I P E     D I S P L A Y     S T R U C T U R E S
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct treeRecipeDisplayState {
	SMFBlock				*recipeFormat;
	int						count;				// count of how many of this recipe the player has
	const DetailRecipe		*recipe;
	U32						hash;
	StuffBuff				text;
	AtlasTex				*icon;
	AtlasTex				*icon2;
	int						reformat;			// does this require reformatting?
	int						have_needed;		// do we have the salvage to make this?
	bool					have_contact;		// do we already have the Contact this recipe wants to reward?
	float					height;				// height of this recipe (split pane mode)
	uiEnhancement			*pEnhancement;		// pointer to enhancement drawing function if this recipe is an enhancement
} treeRecipeDisplayState;

extern ToolTip recipeTip;

void recipeInventoryReparse(void);
void recipe_drag(int index, AtlasTex *icon, AtlasTex *overlay, float scale);
int recipeInventoryWindow();
float uiIncarnateRecipeDraw(treeRecipeDisplayState *pState, float x, float y, float z, float width, float scale);

#if !TEST_CLIENT
AtlasTex * uiRecipeDrawIconEx(const DetailRecipe *pRec, int level, float x, float y, float z, float scale, int window, int showTooltip, int rgba, float tooltipOffsetY, int bDrawEnhancementsAsEnhancements);
#define uiRecipeDrawIcon(pRec, level, x, y, z, scale, window, showTooltip, rgba) uiRecipeDrawIconEx(pRec, level, x, y, z, scale, window, showTooltip, rgba, 0.0f, false)
void uiRecipeGetRequirementText(StuffBuff *text, const DetailRecipe *dr, int count, SalvageInventoryItem **components, 
								int *have_needed, char *blueColor, char *whiteColor, char *redColor);
#endif


#endif //UIWORKSHOP_H
