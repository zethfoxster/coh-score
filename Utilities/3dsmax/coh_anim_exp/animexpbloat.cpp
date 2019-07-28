#include "AnimExp.h"
#include "SkelExp.h"


int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	// Initialize the custom controls. This should be done only once.
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}

	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 2;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
	case 0: return GetAnimExpDesc();
	break;
	case 1: return GetSkelExpDesc();
	default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class AnimExpClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new AnimExp; } 
	const TCHAR*	ClassName() { return "AnimExp"; }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		ClassID() { return AnimExp_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
};

static AnimExpClassDesc AnimExpDesc;

ClassDesc* GetAnimExpDesc()
{
	return &AnimExpDesc;
}

class SkelExpClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new SkelExp; } 
	const TCHAR*	ClassName() { return "SkelExp"; }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		ClassID() { return SkelExp_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
};

static SkelExpClassDesc SkelExpDesc;

ClassDesc* GetSkelExpDesc()
{
	return &SkelExpDesc;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}

int AnimExp::ExtCount()
{
	return 1;
}

const TCHAR * AnimExp::Ext(int n)
{
	switch(n) {
	case 0:
		return _T("animx");
	break;
	}
	return _T("");
}

const TCHAR * AnimExp::LongDesc()
{
	return GetString(IDS_LONGDESC);
}

const TCHAR * AnimExp::ShortDesc()
{
	return GetString(IDS_SHORTDESC);
}

const TCHAR * AnimExp::AuthorName() 
{
	return _T("NCsoft");
}

const TCHAR * AnimExp::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR * AnimExp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * AnimExp::OtherMessage2() 
{
	return _T("");
}

unsigned int AnimExp::Version()
{
	return VERSION;
}

int SkelExp::ExtCount()
{
	return 1;
}

const TCHAR * SkelExp::Ext(int n)
{
	switch(n) {
	case 0:
		return _T("skelx");
	}
	return _T("");
}

const TCHAR * SkelExp::LongDesc()
{
	return "3ds Max Skeleton/Scaling Info exporter for CoH";
}

const TCHAR * SkelExp::ShortDesc()
{
	return "Skeleton Exporter - NCsoft CoH";
}

const TCHAR * SkelExp::AuthorName() 
{
	return _T("NCsoft");
}

const TCHAR * SkelExp::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR * SkelExp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * SkelExp::OtherMessage2() 
{
	return _T("");
}

unsigned int SkelExp::Version()
{
	return 100;
}


static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, 
										WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
case WM_INITDIALOG:
	CenterWindow(hWnd, GetParent(hWnd)); 
	break;
case WM_COMMAND:
	switch (LOWORD(wParam)) {
case IDOK:
	EndDialog(hWnd, 1);
	break;
	}
	break;
default:
	return FALSE;
	}
	return TRUE;
}       

void AnimExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

void SkelExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}


/****************************************************************************

Configuration.
To make all options "sticky" across sessions, the options are read and
written to a configuration file every time the exporter is executed.

****************************************************************************/

TSTR AnimExp::BuildCfgPath( const char* fname )
{
	TSTR filename;

	filename += ip->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += fname;

	return filename;
}


TSTR AnimExp::GetCfgFilename()
{
	return BuildCfgPath( CFGFILENAME );
}

// NOTE: Update anytime the CFG file changes
#define CFG_VERSION 0x03

BOOL AnimExp::ReadConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "rb");
	if (!cfgStream)
		return FALSE;

	// First item is a file version
	int fileVersion = _getw(cfgStream);

	if (fileVersion > CFG_VERSION) {
		// Unknown version
		fclose(cfgStream);
		return FALSE;
	}

	SetIncludeMesh(fgetc(cfgStream));
	SetIncludeAnim(fgetc(cfgStream));
	SetIncludeMtl(fgetc(cfgStream));
	SetIncludeMeshAnim(fgetc(cfgStream));
	SetIncludeCamLightAnim(fgetc(cfgStream));
	SetIncludeIKJoints(fgetc(cfgStream));
	SetIncludeNormals(fgetc(cfgStream));
	SetIncludeTextureCoords(fgetc(cfgStream));
	SetIncludeObjGeom(fgetc(cfgStream));
	SetIncludeObjShape(fgetc(cfgStream));
	SetIncludeObjCamera(fgetc(cfgStream));
	SetIncludeObjLight(fgetc(cfgStream));
	SetIncludeObjHelper(fgetc(cfgStream));
	SetAlwaysSample(fgetc(cfgStream));
	SetMeshFrameStep(_getw(cfgStream));
	SetKeyFrameStep(_getw(cfgStream));

	// Added for version 0x02
	if (fileVersion > 0x01) {
		SetIncludeVertexColors(fgetc(cfgStream));
	}

	// Added for version 0x03
	if (fileVersion > 0x02) {
		SetPrecision(_getw(cfgStream));
	}

	fclose(cfgStream);

	return TRUE;
}


void AnimExp::WriteConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "wb");
	if (!cfgStream)
		return;

	// Write CFG version
	_putw(CFG_VERSION,				cfgStream);

	fputc(GetIncludeMesh(),			cfgStream);
	fputc(GetIncludeAnim(),			cfgStream);
	fputc(GetIncludeMtl(),			cfgStream);
	fputc(GetIncludeMeshAnim(),		cfgStream);
	fputc(GetIncludeCamLightAnim(),	cfgStream);
	fputc(GetIncludeIKJoints(),		cfgStream);
	fputc(GetIncludeNormals(),		cfgStream);
	fputc(GetIncludeTextureCoords(),	cfgStream);
	fputc(GetIncludeObjGeom(),		cfgStream);
	fputc(GetIncludeObjShape(),		cfgStream);
	fputc(GetIncludeObjCamera(),	cfgStream);
	fputc(GetIncludeObjLight(),		cfgStream);
	fputc(GetIncludeObjHelper(),	cfgStream);
	fputc(GetAlwaysSample(),		cfgStream);
	_putw(GetMeshFrameStep(),		cfgStream);
	_putw(GetKeyFrameStep(),		cfgStream);
	fputc(GetIncludeVertexColors(),	cfgStream);
	_putw(GetPrecision(),			cfgStream);

	fclose(cfgStream);
}


BOOL AnimExp::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
}

BOOL SkelExp::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
}

