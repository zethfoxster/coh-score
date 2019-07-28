/******************************************************************************
A .ANIMX is the text file format exported by the animation exporter in 3DS MAX.
The purpose of this code is to parse that source data into something we
can operate on. The name is derived from: ANIMation Export.
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include "import_animx.h"

///////////////////////////////////////////////////////////////////////////////
// ANIMX animation export format standalone code for parsing
// (does not use structparser like the GetAnimation2 tool)
///////////////////////////////////////////////////////////////////////////////

typedef struct TAnimX_Transform
{
	float vAxis[3];
	float fAngle;
	float vTranslation[3];
	float vScale[3];
} TAnimX_Transform;

typedef struct TAnimX_Bone
{
	std::string					name;
	std::vector<TAnimX_Transform>	transforms;
} TAnimX_Bone;

typedef struct TAnimX
{
	int							iVersion;
	std::string					name_source;	// MAX file animation exported from
	int							iTotalFrames;	// exported frame range, i.e. max key values a node can have, could have less
	int							iFirstFrame;

	std::vector<TAnimX_Bone>	bones;			// Array of node tracks

	void release(void)
	{
		bones.clear();
		iVersion = 0;
		name_source.clear();
		iTotalFrames = 0;
		iFirstFrame = 0;

	}
}TAnimX;

///////////////////////////////////////////////////////////////////////////////
//                             DEBUG
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                             Globals
///////////////////////////////////////////////////////////////////////////////

TAnimX g_imported_animx;

///////////////////////////////////////////////////////////////////////////////
//                             PARSER
///////////////////////////////////////////////////////////////////////////////

// trims characters in the trim set (which defaults to whitespace) from the
// beginning and end of a string
inline std::string trim(const std::string& src, const std::string& trim_set = " \t\r\n")
{
	size_t last = src.find_last_not_of(trim_set);
	if (last == std::string::npos) return std::string();
	size_t first = src.find_first_not_of(trim_set);
	if (first == std::string::npos) first = 0;
	return src.substr(first, (last-first)+1);
}

void split( std::vector<std::string>& o_tokens, const std::string& src, const std::string& delimiter )
{
	size_t  start = 0, end = 0;
	o_tokens.clear();

	while ( end != std::string::npos )
	{
		end = src.find( delimiter, start );
		o_tokens.push_back( src.substr( start, (end == std::string::npos) ? std::string::npos : end - start ) );
		start =(end > (std::string::npos - delimiter.size())) ? std::string::npos : end + delimiter.size();
	}
}

unsigned int tokenize( std::vector<std::string>& o_tokens, const std::string& src, const std::string& delimiters )
{
	size_t prev_pos = 0;
	size_t pos = 0;
	unsigned int token_count = 0;
	o_tokens.clear();
	pos = src.find_first_of(delimiters, pos);
	while (pos != std::string::npos)
	{
		std::string token = src.substr(prev_pos, pos - prev_pos);
		o_tokens.push_back(token);
		token_count++;
		prev_pos = ++pos;
		pos = src.find_first_of(delimiters, pos);
	}

	if (prev_pos < src.length())
	{
		o_tokens.push_back(src.substr(prev_pos));
		token_count++;
	}

	return token_count;
}


/******************************************************************************
	Parse the text file, returns true on success
*****************************************************************************/

static bool ParseTransform(std::ifstream& file, TAnimX_Transform& transform)
{
	bool in_struct = false;
	std::string line;
	while( std::getline( file, line ) )
	{
		line = trim(line);
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;

		if (line[0] == '{')
		{
			in_struct = true;
			continue;
		}

		if (line[0] == '}')
			return true;

		// parse fields
		std::vector<std::string> tokens;
		tokenize(tokens, line, " \t");
		if (_stricmp("Axis",tokens[0].c_str())==0)
		{
			if (tokens.size()<4)
				return false;	// malformed
			transform.vAxis[0] = (float)atof(tokens[1].c_str());
			transform.vAxis[1] = (float)atof(tokens[2].c_str());
			transform.vAxis[2] = (float)atof(tokens[3].c_str());
		}
		else if (_stricmp("Translation",tokens[0].c_str())==0)
		{
			if (tokens.size()<4)
				return false;	// malformed
			transform.vTranslation[0] = (float)atof(tokens[1].c_str());
			transform.vTranslation[1] = (float)atof(tokens[2].c_str());
			transform.vTranslation[2] = (float)atof(tokens[3].c_str());
		}
		else if (_stricmp("Scale",tokens[0].c_str())==0)
		{
			if (tokens.size()<4)
				return false;	// malformed
			transform.vScale[0] = (float)atof(tokens[1].c_str());
			transform.vScale[1] = (float)atof(tokens[2].c_str());
			transform.vScale[2] = (float)atof(tokens[3].c_str());
		}
		else if (_stricmp("Angle",tokens[0].c_str())==0)
		{
			if (tokens.size()<2)
				return false;	// malformed
			transform.fAngle = (float)atof(tokens[1].c_str());
		}
	}
	return false;
}

static bool ParseBone(std::ifstream& file, TAnimX_Bone& bone)
{
	bool in_bone = false;
	std::string line;
	while( std::getline( file, line ) )
	{
		line = trim(line);
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;

		if (line[0] == '{')
		{
			in_bone = true;
			continue;
		}

		if (line[0] == '}')
			return true;

		// header
		std::vector<std::string> tokens;
		tokenize(tokens, line, " \t");
		if (_stricmp("Transform",tokens[0].c_str())==0)
		{
			// parse a Transform structure
			TAnimX_Transform transform;
			bone.transforms.push_back(transform);
			if (!ParseTransform(file, bone.transforms.back()) )
				return false;	// malformed file
		}
		else
		{
			//skip unknown line?
		}
	}
	return false;
}

static bool ParseANIMX( const char* sourcepath, TAnimX* pAnimX ) 
{
	std::ifstream file(sourcepath);
	std::string line;
	while( std::getline( file, line ) )
	{
		line = trim(line);
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;

		// header
		std::vector<std::string> tokens;
		tokenize(tokens, line, " \t");
		if (_stricmp("Version",tokens[0].c_str())==0 && tokens.size()>1)
		{
			pAnimX->iVersion = atoi(tokens[1].c_str());
		}
		else if (_stricmp("SourceName",tokens[0].c_str())==0 && tokens.size()>1)
		{
			pAnimX->name_source = trim(tokens[1],"\""); // should probably be a join of remaining tokens
		}
		else if (_stricmp("TotalFrames",tokens[0].c_str())==0 && tokens.size()>1)
		{
			pAnimX->iTotalFrames = atoi(tokens[1].c_str());
		}
		else if (_stricmp("FirstFrame",tokens[0].c_str())==0 && tokens.size()>1)
		{
			pAnimX->iFirstFrame = atoi(tokens[1].c_str());
		}
		else if (_stricmp("Bone",tokens[0].c_str())==0 && tokens.size()>1)
		{
			// parse a Bone structure
			TAnimX_Bone bone;
			bone.name = trim(tokens[1],"\"");
			pAnimX->bones.push_back(bone);
			if (!ParseBone(file, pAnimX->bones.back()) )
				return false;	// malformed file
		}
		else
		{
			//skip unknown line?
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//                             DATA ACCESS
///////////////////////////////////////////////////////////////////////////////

#define MIN(a,b)	((a) < (b) ? (a) : (b))

/******************************************************************************
 Read the input file. Returns the number of animation frames, or 0 for error
*****************************************************************************/
unsigned int LoadAnimation( const char* sourcepath ) 
{
	FreeAnimation();

	// Read ANIMX file
	if (!ParseANIMX(sourcepath, &g_imported_animx))
	{
		printf("ERROR: Could not parse import file: '%s'\n", sourcepath );
		return 0;
	}

	// How many nodes are in this animation?
	if (g_imported_animx.bones.size() < 1)
	{
		printf("ERROR: No nodes in animation data: '%s'\n", sourcepath );
		return 0;
	}
	
	// How many frames are in this animation?
	// Note that an individual node may have less than this number of frames
	// if the node stops animating beyond some key in the move.
	unsigned int numFrames = g_imported_animx.iTotalFrames;
	if (numFrames < 2)
	{
		printf("ERROR: Not enough frames in animation data: '%s'\n", sourcepath );
		return 0;
	}

	return numFrames;
}

/******************************************************************************
	Free data from last read animation
*****************************************************************************/
void FreeAnimation( void ) 
{
	g_imported_animx.release();
}

/******************************************************************************
 Given a node name return the handle (pointer) to the information for that
 in the import data if any.
*****************************************************************************/
NodeAnimHandle GetNodeAnimHandle( const char* node_name )
{
	for (int i=0; i<(int)g_imported_animx.bones.size(); i++)
	{
		if (_stricmp(g_imported_animx.bones[i].name.c_str(), node_name)==0)
			return &g_imported_animx.bones[i];
	}
	return NULL;	// not present in data
}

/******************************************************************************
 Given a node name and a frame number return the node transform information:
	t = 3 floats for translation
	rot = 4 floats in axis angle
	scale = 3 floats

	Note that a particular node may only have real animation data out to a
	frame n < numFrames in the animation. (e.g., if the joint stopped moving
	and held its location before the move ended)

*****************************************************************************/
static float s_fail_floats[8];

float*	GetNodeFrameTranslation( NodeAnimHandle hNode, int iFrame )
{
	TAnimX_Bone* pBone = (TAnimX_Bone*)hNode;
	int iFrameMax = (int)pBone->transforms.size() - 1;
	int iFrameClamped = MIN( iFrame, iFrameMax );

	if (iFrameClamped >= 0)
	{
		float* pFloats = pBone->transforms[iFrameClamped].vTranslation;
		return pFloats;
	}
	else
	{
		return s_fail_floats;
	}
}

float*	GetNodeFrameAxisAngle( NodeAnimHandle hNode, int iFrame )
{
	TAnimX_Bone* pBone = (TAnimX_Bone*)hNode;
	int iFrameMax = (int)pBone->transforms.size() - 1;
	int iFrameClamped = MIN( iFrame, iFrameMax );

	if (iFrameClamped >= 0)
	{
		float* pFloats = pBone->transforms[iFrameClamped].vAxis;
		return pFloats;
	}
	else
	{
		return s_fail_floats;
	}
}

float*	GetNodeFrameScale( NodeAnimHandle hNode, int iFrame )
{
	TAnimX_Bone* pBone = (TAnimX_Bone*)hNode;
	int iFrameMax = (int)pBone->transforms.size() - 1;
	int iFrameClamped = MIN( iFrame, iFrameMax );

	if (iFrameClamped >= 0)
	{
		float* pFloats = pBone->transforms[iFrameClamped].vScale;
		return pFloats;
	}
	else
	{
		return s_fail_floats;
	}
}
