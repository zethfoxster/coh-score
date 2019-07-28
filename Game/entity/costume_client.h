/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef COSTUME_CLIENT_H__
#define COSTUME_CLIENT_H__

#include "gameData/BodyPart.h"	// for BodyPart
#include "costume.h"			// for BodyType
#include "Color.h"

typedef struct Entity Entity;

extern Color costume_color[];
extern Color skin_color[];
//
// The accessors are used to set up the various attributes of the costume.
// These don't directly change what the costume looks like, they just set
// up the data structure.
//
//
// The full gamut isn't here (i.e. it's short all the Get... functions) as
// only the ones which are used are defined.
//

// Clears out costume data
//void costume_init(Entity* e);

void setGender( Entity *e, const char * gender );
// Given a string with a well-known body type name in it, set the
// entity's bodytype correctly. This is a wrapper for costume_SetBodyType.
// Primarily, this is a backwards compatibility function.

void load_ChestGeoLinkList(void);

//
// These functions are used to fetch information about the costume
//
void get_color( Entity *e, Color colorSkin, Color color1, Color color2, int *rgba1, int *rgba2, const char *pchTex );
	// given three color indexes, writes the proper color1 into rgba1 and color2 into rgba2

int costume_GetColorsLinked(Costume* costume);
	// Returns true if the colors on the costume are linked, false otherwise.

void costume_GetColors(Costume* costume, int iIdxBone, Color *pcolorSkin, Color *pcolor1, Color *pcolor2);
	// Looks up the rgba values for the particular body part.
	// Use this to get a part's "real" colors from its indicies.

int calc_tex_types1( const char *name, int *single_texture, int *dual_pass );
	// Given a texture base name, determines if is paired with any other
	// textures or if it needs two passes.

void saveVisualTextFile(void);
	// Dump a text file which describes the costume. Used from the costume
	// menu and is used in-house only.

//
// These functions actually make visual changes to the costume.
//

void costume_Apply(Entity *pe);
	// Calls lower level routines to actually make the changes made with the
	// costume_Set... functions visible. You must call this to see any
	// costume changes actually occur.

void scaleHero( Entity *e, float val );
	// Visually changes the entity's scale to val (-9 to 9 which corresponds
	// to +- 20% or so). costume_Apply does this for you automatically.
	// This does NOT set the stored scale value, it affects the visuals only.
	// Use costume_SetScale to set the stored scale value.
	// Primarily, this is a backwards compatibility function.

void doChangeGeo( Entity *e, const BodyPart *bone, const char *name, const char *tex1, const char *tex2 );
int doChangeTex( Entity *e, const BodyPart *bone, const char *name1, const char *name2 );
void doDelAllCostumeFx( Entity *e, int firstIndex);
	// Helper functions which wrap the lower-level doChangeGeo and doChangeTex.
	// These functions apply the appropriate name munging (using the
	// geometry and texture prefixes) to arrive at the full name. Then, they
	// call the low-level functions.
	// costume_Apply will call these for you automatically.

// these functions are special hacks for capes
const char ** getChestGeoLink( Entity *e );
char * getChestGeoName( Entity *e );			// there are only three real geometries, figure out which one
bool bodyPartIsLinkedToChest( const BodyPart * bp );  // only bones specifically for capes need to be mucked with


void costumereward_add( const char * name, int isPCC ); // add value to the costume reward hash table
void costumereward_remove( const char * name, int isPCC );
void costumereward_clear( int isPCC ); // clear the hash table
bool costumereward_find( const char * name, int isPCC ); // see if a value is in the hash table
bool costumereward_findall(const char **keys, int isPCC);

int determineTextureNames(const BodyPart *bone, const Appearance *appearance, const char *name1, const char *name2, char *out1, size_t out1size, char *out2, size_t out2size, bool *found1, bool *found2);

#endif /* #ifndef COSTUME_CLIENT_H__ */

/* End of File */

