#include "fxutil.h"
#include "Color.h"
#include "camera.h"
#include "mathutil.h"
#include "assert.h"
#include "player.h"
#include <stdlib.h>
#include "utils.h"
#include "particle.h"
#include "genericlist.h"
#include "rgb_hsv.h"


//// Tools ############################################## many may be moved out


void fxFindWSMWithOffset(Mat4 result, GfxNode * node, Vec3 offset)
{
	Mat4 old_result;

	copyMat4(node->mat, result);
	addVec3(result[3], offset, result[3]);
	
	while(node->parent)
	{		
		copyMat4(result, old_result); 
		mulMat4(node->parent->mat, old_result, result);
		node = node->parent;
	}
}


F32 fxLookAt(const Vec3 eye, const Vec3 target, Mat3 result)
{
	Vec3 viewVec;
	Vec3 pyr;

	subVec3( target, eye, viewVec );    
	
	if( viewVec[0] == 0.0 && viewVec[1] == 0.0 && viewVec[2] == 0.0 )
	{
		copyMat4( unitmat, result );
		return 0; 
	}

	normalVec3( viewVec );     
	pyr[2] = 0;
	getVec3YP( viewVec, &pyr[1], &pyr[0] );  
	createMat3RYP(result,pyr); 
	return 1;
}


F32 fxFaceTheCamera(Vec3 location, Mat4 result)
{
	Vec3 temp;
	copyVec3(location, temp);
	return fxLookAt(temp, cam_info.cammat[3], result);
}

//TO Do: this isn't the greatest function ever, as gfxTreeFindRecur
//makes a strange string compare, and no consideration is given to leaving the current seq
//Also it's unused right now
//needs new place? gfxtree?
/*GfxNode * fxGetGfxNode(SeqInst * seq, char * object)
{
	if(!seq || !seq->gfx_root || !object) 
		return 0;
	
	return gfxTreeFindRecur(object, seq->gfx_root); 	
}*/

//Tools #############################################################
//move some to a general file or to mathutil or trash altogether
void quickSwap(F32 a, F32 b)
{
	F32 t;
	t=a;
	a=b;
	b=t;
}

void getRandomPointOnSphere(Vec3 initial_position, F32 radius, Vec3 final_position )
{
	static Vec3 x = {  0.371391, -0.557086, 0.742781 };
	static Vec3 y = { -0.365148,  0.182574, 0.912871 };
	static Vec3 z;

	copyVec3( x, z );
	addVec3( x, y, x );
	copyVec3( z, y );

	normalVec3(x);

	quickSwap(x[0], x[qrand()%3]);
	quickSwap(x[1], x[qrand()%3]);
	quickSwap(x[2], x[qrand()%3]);

	x[0] *= qnegrand();
	x[1] *= qnegrand();
	x[2] *= qnegrand();

	final_position[0] = initial_position[0] + x[0]*radius;
	final_position[1] = initial_position[1] + x[1]*radius;
	final_position[2] = initial_position[2] + x[2]*radius;
}


///* Untested fast Arctan...
F32 fastArcTan(F32 tan)
{
	F32 arctan;
	arctan = tan/(1.0 + (0.280872 * tan * tan));
	return arctan;
}

/*returns angle between two vectors*/
/*Does a fast arctan*/
F32 findAngle(Vec3 start, Vec3 end)
{
	Vec3 end2;
	F32  angle;
	F32  rads, tan;
	int flag = 1;

	subVec3(end, start, end2);
	if(!end2[0])
		return 64;
	tan = end2[1] / end2[0];

	//If |x| <= 1 we can use the approximation x/(1 + 0.28 * x^2), For |x|>=1 we can use the approximation pi/2 - x/(x^2 + 0.28). 
	if(tan < 0) { tan *= -1; flag = -1;} //mm monkey with this
	if( tan > 1 )
		rads = 3.14159/2 - tan/(tan * tan + 0.280872);
	else
		rads = tan /(1.0 + 0.280872 * tan * tan); 
	rads *= flag;	//mm monkey with this
	//rads = atan(tan); 
	angle = rads * 40.743679999;//(DEGREES_IN_CIRCLE * 0.5 * 1 / 3.14159)
	if(end2[0] < 0) //mm monkey with this
		angle = angle + 128; //DEGREES_IN_CIRCLE * 0.5

	return (int)angle;
}//*/


static void calculateCustomColor(Vec3* result, ColorNavPoint const* navpoint,
								 ColorPair const* custom)
{
	F32 p, s, b;

	if (custom==0 || navpoint->primaryTint==0.0f && navpoint->secondaryTint==0.0f)
	{
		setVec3(*result, navpoint->rgb[0], navpoint->rgb[1], navpoint->rgb[2]);
		return;
	}

	p = navpoint->primaryTint * 0.01f;
	s = navpoint->secondaryTint * 0.01f;
	b = 1.0f - p - s;
	p = MINMAX(p, 0.0f, 1.0f);
	s = MINMAX(s, 0.0f, 1.0f);
	b = MINMAX(b, 0.0f, 1.0f);

	if (p + s + b == 0.0f)
		b = 1.0f;
	else
	{
		F32 scale = 1.0f / (p + s + b);
		p *= scale;
		s *= scale;
		b *= scale;
	}

	setVec3(*result,
		p*custom->primary.rgb[0] + s*custom->secondary.rgb[0] + b*navpoint->rgb[0],
		p*custom->primary.rgb[1] + s*custom->secondary.rgb[1] + b*navpoint->rgb[1],
		p*custom->primary.rgb[2] + s*custom->secondary.rgb[2] + b*navpoint->rgb[2]);
}

Color sampleColorPath(ColorNavPoint const* navPoints, 
					  ColorPair const* customColors, F32 time)
{
	Color result;
	Vec3 vLower, vUpper;
	int lower = 0;
	F32 t;

	time = max(0.0f, time);

	while (lower < MAX_COLOR_NAV_POINTS - 1 && 
			navPoints[lower].time < navPoints[lower + 1].time && 
			time <= navPoints[lower + 1].time)
	{
		++lower;
	}

	if (time >= navPoints[lower].time)
	{
		Vec3 vResult;
			
		calculateCustomColor(&vResult, &navPoints[lower], customColors);
		result.r = vResult[0];
		result.g = vResult[1];
		result.b = vResult[2];
		result.a = 255;
		return result;
	}

	calculateCustomColor(&vLower, &navPoints[lower], customColors);
	calculateCustomColor(&vUpper, &navPoints[lower + 1], customColors);
	t = (time - navPoints[lower].time) / 
		(navPoints[lower + 1].time - navPoints[lower].time);
	result.r = vLower[0] + (vUpper[0] - vLower[0]) * t;
	result.g = vLower[1] + (vUpper[1] - vLower[1]) * t;
	result.b = vLower[2] + (vUpper[2] - vLower[2]) * t;
	result.a = 255;
	return result;
}

void partBuildColorPath(ColorPath * colorpath, ColorNavPoint const navpoint[], 
						ColorPair const* customColors, bool invertTint)
{
	int timeidx = 0, i, lastpoint = 0;
	F32 curr[3];
	F32 dv[3]; 
	F32 timedelta;
	U8 * path;
	Vec3 lastNav;
	
	colorpath->length = 0;

	//Get the length the highest time mentioned + 1
	for(i = 1 ; i < MAX_COLOR_NAV_POINTS && navpoint[i].time > colorpath->length ; i++)
		colorpath->length = navpoint[i].time;

	colorpath->length = colorpath->length + 1;

	/*build the path*/
	path = colorpath->path = malloc(sizeof(U8) * 3 * (colorpath->length));
	assert(path);
	timeidx	= 0;

	//loop through, Setting each navpoint color and filling gaps between navpoints:
	for( i = 0 ; i+1 < MAX_COLOR_NAV_POINTS && navpoint[i+1].time > navpoint[i].time ; i++ )
	{
		Vec3 nav0;
		Vec3 nav1;
		timedelta = (F32)(navpoint[i+1].time - navpoint[i].time);
		calculateCustomColor(&nav0, &navpoint[i], customColors);
		calculateCustomColor(&nav1, &navpoint[i+1], customColors);
		if (invertTint)
		{
			nav0[0] = 255.0f - nav0[0];
			nav0[1] = 255.0f - nav0[1];
			nav0[2] = 255.0f - nav0[2];
			nav1[0] = 255.0f - nav1[0];
			nav1[1] = 255.0f - nav1[1];
			nav1[2] = 255.0f - nav1[2];
		}
		//Set amount to increment color each 1/30th of a second
		dv[0] = ( nav1[0] - nav0[0] ) / timedelta;
		dv[1] = ( nav1[1] - nav0[1] ) / timedelta;
		dv[2] = ( nav1[2] - nav0[2] ) / timedelta;

		//Set the Color at the navpoint
		path[ timeidx * 3 + 0] = curr[0] = nav0[0];
		path[ timeidx * 3 + 1] = curr[1] = nav0[1];
		path[ timeidx * 3 + 2] = curr[2] = nav0[2];

		timeidx++;
		//Chug through 
		while( timeidx <= navpoint[i+1].time )
		{
			path[ timeidx * 3 + 0 ] = curr[0] = curr[0] + dv[0];
			path[ timeidx * 3 + 1 ] = curr[1] = curr[1] + dv[1];
			path[ timeidx * 3 + 2 ] = curr[2] = curr[2] + dv[2];
			timeidx++;
		}

		lastpoint = i+1;
	}

	calculateCustomColor(&lastNav, &navpoint[lastpoint], customColors);
	while (timeidx < colorpath->length) 
	{
		if (invertTint)
		{
			path[ timeidx * 3 + 0 ] = 255.0f - lastNav[0];
			path[ timeidx * 3 + 1 ] = 255.0f - lastNav[1];
			path[ timeidx * 3 + 2 ] = 255.0f - lastNav[2];
		}
		else
		{
			path[ timeidx * 3 + 0 ] = lastNav[0];
			path[ timeidx * 3 + 1 ] = lastNav[1];
			path[ timeidx * 3 + 2 ] = lastNav[2];
		}
		++timeidx;
	}
}

//Sort the linked list...got sort_linked_list from
//http://www.efgh.com/software/llmsort.htm
void * sort_linked_list(void *p, unsigned index,
  int (*compare)(void *, void *, void *), void *pointer, unsigned long *pcount)
{
  unsigned base;
  unsigned long block_size;

  struct record
  {
    struct record *next[1];
    /* other members not directly accessed by this function */
  };

  struct tape
  {
    struct record *first, *last;
    unsigned long count;
  } tape[4];

  /* Distribute the records alternately to tape[0] and tape[1]. */

  tape[0].count = tape[1].count = 0L;
  tape[0].first = NULL;
  base = 0;
  while (p != NULL)
  {
    struct record *next = ((struct record *)p)->next[index];
    ((struct record *)p)->next[index] = tape[base].first;
    tape[base].first = ((struct record *)p);
    tape[base].count++;
    p = next;
    base ^= 1;
  }

  /* If the list is empty or contains only a single record, then */
  /* tape[1].count == 0L and this part is vacuous.               */

  for (base = 0, block_size = 1L; tape[base+1].count != 0L;
    base ^= 2, block_size <<= 1)
  {
    int dest;
    struct tape *tape0, *tape1;
    tape0 = tape + base;
    tape1 = tape + base + 1;
    dest = base ^ 2;
    tape[dest].count = tape[dest+1].count = 0;
    for (; tape0->count != 0; dest ^= 1)
    {
      unsigned long n0, n1;
      struct tape *output_tape = tape + dest;
      n0 = n1 = block_size;
      while (1)
      {
        struct record *chosen_record;
        struct tape *chosen_tape;
        if (n0 == 0 || tape0->count == 0)
        {
          if (n1 == 0 || tape1->count == 0)
            break;
          chosen_tape = tape1;
          n1--;
        }
        else if (n1 == 0 || tape1->count == 0)
        {
          chosen_tape = tape0;
          n0--;
        }
        else if ((*compare)(tape0->first, tape1->first, pointer) > 0)
        {
          chosen_tape = tape1;
          n1--;
        }
        else
        {
          chosen_tape = tape0;
          n0--;
        }
        chosen_tape->count--;
        chosen_record = chosen_tape->first;
        chosen_tape->first = chosen_record->next[index];
        if (output_tape->count == 0)
          output_tape->first = chosen_record;
        else
          output_tape->last->next[index] = chosen_record;
        output_tape->last = chosen_record;
        output_tape->count++;
      }
    }
  }

  if (tape[base].count > 1L)
    tape[base].last->next[index] = NULL;
  if (pcount != NULL)
    *pcount = tape[base].count;
  return tape[base].first;
}


//doess this String ptr have an interesting value?
int fxHasAValue( char * string )
{
	if(string && string[0] && string[0] != '0' )
		return 1;
	return 0;
}

//Given a magniitude and a value representing what power 1.0 should be and what 10.0 should be,
//generate a power value between 1 and 10 for that magnitude
//(This is not perfect because it really scopes from 0 to 10 though 0 is not a valid power level, and that is 
//bumped to 1
U32 magnitudeToFxPower( F32 magnitude, F32 min_value, F32 max_value )
{
	F32 power, multiplier, relative_value;

	multiplier = FX_POWER_RANGE / ( max_value - min_value );
	relative_value = magnitude - min_value;
	power = (relative_value * multiplier);

	power =	MAX(1.0, power); 
	power = MIN(FX_POWER_RANGE, power );
	return (U32)(power+0.5);
}


void partCreateHueShiftedPath(ParticleSystemInfo *sysInfo, Vec3 hsvShift, 
							  ColorPath* colorPath)
{
	static const F32 oneOver255 = 1.0/255.;
	U8 *pathSrc, *pathDst;
	int i;

	colorPath->length = sysInfo->colorPathDefault.length;
	pathDst = colorPath->path = malloc(sizeof(U8) * 3 * (colorPath->length));
	pathSrc = sysInfo->colorPathDefault.path;

	for (i=0; i<sysInfo->colorPathDefault.length; i++) {
		Vec3 rgb = {pathSrc[i*3+0]*oneOver255, pathSrc[i*3+1]*oneOver255, pathSrc[i*3+2]*oneOver255};
		Vec3 hsv;
		rgbToHsv(rgb, hsv);
		hsvShiftScale(hsv, hsvShift, hsv);
		hsvToRgb(hsv, rgb);

		pathDst[i*3+0] = rgb[0]*255;
		pathDst[i*3+1] = rgb[1]*255;
		pathDst[i*3+2] = rgb[2]*255;
	}
}

void partRadialJitter(Vec3 subject, F32 angle)
{
	if ( !subject[0] && !subject[2] )
	{
		F32 fScale = subject[1];
		getRandomAngularDeflection(subject, angle);
		scaleVec3(subject, fScale, subject);
	}
}

