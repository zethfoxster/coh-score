#include "SkelExp.h"
#include "3dsmaxport.h"

HINSTANCE hSkelInstance;

static BOOL showPrompts;
static BOOL exportSelected;

SkelExp::SkelExp()
{
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
	m_depth = 0;
}

SkelExp::~SkelExp()
{
}

// Dialog proc
static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg,
									  WPARAM wParam, LPARAM lParam)
{
	Interval animRange;
	ISpinnerControl  *spin;

	SkelExp *exp = DLGetWindowLongPtr<SkelExp*>(hWnd); 
	switch (msg) {
	case WM_INITDIALOG:
		exp = (SkelExp*)lParam;
		DLSetWindowLongPtr(hWnd,lParam); 
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
DWORD WINAPI sfn(LPVOID arg)
{
	return(0);
}

// As an aid to any humans trying to read this file
// write the hierarchy into the text
void SkelExp::WriteHierarchy()
{
	std::vector<int> deco_stack;
	WriteHierarchyRecurse( ip->GetRootNode(), deco_stack, false );
}

void SkelExp::WriteHierarchyRecurse( INode *node, std::vector<int> &deco_stack, bool bHasSibling )
{
	if (node)
	{
		char *name = node->GetName();
		fprintf( pStream, "#\t" );
		std::vector<int>::iterator iter;
		for ( iter = deco_stack.begin(); iter != deco_stack.end(); ++iter )
			fprintf( pStream, "%s ", *iter ? "|" : " " );
		fprintf( pStream, "|_ %s\n", name );

		for (int c = 0; c < node->NumberOfChildren(); c++)
		{
			bool bChildHasSibling = c < (node->NumberOfChildren() - 1);
			deco_stack.push_back(bHasSibling ? 1:0);
			WriteHierarchyRecurse( node->GetChildNode(c), deco_stack, bChildHasSibling );
			deco_stack.pop_back();
		}
	}
}

// write the header of the export file
void SkelExp::WriteHeader()
{
	int version_major = VERSION/100;
	int version_minor = VERSION - version_major*100;

	fprintf( pStream, "# NCsoft CoH Skeleton Export\n" );
	fprintf( pStream, "# Generated from 3D Studio Max using:\n" );
	fprintf( pStream, "#\t\tPlugin: 'coh_anim_exp', Version %d, Revision %d\n", version_major, version_minor );						
	TCHAR* fn = ip->GetCurFileName();
	if (fn && _tcslen(fn) > 0)
		fprintf(pStream, _T("#\t\tSource File: %s\n\n"), fn );

	fprintf(pStream,_T("Version %d\n"), VERSION );
	fprintf(pStream,_T("SourceName %s\n"), (fn && _tcslen(fn) > 0) ? fn:"" );

	// text representation of node hierarchy
	fprintf( pStream, "\n# NODE HIERARCHY\n" );
	WriteHierarchy();
	fprintf( pStream, "\n" );
}

// Start the exporter!
// This is the real entry point to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int SkelExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	// Grab the interface pointer.
	ip = i;

	// Get the options the user selected the last time
	//ReadConfig();

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000000 && showPrompts) {
		// Prompt when holding shift
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hSkelInstance, MAKEINTRESOURCE(IDD_ASCIIEXPORT_DLG),
			ip->GetMAXHWnd(), ExportDlgProc, (LPARAM)this))
		{
			return 1;
		}
	}
	
	// Open the stream
	pStream = _tfopen(name,_T("w"));
	if (!pStream) {
		return 0;
	}

	WriteHeader();
	
	// Startup the progress bar.
	ip->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, sfn, NULL);

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
		nodeEnum(ip->GetRootNode()->GetChildNode(idx), true, &iCount);
		nTotalNodeCount += iCount;
	}

	// Call our node enumerator.
	// The nodeEnum function will recurse into itself and 
	// export each object found in the scene.
	
	for (int idx=0; idx<numChildren; idx++) {
		if (ip->GetCancel())
			break;
		nodeEnum(ip->GetRootNode()->GetChildNode(idx), false, NULL);
	}

	// We're done. Finish the progress bar.
	ip->ProgressEnd();

	// Close the stream
	fclose(pStream);

	// Write the current options to be used next time around.
	//WriteConfig();

	return 1;
}

void SkelExp::indent(void)
{
	for (int i=0; i< m_depth;++i)
	{
		fprintf( pStream, "    " );
	}
}

void SkelExp::SampleController(INode *n, Control *c) {
	TimeValue t;
	Point3 trans;
	Point3 scale;
	Quat rot; // thrown away in favor of aax format
	Matrix3 pmat;
	AngAxis aax;
	Interval ivalid;
	Point3 firstFrameTrans;
	int tpf = GetTicksPerFrame();

	// Always export frame 0 as the base pose. Allowing the animator to choose
	// the pose frame might be more flexible but also more error prone given the
	// strict base pose requirements of COH.
	int s = 0; // ip->GetAnimRange().Start()/tpf;

	pmat.IdentityMatrix();

	// Sample the controller at the given frame for the export

	// Sample controller at the frame time in PARENT LOCAL SPACE
	t = s*tpf;
	ivalid = FOREVER;
	//pmat = n->GetParentTM(t);
	c->GetValue(t, &pmat, ivalid, CTRL_RELATIVE);

	// extract the transform components
	DecomposeMatrix(pmat, trans, rot, scale);
	aax.Set(pmat);

	// write to export file
	m_depth += 1;	// increase indent for subfields
//	indent(); fprintf( pStream, "Transform\n" );
//	indent(); fprintf( pStream, "{\n" );
//		m_depth += 1;	// increase indent for subfields
		indent(); fprintf( pStream, "Axis %.7g %.7g %.7g \n", aax.axis[0], aax.axis[1], aax.axis[2] );
		indent(); fprintf( pStream, "Angle %.7g\n", aax.angle );
		indent(); fprintf( pStream, "Translation %.7g %.7g %.7g \n", trans.x, trans.y, trans.z );
		indent(); fprintf( pStream, "Scale %.7g %.7g %.7g \n",  scale.x, scale.y, scale.z );
		indent(); fprintf( pStream, "\n" );
		// dump the complete xform matrix well just for completeness and validation.
		// Not much data being written so it's not much of a burden
		for (int j = 0; j < 4; ++j)
		{
			indent(); fprintf( pStream, "Row%d %.7g %.7g %.7g \n", j, pmat.GetRow(j).x, pmat.GetRow(j).y, pmat.GetRow(j).z );
		}
//		m_depth -= 1;
//	indent(); fprintf( pStream, "}\n" );
	if (n->NumberOfChildren())
	{
		indent(); fprintf( pStream, "\n" );
		indent(); fprintf( pStream, "Children %d\n\n", n->NumberOfChildren() );
	}
	m_depth -= 1;
}

// This method is the main object exporter.
// It is called once of every node in the scene. The objects are
// exported as they are encountered.

// Before recursing into the children of a node, we will export it.
// The benefit of this is that a nodes parent is always before the
// children in the resulting file. This is desired since a child's
// transformation matrix is optionally relative to the parent.

void SkelExp::dumpSkelNode(INode *node)
{
	char *name = node->GetName();
	Control *c;

	indent(); fprintf( pStream, "Bone \"%s\"\n", name );
	indent(); fprintf( pStream, "{\n" );

	c = node->GetTMController();
	SampleController(node, c);
}


BOOL SkelExp::nodeEnum(INode* node, bool bCount, int* piCount) 
{
	bool bExportNode = false;

//	if(exportSelected && node->Selected() == FALSE)
//		return TREE_CONTINUE;


	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
		return FALSE;

	if(!exportSelected || node->Selected()) {

		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to evaluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 

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
				if (GetIncludeObjGeom()) bExportNode = true; 
				break;
			case CAMERA_CLASS_ID:
				if (GetIncludeObjCamera()) bExportNode = true; 
				break;
			case LIGHT_CLASS_ID:
				if (GetIncludeObjLight()) bExportNode = true; 
				break;
			case SHAPE_CLASS_ID:
				if (GetIncludeObjShape()) bExportNode = true; 
				break;
			case HELPER_CLASS_ID:
				if (GetIncludeObjHelper()) bExportNode = true; 
				break;
			default:
				bExportNode = true; 
				break;
				}
			}
		}
	}	
	
	// do we export this node and it's child hierarchy?
	if ( bExportNode )
	{
		dumpSkelNode(node); 
	}

	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	m_depth += 1;

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		int iCount = 0;
		if (!nodeEnum(node->GetChildNode(c), bCount, &iCount))
		{
			m_depth -= 1;
			return FALSE;
		}
		if (bCount && piCount)
			*piCount += iCount;
	}
	m_depth -= 1;

	// close the scope for this node
	if ( bExportNode )
	{
		indent(); fprintf( pStream, "}\n\n" );
	}

	return TRUE;
}