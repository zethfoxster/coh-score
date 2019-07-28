/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef CMDPARSETYPES_H
#define CMDPARSETYPES_H

typedef enum CmdParseType 
{
	PARSETYPE_S32 = 1,
	PARSETYPE_U32,
	PARSETYPE_FLOAT,
	PARSETYPE_EULER,
	PARSETYPE_VEC3,
	PARSETYPE_MAT4,
	PARSETYPE_STR, // string that ends at whitespace or ,
	PARSETYPE_SENTENCE, // string that ends at the end of the command. Only one per command is allowed, and must be last
	PARSETYPE_TEXTPARSER, //Serialized textparser data
} CmdParseType;

#endif //CMDPARSETYPES_H
