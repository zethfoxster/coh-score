#include "imageCapture.h"
#ifndef UIPICTUREBROWSER_H
#define UIPICTUREBROWSER_H

//MOVEMENT
typedef enum pictBrowseMovement
{
	PICTBROWSE_LEFT,
	PICTBROWSE_RIGHT,
} PictBrowseMovement;

typedef enum pictBrowseType
{
	PICTBROWSE_IMAGE = 0,
	PICTBROWSE_MOVIE = 1,
} PictBrowseType;

typedef struct PictureBrowser
{
	int state;
	int currChoice;
	int wdw;
	int changed;
	int type;
	int idx;


	float x;
	float y;
	float z;
	float sc;

	float wd;
	float ht;
	float maxWd;
	float maxHt;

	float transitionTime;	//time between a click and when an icon is finished moving.
	AtlasTex **elements;	//the should be an earray of all images appearing in the viewer.
	MMPinPData *mmdata;
	int *costumeChanges;
	int currentCostume;

}PictureBrowser;

#define PB_MM_COLOR 0x3d6151ff

PictureBrowser *picturebrowser_create();
// This is the default picture browser, it displays in the window the current selected string.  eArray of images to view is passed in here
void picturebrowser_init( PictureBrowser *cb, float x, float y, float z, float wd, float ht, AtlasTex **elements, int wdw );
void picturebrowser_setAnimationAvailable(PictureBrowser *pb, int allowAnimation);

void picturebrowser_clearElements();
void picturebrowser_clearEntities();

// for dynamically populating a browser
void picturebrowser_addPictures( PictureBrowser *cb, AtlasTex **elements);

// clear and free all of the elements
void picturebrowser_clearPictures( PictureBrowser *cb);

// clear out a pictureBrowser structure to be re-inited
void picturebrowser_destroy(PictureBrowser *cb);

// sets a specific element to be selected
void picturebrowser_setId( PictureBrowser *cb, int id);

// set the location of a picture browser
void picturebrowser_setLoc( PictureBrowser *cb, float x, float y, float z );
void picturebrowser_setScale(PictureBrowser *pb, float scale);

// Draws the picture browser
int picturebrowser_display( PictureBrowser *cb );

// Moves the picture browser's selection left or right
void picturebrowser_pan( PictureBrowser *cb, int direction);

void picturebrowser_setAnimation(int toggle);

void picturebrowser_setMMImage( PictureBrowser *pb, int *current, int *costumeList );
void picturebrowser_setMMImageCostume( PictureBrowser *pb, const cCostume *c );
void picturebrowser_setEmoteByName(PictureBrowser *pb, char * emote);

PictureBrowser *picturebrowser_getById(int id);

#endif
