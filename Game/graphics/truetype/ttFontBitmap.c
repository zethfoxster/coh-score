#include "truetype/ttFontBitmap.h"
#include "ttFont.h"
#include <assert.h>
#include "mathutil.h"	// for MAX() + MIN()

#include "ft2build.h"
#include <freetype/freetype.h>  // freetype library interface
#include "truetype/ttFontDraw.h"
#include "file.h"

//------------------------------------------------------------------------------
// TTFontBitmap
//------------------------------------------------------------------------------
TTFontBitmap* createTTFontBitmap(){
	return calloc(1, sizeof(TTFontBitmap));
}

void destroyTTFontBitmap(TTFontBitmap* bitmap){
	if(!bitmap)
		return;
	
	SAFE_FREE(bitmap->bitmap);
	SAFE_FREE(bitmap->bitmapRGBA);

	free(bitmap);
}

TTFontBitmap* cloneTTFontBitmap(TTFontBitmap* original){
	TTFontBitmap* clone = createTTFontBitmap();
	int bitmapMemorySize =  ttFontBitmapGetBitmapMemFootprint(original);
	memcpy(clone, original, sizeof(TTFontBitmap));
	clone->bitmap = bitmapMemorySize ? malloc(bitmapMemorySize) : NULL;
	memcpy(clone->bitmap, original->bitmap, bitmapMemorySize);
	return clone;
}

void ttFontBitmapClear(TTFontBitmap* bitmap){
	memset(bitmap->bitmap, 0, ttFontBitmapGetBitmapMemFootprint(bitmap));
}

int ttFontBitmapGetBitmapMemFootprint(TTFontBitmap* bitmap){
	return bitmap->bitmapInfo.width * bitmap->bitmapInfo.height * sizeof(unsigned short);
}


//------------------------------------------------------------------------------
// TTFontBitmap utilties
//------------------------------------------------------------------------------
#define TTF_IN_BITMAP 0
#define TTF_BEYOND_BITMAP_WIDTH 1
#define TTF_BEYOND_BITMAP_HEIGHT 2
int ttFontBitmapPointInBitmap(TTFontBitmap* bitmap, int x, int y){
	int returnVal = 0;

	if(x < 0 || x >= bitmap->bitmapInfo.width)
		returnVal |= TTF_BEYOND_BITMAP_WIDTH;

	if(y < 0 || y >= bitmap->bitmapInfo.height)
		returnVal |= TTF_BEYOND_BITMAP_HEIGHT;

	return returnVal;
}

LumAlphaPixel* ttFontGetPixelInBitmap(TTFontBitmap* bitmap, int x, int y){
	// If the specified point is not in the bitmap, return nothing.
	if(TTF_IN_BITMAP != ttFontBitmapPointInBitmap(bitmap, x, y))
		return NULL;
	else
		return (LumAlphaPixel*)(bitmap->bitmap + bitmap->bitmapInfo.width * y + x);
}


void ttFontBitmapBlit(TTFontBitmap* src, TTFontRectangle* srcRegion, TTFontBitmap* dst, TTFontRectangle* dstRegion, ttFontBitmapBlitProcessor proc){
	int rowCounter;
	int columnCounter;
	LumAlphaPixel* srcRowBuffer = (LumAlphaPixel*)src->bitmap;
	LumAlphaPixel* dstRowBuffer = (LumAlphaPixel*)dst->bitmap;
	int pointInDstBitmap;
	TTFontBBlitContext context;

	assert(srcRegion->height == dstRegion->height && srcRegion->width == dstRegion->width);
	dstRowBuffer += dst->bitmapInfo.width * dstRegion->y;
	srcRowBuffer += src->bitmapInfo.width * srcRegion->y;
	context.src = src;
	context.dst = dst;

	// For each of the rows in the region to be copied...
	for(rowCounter = 0; rowCounter < srcRegion->height; rowCounter++){

		// For each of the columns in the region to be copied...
		for(columnCounter = 0; columnCounter < srcRegion->width; columnCounter++){
			// Figure out which pixel from each image we are to operate on.
			int srcX = context.srcX = srcRegion->x + columnCounter;
			int srcY = context.srcY = srcRegion->y + rowCounter + srcRegion->y;
			int dstX = context.dstX = dstRegion->x + columnCounter;
			int dstY = context.dstY = dstRegion->y + rowCounter + dstRegion->y;
			context.srcPixel = &srcRowBuffer[srcX];
			context.dstPixel = &dstRowBuffer[dstX];

			// Are we trying to write to a valid destination pixel?
			pointInDstBitmap = ttFontBitmapPointInBitmap(dst, dstX, dstY);

			// If the point is beyond both the width and height of the dst image,
			// it will be impossible to perform any more writes into the dst image.
			// We are done.
			if((TTF_BEYOND_BITMAP_WIDTH | TTF_BEYOND_BITMAP_HEIGHT) == pointInDstBitmap)
				return;
			else if(TTF_IN_BITMAP != pointInDstBitmap)
				// If the point is not in the bitmap, then the point is is outside of the
				// bitmap in only one dimension.  It is still possible to eventually get 
				// to a point where some data can be written to the dst image.
				// Move on to the next pixel.
				continue;

			// If the source point is in the source image, then copy the point.
			if(TTF_IN_BITMAP != ttFontBitmapPointInBitmap(src, srcX, srcY)){
				dstRowBuffer[dstX].luminance = 0;
				dstRowBuffer[dstX].alpha = 0;
				break;
			}

			proc(&context);
		}

		// Move on to the next row.
		srcRowBuffer += src->bitmapInfo.width;
		dstRowBuffer += dst->bitmapInfo.width;
	}
}



//------------------------------------------------------------------------------
// TTFontBitmap blitting functions
//------------------------------------------------------------------------------
void TTBBlitCopy(TTFontBBlitContext* context){
	context->dstPixel->luminance = context->srcPixel->luminance;
	context->dstPixel->alpha = context->srcPixel->alpha;
}

void TTBBlitShadowBlend(TTFontBBlitContext* context){
	LumAlphaPixel pixel;

	pixel.luminance = 0;
	pixel.alpha = 0;

	// Here, we want to copy the luminance channel of the source image and add the
	// alpha channels.
	pixel = *(context->srcPixel);

	if(context->srcPixel->alpha == 255){
		// If the source pixel is full brght, then definitely use the source image.
		pixel.luminance = context->srcPixel->luminance;
		pixel.alpha = 255; //context->srcPixel->alpha;
	}
	else if(context->dstPixel->alpha != 0 && context->srcPixel->alpha != 0){
		pixel.alpha = MAX(context->srcPixel->alpha,context->dstPixel->alpha);
		pixel.luminance = ((unsigned int)context->srcPixel->luminance * ((float)context->srcPixel->alpha / 255) + (unsigned int)context->dstPixel->luminance * (1.0f - ((float)context->srcPixel->alpha / 255)));
	}else if (context->srcPixel->alpha != 0) {
		// If the destination pixel is not completely transparent, the use the
		// existing destination image.
		pixel.luminance = context->srcPixel->luminance;
		pixel.alpha = context->srcPixel->alpha;
	}
	else
	{
		pixel.luminance = context->dstPixel->luminance;
		pixel.alpha = context->dstPixel->alpha;
	}

	*(context->dstPixel) = pixel;
}

// Grow the bitmap by speading the bitmap one pixel.
void TTBBlitGrow(TTFontBBlitContext* context){
	static const char growDirections[][2] = {
		{-1, -1},	// upper left
		{0, -1},	// up
		{1, -1},	// upper right
		{-1, 0},	// left
		{0, 0},		// center
		{1, 0},		// right
		{-1, 1},	// lower left
		{0, 1},		// bottm
		{1, 1},		// lower right
	};

	int i;

	if(context->srcPixel->alpha == 0){
		return;
	}

	// Try to grow into the neighboring pixels.
	for(i = 0; i < sizeof(growDirections) / sizeof(growDirections[0]); i++){
		LumAlphaPixel* dstPixel;
		int targetX = context->dstX + growDirections[i][0];		
		int targetY = context->dstY + growDirections[i][1];

		dstPixel = ttFontGetPixelInBitmap(context->dst, targetX, targetY);
		if(!dstPixel)
			continue;

		dstPixel->luminance = MIN((unsigned int)dstPixel->luminance + context->srcPixel->luminance, 255);
		dstPixel->alpha = MIN((unsigned int)dstPixel->alpha + context->srcPixel->alpha, 255);
	}
}

typedef struct{
	unsigned int dimension;	// matrix width = height = dimension
	float* matrix;	// matrix data
} ConvolutionMatrix;

ConvolutionMatrix* convolutionMatrixCreate(unsigned int dimension){
	ConvolutionMatrix* matrix = calloc(1, sizeof(ConvolutionMatrix));

	// Make sure the dimension is an odd number.
	if((float)dimension / 2 == dimension / 2)
		dimension++;

	matrix->matrix = calloc(dimension * dimension, sizeof(float));
	return matrix;
}

void convolutionMatrixDestroy(ConvolutionMatrix* matrix){
	free(matrix->matrix);
	free(matrix);
}

ConvolutionMatrix* convolutionMatrixCreateGaussian(){
	return NULL;
}

//void TTFontBitmapConvolute(TTFontBitmap* src, TTFontBitmap* dst, ConvolutionMatrix* matrix){
//	int rowCounter;
//	int columnCounter;
//	LumAlphaPixel* srcRowBuffer = (LumAlphaPixel*)src->bitmap;
//	LumAlphaPixel* dstRowBuffer = (LumAlphaPixel*)dst->bitmap;
//	int pointInDstBitmap;
//	TTFontBBlitContext context;
//
//	assert(src->bitmapInfo.height == dst->bitmapInfo.height && src->bitmapInfo.width == dst->bitmapInfo.width);
//	dstRowBuffer += dst->bitmapInfo.realSize * dst->bitmapInfo.height;
//	srcRowBuffer += src->bitmapInfo.realSize * src->bitmapInfo.height;
//	context.src = src;
//	context.dst = dst;
//
//	// For each of the rows in the region to be copied...
//	for(rowCounter = 0; rowCounter < srcRegion->height; rowCounter++){
//
//		// For each of the columns in the region to be copied...
//		for(columnCounter = 0; columnCounter < src->bitmapInfo.height; columnCounter++){
//			// Figure out which pixel from each image we are to operate on.
//			int srcX = context.srcX = srcRegion->x + columnCounter;
//			int srcY = context.srcY = srcRegion->y + rowCounter + srcRegion->y;
//			int dstX = context.dstX = dstRegion->x + columnCounter;
//			int dstY = context.dstY = dstRegion->y + rowCounter + dstRegion->y;
//			context.srcPixel = &srcRowBuffer[srcX];
//			context.dstPixel = &dstRowBuffer[dstX];
//
//			{
//				int xIndex, yIndex;		// for-loop indices
//				int xOffset, yOffset;	// Offset from matrix center
//				for(yOffset = matrix->dimension/2, yIndex = 0; yIndex < matrix->dimension; yIndex++){
//					for(xOffset = matrix->dimension/2, xIndex = 0; xIndex < matrix->dimension; xIndex++){
//						LumAlphaPixel* srcPixel = ttFontGetPixelInBitmap(src, srcX + xOffset, srcY + yOffset);
//
//						// Skip all pixels referenced by the matrix that is not present in the image.
//						if(!srcPixel)
//							continue;
//
//						dstPixel.luminance += srcPixel->luminance * matrix->matrix + yIndex * matrix->dimension + xIndex;
//					}
//				}
//			}
//		}
//
//		// Move on to the next row.
//		srcRowBuffer += src->bitmapInfo.realSize;
//		dstRowBuffer += dst->bitmapInfo.realSize;
//	}
//}


typedef struct{
	unsigned char xDirection;
	unsigned char yDirection;
	float shadowReduction;
} ShadowSpreadCoef;


void TTBBlitShadowSpread(TTFontBBlitContext* context){
	static const ShadowSpreadCoef shadowDirections[] = {
		{-1, -1, 0.5},	// upper left
		{0, -1, 0.5},	// up
		{1, -1, 0.5},	// upper right
		{-1, 0, 0.5},	// left
		{0, 0, 1.0},	// center
		{1, 0, 0.5},	// right
		{-1, 1, 0.5},	// lower left
		{0, 1, 0.5},	// bottm
		{1, 1, 0.5},	// lower right
	};

	int i;

	if(context->srcPixel->alpha == 0){
		return;
	}

	for(i = 0; i < sizeof(shadowDirections) / sizeof(shadowDirections[0]); i++){
		LumAlphaPixel* dstPixel;
		int targetX = context->dstX + shadowDirections[i].xDirection;		
		int targetY = context->dstY + shadowDirections[i].yDirection;

		dstPixel = ttFontGetPixelInBitmap(context->dst, targetX, targetY);
		if(!dstPixel)
			continue;

		dstPixel->luminance = MIN((unsigned int)dstPixel->luminance + context->srcPixel->luminance, 255);
		dstPixel->alpha = MIN((unsigned int)dstPixel->alpha * shadowDirections[i].shadowReduction * 0.80 + context->srcPixel->alpha * shadowDirections[i].shadowReduction * 0.80, 255);
	}
}

void TTBBlitAdd(TTFontBBlitContext* context){
	context->dstPixel->luminance = context->srcPixel->luminance;
	context->dstPixel->alpha = MIN(255, (unsigned int)context->srcPixel->alpha + context->dstPixel->alpha);
}

void ttBitmapTurnBlack(TTFontBitmap* bitmap){
	int rowCounter;
	int columnCounter;
	unsigned short* rowBuffer = bitmap->bitmap;

	// For each of the rows in the region to be copied...
	for(rowCounter = 0; rowCounter < bitmap->bitmapInfo.height; rowCounter++){

		// For each of the columns in the region to be copied...
		for(columnCounter = 0; columnCounter < bitmap->bitmapInfo.width; columnCounter++){
			LumAlphaPixel* pixel;
			int dstX = columnCounter;

			pixel = (LumAlphaPixel*)&rowBuffer[dstX];
			// Make the entire image black.
			// Take the luminace value, zero it, and store it back.
			pixel->luminance = 0;
			pixel->alpha = pixel->alpha / 5 * 4;
		}

		// Move on to the next row.
		rowBuffer += bitmap->bitmapInfo.width;
	}
}

void ttBitmapTurnWhite(TTFontBitmap* bitmap){
	int rowCounter;
	int columnCounter;
	unsigned short* rowBuffer = bitmap->bitmap;

	// For each of the rows in the region to be copied...
	for(rowCounter = 0; rowCounter < bitmap->bitmapInfo.height; rowCounter++){

		// For each of the columns in the region to be copied...
		for(columnCounter = 0; columnCounter < bitmap->bitmapInfo.width; columnCounter++){
			LumAlphaPixel* pixel;
			int dstX = columnCounter;

			pixel = (LumAlphaPixel*)&rowBuffer[dstX];

			// Turn the pixel full white if the pixel is not full black.
			if(pixel->luminance && pixel->alpha)
			{
				pixel->luminance = 255;
				pixel->alpha = 255;
			}
		}

		// Move on to the next row.
		rowBuffer += bitmap->bitmapInfo.width;
	}
}

void ttFontBitmapAddDropShadow(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams){
	TTFontBitmap* bitmap = *origBitmap;
	TTFontBitmap* clone;
	TTFontBitmap* cloneShadow;
	TTFontBitmap* finalBitmap;
	TTFontRectangle srcRegion;
	TTFontRectangle dstRegion;
	int xOffset;
	int yOffset;
	int softShadowSpread = 0;

	if(renderParams->softShadow)
		softShadowSpread = renderParams->softShadowSpread;

	//if(softShadowSpread == 0)
	//	softShadowSpread = 2;

	// Make a bitmap that is large enough to hold the final image.
	xOffset = renderParams->dropShadowXOffset;
	yOffset =  renderParams->dropShadowYOffset;

	finalBitmap = createTTFontBitmap();
	memcpy(&finalBitmap->bitmapInfo, &bitmap->bitmapInfo, sizeof(TTFontBitmapInfo));
	finalBitmap->bitmapInfo.width += xOffset + softShadowSpread * 2;
	finalBitmap->bitmapInfo.height += yOffset + softShadowSpread * 2;

	finalBitmap->bitmap = calloc(1, finalBitmap->bitmapInfo.width * finalBitmap->bitmapInfo.height * sizeof(unsigned short));

	clone = cloneTTFontBitmap(bitmap);
	cloneShadow = cloneTTFontBitmap(bitmap);

	srcRegion.x = 0;
	srcRegion.y = 0;
	srcRegion.width = cloneShadow->bitmapInfo.width;
	srcRegion.height = cloneShadow->bitmapInfo.height;

	dstRegion.x = xOffset + softShadowSpread;
	dstRegion.y = yOffset + softShadowSpread;
	dstRegion.width = cloneShadow->bitmapInfo.width;
	dstRegion.height = cloneShadow->bitmapInfo.height;

	ttBitmapTurnBlack(cloneShadow);			// turn this bitmap into a shadow.			
	ttFontBitmapClear(bitmap);
	ttFontBitmapBlit(cloneShadow, &srcRegion, finalBitmap, &dstRegion, TTBBlitCopy);


	if(renderParams->softShadow){
		TTFontBitmap* src = finalBitmap;
		TTFontBitmap* dst = cloneTTFontBitmap(finalBitmap);
		TTFontBitmap* tmp;
		int i;

		dstRegion.x = 0;
		dstRegion.y = 0;
		dstRegion.width = finalBitmap->bitmapInfo.width;
		dstRegion.height = finalBitmap->bitmapInfo.height;
		for(i = 0; i < softShadowSpread; i++){
			ttFontBitmapBlit(src, &dstRegion, dst, &dstRegion, TTBBlitShadowSpread);
			tmp = src;
			src = dst;
			dst = tmp;
		}
		destroyTTFontBitmap(dst);
		finalBitmap = src;
	}

	dstRegion.x = softShadowSpread;
	dstRegion.y = softShadowSpread;
	dstRegion.width = cloneShadow->bitmapInfo.width;
	dstRegion.height = cloneShadow->bitmapInfo.height;
	ttFontBitmapBlit(clone, &srcRegion, finalBitmap, &dstRegion, TTBBlitShadowBlend);

	destroyTTFontBitmap(cloneShadow);
	destroyTTFontBitmap(clone);
	destroyTTFontBitmap(bitmap);
	*origBitmap = finalBitmap;
}

void ttFontBitmapAddOutline(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams){
	TTFontBitmap* bitmap = *origBitmap;
	TTFontBitmap* clone;
	TTFontBitmap* finalBitmap;

	TTFontRectangle srcRegion;
	TTFontRectangle dstRegion;
	int i;
	int growSize = renderParams->outlineWidth;

	// Default grow size is 3
	if(0 == growSize)
		growSize = 3;

	// Make a bitmap that is large enough to hold the final image.
	finalBitmap = createTTFontBitmap();
	memcpy(&finalBitmap->bitmapInfo, &bitmap->bitmapInfo, sizeof(TTFontBitmapInfo));
	finalBitmap->bitmapInfo.width += growSize * 2;
	finalBitmap->bitmapInfo.height += growSize * 2;

	finalBitmap->bitmap = calloc(1, finalBitmap->bitmapInfo.width * finalBitmap->bitmapInfo.height * sizeof(unsigned short));

	clone = cloneTTFontBitmap(bitmap);

	// Copy the shadow into the final image.
	{
		srcRegion.x = 0;
		srcRegion.y = 0;
		srcRegion.width = clone->bitmapInfo.width;
		srcRegion.height = clone->bitmapInfo.height;

		dstRegion.x = growSize;
		dstRegion.y = growSize;
		dstRegion.width = clone->bitmapInfo.width;
		dstRegion.height = clone->bitmapInfo.height;

		ttBitmapTurnBlack(clone);			// turn this bitmap into a shadow.
		ttFontBitmapBlit(clone, &srcRegion, finalBitmap, &dstRegion, TTBBlitCopy);
	}

	{
		TTFontBitmap* src = finalBitmap;
		TTFontBitmap* dst = cloneTTFontBitmap(finalBitmap);
		TTFontBitmap* tmp;

		dstRegion.x = 0;
		dstRegion.y = 0;
		dstRegion.width = finalBitmap->bitmapInfo.width;
		dstRegion.height = finalBitmap->bitmapInfo.height;
		for(i = 0; i < growSize; i++){
			ttFontBitmapBlit(src, &dstRegion, dst, &dstRegion, TTBBlitGrow);

			tmp = src;
			src = dst;
			dst = tmp;
		}

		destroyTTFontBitmap(dst);

		finalBitmap = src;

		dstRegion.x = growSize;
		dstRegion.y = growSize;
		dstRegion.width = clone->bitmapInfo.width;
		dstRegion.height = clone->bitmapInfo.height;
		ttFontBitmapBlit(bitmap, &srcRegion, finalBitmap, &dstRegion, TTBBlitShadowBlend);
	}

	destroyTTFontBitmap(clone);
	destroyTTFontBitmap(bitmap);
	*origBitmap = finalBitmap;

	finalBitmap->bitmapInfo.bitmapTop += growSize * 2;

	// JS:	Special outlining logic
	//		When drawing a string in outline mode, what we really want is a string with a
	//		outline around the string, and not a string of outlined characters.
	//		So instead of accounting for the growSize in either direction of the glyph,
	//		we intentionally only add growSize here.  When the glphys are lined up, neighboring
	//		glyph outlines would overlap, resulting in a outlined string.  The only other thing
	//		to track is to make sure the string width is calculated correctly by adding another
	//		growSize to the width after the normal measurements are done.
	finalBitmap->bitmapInfo.advanceX += growSize;
}

void ttFontBitmapMakeBold2(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams){
	TTFontBitmap* bitmap = *origBitmap;
	TTFontBitmap* finalBitmap;
	TTFontBitmap* finalBitmap2;
	TTFontRectangle srcRegion;
	TTFontRectangle dstRegion;
	int i;
	int growSize = 1;
	char growDirections[][2] = {
		{-1, -1},	// upper left
		{0, -1},	// up
		{1, -1},	// upper right
		{-1, 0},	// left
		{0, 0},		// center
		{1, 0},		// right
		{-1, 1},	// lower left
		{0, 1},		// bottm
		{1, 1},		// lower right
	};

	// Make a bitmap that is large enough to hold the final image.
	finalBitmap = createTTFontBitmap();
	memcpy(&finalBitmap->bitmapInfo, &bitmap->bitmapInfo, sizeof(TTFontBitmapInfo));
	finalBitmap->bitmapInfo.width += growSize * 2;
	finalBitmap->bitmapInfo.height += growSize * 2;
	finalBitmap->bitmap = calloc(1, finalBitmap->bitmapInfo.width * finalBitmap->bitmapInfo.height * sizeof(unsigned short));

	finalBitmap2 = cloneTTFontBitmap(finalBitmap);

	// Try to grow into the neighboring pixels.
	for(i = 0; i < sizeof(growDirections) / sizeof(growDirections[0]); i++){
		int targetX = growDirections[i][0];		
		int targetY = growDirections[i][1];

		srcRegion.x = 0;
		srcRegion.y = 0;
		srcRegion.width = bitmap->bitmapInfo.width;
		srcRegion.height = bitmap->bitmapInfo.height;

		dstRegion.x = targetX;
		dstRegion.y = targetY;
		dstRegion.width = bitmap->bitmapInfo.width;
		dstRegion.height = bitmap->bitmapInfo.height;

		ttFontBitmapBlit(bitmap, &srcRegion, finalBitmap, &dstRegion, TTBBlitAdd);
	}

	destroyTTFontBitmap(bitmap);
	*origBitmap = finalBitmap;
	finalBitmap->bitmapInfo.bitmapTop += growSize;
}


void ttFontBitmapMakeBold(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams){
	TTFontBitmap* bitmap = *origBitmap;
	TTFontBitmap* finalBitmap;
	TTFontRectangle srcRegion;
	TTFontRectangle dstRegion;
	int growSize;


	growSize = MIN(2, renderParams->renderSize * 0.2); // Default grow size is 30% of original glyph.

	// Make a bitmap that is large enough to hold the final image.
	finalBitmap = createTTFontBitmap();
	memcpy(&finalBitmap->bitmapInfo, &bitmap->bitmapInfo, sizeof(TTFontBitmapInfo));
	finalBitmap->bitmapInfo.width += growSize;

	finalBitmap->bitmap = calloc(1, finalBitmap->bitmapInfo.width * finalBitmap->bitmapInfo.height * sizeof(unsigned short));

	// Copy the bitmap into the final image.

	{
		int i;

		srcRegion.x = 0;
		srcRegion.y = 0;
		srcRegion.width = bitmap->bitmapInfo.width;
		srcRegion.height = bitmap->bitmapInfo.height;

		dstRegion.x = 0;
		dstRegion.y = 0;
		dstRegion.width = bitmap->bitmapInfo.width;
		dstRegion.height = bitmap->bitmapInfo.height;

		for(i = 0; i < growSize; i++)
		{
			dstRegion.x += 1;
			ttFontBitmapBlit(bitmap, &srcRegion, finalBitmap, &dstRegion, TTBBlitShadowBlend);
		}
	}

	destroyTTFontBitmap(bitmap);
	*origBitmap = finalBitmap;
	//finalBitmap->bitmapInfo.bitmapLeft += growSize;
	finalBitmap->bitmapInfo.advanceX += growSize;
}

void ttFontBitmapDumpToFile(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams, char* filename){
	int rowCounter;
	int columnCounter;
	LumAlphaPixel* rowBuffer = (LumAlphaPixel*)(*origBitmap)->bitmap;
	int pointInDstBitmap;
	FILE* output;

	output = fopen(filename, "at");
	if(!output)
	{
		printf("unable to dump glyph to text file");
		return;
	}

	// For each of the rows in the region to be copied...
	for(rowCounter = 0; rowCounter < (*origBitmap)->bitmapInfo.height; rowCounter++){
		// For each of the columns in the region to be copied...
		for(columnCounter = 0; columnCounter < (*origBitmap)->bitmapInfo.width; columnCounter++){
			LumAlphaPixel* pixel;
			// Figure out which pixel from each image we are to operate on.
			int x = columnCounter;
			int y = rowCounter;

			pixel = &rowBuffer[columnCounter];

			// Are we trying to write to a valid destination pixel?
			pointInDstBitmap = ttFontBitmapPointInBitmap(*origBitmap, columnCounter, rowCounter);

			// If the point is beyond both the width and height of the dst image,
			// it will be impossible to perform any more writes into the dst image.
			// We are done.
			if((TTF_BEYOND_BITMAP_WIDTH | TTF_BEYOND_BITMAP_HEIGHT) == pointInDstBitmap)
				goto exit;
			else if(TTF_IN_BITMAP != pointInDstBitmap)
				// If the point is not in the bitmap, then the point is is outside of the
				// bitmap in only one dimension.  It is still possible to eventually get 
				// to a point where some data can be written to the dst image.
				// Move on to the next pixel.
				continue;


			fprintf(output, "%2x ", pixel->alpha);
		}

		// Move on to the next row.
		rowBuffer += (*origBitmap)->bitmapInfo.width;
		fprintf(output, "\n");
	}
exit:
	fprintf(output, "\n\n");
	fclose(output);
}
