#ifndef DOUBLEFUSION_H
#define DOUBLEFUSION_H

// CoX interface to double fusion library
typedef struct BasicTexture BasicTexture;
typedef struct Model Model;
typedef struct DefTracker DefTracker;
typedef struct TraverserDrawParams TraverserDrawParams;

void doubleFusion_Init(void);
void doubleFusion_UpdateLoop(void);
void doubleFusion_Shutdown(void);

void doubleFusion_StartZone(char * pchZoneName);
void doubleFusion_ExitZone(void);

BasicTexture *doubleFusionGetLoadingScreenBasicTexture();
BasicTexture *doubleFusionGetTexture(char* pchName);

void doubleFusion_Goto( int forward, int broken );
void doubleFusion_WriteFile(char * pchName);

#endif