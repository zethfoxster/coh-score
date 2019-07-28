#ifndef _IMAGESERVER_H
#define _IMAGESERVER_H


#include "stdtypes.h"
#include "costume.h"

#define MAX_CAPTION 200

typedef struct ImageServerParams
{
    int x_res;
	int y_res;
	int bgcolor;
	char bgtexture[256];
	char outputname[MAX_PATH];
	char caption[MAX_CAPTION];
	int headshot;
	int deleteCSV;
} ImageServerParams;


/**********************************************************************func*
 * imageserver_MakeSafeFilename
 *
 * This function will modify the given a string (which is unique among a
 *   set of strings), such that it's safe for making using as a filename.
 * The filename will also be unique assuming that the incoming string
 *   only uses alphanumerics, spaces, and the apostrophe ('), hyphen (-),
 *   and  period(.) (Hero names follow these rules.)
 */
void imageserver_MakeSafeFilename(char *pch);


/**********************************************************************func*
 * imageserver_MakeSubdirFilename
 *
 * If subDir is non-zero, creates a directory name: "pchDir/subDir/pchSafeName" + "pchAppend"
 * or "misc" if the subDir is not between 'a' and 'z' (case-insensitive)
 *
 * If useSubdir is zero, creates a directory name: "pchDir/pchSafeName" + "pchAppend"
 *
 * The returned string is local to the function; copy it away.
 *
 */
char *imageserver_MakeSubdirFilename(const char *pchDir, const char *pchSafeName, const char *pchAppend, char subDir);


void imageserver_CopyParams( ImageServerParams *dest, ImageServerParams *src );

void imageserver_InitDummyCostume( Costume * costume );

#define imageserver_ReadFromCSV(pCostume,imageParams,csvFileName) imageserver_ReadFromCSV_EX((pCostume),(imageParams),(csvFileName),0)

int imageserver_ReadFromCSV_EX( Costume ** pCostume, ImageServerParams *imageParams, char * csvFileName, char **static_strings );

int imageserver_WriteToCSV( const cCostume * pCostume, ImageServerParams *imageParams, char * csvFileName );

void imageserver_SaveShot( const cCostume * costume, ImageServerParams *imageParams, char * tgaFileAndPathName );

void imageserver_Go(void);


#endif //_IMAGESERVER_H
