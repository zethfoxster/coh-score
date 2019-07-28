#ifndef __dds_h__
#define __dds_h__


typedef struct tDDS_ImageInfo {
	DWORD compressionTypeFOURCC;	// 'RGBA', 'RGB ', 'DXT1', 'DXT3', 'DXT5'
	DWORD offsetToImage0;
	DWORD bytesPerPixel;
	// How to locate image colors
	DWORD redBitsMask;
	DWORD greenBitsMask;
	DWORD blueBitsMask;
	DWORD alphaBitsMask;
} tDDS_ImageInfo;

typedef struct tDDS_MipMapInfo {
	DWORD imageDataOfs;		// offset from tDDS_ImageInfo::offsetToImage0
	DWORD mipMapWidth;
	DWORD mipMapHeight;
} tDDS_MipMapInfo;


//
//	These functions don't allocate a memory buffer. This just get information about
//	the contents.
//

// Returns the number of mipMaps[] elements filled in.
DWORD ddsGetImageInfo( 
		/*IN */ char* fileData, 
		/*OUT*/ tDDS_ImageInfo* pImageInfo, 
		/*OUT*/ tDDS_MipMapInfo mipMaps[], 
		/*IN */ DWORD mipMapsArrSz );

// Returns the number of mipMaps[] elements filled in.
DWORD ddsGetImageInfoFromFile( 
		/*IN */ const char* szDdsFilePath, 
		/*OUT*/ tDDS_ImageInfo* pImageInfo, 
		/*OUT*/ tDDS_MipMapInfo mipMaps[], 
		/*IN */ DWORD mipMapsArrSz );

#endif // __dds_h__
