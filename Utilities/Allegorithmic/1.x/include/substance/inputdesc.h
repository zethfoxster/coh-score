/** @file inputdesc.h
	@brief Substance input description structure
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceInputDesc structure. Filled via the
	substanceHandleGetInputDesc function declared in handle.h.
*/

#ifndef _SUBSTANCE_INPUTDESC_H
#define _SUBSTANCE_INPUTDESC_H


/** @brief Substance input type enumeration

	Mutually exclusive values. 4 bits format */
typedef enum
{
	Substance_IType_Float,       /**< Float (scalar) type */
	Substance_IType_Float2,      /**< 2D Float (vector) type */
	Substance_IType_Float3,      /**< 3D Float (vector) type */
	Substance_IType_Float4,      /**< 4D Float (vector) type (e.g. color) */
	Substance_IType_Integer,     /**< Integer type (int 32bits, enum or bool) */
	Substance_IType_Image,       /**< Compositing entry: bitmap/texture data */
	Substance_IType_Mesh,        /**< Mesh type */
	Substance_IType_String       /**< NULL terminated char string */
	
} SubstanceInputType;


/** @brief Substance input description structure definition

	Filled using the substanceHandleGetInputDesc function declared in handle.h. */
typedef struct
{
	/** @brief Input unique identifier */
	unsigned int inputId;
	
	/** @brief Input type */
	SubstanceInputType inputType;
	
	/** @brief Current value of the Input. Depending on the type. */
	union
	{
		const float *typeFloatX;   /**< Float, Float2, Float3, Float4 */
		const int *typeInteger;    /**< Integer input */
		const char **typeString;   /**< String input (pointer to the string) */
		/* etc. */
	} value;

} SubstanceInputDesc;

#endif /* ifndef _SUBSTANCE_INPUTDESC_H */
