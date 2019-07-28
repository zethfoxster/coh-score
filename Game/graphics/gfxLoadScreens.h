
int loadingScreenVisible(void);
F32 getLoadScreenVolumeAdjustment(bool isMusic);

bool comicLoadScreenEnabled(void);
extern char **g_comicLoadScreen_pageNames;
extern int g_comicLoadScreen_pageIndex;

void gfxSetBG(const char *texname);
void gfxSetBGMapname(char* mapname);
void loadBG(void);
void showBG(void);
void showBGAlpha(int coverAlpha);
void loadScreenResetBytesLoaded(void);
void showBgReset(void);
void showBgAdd(int val);
void loadUpdate(char *msg,int num_bytes);
void finishLoadScreen(void);
void hideLoadScreen(void);
const char *getDefaultLoadScreenName(void);

typedef struct AtlasTex AtlasTex;
void setBackGroundMapDebug( AtlasTex* t1, AtlasTex *t2, AtlasTex* t3 );
