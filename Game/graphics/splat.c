#include "splat.h"
#include "mathutil.h"
#include "stdtypes.h"
#include "gridcoll.h"
#include "font.h"
#include "cmdcommon.h"
#include "fxutil.h"
#include "cmdgame.h"
#include "tex.h"
#include "model.h"
#include "tricks.h"
#include "anim.h"
#include "utils.h"
#include "renderprim.h"
//Debug 

Line	lightlines[1000];
int	lightline_count;
//End Debug

typedef struct BoxPlanes
{	
	Vec4	leftPlane;
	Vec4	rightPlane;
	Vec4	bottomPlane;
	Vec4	topPlane;
	Vec4	frontPlane;
	Vec4	backPlane;
} BoxPlanes;

//Debug 
int splatMemAlloced;

Vec3 ga;  //debug help for light lines
Vec3 gb;  //""

void splatLine( Vec3 va, Vec3 vb, U8 r, U8 b, U8 g, U8 a, F32 width )
{
	Line * line;
	if( lightline_count < 1000 ) 
	{
		line = &(lightlines[lightline_count++]);    
		copyVec3( va, line->a);    
		copyVec3( vb, line->b);   
		line->color[0] = r;    
		line->color[1] = g; 
		line->color[2] = b; 
		line->color[3] = a;
		line->width = width; 
	}
}


//v1 is on positive side of plane, v2 is on negative side of plane. Find spot line hits plane
static void getClippedVert( Vec4 plane, Vec3 v1, Vec3 v2, Vec3 intersectionVert )
{
	Vec3 sv1, sv2;
	F32 t;
	t = dotVec4Vec3(plane, v1) / (plane[0] * (v1[0] - v2[0]) + plane[1] * (v1[1] - v2[1]) + plane[2] * (v1[2] - v2[2]));
	scaleVec3( v1, (1.0f - t), sv1 );
	scaleVec3( v2, t, sv2 );
	addVec3( sv1, sv2, intersectionVert ); //TO DO pointer
}

//given plane and convex poly 'oldVerts', make 'newVerts' the polyclipped to positive side of plane.  
static int ClipPolygonAgainstPlane( Vec4 plane, int oldVertCount, Vec3 * oldVerts, Vec3 * newVerts )
{
	int newVertCount = 0;
	int negative[10]; //10 since six clips to a tri will generate max 9 verts
	F32 dist;
	int a, b;

	// Fill out negative array.  ( Classify vertices as on one side of the plane or the other )
	{
		int i, negativeCount = 0;
		for ( i = 0; i < oldVertCount; i++ )
		{
			dist = dotVec4Vec3( plane, oldVerts[i]);
			negative[i] = ( dist < 0.0f ) ? 1 : 0;
			negativeCount += ( dist < 0.0f ) ? 1 : 0;
		}
		if (negativeCount == oldVertCount) //Discard this poly if entirely outside the clip area
			return 0;
		//Shouldn't you also if (negativeCount = 0) know to just copy over the whole quad and not do any more checking?
	}

	//Passes basic tests, clip it.  a = current vert, b = previous vert
	for ( a = 0; a < oldVertCount; a++)
	{
		// b is the index of the previous vertex
		b = (a != 0) ? a - 1 : oldVertCount - 1; //neat little wrap around

		if (negative[a]) // Current vertex is on negative side of plane
		{
			if (!negative[b]) // But previous vertex is on positive side.
			{	
				getClippedVert( plane, oldVerts[b], oldVerts[a], newVerts[newVertCount] );
				newVertCount++;
			}
		}
		else // (!negative[a]) Current vertex is on positive side of plane,
		{
			if (negative[b]) // But previous vertex is on negative side.
			{
				getClippedVert( plane, oldVerts[a], oldVerts[b], newVerts[newVertCount] );
				newVertCount++;
			}
			// Include current vertex
			copyVec3( oldVerts[a], newVerts[newVertCount] );
			newVertCount++;
		}
	}

	return newVertCount; // Return number of vertices in clipped polygon
}

//ClipTriangle to splat's bounding box and add resulting poly to Splat's Tris and Verts
static void ClipTriangleAndAddToSplat( Splat * splat, BoxPlanes * planes,  Vec3 * verts )
{
	Vec3 newVerts_A[9]; //Fit tri verts into first three slots.  Each of 6 clip planes can add one vert
	Vec3 newVerts_B[9]; 
	Vec3 * newVerts;
	int newVertCnt;

	// Do Clipping //////////////////////////////
	copyVec3( verts[0], newVerts_A[0] ); //Seed tri verts into first three slots.
	copyVec3( verts[1], newVerts_A[1] ); 
	copyVec3( verts[2], newVerts_A[2] );
	newVertCnt = 3;

	newVerts = newVerts_A;     

	//*
	newVertCnt = ClipPolygonAgainstPlane( planes->leftPlane, newVertCnt, newVerts_A, newVerts_B ); //Start with tri
	newVerts = newVerts_B;   
	if ( newVertCnt != 0 )
	{
		newVertCnt	= ClipPolygonAgainstPlane( planes->rightPlane, newVertCnt, newVerts_B, newVerts_A );
		newVerts	= newVerts_A;
		if ( newVertCnt != 0 )
		{
			newVertCnt	= ClipPolygonAgainstPlane( planes->bottomPlane, newVertCnt, newVerts_A, newVerts_B );
			newVerts	= newVerts_B;
			if ( newVertCnt != 0 )
			{
				newVertCnt	= ClipPolygonAgainstPlane( planes->topPlane, newVertCnt, newVerts_B, newVerts_A );
				newVerts	= newVerts_A;
				if ( newVertCnt != 0 )
				{
					newVertCnt	= ClipPolygonAgainstPlane( planes->backPlane, newVertCnt, newVerts_A, newVerts_B );
					newVerts	= newVerts_B;
					if ( newVertCnt != 0 )
					{
						newVertCnt	= ClipPolygonAgainstPlane( planes->frontPlane, newVertCnt, newVerts_B, newVerts_A );
						newVerts	= newVerts_A;
					}
				}
			}
		}
	}
	//*/

	// Add New Tris and Verts to Splat //////////////////////////////
	//TO DO : how do you know whether to use newVertex_A or tempVertex????
	if ( newVertCnt != 0 && !( splat->vertCnt + newVertCnt >= MAX_SHADOW_TRIS ) )
	{
		Triangle * triangle;
		int oldVertCnt, a;

		//Allocate more memory if needed
		if( splat->vertCnt + newVertCnt >= splat->currMaxVerts )
		{
			//Debug 
			splatMemAlloced -= splat->currMaxVerts * sizeof( Vec3 );
			splatMemAlloced -= splat->currMaxVerts * sizeof( Vec2 );
			splatMemAlloced -= splat->currMaxVerts * sizeof( Vec2 );
			splatMemAlloced -= splat->currMaxVerts * 4 * sizeof(U8);

			while( splat->vertCnt + newVertCnt >= splat->currMaxVerts )
				splat->currMaxVerts *= 2;

			splat->verts	= realloc( splat->verts,	splat->currMaxVerts * sizeof( Vec3 ) );
			splat->sts		= realloc( splat->sts,		splat->currMaxVerts * sizeof( Vec2 ) );
			splat->sts2		= realloc( splat->sts2,		splat->currMaxVerts * sizeof( Vec2 ) );
			splat->colors	= realloc( splat->colors,	splat->currMaxVerts * 4 * sizeof(U8) );

			//Debug 
			splatMemAlloced += splat->currMaxVerts * sizeof( Vec3 );
			splatMemAlloced += splat->currMaxVerts * sizeof( Vec2 );
			splatMemAlloced += splat->currMaxVerts * sizeof( Vec2 );
			splatMemAlloced += splat->currMaxVerts * 4 * sizeof(U8);


		}
		if( splat->triCnt + (newVertCnt - 2) >= splat->currMaxTris )
		{
			//Debug 
			splatMemAlloced -= splat->currMaxTris * sizeof( Triangle );

			while( splat->triCnt + (newVertCnt - 2) >= splat->currMaxTris )
				splat->currMaxTris *= 2;

			splat->tris		= realloc( splat->tris, splat->currMaxTris * sizeof( Triangle ) );

			//Debug 
			splatMemAlloced += splat->currMaxTris * sizeof( Triangle );
		}

		// Add polygon as a triangle fan
		oldVertCnt = splat->vertCnt;

		triangle = &( splat->tris[ splat->triCnt ] );
		splat->triCnt += (newVertCnt - 2);

		for ( a = 2 ; a < newVertCnt ; a++ )
		{
			triangle->index[0] = (U16)  oldVertCnt;
			triangle->index[1] = (U16) (oldVertCnt + a - 1);
			triangle->index[2] = (U16) (oldVertCnt + a);
			triangle++;
		}

		//TO DO : this right?
		for( a = 0 ; a < newVertCnt ; a++ )
		{
			copyVec3( newVerts[ a ], splat->verts[ oldVertCnt + a ] );
		}
		splat->vertCnt = oldVertCnt + newVertCnt;
	}
}


//Given a point and a normal, calculate a plane
void calcPlane( Vec3 pointInput, Vec3 normalInput, Vec4 planeOutput )
{
	copyVec3( normalInput, planeOutput );
	planeOutput[3] = -1 * dotVec3( normalInput, pointInput ); 
}

// Assign splat->sts (texture mapping coordinates)
static void assignSts( Splat * splat, Vec3 center, Vec3 tangent, Vec3 binormal, F32 one_over_w, F32 one_over_h )
{
	int a;
	F32 s, t; 
	Vec3 v;

	for ( a = 0; a < splat->vertCnt; a++) 
	{
		subVec3( splat->verts[a], center, v );

		s = dotVec3(v, tangent)  * one_over_w + 0.5F;
		t = dotVec3(v, binormal) * one_over_h + 0.5F;   

		splat->sts[a][0] = s;     
		splat->sts[a][1] = t; 

		splat->sts2[a][0] = s; 
		splat->sts2[a][1] = t;		
	}
}

static void splatAddBasePlate( Splat * splat, Vec3 upperLeft, Vec3 upperRight, Vec3 lowerLeft, Vec3 lowerRight )
{ 
	Triangle * triangle;
	int oldVertCnt, a;

	Vec3 newVerts[4];
	int newVertCnt;

	assertmsg( 0 , "This fuction is broken, but nobody uses it anyway, to fix it add dynarray add to tri and vert count" );

	newVertCnt = 4;

	copyVec3( upperLeft, newVerts[0] );
	copyVec3( upperRight, newVerts[1] );
	copyVec3( lowerLeft, newVerts[2] );
	copyVec3( lowerRight, newVerts[3] );

	oldVertCnt = splat->vertCnt;

	if (!( oldVertCnt + newVertCnt >= MAX_SHADOW_TRIS ))
	{
		// Add polygon as a triangle fan
		triangle = &( splat->tris[ splat->triCnt ] );
		splat->triCnt += newVertCnt - 2;

		for ( a = 2 ; a < newVertCnt ; a++ )
		{
			triangle->index[0] = (U16)  oldVertCnt;
			triangle->index[1] = (U16) (oldVertCnt + a - 1);
			triangle->index[2] = (U16) (oldVertCnt + a);
			triangle++;
		}

		//TO DO : this right?
		for( a = 0 ; a < newVertCnt ; a++ )
		{
			copyVec3( newVerts[ a ], splat->verts[ oldVertCnt + a ] );
		}
		splat->vertCnt = oldVertCnt + newVertCnt;
	}
}


void drawPlane( Vec3 planePoint, Vec3 planeNormal )
{
	int i;
	Vec3 p;
	addVec3( planePoint, planeNormal, gb ); 
	drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	zeroVec3( p );

	for( i = 0 ; i < 20 ; i++ )         
	{ 
		p[0] += i*i;
		p[1] += i;
		p[2] += i*i*i;

		crossVec3( p, planeNormal, gb );
		normalVec3( gb );
		addVec3( planePoint, gb, gb );
		drawLine3DWidth( planePoint, gb, 0xffffffff, 4);
	}
}

//splat		: structure to be filled out with a nice splat. 
//coll		: structure with triangles splat might hit
//center	: center of box to clip splat to
//normal	: back toward projector to give box orientation
//tangent	: box normal tangent ( arbitary if shadows are circles, needed for cars )
//width, height, depth = dimensions of the box
static int applySplat( Splat * splat, CollInfo * coll, Vec3 center, Vec3 normal, Vec3 tangent, F32 width, F32 height, F32 depth )
{
	BoxPlanes planes;
	Vec3 binormal;
	Vec3 planePoint;
	Vec3 planeNormal;

	Splat * splat2;

	if( game_state.simpleShadowDebug )
		xyprintf( 2, 2, "Updated Splat" ); 

	//Record parameters
	splat->height	= height; 
	splat->width	= width;
	splat->depth	= depth;
	copyVec3( center, splat->center );
	copyVec3( normal, splat->normal );
	copyVec3( tangent, splat->tangent );

	crossVec3( normal, tangent, binormal );
	copyVec3( binormal, splat->binormal );

	// Calculate boundary planes
	///////////////LeftPlane 
	scaleVec3( tangent, (width * 1.0f), planePoint );  
	addVec3( center, planePoint, planePoint ); 
	scaleVec3( tangent, -1.0, planeNormal );   
	calcPlane( planePoint, planeNormal, planes.leftPlane );  

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	////////////////RightPlane
	scaleVec3( tangent, (-1 * width * 1.0f), planePoint ); 
	addVec3( center, planePoint, planePoint );  
	scaleVec3( tangent, 1.0, planeNormal );
	calcPlane( planePoint, planeNormal, planes.rightPlane ); 

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	///////////////BottomPlane
	scaleVec3( binormal, (-1 * height * 1.0f), planePoint );  
	addVec3( center, planePoint, planePoint ); 
	scaleVec3( binormal, 1.0, planeNormal );
	calcPlane( planePoint, planeNormal, planes.bottomPlane );

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	///////////////TopPlane
	scaleVec3( binormal, (height * 1.0f), planePoint ); 
	addVec3( center, planePoint, planePoint );  
	scaleVec3( binormal, -1.0, planeNormal );
	calcPlane( planePoint, planeNormal, planes.topPlane );

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	///////////////FrontPlane
	scaleVec3( normal, (depth * 0.5f), planePoint );  //*0.5?  
	addVec3( center, planePoint, planePoint ); 
	scaleVec3( normal, -1.0, planeNormal );
	calcPlane( planePoint, planeNormal, planes.frontPlane );

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	///////////////BackPlane
	scaleVec3( normal, (-0.5f * depth), planePoint );  //*0.5?
	addVec3( center, planePoint, planePoint ); 
	scaleVec3( normal, 1.0, planeNormal ); 
	calcPlane( planePoint, planeNormal, planes.backPlane ); 

	if( game_state.simpleShadowDebug & 1 )
	{
		addVec3( planePoint, planeNormal, gb );
		drawLine3DWidth( planePoint, gb, 0xffffff00, 10);
	}

	// Begin with empty mesh
	splat->vertCnt	= 0; 
	splat->triCnt	= 0;

	splat2 = splat->invertedSplat;

	if( splat2 )
	{
		splat2->vertCnt	= 0;
		splat2->triCnt	= 0;
		splat2->flags			= splat->flags;
		splat2->stAnim			= splat->stAnim;
		splat2->falloffType		= splat->falloffType;
		splat2->normalFade		= splat->normalFade;
		//fadeCenter??????	
	
		//Get  splat one's orientation
		///////////////FrontPlane
		scaleVec3( normal, (depth * 0.5), planePoint );  //*0.5? 
		addVec3( center, planePoint, planePoint ); 
		scaleVec3( normal, -1.0, planeNormal );
		calcPlane( planePoint, planeNormal, planes.frontPlane );

		///////////////BackPlane
		scaleVec3( normal, 0, planePoint );  //*0.5?
		addVec3( center, planePoint, planePoint ); 
		scaleVec3( normal, 1.0, planeNormal );
		calcPlane( planePoint, planeNormal, planes.backPlane ); 
	}
	
	//Assign splat->tris and splat->verts
	// Determine which surfaces may be affected by this splat and call ClipTriangleAndAddToSplat() for each
	{
		int i;

		for( i = 0 ; i < coll->coll_count ; i++ )  
		{
			ClipTriangleAndAddToSplat( splat, &planes, coll->tri_colls[i].verts );
		}
	}

	/////////////Splat2 //////////////////////////////////////////////
	if( splat2 )
	{
		///////////////FrontPlane
		scaleVec3( normal, 0, planePoint );  //*0.5? 
		addVec3( center, planePoint, planePoint ); 
		scaleVec3( normal, -1.0, planeNormal );
		calcPlane( planePoint, planeNormal, planes.frontPlane );

		///////////////BackPlane
		scaleVec3( normal, (-0.5 * depth), planePoint );  //*0.5?
		addVec3( center, planePoint, planePoint ); //TO DO find fadeCenter
		scaleVec3( normal, 1.0, planeNormal );
		calcPlane( planePoint, planeNormal, planes.backPlane ); 

		{
			int i;

			for( i = 0 ; i < coll->coll_count ; i++ )  
			{
				ClipTriangleAndAddToSplat( splat2, &planes, coll->tri_colls[i].verts );
			}
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////

//############################################################################
//############# Splat Shadows 
#include "seq.h"

//#define FALL_OFF_TO_MAX 100 //as you get to the MAX_SHADOW_TRIS, turn down the alpha so it won't pop out
//Debug 
extern Line	lightlines[1000];
extern int	lightline_count;
//End Debug

static void doShadowDebug( Vec3 projectionStart, Vec3 projectionEnd, CollInfo * coll, SeqInst * seq, Vec3 shadowSize )
{
	Line * line;
	Line * hitLine;
	//Line * hitLine2;
	Line * radiusLine;
	Vec3 point;
	Vec3 point2;
	int i; 
	line		= &(lightlines[lightline_count++]);  
	hitLine		= &(lightlines[lightline_count++]);  
	//hitLine2	= &(lightlines[lightline_count++]); 

	//Projection Line = Yellow
	copyVec3( projectionStart, line->a); //line->a[0]+=0.03;  line->a[1]+=1.03;      
	copyVec3( projectionEnd,   line->b); //line->b[0]+=0.03; 
	line->color[0] = 0;   
	line->color[1] = 255;
	line->color[2] = 100; 
	line->color[3] = 255;
	line->width = 4.0; 

	//Thick Gather Collision point = Blue
	copyVec3( projectionStart, hitLine->a);  //hitLine->a[1]+=2.03; 
	copyVec3( coll->mat[3],     hitLine->b);
	hitLine->color[0] = 100;
	hitLine->color[1] = 100;
	hitLine->color[2] = 255;
	hitLine->color[3] = 255;
	hitLine->width = 2.0; 

	//Normal call collision point = Red
	//copyVec3( projectionStart, hitLine2->a);     
	//copyVec3( coll2.mat[3],     hitLine2->b); 
	//hitLine2->color[0] = 255;
	//hitLine2->color[1] = 50;
	///hitLine2->color[2] = 50; 
	//hitLine2->color[3] = 255; 
	//hitLine2->width = 4.0;

	for( i = 0 ; i < 30 ; i++ ) 
	{
		point[1] = projectionStart[1];
		point[0] = projectionStart[0] + ( sin( RAD(i*12) ) * shadowSize[0] );
		point[2] = projectionStart[2] + ( cos( RAD(i*12) ) * shadowSize[2] );

		point2[1] = projectionEnd[1];
		point2[0] = projectionEnd[0] + ( sin( RAD(i*12) ) * shadowSize[0] );
		point2[2] = projectionEnd[2] + ( cos( RAD(i*12) ) * shadowSize[2] );

		radiusLine = &(lightlines[lightline_count++]);  
		copyVec3( point, radiusLine->a);
		copyVec3( point2, radiusLine->b);

		radiusLine->color[0] = 100;  
		radiusLine->color[1] = 100;
		radiusLine->color[2] = 255;
		radiusLine->color[3] = 255;
		radiusLine->width = 1.0; 
	}

	
	//Debug
	xyprintf( 80, 10, "Tris Found: %d", coll->coll_count );     
	xyprintf( 80, 11, "Start %f, %f, %f", projectionStart[0], projectionStart[1], projectionStart[2]);
	xyprintf( 80, 12, "End   %f, %f, %f", projectionEnd[0], projectionEnd[1], projectionEnd[2]);
	xyprintf( 80, 13, "Hit   %f, %f, %f", coll->mat[3][0], coll->mat[3][1], coll->mat[3][2]);
	//End Debug 
} //End Debug


static void initSplat( Splat * splat, const char * texture1, const char * texture2 )
{
	splat->currMaxVerts = 128;
	splat->currMaxTris  = 128;

	splat->verts	= calloc( splat->currMaxVerts, sizeof( Vec3 ) ); //TO DO : not big enough, my math is wrong?
	splat->sts		= calloc( splat->currMaxVerts, sizeof( Vec2 ) );
	splat->sts2		= calloc( splat->currMaxVerts, sizeof( Vec2 ) );
	splat->colors	= calloc( splat->currMaxVerts, 4 * sizeof(U8) );

	//Debug 
	splatMemAlloced += splat->currMaxVerts * sizeof( Vec3 );
	splatMemAlloced += splat->currMaxVerts * sizeof( Vec2 );
	splatMemAlloced += splat->currMaxVerts * sizeof( Vec2 );
	splatMemAlloced += splat->currMaxVerts * 4 * sizeof(U8);
	//End Debug 

	splat->tris		= calloc( splat->currMaxTris, sizeof( Triangle ) );

	//Debug 
	splatMemAlloced += splat->currMaxTris * sizeof( Triangle );
	//End debug

	splat->texture1 = texLoadBasic( texture1, TEX_LOAD_IN_BACKGROUND, TEX_FOR_UTIL); 

	///Debug 
	if( texture2 )
		splat->texture2 = texLoadBasic( texture2, TEX_LOAD_IN_BACKGROUND, TEX_FOR_UTIL);
	else
		splat->texture2 = white_tex;

	if( splat->texture1==white_tex && !strStartsWith(texture1, "white") )
	{
		printToScreenLog( 1, "FXSPLAT: Bad Texture Name %s", texture1 );
	}

	if( texture2 && splat->texture2==white_tex && !strStartsWith(texture2, "white") )
	{
		splat->texture2 = white_tex;
		printToScreenLog( 1, "FXSPLAT: Bad Texture Name %s", texture2 );
	}
	///End Debug


	//glTexParameterf( splat->texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture2->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture2->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); CHECKGL; //TO DO : Use texture trick


	//	glTexParameterf( splat->texture->target, GL_TEXTURE_WRAP_T, GL_REPEAT ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture->target, GL_TEXTURE_WRAP_S, GL_REPEAT ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture2->target, GL_TEXTURE_WRAP_T, GL_REPEAT ); CHECKGL; //TO DO : Use texture trick
	//glTexParameterf( splat->texture2->target, GL_TEXTURE_WRAP_S, GL_REPEAT ); CHECKGL; //TO DO : Use texture trick
}

MP_DEFINE(Splat);

Splat* createSplat()
{
	MP_CREATE(Splat, 100);
	
	return MP_ALLOC(Splat);
}

void destroySplat(Splat* splat)
{
	MP_FREE(Splat, splat);
}

GfxNode * initSplatNode( int * nodeId, const char * texture1, const char * texture2, int doInvertedSplat )
{
	Splat * splat;
	GfxNode * node;

	node = gfxTreeInsert( 0 );
	*nodeId	= node->unique_id;

	node->splat		= createSplat();
	splat = (Splat*)node->splat;

	initSplat( splat, texture1, texture2 );


	if( doInvertedSplat )
	{
		splat->invertedSplat = createSplat();
		initSplat( splat->invertedSplat, texture1, texture2 );
	}

	//Hack so we get through the model draw code;
	node->model = modelFind("GEO_HIPS","player_library/G_Object.geo",LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
	if(node->model)
	{
		assert(node->model->tex_count==1); // Otherwise other problems will ensue!
		node->model->flags |= OBJ_ALPHASORT;
	}

	return node;
}

static void getCenter( Vec3 start, Vec3 end, Vec3 center )
{
	subVec3( end, start, center );
	scaleVec3( center, 0.5, center );
	addVec3( start, center, center ); 
}

static void applySplatColorAndAlpha( Splat * splat, SplatParams * splatParams, F32 depth, int fadeDirection )
{
	int i, intAlpha;
	F32 y, alpha;
	F32	maxAlpha;
	Vec3 fadeCenter;
	copyVec3(splat->center, fadeCenter);

	maxAlpha = (F32)splatParams->maxAlpha / 255.0;
	for( i = 0 ; i < splat->vertCnt ; i++ )               
	{
		//TO DO these only work when looking straight down...
 		if( fadeDirection == SPLAT_FALLOFF_SHADOW )  
		{  
			y = ( splatParams->projectionStart[1] - splat->verts[i][1] ) - SHADOW_SPLAT_OFFSET; //change center?
			if( y < 0 ) //if the vert is above the projection start, fade it off quickly this is a random magic number right now,  later make it fit with front clip plane
				alpha = ( 1.0 - (ABS(y) / SHADOW_SPLAT_OFFSET ));  //0.5 is magic number so shadow fades off faster.  May need to fiddle with          
			else  //otherwise do it normallyb
			{
				alpha = ( 1.0 - (ABS(y) / (splatParams->shadowSize[1] + SHADOW_SPLAT_OFFSET) ));  //0.5 is magic number so shadow fades off faster.  May need to fiddle with          
				alpha *= alpha;
			}
		}
		else if( fadeDirection == SPLAT_FALLOFF_UP ) 
		{
			y = fadeCenter[1] - splat->verts[i][1];  
			alpha = 1.0 - (ABS(y) / depth) ; //this is really the only reasonable way to calculate this, since the fall off inside the tri is linear anyway         
		}
		else if( fadeDirection == SPLAT_FALLOFF_DOWN ) 
		{
			y = fadeCenter[1] - splat->verts[i][1]; 
			alpha = 1.0 - (ABS(y) / depth) ; //this is really the only reasonable way to calculate this, since the fall off inside the tri is linear anyway         
		}
		else if( splatParams->falloffType == SPLAT_FALLOFF_NONE ) 
		{
			alpha = 1.0;		
		}

		alpha = maxAlpha * alpha;  //scale alpha by maxAlpha
		intAlpha = round((alpha * 255));  
		intAlpha = MAX(intAlpha, 0);
		splat->colors[i*4 + 0] = splatParams->rgb[0];  //150 draws texture  
		splat->colors[i*4 + 1] = splatParams->rgb[1];  //150 draws texture  
		splat->colors[i*4 + 2] = splatParams->rgb[2];  //150 draws texture  
		splat->colors[i*4 + 3] = intAlpha;
 
		if( 1 && game_state.simpleShadowDebug ) 
			xyprintf( 80, i,  "%d %f %f %f",intAlpha, alpha, y, splat->verts[i][1]);  
	} 
}


int g_splatShadowsUpdated; //debug

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateASplat( SplatParams * splatParams )
{
	static CollInfo coll  = {0};  //Leave these alone after creation, collGrid manages them

	int success = 0;
	Vec3 normal, center, tangent;
	Splat* splat = splatParams->node->splat;
	splat->realWidth = 0.0f;
	splat->realHeight = 0.0f;

	copyVec3( splatParams->projectionStart, splat->previousStart ); 
	splat->vertCnt	= 0; 
	splat->triCnt	= 0;

	tangent[0] = 0;tangent[1] = 0;tangent[2] = 0;    

	//Entities that aren't drawn or are gone don't get drawn.
	splat->drawMe = global_state.global_frame_count; //If the splat wasn't updated, don't draw it.  
	splatParams->node->alpha = 254;   
	splatParams->node->useFlags |= GFX_USE_OBJ;
 
	coll.coll_count = 0;

	if( splatParams->maxAlpha > (U8)(MIN_SHADOW_WORTH_DRAWING * 255) )      
	{
		F32 shadowRadius;
		Mat4 mat;
		Vec3 n;
		int collFlags = 0;
 
		copyMat4( splatParams->mat, splatParams->node->mat ); //Only used for visibility
		copyMat4( splatParams->mat, mat );

		g_splatShadowsUpdated++; //Debug
  
		scaleVec3( splatParams->projectionDirection, -1, normal ); 

		mulVecMat3( normal, mat, n );
		rollMat3( RAD(90), mat );
		mulVecMat3( normal, mat, tangent );
		copyVec3( n, normal );
		normalVec3( normal );
		normalVec3( tangent );

		shadowRadius = MAX( splatParams->shadowSize[0], splatParams->shadowSize[2] ); 
		//shadowRadius /= 2.0; //Radius, not diameter

		//If the image on the texture is round, then it's OK to not bother to collect triangles in the corners if the 
		//square, resulting in a substantially smaller number of tris to collect. Important for powers splats, not so
		//much for static splats
		if( !(splatParams->flags & SPLAT_ROUNDTEXTURE) )
			shadowRadius *= 1.415;

		//Find the Collision point and the collision triangles 
		{
			Vec3 castVec;
			scaleVec3( normal, splatParams->shadowSize[1], castVec );    
			subVec3( splatParams->projectionStart, castVec, splat->projectionEnd ); 
		}

		coll.max_density = splatParams->max_density; 

		PERFINFO_AUTO_START("collGrid", 1);
 
		collFlags |= COLL_IGNOREINVISIBLE | COLL_GATHERTRIS | COLL_DISTFROMSTART | COLL_BOTHSIDES;

		if( splatParams->flags & SPLAT_CAMERA )
			collFlags |= COLL_CAMERA | COLL_PORTAL;
		else
			collFlags |= COLL_DENSITYREJECT;


		success = collGrid(0, splatParams->projectionStart, splat->projectionEnd, &coll, shadowRadius, collFlags);
		PERFINFO_AUTO_STOP();
 
		subVec3( splatParams->projectionStart, splat->projectionEnd, center ); 
		scaleVec3( center, 0.5, center );
		addVec3( splat->projectionEnd, center, center ); 

		//Debug Start
		if( game_state.simpleShadowDebug) //Debug
		{
			drawLine3DWidth( splatParams->projectionStart, splat->projectionEnd, 0xffffffff, 5 );
			addVec3( center, normal, gb );
			drawLine3DWidth( center, gb, 0xff008f00, 5 );
			addVec3( center, tangent, gb ); 
			drawLine3DWidth( center, gb, 0xff8f8fff0, 5 ); 
		}//Debug End

	} 
	//Debug if you get a collision
	if( game_state.simpleShadowDebug )    
		xyprintf( 50, 50, "COLL COUNT %d", coll.coll_count );  


	if( coll.coll_count )
	{
		splat->flags			= splatParams->flags;
		splat->stAnim			= splatParams->stAnim;
		splat->falloffType		= splatParams->falloffType;
		splat->normalFade		= splatParams->normalFade;
		splat->fadeCenter		= splatParams->fadeCenter;

		//TO DO, why does the front plane clip near the character's head instead of at the feet??
 
		//TO DO : if SPLAT_FALLOFF_BOTH, splatParams->fadeCenter to params and fix up applySplat
		PERFINFO_AUTO_START("applySplat", 1);

		applySplat(splat, &coll, center, normal, tangent, splatParams->shadowSize[0], splatParams->shadowSize[2], splatParams->shadowSize[1]);
		PERFINFO_AUTO_STOP();
	}

	if( !coll.coll_count || coll.coll_count > MAX_SHADOW_TRIS )   //or if not. since you might want a base  
	{
		splatParams->node->alpha = 0;
		((Splat*)splatParams->node->splat)->drawMe = 0; //If the splat wasn't updated, don't draw it. 
	}
}

void updateSplatTextureAndColors(SplatParams* splatParams, F32 width, F32 height)
{
	F32 depth;
	int	fadeDirection;
	F32 one_over_w = 1.0f / width;
	F32 one_over_h = 1.0 / height;
	F32 stsReapplied = 0;

	Splat* splat = splatParams->node->splat;

	if (splat->triCnt == 0 && (!splat->invertedSplat || splat->invertedSplat->triCnt == 0 ) )
	{
		splatParams->node->alpha = 0;
		splat->drawMe = 0; //If the splat wasn't updated, don't draw it. 
		return;
	}

	splat->drawMe = global_state.global_frame_count;

	if( splatParams->falloffType & SPLAT_FALLOFF_BOTH  )  //special case 
		fadeDirection = SPLAT_FALLOFF_UP;
	else
		fadeDirection = splat->falloffType;

	if (
		fabsf(splat->realWidth - width) > 0.001f
		|| fabsf(splat->realHeight - height) > 0.001f
		)
	{
		//Non inverted splat fades up if both are needed
		assignSts( splat, splat->center, splat->tangent, splat->binormal, one_over_w, one_over_h );

		splat->realWidth = width;
		splat->realHeight = height;

		if( splat->invertedSplat )
		{
			copyVec3( splat->center, splat->invertedSplat->center );
			copyVec3( splat->normal, splat->invertedSplat->normal );
			copyVec3( splat->tangent, splat->invertedSplat->tangent );

			copyVec3( splat->binormal, splat->invertedSplat->binormal );

			assignSts( splat->invertedSplat, splat->invertedSplat->center, splat->invertedSplat->tangent, splat->invertedSplat->binormal, one_over_w, one_over_h );
		}
		stsReapplied = 1;
	}

	if(1) //invertedSplat 
		depth = ABS( splat->center[1] - splatParams->projectionStart[1] );  
	else 
		depth = ABS( splatParams->projectionStart[1] - splat->projectionEnd[1] ); 

	//TO DO add or the color has changed
	if( stsReapplied || splatParams->node->alpha != splatParams->maxAlpha ) 
	{
		//TO DO : if SPLAT_FALLOFF_BOTH, splatParams->fadeCenter to params and fix up applySplatColorAndAlpha
		applySplatColorAndAlpha( splat, splatParams, depth, fadeDirection ); 

		if( splat->invertedSplat ) 
			applySplatColorAndAlpha( splat->invertedSplat, splatParams, depth, SPLAT_FALLOFF_DOWN );  //to do use better center number
	}

	splat->currMaxAlpha = splatParams->maxAlpha;
}

