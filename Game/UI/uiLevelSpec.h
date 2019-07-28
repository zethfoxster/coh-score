#ifndef UILEVELSPEC_H
#define UILEVELSPEC_H

typedef struct Character Character;
typedef struct Power Power;
typedef struct CBox CBox;
void specMenu();
void specMenu_ForceInit();

void enhancementAnim_send( Packet *pak, int debug );
void drawPowerAndEnhancements( Character *pchar, Power * pow, int x, int y, int z, float scale, int available, int *offset, int selectable, float screenScaleX, float screenScaleY, CBox *clipBox);
void enhancementAnim_update(PowerSystem eSys);

extern int gNumEnhancesLeft;

typedef struct EnhancementAnim
{
	Power	*pow;
	int     expanding;
	int     count;
	int     id[5];
	int     shrink[5];
	float   xoffset;
	float   scale[5];

}EnhancementAnim;

int enhancementAnim_add( Power * pow );
EnhancementAnim *enhancementAnim( Power *pow );
void enhancementAnim_clear();

#endif