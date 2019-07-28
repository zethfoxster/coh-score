#include "Cloth.h"
#include "ClothPrivate.h"
#include <string.h>
#include <stdlib.h>
#include "utils.h"

//////////////////////////////////////////////////////////////////////////////
// Cloth Parse Code
// A simple text stream parser used for generating a cape described by
//   a grid and a "Y Scale" which generates a trapezoid shaped cape
//////////////////////////////////////////////////////////////////////////////

static char *parse_line(char *text, char **last);
static char *parse_cmd(char *text, char **last);

static int parse_word(char *dest, int maxlen, char **last);
static int parse_int(int *dest, char **last);
static int parse_float(F32 *dest, char **last);
static int parse_vec(Vec3 dest, char **last, int max);
static int parse_pos(Vec3 dest, char **last);

#define strmatch(tok, str) (strnicmp(tok, str, strlen(str))==0)

//////////////////////////////////////////////////////////////////////////////

int ClothObjectParseText(ClothObject *obj, const char *intext)
{
	int nchars = (int)strlen(intext);
	char *text = CLOTH_MALLOC(char, nchars+1);
	memcpy(text, intext, nchars);
	text[nchars] = 0;

	{
	int curlod = -1, nextlod = 0;
	int width = 0, height = 0;
	int diagonal = 0;
	int circle = 0;
	Vec3 origin;
	F32 xscale = 1.0f;

	char *last0 = 0;
	char *line = parse_line(text, &last0);

	zeroVec3(origin);
	
	while(line)
	{
		char *last1 = 0;
		char *cmd = parse_cmd(line, &last1);

		if (!cmd || cmd[0] == '#')
		{
			// skip line
		}
		else if (strmatch(cmd, "lod"))
		{
			int lod,minsublod,maxsublod;
			if (!parse_int(&lod, &last1)) return ClothErrorf("Parsing LOD");
			if (lod != nextlod) return ClothErrorf("Unexpected LOD");
			if (!parse_int(&minsublod, &last1)) return ClothErrorf("Parsing minsublod");
			if (!parse_int(&maxsublod, &last1)) return ClothErrorf("Parsing maxsublod");
			if (maxsublod < 0 || maxsublod >= CLOTH_SUB_LOD_NUM)
				maxsublod = CLOTH_SUB_LOD_NUM-1;
			if (maxsublod < minsublod)
				maxsublod = minsublod;
			nextlod++;
			curlod = ClothObjectAddLOD(obj, minsublod, maxsublod);
			if (curlod != nextlod-1) return ClothErrorf("LOD:???");
			width = 0, height = 0;
			diagonal = 0;
			circle = 0;
			zeroVec3(origin);
			xscale = 1.0f;
		}
		else if (strmatch(cmd, "width"))
		{
			if (!parse_int(&width, &last1)) return ClothErrorf("Parsing Width");
		}
		else if (strmatch(cmd, "height"))
		{
			if (!parse_int(&height, &last1)) return ClothErrorf("Parsing Height");
		}
		else if (strmatch(cmd, "origin"))
		{
			if (!parse_pos(origin, &last1)) return ClothErrorf("Parsing Origin");
		}
		else if (strmatch(cmd, "xscale"))
		{
			if (!parse_float(&xscale, &last1)) return ClothErrorf("Parsing XScale");
		}
		else if (strmatch(cmd, "diagonal"))
		{
			if (!parse_int(&diagonal, &last1)) return ClothErrorf("Parsing Diagonal");
		}
		else if (strmatch(cmd, "color"))
		{
			//if (!parse_vec(&Color, &last1)) return ClothErrorf("Parsing Color");
		}
		else if (strmatch(cmd, "colrad"))
		{
			F32 colrad = 1.0f;
			if (!parse_float(&colrad, &last1)) return ClothErrorf("Parsing Colrad");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothSetColRad(obj->LODs[curlod]->mCloth,colrad);
		}
		else if (strmatch(cmd, "drag"))
		{
			F32 drag = 0.0f;
			if (!parse_float(&drag, &last1)) return ClothErrorf("Parsing Drag");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothSetDrag(obj->LODs[curlod]->mCloth,drag);
		}
		else if (strmatch(cmd, "gravity"))
		{
			F32 gravity = 0.0f;
			if (!parse_float(&gravity, &last1)) return ClothErrorf("Parsing Gravity");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothSetGravity(obj->LODs[curlod]->mCloth,gravity);
		}
		else if (strmatch(cmd, "circle"))
		{
			if (!parse_int(&circle, &last1)) return ClothErrorf("Parsing Circle");
		}
		else if (strmatch(cmd, "iterate"))
		{
			int niterations = -1;
			if (!parse_int(&niterations, &last1)) return ClothErrorf("Parsing Iterations");
			ClothSetNumIterations(obj->LODs[curlod]->mCloth, -1, niterations);
		}
		else if (strmatch(cmd, "constrain"))
		{
			int nc,ns=0;
			F32 scale[2];
			S32 clothflags;
			if (!parse_int(&nc, &last1)) return ClothErrorf("Parsing Constraint NConnections");
			if (parse_float(&scale[0], &last1))
				ns++;
			if (parse_float(&scale[1], &last1))
				ns++;
			clothflags = CLOTH_FLAGS_CONNECTIONS(nc);
			if (diagonal&1) clothflags |= CLOTH_FLAGS_DIAGONAL1;
			if (diagonal&2) clothflags |= CLOTH_FLAGS_DIAGONAL2;
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothCalcLengthConstraints(obj->LODs[curlod]->mCloth, clothflags, ns, scale);
			ClothCalcConstants(obj->LODs[curlod]->mCloth,1);
			ClothCopyToRenderData(obj->LODs[curlod]->mCloth, NULL, CCTR_COPY_ALL);
			ClothUpdateNormals(&obj->LODs[curlod]->mCloth->renderData);
		}
		else if (strmatch(cmd, "grid"))
		{
			Vec3 grid;
			if (!parse_pos(grid, &last1)) return ClothErrorf("Parsing Grid");
			if (!width || !height) return ClothErrorf("Parsing: Grid before height/width set");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothBuildGrid(obj->LODs[curlod]->mCloth, width, height, grid, origin, xscale, circle);
		}
		else if ((strmatch(cmd, "attach")) || (strmatch(cmd, "eyelet")))
		{
			int hookidx,x,y;
			if (!parse_int(&hookidx, &last1)) return ClothErrorf("Parsing Eyelet Hook Index");
			if (!parse_int(&x, &last1)) return ClothErrorf("Parsing Eyelet X");
			if (!parse_int(&y, &last1)) return ClothErrorf("Parsing Eyelet Y");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothAddAttachment(obj->LODs[curlod]->mCloth, y*width+x, hookidx, hookidx, 1.0f); 
		}
		else if (strmatch(cmd, "weight"))
		{
			int x,y;
			F32 weight;
			if (!parse_int(&x, &last1)) return ClothErrorf("Parsing Weight X");
			if (!parse_int(&y, &last1)) return ClothErrorf("Parsing Weight Y");
			if (!parse_float(&weight, &last1)) return ClothErrorf("Parsing Weight val");
			if (curlod < 0) return ClothErrorf("Command before LOD");
			ClothSetParticle(obj->LODs[curlod]->mCloth, y*width+x, 0, weight);
		}
		else if (strmatch(cmd, "set"))
		{
			int x,y;
			Vec3 pos;
			if (!parse_int(&x, &last1)) return ClothErrorf("Parsing Set X");
			if (!parse_int(&y, &last1)) return ClothErrorf("Parsing Set Y");
			if (!parse_vec(pos, &last1, 3)) return ClothErrorf("Parsing Set Pos");
			ClothSetParticle(obj->LODs[curlod]->mCloth, y*width+x, pos, -1.0f);
		}
		else
		{
			return ClothErrorf("Parsing Unknown Command: %s", cmd);
		}
		line = parse_line(0, &last0);
	}
	CLOTH_FREE(text);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

static const char *newline = "\n\r";
static const char *wspace = " \t=,()";

static char *parse_line(char *text, char **last)
{
	char *line = strtok_r(text, newline, last);
	return line;
}

static char *parse_cmd(char *text, char **last)
{
	char *cmd = strtok_r(text, wspace, last);
	return cmd;
}

static int parse_word(char *dest, int maxlen, char **last)
{
	char *tok;
	if (!(tok = strtok_r(0, wspace, last)))
		return 0;
	strncpy(dest, tok, maxlen);
	dest[maxlen-1] = 0;
	return 1;
}

static int parse_int(int *dest, char **last)
{
	char *tok;
	if (!(tok=strtok_r(0, wspace, last)))
		return 0;
	*dest = atoi(tok);
	return 1;
}

static int parse_float(F32 *dest, char **last)
{
	char *tok;
	if (!(tok=strtok_r(0, wspace, last)))
		return 0;
	*dest = (F32)atof(tok);
	return 1;
}

static int parse_vec(Vec3 dest, char **last, int max)
{
	char *tok;
	if (!(tok=strtok_r(0, wspace, last)))
		return 0;
	dest[0] = (F32)atof(tok);
	if (max > 1)
	{
		if (!(tok=strtok_r(0, wspace, last)))
			return 0;
		dest[1] = (F32)atof(tok);
	}
	if (max > 2)
	{
		if (!(tok=strtok_r(0, wspace, last)))
			return 0;
		dest[2] = (F32)atof(tok);
	}
	return 1;
}

static int parse_pos(Vec3 dest, char **last)
{
	int res;
	res = parse_vec(dest, last, 3);
	return res;
}

//////////////////////////////////////////////////////////////////////////////
