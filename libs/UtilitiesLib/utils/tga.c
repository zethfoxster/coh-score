#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "file.h"
#include "error.h"
#include "strings_opt.h"
#include "utils.h"
#include "tga.h"

enum
{
	TEXFMT_8BIT,
	TEXFMT_I_8,
	TEXFMT_16BIT,
	TEXFMT_ARGB_1555,
	TEXFMT_ARGB_0565,
	TEXFMT_ARGB_4444,
	TEXFMT_32BIT,
	TEXFMT_ARGB_8888,
	TEXFMT_ARGB_0888,
	TEXFMT_P_8
};

enum TGA_Code {
    TGA_OK,
    TGA_ERRCREATE,
    TGA_ERRWRITE
};

typedef struct
{
	int		width,height;
	int		format;
	int		size;
	U8		*data;
} PrivTexReadInfo;

typedef struct {
    int r, g, b, a;
} TGA_Color;


void TGA_SetImageID(char *s);
void TGA_SetAuthor(char *s);
void TGA_SetSoftwareID(char *s);
void TGA_SetAspect(int width, int height);
int  TGA_Save(
         char *filename,
         int width, int height,
         void (*getpixel)(int x, int y, TGA_Color *col)
     );


typedef struct tgaHeader{
    U8 IDLength;
    U8 CMapType;
    U8 ImgType;
    U8 CMapStartLo;
    U8 CMapStartHi;
    U8 CMapLengthLo;
    U8 CMapLengthHi;
    U8 CMapDepth;
    U8 XOffSetLo;
    U8 XOffSetHi;
    U8 YOffSetLo;
    U8 YOffSetHi;
    U8 WidthLo;
    U8 WidthHi;
    U8 HeightLo;
    U8 HeightHi;
    U8 PixelDepth;
    U8 ImageDescriptor;
} TgaHeader;

/* Definitions for image types. */
#define TGA_NULL		0
#define TGA_CMAP		1
#define TGA_TRUE		2
#define TGA_MONO		3
#define TGA_CMAP_RLE	9
#define TGA_TRUE_RLE	10
#define TGA_MONO_RLE	11


/**************************************************************************
 *
 *  FILE:           TGASAVE.C
 *
 *  MODULE OF:      TGASAVE
 *
 *  DESCRIPTION:    Routines to create a Truevision Targa (TGA) -file.
 *                  See TGASAVE.DOC for a description . . .
 *
 *                  The functions were originally written using Borland's
 *                  C-compiler on an IBM PC -compatible computer, but they
 *                  are compiled and tested on SunOS (Unix) as well.
 *
 *  WRITTEN BY:     Sverre H. Huseby
 *                  Bjoelsengt. 17
 *                  N-0468 Oslo
 *                  Norway
 *
 *                  sverrehu@ifi.uio.no
 *
 *  LAST MODIFIED:
 *
 **************************************************************************/




static char *pix_data;
static int pix_channels,pix_width;
static StuffBuff tga_data_sb;



/**************************************************************************
 *                                                                        *
 *                       P R I V A T E    D A T A                         *
 *                                                                        *
 **************************************************************************/

typedef unsigned Word;          /* At least two bytes (16 bits) */
typedef unsigned char Byte;     /* Exactly one byte (8 bits) */


/*========================================================================*
 =                                                                        =
 =                             I/O Routines                               =
 =                                                                        =
 *========================================================================*/

static FILE
    *OutFile;                   /* File to write to */


/*========================================================================*
 =                                                                        =
 =                               Main routines                            =
 =                                                                        =
 *========================================================================*/

static char
    ImageID[256],
    Author[41],
    SoftwareID[41];
static Word
    ImageHeight,
    ImageWidth,
    AspectWidth,
    AspectHeight = 0;
static void
    (*GetPixel)(int x, int y, TGA_Color *c);





/**************************************************************************
 *                                                                        *
 *                   P R I V A T E    F U N C T I O N S                   *
 *                                                                        *
 **************************************************************************/

/*========================================================================*
 =                                                                        =
 =                         Routines to do file IO                         =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME:           Create()
 *
 *  DESCRIPTION:    Creates a new file, and enables referencing using the
 *                  global variable OutFile. This variable is only used
 *                  by these IO-functions, making it relatively simple to
 *                  rewrite file IO.
 *
 *  PARAMETERS:     filename   Name of file to create.
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error opening the file.
 *
 */
static int Create(char *filename)
{
	if (stricmp(filename, "MEMORY")==0) {
		clearStuffBuff(&tga_data_sb);
		if ((OutFile = fileOpenStuffBuff(&tga_data_sb)) == NULL)
			return TGA_ERRCREATE;
	} else {
		if ((OutFile = fopen(filename, "wb")) == NULL)
			return TGA_ERRCREATE;
	}

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           Write()
 *
 *  DESCRIPTION:    Output bytes to the current OutFile.
 *
 *  PARAMETERS:     buf   Pointer to buffer to write.
 *                  len   Number of bytes to write.
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int Write(void *buf, unsigned len)
{
    if (fwrite(buf, sizeof(Byte), len, OutFile) < (int)len)
        return TGA_ERRWRITE;

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteByte()
 *
 *  DESCRIPTION:    Output one byte to the current OutFile.
 *
 *  PARAMETERS:     b   Byte to write.
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int WriteByte(Byte b)
{
    if (fputc(b, OutFile) == EOF)
        return TGA_ERRWRITE;

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteWord()
 *
 *  DESCRIPTION:    Output one word (2 bytes with byte-swapping, like on
 *                  the IBM PC) to the current OutFile.
 *
 *  PARAMETERS:     w   Word to write.
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int WriteWord(Word w)
{
    if (fputc(w & 0xFF, OutFile) == EOF)
        return TGA_ERRWRITE;

    if (fputc((w >> 8), OutFile) == EOF)
        return TGA_ERRWRITE;

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteZeros()
 *
 *  DESCRIPTION:    Output the given number of zero-bytes to the current
 *                  OutFile.
 *
 *  PARAMETERS:     n   Number of bytes to write.
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int WriteZeros(int n)
{
    while (n--)
        if (fputc(0, OutFile) == EOF)
            return TGA_ERRWRITE;

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           GetFilePos()
 *
 *  DESCRIPTION:    Get the current position in the current OutFile.
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        The byte-offset from the start of the file.
 *
 */
#ifndef _XBOX
static long GetFilePos(void)
#else
static S64 GetFilePos(void)
#endif
{
    return ftell(OutFile);
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           Close()
 *
 *  DESCRIPTION:    Close current OutFile.
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        Nothing
 *
 */
static void Close(void)
{
    fclose(OutFile);
}



/*========================================================================*
 =                                                                        =
 =                              Other routines                            =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteFileHeader()
 *
 *  DESCRIPTION:    Output the Targa file header to the current TGA-file
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int WriteFileHeader(void)
{
    /*
     *  Start with the lengt of the Image ID string.
     */
    if (WriteByte((Byte)strlen(ImageID)) != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  Then the Color Map Type. There's no color map here (0).
     */
    if (WriteByte(0) != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  And then the Image Type. Uncompressed True Color (2) is
     *  the only one supported by TGASAVE.
     */
    if (WriteByte(2) != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  The 5 Color Map Specification bytes should be 0, since we
     *  don'e use no color table.
     */
    if (WriteZeros(5) != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  Now we should output the image specification. The last byte
     *  written (32) indicates that the first pixel is upper left on
     *  the screen. TGASAVE supports 24-bit pixels only.
     */
    if (WriteWord(0) != TGA_OK || WriteWord(0) != TGA_OK)
        return TGA_ERRWRITE;
    if (WriteWord(ImageWidth) != TGA_OK || WriteWord(ImageHeight) != TGA_OK)
        return TGA_ERRWRITE;
	if (pix_channels == 3)
		WriteByte(24);
	else
		WriteByte(32);
	WriteByte(32);

    /*
     *  Now we write the Image ID string.
     */
    if (Write(ImageID, (unsigned int)strlen(ImageID)) != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  This is where the color map should go if this was not
     *  True Color. (Wery interresting...)
     */

    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteImageData()
 *
 *  DESCRIPTION:    Write the image data to the file by calling the
 *                  callback function.
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static U8 *image_data;

static int WriteImageData(void)
{
	int		num_pix = ImageWidth*ImageHeight;
	U8		*pix,*px,*p4=image_data;
	int		i;

	px = pix = malloc(ImageWidth*ImageHeight*pix_channels);
	for(i=0;i<num_pix;i++)
	{
		px[0] = p4[2];
		px[1] = p4[1];
		px[2] = p4[0];
		px+=3;
		if (pix_channels == 4)
			*px++ = p4[3];
		p4+=4;
	}
	fwrite(pix,num_pix*pix_channels,1,OutFile);
	free(pix);
#if 0
    Word x, y;
    TGA_Color col;

    for (y = 0; y < ImageHeight; y++)
        for (x = 0; x < ImageWidth; x++) {
            GetPixel(x, y, &col);
			{
				WriteByte(col.b);
				WriteByte(col.g);
				WriteByte(col.r);
				if (pix_channels == 4)
					WriteByte(col.a);
			}
        }
#endif
    return TGA_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteFileFooter()
 *
 *  DESCRIPTION:    Output the Targa file footer to the current TGA-file.
 *                  This function actually outputs all that follows the
 *                  image data.
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        TGA_OK         OK
 *                  TGA_ERRWRITE   Error writing to the file.
 *
 */
static int WriteFileFooter(void)
{
    long extoffs = 0L;          /* Extension file offset. */


    /*
     *  There's no Developer Area.
     */

    /*
     *  If any special info is given by the TGA_Set...()-functions,
     *  we need an Extension Area.
     */
    if (strlen(Author) || strlen(SoftwareID) || AspectHeight) {
        extoffs = (long)GetFilePos();

        /*
         *  The Extension Area Size is fixed.
         */
        if (WriteWord(495) != TGA_OK)
            return TGA_ERRWRITE;

        /*
         *  Output all fields. Some are not used by this program.
         */
        if (Write(Author, sizeof(Author)) != TGA_OK)
            return TGA_ERRWRITE;
        if (WriteZeros(
                324             /* Author Comments */
              +  12             /* Date/Time Stamp */
              +  41             /* Job Name/ID */
              +   6             /* Job Time */
            ) != TGA_OK)
            return TGA_ERRWRITE;
        if (Write(SoftwareID, sizeof(SoftwareID)) != TGA_OK)
            return TGA_ERRWRITE;
        if (WriteZeros(
                  3             /* Software Version */
              +   4             /* Key Color */
            ) != TGA_OK)
            return TGA_ERRWRITE;
        if (WriteWord(AspectWidth) != TGA_OK
            || WriteWord(AspectHeight) != TGA_OK)
            return TGA_ERRWRITE;
        if (WriteZeros(
                  4             /* Gamma Value */
              +   4             /* Color Correction Offset */
              +   4             /* Postage Stamp Offset */
              +   1             /* Attributs Type */
            ) != TGA_OK)
            return TGA_ERRWRITE;

    }

    /*
     *  Now we get to the actual footer. Write offset of Extension
     *  Area and Developer Area, followed by the signature.
     */
    if (WriteWord(extoffs) != TGA_OK)
        return TGA_ERRWRITE;
    if (WriteWord(0L) != TGA_OK)
        return TGA_ERRWRITE;
    if (Write("TRUEVISION-XFILE.", 18) != TGA_OK)
        return TGA_ERRWRITE;

    return TGA_OK;
}





/**************************************************************************
 *                                                                        *
 *                    P U B L I C    F U N C T I O N S                    *
 *                                                                        *
 **************************************************************************/

/*-------------------------------------------------------------------------
 *
 *  NAME:           TGA_SetImageID()
 *
 *  DESCRIPTION:    Set the image ID string of the file. This is a string
 *                  of up to 255 bytes.
 *
 *  PARAMETERS:     s   The image ID. If none is needed, this function
 *                      need not be called.
 *
 *  RETURNS:        Nothing
 *
 */
void TGA_SetImageID(char *s)
{
    memset(ImageID, 0, sizeof(ImageID));
    strncpy(ImageID, s, sizeof(ImageID) - 1);
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           TGA_SetAuthor()
 *
 *  DESCRIPTION:    Set the name of the image author. This is a string
 *                  of up to 40 bytes.
 *
 *  PARAMETERS:     s   The author name. If none is needed, this function
 *                      need not be called.
 *
 *  RETURNS:        Nothing
 *
 */
void TGA_SetAuthor(char *s)
{
    memset(Author, 0, sizeof(Author));
    strncpy(Author, s, sizeof(Author) - 1);
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           TGA_SetSoftwareID()
 *
 *  DESCRIPTION:    Set the name of the program that generated the image.
 *                  This is a string of up to 40 bytes.
 *
 *  PARAMETERS:     s   The program name. If none is needed, this function
 *                      need not be called.
 *
 *  RETURNS:        Nothing
 *
 */
void TGA_SetSoftwareID(char *s)
{
    memset(SoftwareID, 0, sizeof(SoftwareID));
    strncpy(SoftwareID, s, sizeof(SoftwareID) - 1);
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           TGA_SetAspect()
 *
 *  DESCRIPTION:    Set the pixel aspect ratio.
 *
 *  PARAMETERS:     width    Pixel width.
 *                  height   Pixel height.
 *
 *                  If no aspect ratio is needed, this function need not
 *                  be called.
 *
 *                  If width==height, the pixels are square.
 *
 *  RETURNS:        Nothing
 *
 */
void TGA_SetAspect(int width, int height)
{
    AspectWidth = width;
    AspectHeight = height;
}



/*-------------------------------------------------------------------------
 *
 *  NAME:           TGA_Save()
 *
 *  DESCRIPTION:    Create the TGA-file.
 *
 *                  The pixels are retrieved using a user defined callback
 *                  function. This function should accept three parameters,
 *                  x and y, specifying which pixel to retrieve, and col,
 *                  a pointer to TGA_Color -structure to fill in for return.
 *                  The pixel values sent to this function are as follows:
 *
 *                    x : [0, ImageWidth - 1]
 *                    y : [0,w ImageHeight - 1]
 *
 *                  The function should fill in the color-structure, and
 *                  all three color components (red, green and blur) should
 *                  be in the interval [0, 255].
 *
 *  PARAMETERS:     filename    Name of file to create (including extension).
 *                  width       Number of horisontal pixels in the image.
 *                  height      Number of vertical pixels in the image.
 *                  getpixel    Address of user defined callback function.
 *                              (See above)
 *
 *  RETURNS:        TGA_OK          OK
 *                  TGA_ERRCREATE   Couldn't create file.
 *                  TGA_ERRWRITE    Error writing to the file.
 *
 */
int TGA_Save(char *filename, int width, int height,
             void (*getpixel)(int x, int y, TGA_Color *col))
{
    /*
     *  Initiate variables for new TGA-file
     */
    ImageHeight = height;
    ImageWidth = width;
    GetPixel = getpixel;

    /*
     *  Create file specified
     */
    if (Create(filename) != TGA_OK)
        return TGA_ERRCREATE;

    /*
     *  Write file header.
     */
    if (WriteFileHeader() != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  Write the image data.
     */
    if (WriteImageData() != TGA_OK)
        return TGA_ERRWRITE;

    /*
     *  Write footer, and close the file.
     */
    if (WriteFileFooter() != TGA_OK)
        return TGA_ERRWRITE;
    Close();

    return TGA_OK;
}

static void xGetPixel(int x, int y, TGA_Color *col)
{
char	*base;

	base = pix_data + 4 * (y * pix_width + x);

    col->r = base[0];
    col->g = base[1];
    col->b = base[2];
    col->a = base[3];
}



void tgaSave(char *filename,char *data,int width,int height,int nchannels)
{
	pix_width = width;
	pix_data = data;
	pix_channels = nchannels;
#if 1
    TGA_SetImageID("TGA");
    TGA_SetAuthor("Cryptic");
    TGA_SetSoftwareID("Cryptic");
#endif
    /*
     *  Create the TGA-file.
     */
	image_data = data;
    TGA_Save(filename, width, height, 0);
}

char *tgaSaveToMem(char *data,int width,int height,int nchannels, int *datalen)
{
	pix_width = width;
	pix_data = data;
	pix_channels = nchannels;
#if 1
	TGA_SetImageID("TGA");
	TGA_SetAuthor("Cryptic");
	TGA_SetSoftwareID("Cryptic");
#endif
	/*
	*  Create the TGA-file.
	*/
	image_data = data;
	TGA_Save("MEMORY", width, height, 0);
	*datalen = tga_data_sb.idx - 1; // subtract null terminator
	return tga_data_sb.buff;
}

static int	ReadTGAHeader( FILE *stream, PrivTexReadInfo *info,TgaHeader *tgaHeader)
{
    int			i;
	int		cookie;

	cookie = getc(stream) << 8;
	cookie |= getc(stream);

    // Fill up rest of the TGA header. 
    if ( fread( &(tgaHeader->ImgType), 1, sizeof(TgaHeader)-2, stream ) != 
    	sizeof(TgaHeader)-2) {
    	Errorf("Unexpected end of file.");
    	return 0;
    }
    tgaHeader->IDLength	= (U8) ((cookie >>  8) & 0xFF);
    tgaHeader->CMapType = (U8) ((cookie      ) & 0xFF);

    // Optionally, skip the image id fields.
    for (i= (tgaHeader->IDLength) & 0xFF; i; i--) {
    	int	c;

    	if ((c = getc(stream)) == EOF) {
    		Errorf("Unexpected EOF.");
    		return 0;
    	}
    }

    info->width  = tgaHeader->WidthHi << 8 | tgaHeader->WidthLo;
    info->height = tgaHeader->HeightHi << 8 | tgaHeader->HeightLo;

    if ((info->width <= 0) || (info->height <= 0)) {
    	Errorf("TGA Image: width or height is 0.");
    	return 0;
    }

    switch(tgaHeader->ImgType) {
    case TGA_MONO:
    case TGA_MONO_RLE:			// True color image.
    	if (tgaHeader->PixelDepth != 8) {
	    Errorf("TGA Image: Mono image is not 8 bits/pixel.");
	    return 0;
    	}
    	info->format = TEXFMT_I_8;
    	break;

    case TGA_TRUE:
    case TGA_TRUE_RLE:
    	switch (tgaHeader->PixelDepth ) {
    	case	15: 
    	case	16: 
			info->format = TEXFMT_ARGB_1555; break;
    	case	24: 
    	case	32: 
			info->format = TEXFMT_ARGB_8888; break;
    	default:	
    			Errorf("TGA Image: True color image is not 24/32 bits/pixel.");
    			return 0; 
    			break;
    	}
    	break;
      
    case TGA_CMAP:
    case TGA_CMAP_RLE:			// Color mapped image.
    	if ( tgaHeader->CMapType     != 1 ) {
    		Errorf("TGA Image: Color-mapped TGA image has no palette");
    		return 0;
    	}
    	if (((tgaHeader->CMapLengthLo + tgaHeader->CMapLengthHi * 256L)
    	    +(tgaHeader->CMapStartLo + tgaHeader->CMapStartHi * 256L)) > 256){
    		Errorf("TGA Image: Color-mapped image has > 256 colors");
    		return 0;
    	}
    	info->format = TEXFMT_P_8;
    	break;

    default:
    	Errorf("TGA Image: unsupported format");
    	return  0;
    }
    info->size = info->width*info->height*4;

    return 1;
}

static int	ReadTGAColorMap(FILE *stream, const TgaHeader *tgaHeader, U32 *palette)
{
    int		cmapStart;
    int		cmapLength;
    int		cmapDepth;
    int		i;

    cmapStart   = tgaHeader->CMapStartLo;
    cmapStart  += tgaHeader->CMapStartHi * 256L;

    cmapLength  = tgaHeader->CMapLengthLo;
    cmapLength += tgaHeader->CMapLengthHi * 256L;

    cmapDepth   = tgaHeader->CMapDepth;

    if (tgaHeader->CMapType == 0) return 1;		// no colormap.

    /* Validate some parameters */
    if (cmapStart < 0) {
    	Errorf("TGA Image: Bad Color Map start value.");
    	return 0;
    }

    cmapDepth = (cmapDepth + 1) >> 3;		// to bytes.
    if ((cmapDepth <= 0) || (cmapDepth > 4)) {
    	Errorf("TGA Image: Bad Color Map depth.");
    	return 0;
    }

    // May have to skip the color map.
    if ((tgaHeader->ImgType != TGA_CMAP) && 
    	(tgaHeader->ImgType != TGA_CMAP_RLE)) {
    	/* True color, yet there is a palette, this is OK, just skip. */

    	cmapLength *= cmapDepth;
    	while (cmapLength--) {
    		int		c;

    		c = getc(stream);
    		if (c == EOF) {
    			Errorf("TGA Image: Unexpected EOF reading Color Map.");
    			return 0;
    		}
    	}
    	return 1;
    }

    // This is a real palette that's going to be used. 

    // Verify that it's not too large.
    if ((cmapStart + cmapLength) > 256) {
    	Errorf("TGA Image: Color Map > 256 entries.");
    	return 0;
    }


    // printf("cmapdepth = %d, start = %d, length = %d\n", cmapDepth,
    // 	cmapStart, cmapLength);
    for (i=0; i<256; i++) {
    	int	r, g, b, a;

    	if ((i < cmapStart) || (i >= (cmapStart + cmapLength))) {
			FatalErrorf("This texture has a palette, gettex is confused and will now die (see console for which texture I'm talking about)");
    		palette[i] = 0;
    		// printf("Skipping palette entry %d\n", i);
    		continue;
    	}

    	// Read this colormap entry.
    	switch (cmapDepth) {
    	case 1:	// 8 bpp
    		r = getc(stream);
    		if (r == EOF) {
  		    Errorf("TGA Image: Unexpected End of File.");
		    return 0;
    		}
    		r &= 0xFF;
			FatalErrorf("This texture has a palette, gettex is confused and will now die (see console for which texture I'm talking about)");
    		palette[i] = (r << 24) | (r << 16) | (r << 8) | (r);
   		break;

    	case 2:	// 15, 16 bpp.

   		b = getc(stream);
   		r = getc(stream);
   		if ((r == EOF) || (b == EOF)) {
   			Errorf("TGA Image: Unexpected End of File.");
   			return 0;
   		}
   		r &= 0xFF;
   		b &= 0xFF;
   		g = ((r & 0x3) << 6) + ((b & 0xE0) >> 2);
   		r = (r & 0x7C) << 1;
   		b = (b & 0x1F) << 3;
		FatalErrorf("This texture has a palette, gettex is confused and will now die (see console for which texture I'm talking about)");
   		palette[i] = (r << 16) | (g << 8) | (b) | 0xFF000000L;
   		break;

    	case 3:
    	case 4:
   		b = getc(stream);
   		g = getc(stream);
   		r = getc(stream);
   		a = (cmapDepth == 4) ? getc(stream) : 0x0FF;

   		if ((r == EOF) || (g == EOF) || (b == EOF) | (a == EOF)) {
   			Errorf("TGA Image: Unexpected End of File.");
   			return 0;
   		}
		FatalErrorf("This texture has a palette, gettex is confused and will now die (see console for which texture I'm talking about)");
   		palette[i] = (a << 24) | (r << 16) | (g << 8) | b;
   		// printf("Setting palette %3d to %.08x\n", i, palette[i]);
   		break;

    	default:
    		Errorf("TGA Image: Bad Color Map depth.");
    		return 0;
    	}
    }
    return 1;
}

static	int	tgaRLE, tgaRLEflag, tgaRLEcount, tgaRLEsav[4]; 

static	int	ReadTGARLEPixel( FILE *stream, U8 *data, int pixsize)
{
    int		c, i;

    // Run length encoded data Only
    if (tgaRLEcount == 0) {
    	// Need to restart the run.
    	if ( (tgaRLEcount = c = getc( stream )) == EOF) {
    		Errorf("TGA Image: Unexpected End of File.");
    		return 0;
    	}
    	tgaRLEflag = tgaRLEcount & 0x80;
    	tgaRLEcount = (tgaRLEcount & 0x7F) + 1;

    	if (tgaRLEflag) {
	    // Replicated color, read the color to be replicated 
    	    for (i=0; i<pixsize; i++) {
		if ( (c = getc( stream )) == EOF) {
    		    Errorf("TGA Image: Unexpected End of File\n");
    		    return 0;
    		}
    		tgaRLEsav[i] = (U8) c;
    	    }
    	}
    }

    // Now deliver the data either from input or from saved values.
    tgaRLEcount--;
    if (tgaRLEflag) {
    	// deliver from saved data.
    	for (i=0; i<pixsize; i++) *data++ = (U8) tgaRLEsav[i];
    } else {
    	for (i=0; i<pixsize; i++) {
    	    if ( (c = getc( stream )) == EOF) {
    		Errorf("TGA Image: Unexpected End of File\n");
    		return 0;
	    }
    	    *data++ = (U8) c;
    	}
    }
    return 1;
}

static int ReadTGASpan( FILE *stream, U8 *data, int w, int pixsize)
{
    if (tgaRLE == 0) {
    	if ( fread( data, 1, w * pixsize, stream) != (w * pixsize)) {
    		Errorf("TGA Image: Unexpected End of File\n");
    		return 0;
    	}
    	return 1;
    }

    // Otherwise, RLE data.
    while (w--) {
    	if (!ReadTGARLEPixel( stream, data, pixsize)) {
	    return 0;
    	}
    	data += pixsize;
    }
    return 1;
}

static int	ReadTGAData( FILE *stream, PrivTexReadInfo *info,TgaHeader *tgaHeader, unsigned int read_opts)
{
    int		i, stride;
    int 	bpp;			// bytesPerPixel
    U8*		data;
    long	BigEndian = 0xff000000;
	U32*	pal;

    // printf("TxREAD TGA DATA\n");
    tgaRLEcount = 0;

    bpp = (tgaHeader->PixelDepth + 1) >> 3;

    switch (tgaHeader->ImgType) {
		case TGA_MONO:		tgaRLE = 0; info->format = TEXFMT_I_8; break;
		case TGA_MONO_RLE:	tgaRLE = 1; info->format = TEXFMT_I_8; break;
		case TGA_TRUE:		tgaRLE = 0; info->format = (bpp == 2) ? TEXFMT_ARGB_1555 : TEXFMT_ARGB_8888; break;
		case TGA_TRUE_RLE:	tgaRLE = 1; info->format = (bpp == 2) ? TEXFMT_ARGB_1555 : TEXFMT_ARGB_8888; break;
		case TGA_CMAP:		tgaRLE = 0; info->format = TEXFMT_P_8; break;
		case TGA_CMAP_RLE:	tgaRLE = 1; info->format = TEXFMT_P_8; break;
    }

    // printf("bpp = %d, rle = %d\n", bpp, tgaRLE);

    stride =  info->width * bpp;
    data   =  info->data;
    if ((tgaHeader->ImageDescriptor & 0x20) == 0) {
    	// Origin is lower left
    	data   = data + (info->height-1) * stride;
    	stride = -stride;
    }

    /* If there's a colormap, read it now. */
	pal = 0;
    if (!ReadTGAColorMap(stream, tgaHeader, (U32 *) pal)) 
    	return 0;
    // printf("read in color map\n");

    /* Read in all the data */
    for ( i = 0; i < info->height; i++) {
    	if (!ReadTGASpan( stream, data, info->width, bpp)) {
    			Errorf("TGA Image: Unexpected end of file.");
    			return 0;
    	}
    	data += stride;
    }

    /*
     * BPP == 1 -> P8 or I8 formatted data.
     * BPP == 2 -> ARGB1555 formatted data.
     * BPP == 3  should be translated to ARGB8888 from RGB888.
     * BPP == 4 -> ARGB8888 formatted data.
     */
    // printf("Repacking\n");
    if (( bpp == 1 ) || ( bpp == 3 ))
	{
		//
		// When tTgaReadOpt_AlphaToZeroOnEmptyImage is set, an exception is made to the
		// usual plugging of 0xFF as the alpha value when expanding RGB or grayscale pixels.
		// If no non-zero RGB values are found, alpha is universally set to zero. To do that,
		// we initially set alpha to zero until, or if, we see a non-zero RGB value. Then we go
		// back and re-set alpha to 0xFF on the pixels we've already processed and then switch to
		// 0xFF as the fill value from them on.
		//
		U8	alphaFillChar = ( read_opts & tTgaReadOpt_AlphaToZeroOnEmptyImage ) ? 0x00 : 0xFF;
		int	totalPixels = info->width * info->height;
    	int	npixels = totalPixels;
    	U8	*src      = ((U8 *) info->data) + (npixels - 1) * bpp;
    	U8	*imgStart = ((U8 *) info->data) + (npixels - 1) * 4;
    	U8  *dst      = imgStart;
    	char zeroCompare[3] = { 0 };

    	while (npixels--) {
    		dst[3] = alphaFillChar;
    		dst[2] = (bpp == 1) ? src[0] : src[2];
    		dst[1] = (bpp == 1) ? src[0] : src[1];
    		dst[0] = src[0];
    		
    		if (( alphaFillChar == 0x00 ) &&
    			( memcmp( src, zeroCompare, bpp ) != 0 ))
    		{
    			// Oops! Data is not empty. Now go back and plug 0xFF where we,
    			// on the assumption that the data is blank, were setting 0x00.
    			int pixelI;
    			U8* the_pixel = imgStart;
    			alphaFillChar = 0xFF;
    			for ( pixelI=npixels; pixelI < totalPixels; pixelI++ )
    			{
    				the_pixel[3] = alphaFillChar;
    				the_pixel -= 4;
    			}
    		}
    		dst -= 4;
    		src -= bpp;
    	}
    }
    // printf("Done\n");
    if (*(U8 *)&BigEndian) {
	/* Repack 16bpp and 32bpp cases */
        if (bpp == 2) {
    	    int	npixels = info->width * info->height;
    	    U16	*src = (U16 *) info->data;

    	    while (npixels--) {
            	*src = (*src << 8) | ((*src >> 8) & 0xff);
                src++;
    	    }
        }
        if ((bpp == 3) || (bpp == 4)) {
    	    int		npixels = info->width * info->height;
    	    U32	*src = (U32 *) info->data;

    	    while (npixels--) {
            	*src = (((*src      ) & 0xff)  << 24)|
            	       (((*src >>  8) & 0xff)  << 16)|
            	       (((*src >> 16) & 0xff)  <<  8)|
            	       (((*src >> 24) & 0xff)       );
                src++;
    	    }
        }
    }
    
    return 1;
}

char *tgaLoadFromMemory(char *memory,int size,int *wp,int *hp)
{
	int				res;
	TgaHeader		header;
	int				i;
	U8				*src;
	PrivTexReadInfo info = {0};
	StuffBuff		sb;
	FILE			*file;

	initStuffBuff(&sb, size);
	memcpy(sb.buff, memory, size);
	file = fileOpenStuffBuff(&sb);

	res = ReadTGAHeader(file, &info,&header);
	if (!res)
		return 0;
	if (res)
	{
		info.data = malloc(info.size);
		res = ReadTGAData(file, &info,&header,0xFF);
		if (!res)
		{
			free(info.data);
			info.data = NULL;
			return 0;
		}
	}
	// BGRA -> RGBA
	for(src = info.data,i=0;i<info.height * info.width;i++,src+=4)
	{
		static unsigned char r,g,b;

		r = src[2];
		g = src[1];
		b = src[0];
		src[0]	= r;
		src[1]	= g;
		src[2]	= b;
	}
	*wp = info.width;
	*hp = info.height;

	freeStuffBuff(&sb);
	return info.data;
}

char *tgaLoadEx(FILE *file,int *wp, int *hp, unsigned int read_opts /* tTgaReadOpts */ )
{
	int				res;
	TgaHeader		header;
	PrivTexReadInfo info = {0};

	res = ReadTGAHeader(file, &info,&header);
	if (!res)
		return 0;
	if (res)
	{
		info.data = malloc(info.size);
		res = ReadTGAData(file, &info,&header,read_opts);
		if (!res)
		{
			free(info.data);
			info.data = NULL;
			return 0;
		}
	}
	
	#define kBGRA_Blue	0
	#define kBGRA_Green	1
	#define kBGRA_Red	2
	#define kBGRA_Alpha	3
	
	#define kRGBA_Red	0
	#define kRGBA_Green	1
	#define kRGBA_Blue	2
	#define kRGBA_Alpha	3
	
	// BGRA -> RGBA
	if (! ( read_opts & kTgaReadOpt_DontSwapToRGBA ))
	{
		int				i;
		U8				*src;
		for(src = info.data,i=0;i<info.height * info.width;i++,src+=4)
		{
			static unsigned char r,g,b;
			
			r = src[kBGRA_Red];
			g = src[kBGRA_Green];
			b = src[kBGRA_Blue];
			src[kRGBA_Red]	= r;
			src[kRGBA_Green]= g;
			src[kRGBA_Blue]	= b;
		}
	}
	*wp = info.width;
	*hp = info.height;
	return info.data;
}
