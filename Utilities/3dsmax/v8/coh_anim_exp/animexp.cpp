//************************************************************************** 
// AnimExp.cpp	- CoH Animation Exporter
// 
// Export to .ANIMX text format
// The name is derived from: ANIMation eXport
// 
// Copyright (c) 2009-2010 NCsoft, All Rights Reserved. 
// 
// From: Sample Ascii File Exporter By Christer Janson
// Kinetix Development
// January 20, 1997 CCJ Initial coding
// Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************

#include "AnimExp.h"
#include <fstream>

HINSTANCE hInstance;

static BOOL showPrompts;
static BOOL exportSelected;
static BOOL trace = false;

AnimExp::AnimExp()
{
	if(trace) DebugPrint("AnimExp\n");
	// These are the default values that will be active when 
	// the exporter is ran the first time.
	// After the first session these options are sticky.
	bIncludeMesh = FALSE;
	bIncludeAnim = TRUE;
	bIncludeMtl =  FALSE;
	bIncludeMeshAnim =  TRUE;
	bIncludeCamLightAnim = TRUE;
	bIncludeIKJoints = TRUE;
	bIncludeNormals  =  FALSE;
	bIncludeTextureCoords = FALSE;
	bIncludeVertexColors = FALSE;
	bIncludeObjGeom = TRUE;
	bIncludeObjShape = TRUE;
	bIncludeObjCamera = TRUE;
	bIncludeObjLight = TRUE;
	bIncludeObjHelper = TRUE;
	bAlwaysSample = TRUE;
	nKeyFrameStep = 1;
	nMeshFrameStep = 1;
	nPrecision = 6;
	nStaticFrame = 0;
	nStartFrame = 0;
	nTotalFrames = 0;
}

AnimExp::~AnimExp()
{
	if(trace) DebugPrint("~AnimExp\n");
}

// Dialog proc
static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg,
									  WPARAM wParam, LPARAM lParam)
{
	Interval animRange;
	ISpinnerControl  *spin;

	AnimExp *exp = (AnimExp*)GetWindowLongPtr(hWnd,GWLP_USERDATA); 
	switch (msg) {
	case WM_INITDIALOG:
		exp = (AnimExp*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 
		CheckDlgButton(hWnd, IDC_MESHDATA, exp->GetIncludeMesh()); 
		CheckDlgButton(hWnd, IDC_ANIMKEYS, exp->GetIncludeAnim()); 
		CheckDlgButton(hWnd, IDC_MATERIAL, exp->GetIncludeMtl());
		CheckDlgButton(hWnd, IDC_MESHANIM, exp->GetIncludeMeshAnim()); 
		CheckDlgButton(hWnd, IDC_CAMLIGHTANIM, exp->GetIncludeCamLightAnim()); 
#ifndef DESIGN_VER
		CheckDlgButton(hWnd, IDC_IKJOINTS, exp->GetIncludeIKJoints()); 
#endif // !DESIGN_VER
		CheckDlgButton(hWnd, IDC_NORMALS,  exp->GetIncludeNormals()); 
		CheckDlgButton(hWnd, IDC_TEXCOORDS,exp->GetIncludeTextureCoords()); 
		CheckDlgButton(hWnd, IDC_VERTEXCOLORS,exp->GetIncludeVertexColors()); 
		CheckDlgButton(hWnd, IDC_OBJ_GEOM,exp->GetIncludeObjGeom()); 
		CheckDlgButton(hWnd, IDC_OBJ_SHAPE,exp->GetIncludeObjShape()); 
		CheckDlgButton(hWnd, IDC_OBJ_CAMERA,exp->GetIncludeObjCamera()); 
		CheckDlgButton(hWnd, IDC_OBJ_LIGHT,exp->GetIncludeObjLight()); 
		CheckDlgButton(hWnd, IDC_OBJ_HELPER,exp->GetIncludeObjHelper());

		CheckRadioButton(hWnd, IDC_RADIO_USEKEYS, IDC_RADIO_SAMPLE, 
			exp->GetAlwaysSample() ? IDC_RADIO_SAMPLE : IDC_RADIO_USEKEYS);

		// Setup the spinner controls for the controller key sample rate 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_CONT_STEP), EDITTYPE_INT ); 
		spin->SetLimits(1, 100, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetKeyFrameStep() ,FALSE);
		ReleaseISpinner(spin);

		// Setup the spinner controls for the mesh definition sample rate 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_MESH_STEP), EDITTYPE_INT ); 
		spin->SetLimits(1, 100, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetMeshFrameStep() ,FALSE);
		ReleaseISpinner(spin);

		// Setup the spinner controls for the floating point precision 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_PREC), EDITTYPE_INT ); 
		spin->SetLimits(1, 10, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetPrecision() ,FALSE);
		ReleaseISpinner(spin);

		// Setup the spinner control for the static frame#
		// We take the frame 0 as the default value
		animRange = exp->GetInterface()->GetAnimRange();
		spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_STATIC_FRAME), EDITTYPE_INT ); 
		spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(0, FALSE);
		ReleaseISpinner(spin);

		// Enable / disable mesh options
		EnableWindow(GetDlgItem(hWnd, IDC_NORMALS), exp->GetIncludeMesh());
		EnableWindow(GetDlgItem(hWnd, IDC_TEXCOORDS), exp->GetIncludeMesh());
		EnableWindow(GetDlgItem(hWnd, IDC_VERTEXCOLORS), exp->GetIncludeMesh());
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam; 
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	case IDC_MESHDATA:
		// Enable / disable mesh options
		EnableWindow(GetDlgItem(hWnd, IDC_NORMALS), IsDlgButtonChecked(hWnd,
			IDC_MESHDATA));
		EnableWindow(GetDlgItem(hWnd, IDC_TEXCOORDS), IsDlgButtonChecked(hWnd,
			IDC_MESHDATA));
		EnableWindow(GetDlgItem(hWnd, IDC_VERTEXCOLORS), IsDlgButtonChecked(hWnd,
			IDC_MESHDATA));
		break;
	case IDOK:
		exp->SetIncludeMesh(IsDlgButtonChecked(hWnd, IDC_MESHDATA)); 
		exp->SetIncludeAnim(IsDlgButtonChecked(hWnd, IDC_ANIMKEYS)); 
		exp->SetIncludeMtl(IsDlgButtonChecked(hWnd, IDC_MATERIAL)); 
		exp->SetIncludeMeshAnim(IsDlgButtonChecked(hWnd, IDC_MESHANIM)); 
		exp->SetIncludeCamLightAnim(IsDlgButtonChecked(hWnd, IDC_CAMLIGHTANIM)); 
#ifndef DESIGN_VER
		exp->SetIncludeIKJoints(IsDlgButtonChecked(hWnd, IDC_IKJOINTS)); 
#endif // !DESIGN_VER
		exp->SetIncludeNormals(IsDlgButtonChecked(hWnd, IDC_NORMALS));
		exp->SetIncludeTextureCoords(IsDlgButtonChecked(hWnd, IDC_TEXCOORDS)); 
		exp->SetIncludeVertexColors(IsDlgButtonChecked(hWnd, IDC_VERTEXCOLORS)); 
		exp->SetIncludeObjGeom(IsDlgButtonChecked(hWnd, IDC_OBJ_GEOM)); 
		exp->SetIncludeObjShape(IsDlgButtonChecked(hWnd, IDC_OBJ_SHAPE)); 
		exp->SetIncludeObjCamera(IsDlgButtonChecked(hWnd, IDC_OBJ_CAMERA)); 
		exp->SetIncludeObjLight(IsDlgButtonChecked(hWnd, IDC_OBJ_LIGHT)); 
		exp->SetIncludeObjHelper(IsDlgButtonChecked(hWnd, IDC_OBJ_HELPER));
		exp->SetAlwaysSample(IsDlgButtonChecked(hWnd, IDC_RADIO_SAMPLE));

		spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN)); 
		exp->SetKeyFrameStep(spin->GetIVal()); 
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN)); 
		exp->SetMeshFrameStep(spin->GetIVal());
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN)); 
		exp->SetPrecision(spin->GetIVal());
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
		exp->SetStaticFrame(spin->GetIVal() * GetTicksPerFrame());
		ReleaseISpinner(spin);

		EndDialog(hWnd, 1);
		break;
	case IDCANCEL:
		EndDialog(hWnd, 0);
		break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}       



// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

// Read list of valid node names for export (e.g. bone names)
// If the list is empty then we fall back on exporting all nodes

#define FNAME_EXPORTABLE_NAMES "coh_anim_names.txt"
void AnimExp::ReadExportableNames(void)
{
	m_ExportNames.clear();

	TSTR filepath = BuildCfgPath( FNAME_EXPORTABLE_NAMES );

	std::ifstream inputFile;
	inputFile.open( filepath );
	while (inputFile.good())
	{
		std::string line;
		getline( inputFile, line );
		printf(line.c_str());
		m_ExportNames.push_back(line);
	}
}

// test node name against the white list, comparison is case INsensitive
bool AnimExp::IsNameExportable(const char* nodeName )
{
	// special case: if no white list every node name is ok, 
	if (m_ExportNames.empty())
	{
		return true;
	}

	bool bNameAllowed = false;
	for (unsigned int i= 0; i< m_ExportNames.size(); ++i )
	{
		if (_stricmp(nodeName, m_ExportNames[i].c_str()) == 0)
			return true;
	}

	return false;	// not found found on list
}

// write the header of the export file
void AnimExp::WriteHeader()
{
	int version_major = VERSION/100;
	int version_minor = VERSION - version_major*100;

	fprintf( pStream, "# NCsoft CoH Animation Export\n" );
	fprintf( pStream, "# Generated from 3D Studio Max using:\n" );
	fprintf( pStream, "#\t\tPlugin: 'coh_anim_exp', Version %d, Revision %d\n", version_major, version_minor );						
	TCHAR* fn = ip->GetCurFileName();
	if (fn && _tcslen(fn) > 0)
		fprintf(pStream, _T("#\t\tSource File: %s\n\n"), fn );

	fprintf(pStream,_T("Version %d\n"), VERSION );
	fprintf(pStream,_T("SourceName %s\n"), (fn && _tcslen(fn) > 0) ? fn:"unknown" );
	fprintf(pStream,_T("TotalFrames %d\n"), nTotalFrames );
	fprintf(pStream,_T("FirstFrame %d\n\n"), nStartFrame );
}

// Start the exporter!
// This is the real entry point to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int AnimExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
	if(trace) DebugPrint("export starting\n");
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	// Grab the interface pointer.
	ip = i;

	// Get the options the user selected the last time
	ReadConfig();

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000000 && showPrompts) {
		// Prompt when holding shift
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ASCIIEXPORT_DLG),
			ip->GetMAXHWnd(), ExportDlgProc, (LPARAM)this))
		{
			return 1;
		}
	}
	
	// Read the list of exportable node names
	ReadExportableNames();

	// Open the stream
	pStream = _tfopen(name,_T("w"));
	if (!pStream) {
		return 0;
	}
	int tpf = GetTicksPerFrame();
	int s = ip->GetAnimRange().Start()/tpf, 
		e = ip->GetAnimRange().End()/tpf;

	nTotalFrames = e - s + 1;
	nStartFrame = s;

	WriteHeader();

	// Startup the progress bar.
	ip->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, fn, NULL);

	// Get a total node count by traversing the scene
	// We don't really need to do this, but it doesn't take long, and
	// it is nice to have an accurate progress bar.
	nTotalNodeCount = 0;
	nCurNode = 0;
	
	int numChildren = ip->GetRootNode()->NumberOfChildren();

	// first count total nodes
	nTotalNodeCount = 0;
	for (int idx=0; idx<numChildren; idx++) {
		int iCount = 0;
		if (ip->GetCancel())
			break;
		if(trace) DebugPrint("first idx %d of %d\n", idx, ip->GetRootNode()->NumberOfChildren());
		nodeEnum(ip->GetRootNode()->GetChildNode(idx), true, &iCount);
		nTotalNodeCount += iCount;
	}
	
	// Call our node enumerator.
	// The nodeEnum function will recurse into itself and 
	// export each object found in the scene.
	
	for (int idx=0; idx<numChildren; idx++) {
		if (ip->GetCancel())
			break;
		if(trace) DebugPrint("second idx %d of %d\n", idx, ip->GetRootNode()->NumberOfChildren());
		nodeEnum(ip->GetRootNode()->GetChildNode(idx), false, NULL);
	}

	// We're done. Finish the progress bar.
	ip->ProgressEnd();

	// Close the stream
	fclose(pStream);

	// Write the current options to be used next time around.
	WriteConfig();

	if(trace) DebugPrint("export done\n");
	return 1;
}

void AnimExp::SampleController(INode *n, Control *c) {
	TimeValue t;
	Point3 trans;
	Point3 scale;
	Quat rot; // thrown away in favor of aax format
	Matrix3 pmat, M_lastmove;
	AngAxis aax;
	Interval ivalid;
	Point3 firstFrameTrans;
	int tpf = GetTicksPerFrame();
	int s = ip->GetAnimRange().Start()/tpf, 
		e = ip->GetAnimRange().End()/tpf;
	int f_lastmove;

	// Sample the controller at every frame in the anim range
	// and determine at what frame the node stops moving at
	// so that we only export keys until it becomes static
	// until the end of the motion

	// seed 'last move' with initial xform
	f_lastmove = s;		// frame number of 'last move'
	c->GetValue( f_lastmove*tpf, &M_lastmove, FOREVER, CTRL_RELATIVE);

	for (int f = s; f <= e; f++)
	{
		// Sample controller at current frame time in WORLD SPACE
		t = f*tpf;
		ivalid = FOREVER;
		pmat = n->GetParentTM(t);
		c->GetValue(t, &pmat, ivalid, CTRL_RELATIVE);

		if ( !M_lastmove.Equals( pmat ) )
		{
			M_lastmove = pmat;
			f_lastmove = t/tpf;
		}
	}
	fprintf( pStream, "\t# frames: %d\n\n", f_lastmove - s + 1 );

	// Sample the controller at every frame in the anim range
	// until it stops moving
	for (int f = s; f <= f_lastmove; f++)
	{
		// Sample controller at current frame time in WORLD SPACE
		t = f*tpf;
		ivalid = FOREVER;
		pmat = n->GetParentTM(t);
		c->GetValue(t, &pmat, ivalid, CTRL_RELATIVE);

		// extract the transform components
		DecomposeMatrix(pmat, trans, rot, scale);
		aax.Set(pmat);

		// write to export file
		fprintf( pStream, "\tTransform\n\t{\n" );
		fprintf( pStream, "\t\tAxis %.7g %.7g %.7g \n", aax.axis[0], aax.axis[1], aax.axis[2] );
		fprintf( pStream, "\t\tAngle %.7g\n", aax.angle );
		fprintf( pStream, "\t\tTranslation %.7g %.7g %.7g \n", trans.x, trans.y, trans.z );
		fprintf( pStream, "\t\tScale %.7g %.7g %.7g \n",  scale.x, scale.y, scale.z );
		fprintf( pStream, "\t}\n\n" );
	}
}

// This method is the main object exporter.
// It is called once of every node in the scene. The objects are
// exported as they are encountered.

// Before recursing into the children of a node, we will export it.
// The benefit of this is that a nodes parent is always before the
// children in the resulting file. This is desired since a child's
// transformation matrix is optionally relative to the parent.

void AnimExp::dumpAnimNode(INode *node)
{
	char *name = node->GetName();
	Control *c;

	if (IsNameExportable(name))
	{
		fprintf( pStream, "Bone \"%s\"\n{\n", name );

		c = node->GetTMController();
		SampleController(node, c);

		fprintf( pStream, "}\n\n" );
	}
}


BOOL AnimExp::nodeEnum(INode* node, bool bCount, int* piCount) 
{
	if(trace) DebugPrint("node '%s'\n", node->GetName());
//	if(exportSelected && node->Selected() == FALSE)
//		return TREE_CONTINUE;


	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
		return FALSE;

	if(!exportSelected || node->Selected()) {

		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to evaluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(ip->GetTime()); 

		// The obj member of ObjectState is the actual object we will export.
		if (os.obj) {
			if (piCount)
				(*piCount)++;
			if (!bCount)
			{
				nCurNode++;
				ip->ProgressUpdate((int)((float)nCurNode/nTotalNodeCount*100.0f)); 

				// We look at the super class ID to determine the type of the object.
				switch(os.obj->SuperClassID()) {
				case GEOMOBJECT_CLASS_ID: 
					if (GetIncludeObjGeom()) dumpAnimNode(node); 
					break;
				case CAMERA_CLASS_ID:
					if (GetIncludeObjCamera()) dumpAnimNode(node); 
					break;
				case LIGHT_CLASS_ID:
					if (GetIncludeObjLight()) dumpAnimNode(node); 
					break;
				case SHAPE_CLASS_ID:
					if (GetIncludeObjShape()) dumpAnimNode(node); 
					break;
				case HELPER_CLASS_ID:
					if (GetIncludeObjHelper()) dumpAnimNode(node); 
					break;
				default:
					dumpAnimNode(node); 
					break;
				}
			}
		}
	}	
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		int iCount = 0;
		if(trace) DebugPrint("recurse %d\n", c);
		if (!nodeEnum(node->GetChildNode(c), bCount, &iCount))
			return FALSE;
		if (bCount && piCount)
			*piCount += iCount;
	}

	return TRUE;
}
