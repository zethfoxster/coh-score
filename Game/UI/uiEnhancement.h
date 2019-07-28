#ifndef UIENHANCEMENT_H
#define UIENHANCEMENT_H

typedef struct ContextMenu ContextMenu;
typedef struct CBox CBox;
typedef struct TrayObj TrayObj;
typedef struct DetailRecipe DetailRecipe;
typedef struct Power Power;
typedef struct BasePower BasePower;
typedef struct Boost Boost;

#include "uiTooltip.h"

// structure used for drawing an enhancement
typedef struct uiEnhancement {
	const BasePower			*pBoostBase;		// a pointer to the base
	int						level;
	ToolTip					toolTip;
	bool					isDisplayingToolTip;
	StuffBuff				text;
	int						refresh;
	const Boost				*pBoost;
	const DetailRecipe		*pRecipe;
	int						color;
} uiEnhancement;

int enhancementWindow();
void enhancement_ContextMenu( CBox *box, TrayObj * to );
void enhancementWindowFlash(void);

//////////////////////////////////////////////////////////////////////////////////
// structure used for drawing an enhancement

const BasePower *uiEnhancementGetBase(uiEnhancement *pEnh);

uiEnhancement *uiEnhancementCreate(const char *boostName, int level);
uiEnhancement *uiEnhancementCreateFromRecipe(const DetailRecipe *recipe, int level, int bDrawEnhancementsAsEnhancements);
uiEnhancement *uiEnhancementCreateFromBoost(const Boost *boost);
void uiEnhancementFree(uiEnhancement **pEnh);
AtlasTex *uiEnhancementDrawEx(uiEnhancement *pEnh, float x, float y, float z, float scale, float textScale, int menu, int window, int showToolTip, bool bDrawCentered, float tooltipOffsetY);
#define uiEnhancementDraw(pEnh, x, y, z, scale, textScale, menu, window, showToolTip) uiEnhancementDrawEx(pEnh, x, y, z, scale, textScale, menu, window, showToolTip, 0, 0.0f)
#define uiEnhancementDrawCentered(pEnh, x, y, z, scale, textScale, menu, window, showToolTip, yOff) uiEnhancementDrawEx(pEnh, x, y, z, scale, textScale, menu, window, showToolTip, 1, yOff)
void drawEnhancementFrame( float centerX, float centerY, float z, float sc, int dragging, int withinMaxSize, int color );
void uiEnhancementDontDraw(uiEnhancement *pEnh);
void uiEnhancementRefresh(uiEnhancement *pEnh);
void uiEnhancementRefreshPower(Power *pPower);
AtlasTex *uiEnhancementGetTexture(uiEnhancement *pEnh);
AtlasTex *uiEnhancementGetPog(uiEnhancement *pEnh);
void uiEnhancementSetColor(uiEnhancement *pEnh, int color);
void uiEnhancementAddHelpText(const Boost *pBoost, const BasePower *pBase, int iLevel, StuffBuff *pBuf);
char *uiEnhancementGetToolTipText(uiEnhancement *pEnh);
void uiEnhancementGetInfoText(StuffBuff *text, const BasePower *pBase, const Boost *pBoost, int level, int bRecipe);

#endif
