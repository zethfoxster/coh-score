#ifndef ENTDEBUG_H
#define ENTDEBUG_H

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct CameraInfo CameraInfo;

int entDebugMenuVisible();
void entDebugClearLines();
void entDebugAddLine(const Vec3 p1, int argb1, const Vec3 p2, int argb2);
void entDebugAddText(Vec3 pos, const char* textString, int flags);
void displayDebugInterface2D(void);
void displayDebugInterface3D(void);
void entDebugLoadFileMenus(int forced);
void entDebugReceiveCommands(Packet* pak);
void entDebugReceiveServerPerformanceUpdate(Packet* pak);
void entDebugClearServerPerformanceInfo();
void displayEntDebugInfoTextBegin();
void displayEntDebugInfoTextEnd();
void entDebugReceiveAStarRecording(Packet* pak);
void receiveEntDebugInfo(Entity* e, Packet* pak);
void receiveGlobalEntDebugInfo(Packet *pak);
void receiveDebugPower(Packet *pak);

int getDebugCameraPosition(CameraInfo* caminfo);
void updateCutSceneCameras();


void setMouseOverDebugUI(int on);
void drawFilledBox4(int x1, int y1, int x2, int y2, int argb_ul, int argb_ur, int argb_ll, int argb_lr);
void drawFilledBox(int x1, int y1, int x2, int y2, int argb);
int drawButton2D(int x1, int y1, int dx, int dy, int centered, char* text, float scale, char* command, int* setMe);
void printDebugString(	char* outputString,
						int x,
						int y,
						float scale,
						int lineHeight,
						int overridecolor,
						int defaultcolor,
						U8 alpha,
						int* getWidthHeightBuffer);
#define ALIGN_BOTTOM	0
#define ALIGN_VCENTER	(1 << 0)
#define ALIGN_TOP		(1 << 1)
#define ALIGN_LEFT		0
#define ALIGN_HCENTER	(1 << 2)
#define	ALIGN_RIGHT		(1 << 3)
void printDebugStringAlign(char* outputString,
					  int x,
					  int y,
					  int dx,
					  int dy,
					  int align,
					  float scale,
					  int lineHeight,
					  int overridecolor,
					  int defaultcolor,
					  U8 alpha);


#endif