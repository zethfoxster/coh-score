#ifndef __gettex_h__
#define __gettex_h__

typedef enum tGetTexOpt
{
	kGetTexOpt_SrcFile,			// nvdxt.exe: "-file"
	kGetTexOpt_OutputDir,		// nvdxt.exe: "-outdir"
	kGetTexOpt_OutputFile,		// doesn't map directly to nvdtx.exe flag
	kGetTexOpt_NoMipmap,		// nvdxt.exe: "-nomipmap"
	kGetTexOpt_Mipmap,			// dxtex:	  "-m"
	kGetTexOpt_Cubic,			// nvdxt.exe: "-cubic"
	kGetTexOpt_Kaiser,			// nvdxt.exe: "-kaiser"
	kGetTexOpt_DXT5,			// nvdxt.exe: "-dxt5"
	kGetTexOpt_DXT5nm,			// not supported by nvdxt.exe standalone tool.
	kGetTexOpt_DXT1,			// nvdxt.exe: "-dxt1"
	kGetTexOpt_U8888,			// nvdxt.exe: "-u8888"
	kGetTexOpt_U888,			// nvdxt.exe: "-u888" // no alpha
	kGetTexOpt_U1555,			// nvdxt.exe: "-u1555"
	kGetTexOpt_U4444,			// nvdxt.exe: "-u4444"
	kGetTexOpt_U565,			// nvdxt.exe: "-u565" // no alpha
	kGetTexOpt_MakeN4,			// nvdxt.exe: "-n4"
	kGetTexOpt_HeightRgb,		// nvdxt.exe: "-rgb"
	kGetTexOpt_SrcIsNormalMap,
	kGetTexOpt_SpecularInAlpha,	// Specular is stored in the alpha channel
	kGetTexOpt_NormalizeMipMap,
	//---------------------------------------------------
	//	Legacy options
	//---------------------------------------------------
	kGetTexOpt_ToolPath,
	//--------------------
	kGetTexOpt_COUNT
} tGetTexOpt;


#endif // __gettex_h__
