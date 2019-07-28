#ifndef UIRETICLE_H
#define UIRETICLE_H

#include "uiInclude.h"

typedef enum Show
{
	kShow_HideAlways,
	kShow_Always      = 0x01,
	kShow_OnMouseOver = 0x02,
	kShow_Selected    = 0x04,
} Show;

void drawStuffOnEntities(void);
void drawStuffOnPlayer(void);

void customTarget( bool bCycle, bool bBackwards, char * str );
void selectTarget( bool bCycle, bool bBackwards, int friendorfoe, int deadoralive, int basepassiveornot, 
				   int petornot, int teammateornot, char *name );

void neighborhoodCheck(void);

AtlasTex *GetArchetypeIcon(Entity *e);
AtlasTex *GetOriginIcon(Entity *e);
AtlasTex *GetAlignmentIcon(Entity *e);
AtlasTex* uiGetOriginTypeIconByName(const char* origin);
AtlasTex* uiGetArchetypeIconByName(const char* archetype);
AtlasTex* uiGetAlignmentIconByType(int alignmentType);
int getReticleColor(Entity* e, int glow, int selected, int isName);

typedef enum EBubbleType
{
	kBubbleType_Chat,
	kBubbleType_ChatContinuation,
	kBubbleType_Emote,
	kBubbleType_EmoteContinuation,
	kBubbleType_Private,
	kBubbleType_Caption,
} EBubbleType;

typedef enum EBubbleLocation
{
	kBubbleLocation_Above,
	kBubbleLocation_Top,
	kBubbleLocation_Bottom,
	kBubbleLocation_Left,
	kBubbleLocation_Right,
	kBubbleLocation_Somewhere,
} EBubbleLocation;

int drawChatBubble(EBubbleType type, EBubbleLocation location, char *pch, int x, int y, int z, float scale, int colorFG, int colorBG, int colorBorder, Vec3 headScreenPos);

#endif