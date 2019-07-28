//************************************************************************** 
//* AnimImp.cpp	- CoH Animation Importer
//* 
//* Import the .ANIMX text format
//* The name is derived from: ANIMation Export
//* 
//* Copyright (c) 2009 NCsoft, All Rights Reserved. 
//* 
//***************************************************************************

#include "AnimImp.h"
#include "import_animx.h"

HINSTANCE hInstance;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if( fdwReason == DLL_PROCESS_ATTACH )
	{
		hInstance = hinstDLL;
		DisableThreadLibraryCalls(hInstance);
	}

	return(TRUE);
}


//------------------------------------------------------

class AnimImpClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new AnimImport; }
	const TCHAR *	ClassName() { return "AnimImport"; }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return AnimExp_CLASS_ID; }
	const TCHAR* 	Category() { return "Scene Import"; }
};

static AnimImpClassDesc AnimImpDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return "City of Heroes Animation Import (.ANIMX)"; }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &AnimImpDesc; break;
		default: return 0; break;
	}

}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//
// .3DS import module functions follow:
//

AnimImport::AnimImport() {
	m_numFrames=0;
}

AnimImport::~AnimImport() {
}

int
AnimImport::ExtCount() {
	return 1;
}

const TCHAR *
AnimImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("ANIMX");
	}
	return _T("");
}

const TCHAR *
AnimImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return "3ds max Animation Importer for City of Heroes";
}

const TCHAR *
AnimImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return "Animation Import - Paragon CoH";
}

const TCHAR *
AnimImport::AuthorName() {			// ASCII Author name
	return "Paragon Studios";
}

const TCHAR *
AnimImport::CopyrightMessage() {	// ASCII Copyright message
	return "Copyright 2009-2010 Paragon Studios";
}

const TCHAR *
AnimImport::OtherMessage1() {		// Other message #1
	return _T("");
}

const TCHAR *
AnimImport::OtherMessage2() {		// Other message #2
	return _T("");
}

unsigned int
AnimImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 101;
}

void
AnimImport::ShowAbout(HWND hWnd) {			// Optional
}

BOOL AnimImport::nodeEnum(INode* node) 
{
	char *name = node->GetName();
	NodeAnimHandle hNodeAnim = GetNodeAnimHandle(name);
	int tpf = GetTicksPerFrame();
	int c = ip->GetTime()/tpf;
	//printf("node: %s\n", name);

	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
		return FALSE;

	if (!hNodeAnim) {
		//printf("  (No animation found)\n");
	}
	else
	{
		for (int i=0; i<m_numFrames; i++)
		{
			TimeValue t = (c+i)*tpf;
			Matrix3 m, M_trans, M_rot, M_scale;

			// Rebuild the NodeTM matrix from the
			// pieces derived from DecomposeMatrix in the exporter

			float* pTranslation = GetNodeFrameTranslation( hNodeAnim, i );
			Point3 p;
			M_trans.IdentityMatrix();
			p[0] = pTranslation[0];
			p[1] = pTranslation[1];
			p[2] = pTranslation[2];
			M_trans.SetTrans( p );

			float* pAxisAngle = GetNodeFrameAxisAngle( hNodeAnim, i );
			p[0] = pAxisAngle[0];
			p[1] = pAxisAngle[1];
			p[2] = pAxisAngle[2];
			M_rot.SetAngleAxis(p, -pAxisAngle[3]);

			float* pScale = GetNodeFrameScale( hNodeAnim, i );
			p[0] = pScale[0];
			p[1] = pScale[1];
			p[2] = pScale[2];
			M_scale.SetScale(p);

			m = M_scale * M_rot * M_trans;

			node->SetNodeTM(t, m);
		}
	}

	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c)))
			return FALSE;
	}
	return TRUE;
}

int AnimImport::loadAnimFile(const TCHAR *name)
{
	m_numFrames = LoadAnimation( (const char*) name );
	return 1;
}

// Dummy function for progress bar
DWORD WINAPI dummy_progress_fn(LPVOID arg)
{
	return(0);
}

#define MAX(a,b) (((a)>(b))?(a):(b))

int AnimImport::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts)
{
	ip = i;

	int tpf = GetTicksPerFrame();
	int s = ip->GetAnimRange().Start()/tpf, 
		e = ip->GetAnimRange().End()/tpf,
		c = ip->GetTime()/tpf;

	loadAnimFile(name);
	if (m_numFrames == 0)
	{
		// only if we need a specific error, the IMPEXP_FAIL return puts up a generic one
//		MessageBox(NULL,"Import failed due to parsing error or zero frames.","ANIMX Import",MB_OK);
		return IMPEXP_FAIL;
	}

	// Startup the progress bar.
	ip->ProgressStart("Importing ANIMX...", TRUE, dummy_progress_fn, NULL);

	SuspendAnimate();
	AnimateOn();

	e = MAX(e, c+m_numFrames-1);
	ip->SetAnimRange(Interval(s*GetTicksPerFrame(), e*GetTicksPerFrame()));
	int numChildren = ip->GetRootNode()->NumberOfChildren();
	for (int idx=0; idx<numChildren; idx++) {
		int percent = (int)(100*(float)idx/numChildren);
		ip->ProgressUpdate(percent);
		if (ip->GetCancel())
			break;
		nodeEnum(ip->GetRootNode()->GetChildNode(idx));
	}

	ResumeAnimate();
	FreeAnimation();

	// We're done. Finish the progress bar.
	ip->ProgressEnd();

	return IMPEXP_SUCCESS;
}
