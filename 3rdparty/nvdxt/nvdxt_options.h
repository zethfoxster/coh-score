/****************************************************************************************
	
    Copyright (C) NVIDIA Corporation 2003

    TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
    *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
    BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
    WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
    BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
    ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
    BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*****************************************************************************************/
#pragma once

#include <windows.h>
#include <tPixel.h>

typedef HRESULT (*MIPcallback)(void * data, int miplevel, DWORD size, int width, int height, void * user_data);
            

_inline char * GetDXTCVersion() { return "Version 6.74";}


enum
{

    dBackgroundNameStatic = 3,
    dProfileDirNameStatic = 4,
    dSaveButton = 5,
    dProfileNameStatic = 6,





    dbSwapRGB = 29,
    dGenerateMipMaps = 30,
    dMIPMapSourceFirst = dGenerateMipMaps,
    dUseExistingMipMaps = 31,
    dNoMipMaps = 32,
    dMIPMapSourceLast = dNoMipMaps,

    dSpecifiedMipMapsCombo = 39,



    dbShowDifferences = 40,
    dbShowFiltering = 41,
    dbShowMipMapping = 42,
    dbShowAnisotropic = 43,

    dChangeClearColorButton = 50,
    dDitherColor = 53,

    dLoadBackgroundImageButton = 54,
    dUseBackgroundImage = 55,

    dbBinaryAlpha = 56,
    dAlphaBlending = 57,

    dFadeColor = 58,  //.
    dFadeAlpha = 59,

    dFadeToColorButton = 60,
    dAlphaBorder = 61,
    dBorder = 62,
    dBorderColorButton = 63,

    dNormalMapCombo = 64,

    dDitherMIP0 = 66,
    dGreyScale = 67,
    dQuickCompress = 68,
    dLightingEnable = 69,

    dbPreviewDisableMIPmaps = 71,




    dZoom = 79,


    dTextureTypeCombo = 80,

    dFadeAmount = 90,
    dFadeToAlpha = 91,
    dFadeToDelay = 92,

    dBinaryAlphaThreshold = 94,

    dFilterGamma = 100,
    dFilterBlur = 101,
    dFilterWidth = 102,
    dbOverrideFilterWidth = 103,
    dLoadProfile = 104,
    dSaveProfile = 105,
    dSharpenMethodCombo = 106,
    dProfileCombo = 107,
    dSelectProfileDirectory = 108,





    dViewDXT1 = 200,
    dViewDXT2 = 201,
    dViewDXT3 = 202,
    dViewDXT5 = 203,
    dViewA4R4G4B4 = 204,
    dViewA1R5G5B5 = 205,
    dViewR5G6B5 = 206,
    dViewA8R8G8B8 = 207,


    // 3d viewing options
    d3DPreviewButton = 300, 
    d2DPreviewButton = 301, 
    dPreviewRefresh = 302, 




    dAskToLoadMIPMaps = 400,
    dShowAlphaWarning = 401,
    dShowPower2Warning = 402,
    dTextureFormatBasedOnAlpha = 403,
    dSystemMessage = 404,
    dHidePreviewButtons = 405,

    dSpecifyAlphaBlendingButton = 500,
    dUserSpecifiedFadingAmounts = 501,
    dSharpenSettingsButton = 502,
    dFilterSettingsButton = 503,
    dNormalMapGenerationSettingsButton = 504,
    dConfigSettingsButton = 505,
    dFadingDialogButton = 506,
    dPreviewDialogButton = 507,
    dResetDefaultsButton = 508,
    dImageDialogButton = 510,



    dSaveTextureFormatCombo = 600,
    dMIPFilterCombo = 601,


    ///////////  Normal Map


    dScaleEditText = 1003,
    dProxyItem = 1005,
    dMinZEditText = 1008,



    dALPHA = 1009,
    dFirstCnvRadio = dALPHA,
    dAVERAGE_RGB = 1010,
    dBIASED_RGB = 1011,
    dRED = 1012,
    dGREEN = 1013,
    dBLUE = 1014,
    dMAX = 1015,
    dCOLORSPACE = 1016,
    dNORMALIZE = 1017,
    dConvertToHeightMap = 1018,
    dLastCnvRadio = dConvertToHeightMap,

    d3DPreview = 1021,      
    dDecalTexture = 1022,
    dbUseDecalTexture = 1023,
    dbBrighten = 1024,
    dbAnimateLight = 1025,
    dStaticDecalName = 1026,
    dbSignedOutput = 1027,
    dbNormalMapSwapRGB = 1028,


    dbWrap = 1030,
    dbMultipleLayers = 1031,
    db_16_16 = 1032,

    dAlphaNone = 1033,
    dFirstAlphaRadio = dAlphaNone,
    dAlphaHeight = 1034,
    dAlphaClear = 1035,
    dAlphaWhite = 1036,
    dLastAlphaRadio = dAlphaWhite,

    dbInvertY = 1037,
    db_12_12_8 = 1038,
    dbInvertX = 1039,

    dFilter4x = 1040,
    dFirstFilterRadio = dFilter4x,
    dFilter3x3 = 1041,
    dFilter5x5 = 1042,
    dFilterDuDv = 1043,
    dFilter7x7 = 1044,
    dFilter9x9 = 1045,
    dLastFilterRadio = dFilter9x9,


    dbNormalMapConversion = 1050,
    dbErrorDiffusion = 1051,


    dCustomFilterFirst = 2000,
    // 5x5  Filter 0- 24

    dCustomDiv = 2025,
    dCustomBias = 2026,

    dUnSharpRadius = 2027,
    dUnSharpAmount = 2028,
    dUnSharpThreshold = 2029,

    dXSharpenStrength = 2030,
    dXSharpenThreshold = 2031,



    dCustomFilterLast = dXSharpenThreshold,  


    dSharpenTimesMIP0 = 2100,
    dSharpenTimesFirst = dSharpenTimesMIP0,

    dSharpenTimesMIP1 = 2101,
    dSharpenTimesMIP2 = 2102,
    dSharpenTimesMIP3 = 2103,
    dSharpenTimesMIP4 = 2104,
    dSharpenTimesMIP5 = 2105,
    dSharpenTimesMIP6 = 2106,
    dSharpenTimesMIP7 = 2107,
    dSharpenTimesMIP8 = 2108,
    dSharpenTimesMIP9 = 2109,
    dSharpenTimesMIP10 = 2110,
    dSharpenTimesMIP11 = 2111,
    dSharpenTimesMIP12 = 2112,
    dSharpenTimesMIP13 = 2113,
    dSharpenTimesLast = dSharpenTimesMIP13,

    dDeriveDiv = 2200,   // balance
    dDeriveBias = 2201,



};






// Windows handle for our plug-in (seen as a dynamically linked library):
extern HANDLE hDllInstance;
//class CMyD3DApplication;

typedef enum RescaleTypes
{
    RESCALE_NONE,               // no rescale
    RESCALE_NEAREST_POWER2,     // rescale to nearest power of two
    RESCALE_BIGGEST_POWER2,   // rescale to next bigger power of 2
    RESCALE_SMALLEST_POWER2,  // rescale to smaller power of 2 
    RESCALE_NEXT_SMALLEST_POWER2,  // rescale to next smaller power of 2
    RESCALE_PRESCALE,           // rescale to this size
    RESCALE_RELSCALE,           // relative rescale
    RESCALE_CLAMP,              //
    RESCALE_LAST,              //


} RescaleTypes;


typedef enum SharpenFilterTypes
{
    kSharpenFilterNone,
    kSharpenFilterNegative,
    kSharpenFilterLighter,
    kSharpenFilterDarker,
    kSharpenFilterContrastMore,
    kSharpenFilterContrastLess,
    kSharpenFilterSmoothen,
    kSharpenFilterSharpenSoft,
    kSharpenFilterSharpenMedium,
    kSharpenFilterSharpenStrong,
    kSharpenFilterFindEdges,
    kSharpenFilterContour,
    kSharpenFilterEdgeDetect,
    kSharpenFilterEdgeDetectSoft,
    kSharpenFilterEmboss,
    kSharpenFilterMeanRemoval,
    kSharpenFilterUnSharp,
    kSharpenFilterXSharpen,
    kSharpenFilterWarpSharp,
    kSharpenFilterCustom,
    kSharpenFilterLast,
} SharpenFilterTypes;



typedef enum MIPFilterTypes
{
    kMIPFilterPoint ,    
    kMIPFilterBox ,      
    kMIPFilterTriangle , 
    kMIPFilterQuadratic ,
    kMIPFilterCubic ,    

    kMIPFilterCatrom ,   
    kMIPFilterMitchell , 

    kMIPFilterGaussian , 
    kMIPFilterSinc ,     
    kMIPFilterBessel ,   

    kMIPFilterHanning ,  
    kMIPFilterHamming ,  
    kMIPFilterBlackman , 
    kMIPFilterKaiser,
    kMIPFilterLast,
} MIPFilterTypes;


typedef enum TextureFormats
{
    kDXT1 ,
    kDXT1a ,  // DXT1 with one bit alpha
    kDXT3 ,   // explicit alpha
    kDXT5 ,   // interpolated alpha
    k4444 ,   // a4 r4 g4 b4
    k1555 ,   // a1 r5 g5 b5
    k565 ,    // a0 r5 g6 b5
    k8888 ,   // a8 r8 g8 b8
    k888 ,    // a0 r8 g8 b8
    k555 ,    // a0 r5 g5 b5
    k8   ,   // paletted
    kV8U8 ,   // DuDv 
    kCxV8U8 ,   // normal map
    kA8 ,            // alpha only
    k4  ,            // 16 bit color      
    kQ8W8V8U8,
    kA8L8,
    kR32F,
    kA32B32G32R32F,
    kA16B16G16R16F,
    kL8,       // luminance
    kTextureFormatLast
} TextureFormats;


typedef enum TextureTypes
{
    kTextureType2D,
    kTextureTypeCube,
    kTextureTypeVolume,  
    kTextureTypeLast,  
} TextureTypes;

typedef enum NormalMapTypes
{
    kColorTextureMap,
    kTangentSpaceNormalMap,
    kObjectSpaceNormalMap,
} NormalMapTypes;


#define CUSTOM_FILTER_ENTRIES 27
#define UNSHARP_ENTRIES 3
#define XSHARP_ENTRIES 3
//#define WARPSHARP_ENTRIES 5

#define CUSTOM_DATA_ENTRIES (CUSTOM_FILTER_ENTRIES+UNSHARP_ENTRIES+XSHARP_ENTRIES)
#define SHARP_TIMES_ENTRIES 14

typedef unsigned char Boolean;  // for photoshop scripting

typedef struct CompressionOptions
{
    RescaleTypes    rescaleImageType;     // changes to just rescale
    float           scaleX;
    float           scaleY;

    long            MipMapType;         // dNoMipMaps, dSpecifyMipMaps, dUseExistingMipMaps, dGenerateMipMaps

    long            SpecifiedMipMaps;   // if dSpecifyMipMaps or dUseExistingMipMaps is set (number of mipmaps to generate)

    MIPFilterTypes  MIPFilterType;      // for MIP map, select from MIPFilterTypes

    Boolean         bBinaryAlpha;       // zero or one alpha channel

    NormalMapTypes  normalMapType;  // 

    Boolean         bRGBE;              // convert to RGBE
    Boolean         bAlphaBorder;       // make an alpha border
    Boolean         bBorder;            // make a color border
    tPixel          BorderColor;        // color of border


    Boolean         bFadeColor;         // fade color over MIP maps
    Boolean         bFadeAlpha;         // fade alpha over MIP maps

    tPixel          FadeToColor;        // color to fade to
    long            FadeToAlpha;        // alpha value to fade to (0-255)

    long            FadeToDelay;        // start fading after 'n' MIP maps

    long            FadeAmount;         // percentage of color to fade in

    long            BinaryAlphaThreshold;  // When Binary Alpha is selected, below this value, alpha is zero


    Boolean         bDitherColor;       // enable dithering during 16 bit conversion
    Boolean         bDitherMIP0;// enable dithering during 16 bit conversion for each MIP level (after filtering)
    Boolean         bGreyScale;         // treat image as a grey scale
    Boolean         bQuickCompress;         // Fast compression scheme
    Boolean         bForceDXT1FourColors;  // do not let DXT1 use 3 color representation


    // sharpening after creating each MIP map level



    float custom_data[CUSTOM_DATA_ENTRIES]; 
    int sharpening_passes_per_mip_level[SHARP_TIMES_ENTRIES]; 

    SharpenFilterTypes  SharpenFilterType; 

    Boolean bErrorDiffusion; // diffuse error

    // gamma value for Kaiser, Light Linear
    float FilterGamma;  // not implemented yet
    // alpha value for kaiser filter
    float FilterBlur;
    // width of filter
    float FilterWidth;

    Boolean bOverrideFilterWidth; // use the specified width,instead of the default

	TextureTypes 		TextureType;        
	TextureFormats 		TextureFormat;	    

    Boolean   bSwapRGB;           // swap color positions R and G
    bool bAlphaModulate;            // modulate color by alpha for filtering
    void * user_data;

} CompressionOptions;

__forceinline void setDefaultCompressionOptions(CompressionOptions *options)
{
	int i;
	options->rescaleImageType = RESCALE_NONE; 
	options->scaleX = 1;
	options->scaleY = 1;

	options->MipMapType = dGenerateMipMaps;         // dNoMipMaps, dUseExistingMipMaps, dGenerateMipMaps
	options->SpecifiedMipMaps = 0;   // if dSpecifyMipMaps or dUseExistingMipMaps is set (number of mipmaps to generate)

	options->MIPFilterType = kMIPFilterTriangle;      // for MIP maps

	options->bBinaryAlpha = false;       // zero or one alpha channel

	options->normalMapType = kColorTextureMap;         

	options->bRGBE = false;
	options->bAlphaBorder = false;       // make an alpha border
	options->bBorder = false;            // make a color border
	options->BorderColor.u = 0;        // color of border


	options->bFadeColor = false;         // fade color over MIP maps
	options->bFadeAlpha = false;         // fade alpha over MIP maps

	options->FadeToColor.u = 0;        // color to fade to
	options->FadeToAlpha = 0;        // alpha value to fade to (0-255)

	options->FadeToDelay = 0;        // start fading after 'n' MIP maps

	options->FadeAmount = 0;         // percentage of color to fade in

	options->BinaryAlphaThreshold = 128;  // When Binary Alpha is selected, below this value, alpha is zero


	options->bDitherColor = false;       // enable dithering during 16 bit conversion
	options->bDitherMIP0 = false;// enable dithering during 16 bit conversion for each MIP level (after filtering)
	options->bGreyScale = false;         // treat image as a grey scale
	options->bQuickCompress = false;         // Fast compression scheme
	options->bForceDXT1FourColors = false;  // do not let DXT1 use 3 color representation


	options->SharpenFilterType = kSharpenFilterNone;
	options->bErrorDiffusion = false;



	// gamma value for Kaiser, Light Linear
	options->FilterGamma = 2.2f;
	// alpha value for 
	options->FilterBlur = 1.0f;
	// width of filter
	options->FilterWidth = 10.0f;
	options->bOverrideFilterWidth = false;

	options->TextureType = kTextureType2D;        // regular decal, cube or volume  
	options->TextureFormat = kDXT1;	    

	options->bSwapRGB = false;           // swap color positions R and G
	options->user_data = 0;

	for(i = 0; i<CUSTOM_DATA_ENTRIES; i++)
		options->custom_data[i] = 0;

	for(i = 0; i<SHARP_TIMES_ENTRIES; i++)
		options->sharpening_passes_per_mip_level[i] = 0;

	options->bAlphaModulate = false;
}

