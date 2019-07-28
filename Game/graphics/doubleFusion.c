
#define DFENGINE_C_INTERFACE
#define DF_STATIC_LIB

#include "DFEngineHeader.h"
#include "doubleFusion.h"
#include "file.h"
#include "earray.h"
#include "error.h"
#include "cmdcommon.h"
#include "camera.h"
#include "StashTable.h"

#include "rt_tex.h"
#include "tex.h"
#include "anim.h"
#include "tricks.h"
#include "rendermodel.h"
#include "groupdrawutil.h"
#include "MemoryPool.h"
#include "ogl.h"
#include "StringUtil.h"
#include "gfxwindow.h"
#include "zOcclusion.h"
#include "sprite_text.h"
#include "sprite_font.h"
#include "gridcoll.h"
#include "cmdgame.h"
#include "AppRegCache.h"
#include "fileutil.h"
#include "utils.h"
#include "sysutil.h"
#define DOUBLEFUSION
#include "textureatlasPrivate.h"
#include "groupnetrecv.h"
#include "uiChat.h"
#include "MessageStoreUtil.h"
#include "uiOptions.h"
#include "shlobj.h"
#include "renderprim.h"
#include "AppLocale.h"
#include "win_init.h"
#include "rt_model.h"

typedef struct DFObject
{
	IDFObject *pDFObject;
	Mat4 world_mat;
	GroupDef * pGroup;
	Model * pModel;
	Vec3 lookAt;
	
	int has_orientator;

}DFObject;

static IDFEngine *s_DFEngine;
static DFEngineParams DFParams;

static DFObject ** s_eaDFObjectList;
static IDFObject * s_DFLoadingScreen;
static char * pchCurrZone;

static StashTable g_stDoubleFusionTextures=0;

static BasicTexture *doubleFusionObjectGetTexture(IDFObject *pDFObject)
{
	BasicTexture *bind=NULL;
	DFTextureInfo tTextureInfo;
	char * data;
	int i=0, format, miplevel=0, byte_count, offset=0;

	// Make sure our object is of texture type.
	if (IDFObject_GetType(pDFObject) != DFT_TEXTURE)	
		return 0;	
	// Make sure we can get the texture data directly in memory.
	if (IDFObject_GetContentDeliveryMode(pDFObject) != DFCDM_MEMORY)	
		return 0;

	if( stashFindPointer(g_stDoubleFusionTextures, IDFObject_GetName(pDFObject), &bind) )
	{
		// remake texture
		if( IDFObject_IsCreativeDataChanged( pDFObject ) )
			texGenFree(bind);
		else
			return bind;
	}

	if (DF_FAIL(IDFObject_GetTextureInfo(pDFObject, 0, &tTextureInfo)))	
		return 0;

	// Fill it in
	switch(tTextureInfo.m_eFormat) 
	{
		case DFTF_R8G8B8:
			format = GL_RGB;
			break;
		case DFTF_A8R8G8B8:
			format = GL_RGBA8;
			break;
		case DFTF_A8B8G8R8:
			format = GL_RGBA;
			break;
		case DFTF_X8R8G8B8:
			format = GL_RGBA;
			break;
		case DFTF_DXT1:	
			format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case DFTF_DXT3:
			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case DFTF_DXT5:
			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			printf("Error: Texture format not supported");
			return 0;
	}

	//miplevel = IDFObject_GetTextureLevelCount(pDFObject);
	byte_count = getImageByteCountWithMips( format, tTextureInfo.m_nWidth, tTextureInfo.m_nHeight, miplevel );
	data = calloc(byte_count,sizeof(char));

	//for (i=0; i < miplevel; i++)
	//{
		if( DF_FAIL(IDFObject_GetTextureInfo(pDFObject, i, &tTextureInfo)) )
			return 0;

		// copy the texture data from the DF object to the game texture
		IDFObject_CopyTexture(pDFObject, data+offset, i, tTextureInfo.m_nWidth, tTextureInfo.m_nHeight, (tTextureInfo.m_nWidth<<2), 0);
	//	offset = getImageByteCountWithMips(format, tTextureInfo.m_nWidth, tTextureInfo.m_nHeight, i );
	//}

	if( format == DFTF_X8R8G8B8)
	{
		for(i=3;i<byte_count;i+=4)
			data[i] = 255;
	}

	bind = texGenNew(tTextureInfo.m_nWidth, tTextureInfo.m_nHeight, (char*)IDFObject_GetName(pDFObject) );
	texGenUpdateEx(bind, data, format, miplevel);

	stashAddPointer(g_stDoubleFusionTextures, IDFObject_GetName(pDFObject), bind, 1);

	return bind;
}

BasicTexture *doubleFusionGetLoadingScreenBasicTexture()
{
	return doubleFusionObjectGetTexture(s_DFLoadingScreen);
}

BasicTexture *doubleFusionGetTexture(char * pchName)
{
	char * dfName;
	int i;

	for(i=eaSize(&s_eaDFObjectList)-1; i>=0; i--)
	{
		if( s_eaDFObjectList[i] )
		{
			DFCreativeInfo  creativeInfo = {0};

			dfName = (char*)IDFObject_GetName(s_eaDFObjectList[i]->pDFObject);
			IDFObject_GetCreativeInfo( s_eaDFObjectList[i]->pDFObject, &creativeInfo );
		
			if( stricmp( pchName, dfName ) == 0 )
			{
				if( creativeInfo.m_bIsDefault )
					return NULL; // Don't display defaults
				else
					return doubleFusionObjectGetTexture(s_eaDFObjectList[i]->pDFObject);
			}
		}
	}

	printf( "Failed to get Double Fusion Texture for Object: %s \n", pchName );
	return NULL;
}

int count = 0;
int debug_count = 0;
static void doubleFusionObjectUpdate( DFObject * pDF )
{
	Vec3 camLookAt, vecDist;
	F32 viewAngle, dist, screen_percent=0;
	Mat4 matx;
	int bOnScreen=0, bBlocked=0, bInRange = 0;
	int color = 0xffff0f0f;

 	copyVec3(  cam_info.viewmat[2], camLookAt );
	normalVec3(camLookAt);
	viewAngle = acosf(dotVec3(pDF->lookAt, camLookAt));

  	subVec3( pDF->world_mat[3], cam_info.cammat[3], vecDist);
	dist = lengthVec3Squared(vecDist);
	if( dist < (pDF->pGroup->vis_dist*pDF->pGroup->vis_dist)/2 && dist < 700*700 )
	{
		bInRange = 1;
		if(	viewAngle < RAD(90) ) // the divide by 2 is just to help ensure no false positives
		{
			mulMat4( cam_info.viewmat, pDF->world_mat, matx );

			// checks to see if center of object is on screen
			if ( gfxSphereVisible( matx[3], 1 /*pDF->pModel->radius*/ ) &&
				 gfxBoxIsVisible( pDF->pGroup->min, pDF->pGroup->max, pDF->world_mat ) && 
 	 			 zoTestSphere( matx[3], pDF->pModel->radius, 1 ) )  // occlusion check, not sure this does anything
			{
				CollInfo coll = {0};
				int i;
				Vec3 collTo, bounds, minDist, center;
				F32 jump_dist;
				bOnScreen = 1;
  				color = 0xffff0f0f;

				// Jump the length of the bounding box in look direction to keep coincident objects from blocking collision
 				// Have to make sure that the jump dist is not greater than the dist between camera and object
				subVec3(pDF->pGroup->max, pDF->pGroup->min, bounds);
		 		subVec3(cam_info.cammat[3], pDF->world_mat[3], minDist);
				jump_dist = MIN( lengthVec3(bounds),  lengthVec3(minDist)-1 );
 				normalVec3(minDist);
				
				// Start from the center of our bounds
				scaleVec3( bounds, .5, bounds );
				addVec3( pDF->pGroup->min, bounds, bounds );
	 			mulVecMat4( bounds, pDF->world_mat, center );
				
    			scaleAddVec3(minDist, jump_dist, center, collTo );

				collGrid( 0, cam_info.cammat[3], collTo, &coll, 0, COLL_GATHERTRIS );
  				bBlocked = 0;
				for(i=0; i<coll.coll_count; i++) // we have to check that our object is not counted as a blocker
				{
					DefTracker * def = coll.tri_colls[i].tracker;
					if( def->def->entries )
					{
						int j, found = 0;
						for(j=0; j < def->def->count; j++)  
						{	
							if( def->def->entries[j].def == pDF->pGroup )
								found = 1;
						}
						if(!found)
							bBlocked = 1;
					}
					else if( def->def != pDF->pGroup  )
						bBlocked = 1;
				}

				if( !bBlocked )
				{
					int area, w, h;
					DFEventVisibleParams tVisParams;

					windowClientSize(&w, &h);
					area = gfxScreenBoxBestArea(  pDF->pGroup->min, pDF->pGroup->max, pDF->world_mat );
					tVisParams.m_fVisAngle		= DEG(viewAngle);
			  		screen_percent = tVisParams.m_fVisPercentage	=((float)area)/(w*h)*100;
 					IDFObject_UpdateOnEvent( pDF->pDFObject, DFE_VISIBLE, &tVisParams );
					color = 0xffffffff;
				}

				if( game_state.debug_double_fusion)
				{
					cam_info.cammat[3][1] -= 6;
					drawLine3DWidth(cam_info.cammat[3],collTo,color,1);
					cam_info.cammat[3][1] += 6;
				}

				// Memory leak fix
				if (coll.tri_colls)
				{
					free(coll.tri_colls);
				}
			}
		}
	}


   	if( game_state.debug_double_fusion && bInRange && bOnScreen && !bBlocked  )   
	{
 		DFInstanceTrackingInfo pTrackInfo;
		
		font( &game_9 );
		font_color( 0xffff0088, 0xffff0088);
		cprntEx( 50, 50+40*debug_count, 1000, 2.f, 2.f, 0, "Object:(%.0f, %.0f, %.0f) -- View Angle: %.1f OnScreen: %i Blocked: %i InRange %i ScreenPercent %f", (cam_info.cammat[3][0]-pDF->world_mat[3][0]), (cam_info.cammat[3][1]-pDF->world_mat[3][1]), (cam_info.cammat[3][2]-pDF->world_mat[3][2]), DEG(viewAngle), bOnScreen, bBlocked, bInRange, screen_percent  );

		if(DF_OK==IDFObject_GetTrackingInfo( pDF->pDFObject, &pTrackInfo ))
			cprntEx( 50, 70+40*debug_count, 1000, 2.f, 2.f, 0, "Visible %i, FrameSize %.1f, FrameAngle %.1f, Count %u, TimeExposed %.1f, AvgSize %.1f, AvgAngle %.1f", pTrackInfo.m_bIsVisible, pTrackInfo.m_fCurFrameSize, pTrackInfo.m_fCurFrameAngle, pTrackInfo.m_nExposures, pTrackInfo.m_fTimeExposed, pTrackInfo.m_fAvgSize, pTrackInfo.m_fAvgAngle   );
		debug_count++;
		color = 0xff0fff0f;
		drawBox3D(pDF->pGroup->min, pDF->pGroup->max, pDF->world_mat, color, 4);
	}
 
  	if( !pDF->has_orientator )
		color = 0xffffff0f;
	if( game_state.debug_double_fusion )
		drawBox3D(pDF->pGroup->min, pDF->pGroup->max, pDF->world_mat, color, 2);
	count++;
}

void doubleFusionAddObjectFromModel(Model *model, Mat4 world_mat, 
	Mat4 parent_mat, GroupDef *pDef,  GroupDef *pParent, 
	TraverserDrawParams *draw, DefTracker* tracker )
{
	int i, j;
	bool bMaterialSwap;

	// Determine if any of the textures require have a doublefusion replacement trick
	for (i = 0; i < model->tex_count; i++)
	{
		TexBind *bind = model->tex_binds[i];
		TexBind *basebind = modelSubGetBaseTexture(model, (draw->tex_binds.base||draw->tex_binds.generic)?&draw->tex_binds:0, 1, draw->swapIndex, i, &bMaterialSwap);
		char * name = 0;

		if( basebind->texopt && basebind->texopt->df_name )
			name = basebind->texopt->df_name;
		else if( bind->texopt && bind->texopt->df_name )
			name = bind->texopt->df_name;
		else
		{
			RdrTexList rtexlist = {0};
			BlendModeType first_blend_mode;
			BasicTexture *actualTextures[TEXLAYER_MAX_LAYERS] = {0};

			modelGetFinalTextures(actualTextures, model,
				(draw->tex_binds.base||draw->tex_binds.generic) ? &draw->tex_binds : 0,
				1, draw->swapIndex, i, &rtexlist, &first_blend_mode, true, tracker,
				false, &name);
		}

		// Double Fusion texture replacement
 		if(name)
		{
			IDFObject * newDFObject;
			DFErrorCode result = IDFEngine_CreateDFObject(s_DFEngine, name, &newDFObject);
			if( DF_SUCCESS(result) )
			{
				DFObject * pDF = calloc(1, sizeof(DFObject));
				Vec3 lookAt;

				// look for orientator
				copyVec3( world_mat[2], lookAt );

				for(j=0; j < pParent->count; j++)  
				{	
					if( stricmp( pParent->entries[j].def->name, "DF_Ad_Orientator" ) == 0 )
					{
						Mat4 mat;
 						mulMat4Inline(parent_mat,pParent->entries[j].mat,mat);
						copyVec3(mat[2], lookAt);
						scaleVec3( lookAt, -1, lookAt);
						pDF->has_orientator = 1;
					}
				}

				pDF->pDFObject = newDFObject;
				pDF->pModel = model;
				pDF->pGroup = pDef;
				copyMat4( world_mat, pDF->world_mat );
				copyVec3( lookAt, pDF->lookAt  );
				normalVec3(pDF->lookAt);

				eaPush(&s_eaDFObjectList, pDF );

				//IDFObject_SetLocalBoundingBox(newDFObject, model->min, model->max);
				//	groupGetBoundsMinMax
				//IDFObject_SetLocalLookAt( newDFObject, lookAt );
				// this will be more complicated later
				//IDFObject_SetWorldMatrix( newDFObject, (const float*)world_mat );
			}
			else
			{
				printf( "Double Fusion Object Failed to load: %i\n", result );
			}
		} 
	}
}

static int doubleFusionAddObjectCallback(GroupDef *def, DefTracker *tracker, Mat4 world_mat, TraverserDrawParams *draw)
{
	int i,j;
	GroupEnt *childGroupEnt;

	for(childGroupEnt = def->entries,i=0; i < def->count; i++,childGroupEnt++)  
	{	
		Mat4 mat;
		mulMat4Inline(world_mat,childGroupEnt->mat,mat);
		if (childGroupEnt->def->model)
		{
			doubleFusionAddObjectFromModel(childGroupEnt->def->model, mat, 
				world_mat, childGroupEnt->def, def, draw, tracker);
		}

		for (j=0; j<EArrayGetSize(&childGroupEnt->def->welded_models); j++) 
		{
			doubleFusionAddObjectFromModel(childGroupEnt->def->welded_models[j],
				mat, world_mat, childGroupEnt->def, def, draw, tracker );
		}
	}
	
	return 1;
}

void doubleFusion_Init(void)
{
	wchar_t buf[1024];
	char filePath[MAX_PATH];
	char szUserFolder[MAX_PATH];
	int result;
	HRESULT hRes;
	static TargetingCriterion locale[] = {
		{"COH_Language", 0 },
		{"COH_Type", "Both"},
		{0}
	};

	if( s_DFEngine )
		return;

	// This method gets the "Local Settings\Application Data" folder of the current user
	hRes = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, szUserFolder);
	if (SUCCEEDED(hRes))
	{
		// Create a folder within this path, for example:NCSoft\CoX
		strcat(szUserFolder, "\\NCSoft\\CoX");
		// If it already exists, calling this will do nothing
		SHCreateDirectoryEx(NULL, szUserFolder, NULL);
	}
	else
	{
		// FAILED ...
		return;
	}

	if( isDevelopmentMode() )
		strcpy( filePath, fileDataDir() );
	else
	{// Find the directory where the game is installed.
		char dir[FILENAME_MAX];
		strcpy(filePath,getExecutableDir(dir));
	}

	strcat( filePath, "\\doublefusion" );
	forwardSlashes(filePath);
		
	s_DFEngine = CreateDFEngine();

	UTF8ToWideStrConvert(filePath, buf, ARRAY_SIZE(buf));
	DFParams.m_pRootDir = (DF_PathChar*)wcsdup(buf);
	DFParams.m_eEngineMode = DFEM_FULL;

	UTF8ToWideStrConvert(szUserFolder, buf, ARRAY_SIZE(buf));
	DFParams.m_pUserRootDir = (DF_PathChar*)wcsdup(buf);

	if( locIsBritish(getCurrentLocale()) )
		locale[0].m_lpszValues = strdup("UK");
	else if( getCurrentLocale() == LOCALE_ID_FRENCH )
		locale[0].m_lpszValues = strdup("FR");
	else if( getCurrentLocale() == LOCALE_ID_GERMAN )
		locale[0].m_lpszValues = strdup("GR");
	else
		locale[0].m_lpszValues = strdup("EN");

	
	DFParams.m_pTargetingRule = locale;

	result = IDFEngine_Start(s_DFEngine, DF_ENGINE_SDK_VERSION, &DFParams);
	if( DF_FAIL(result) )
	{
		return;
	}

	g_stDoubleFusionTextures = stashTableCreateWithStringKeys(128, StashDeepCopyKeys);
}

void doubleFusion_StartZone(char * pchZoneName)
{
	int result;
	if(pchCurrZone)
	{
		if(stricmp(pchCurrZone,pchZoneName)==0)
			return;
		doubleFusion_ExitZone();
	}

	if(!s_DFEngine)
		doubleFusion_Init();

	loadstart_printf("Loading Double Fusion Objects.. ");

	pchCurrZone = strdup(pchZoneName);
	result = IDFEngine_StartZone(s_DFEngine, pchZoneName);
	if(DF_FAIL(result))
	{
		printf( "Failed to start DoubleFusion Zone %s: %i\n", pchZoneName, result );
		return;
	}

	if( stricmp(pchCurrZone, "Hero") == 0 || stricmp(pchCurrZone, "Villain") == 0 )
	{
		Mat4 identity = {0};
		TraverserDrawParams draw = {0};

		identity[0][0] = 1;
		identity[1][1] = 1;
		identity[2][2] = 1;
		draw.callback = doubleFusionAddObjectCallback;
		draw.need_texAndTintCrc = false;
		draw.need_matricies = true;

		draw.need_texswaps = true;
		groupTreeTraverseEx(identity, &draw);
		//groupTreeTraverse(doubleFusionAddObjectCallback, 0);
	}
	else
	{
		if(DF_FAIL( IDFEngine_CreateDFObject(s_DFEngine, "TestUI", &s_DFLoadingScreen) ))
		{
			loadend_printf("Failed to create loading screen!\n");
			return;
		}
		else
			loadend_printf("Got a loading screen!\n");
	}

	loadend_printf("%d Loaded", eaSize(&s_eaDFObjectList) );
}

void doubleFusion_ExitZone(void)
{
	if(!pchCurrZone)
		return;
	eaSetSize(&s_eaDFObjectList,0);
	if(s_DFEngine)
	{
		IDFEngine_DestroyAllDfObjects(s_DFEngine,pchCurrZone);
		IDFEngine_LeaveZone(s_DFEngine,pchCurrZone);
	}
	SAFE_FREE(pchCurrZone);
}



void doubleFusion_UpdateLoop()
{
	Mat44 viewmat;
	int i;

	if(!s_DFEngine || game_state.minimized || game_state.inactiveDisplay )
		return;

 	IDFEngine_Update(s_DFEngine, TIMESTEP/30.f);

	mat43to44( cam_info.viewmat, viewmat );

	//IDFEngine_SetCameraProjMatrix(s_DFEngine, (const float*)rdr_frame_state.projectionMatrix );
	//IDFEngine_SetCameraViewMatrix(s_DFEngine, (const float*)viewmat );

	count = 0; 
	debug_count = 0;
	for(i=eaSize(&s_eaDFObjectList)-1; i>=0; i--)
	{
		doubleFusionObjectUpdate( s_eaDFObjectList[i] );
	}
}

void doubleFusion_UpdateLoadingScreens()
{
	IDFEngine_Update(s_DFEngine, TIMESTEP/30.f);
}

void doubleFusion_Shutdown(void)
{
	doubleFusion_ExitZone();
	if(s_DFEngine)
		IDFEngine_ShutDown(s_DFEngine);
}

static void doubleFusion_GetGotoPosition( DFObject * pObject, FILE *file )
{
	Vec3 vpos, vdir;
	F32 pitch, yaw;

	copyVec3(pObject->lookAt, vdir );
	scaleVec3( vdir, 10, vdir );
	addVec3(vdir, pObject->world_mat[3], vpos ); // puts ten feet away.

	scaleVec3( pObject->lookAt, -1, vdir );
	getVec3YP( vdir, &yaw, &pitch );

	if( file )
		fprintf( file, "setpospyr %1.2f %1.2f %1.2f %1.4f %1.4f %1.4f", vecParamsXYZ(vpos), pitch, yaw, 0 );
	else
		cmdParsef( "setpospyr %1.2f %1.2f %1.2f %1.4f %1.4f %1.4f", vecParamsXYZ(vpos), pitch, yaw, 0 );
}

void doubleFusion_Goto( int forward, int broken )
{
	static int idx = -1;

	if(!s_eaDFObjectList || !eaSize(&s_eaDFObjectList) )
		return;

	if( forward )
	{
		idx++;
		while(broken && idx<eaSize(&s_eaDFObjectList) && s_eaDFObjectList[idx]->has_orientator )
			idx++;
	}
	else
	{
		idx--;
		while(broken && idx>=0 && s_eaDFObjectList[idx]->has_orientator )
			idx--;
	}

	if( idx < 0 )
		idx = eaSize(&s_eaDFObjectList)-1;
	if(idx >= eaSize(&s_eaDFObjectList) )
		idx = 0;

	doubleFusion_GetGotoPosition( s_eaDFObjectList[idx], 0 );
}


void doubleFusion_WriteFile(char * pchName)
{
	int i;
	char filename[FILENAME_MAX];
	FILE *file;

	if(pchName)
		sprintf(filename, "%s", pchName );
	else
		sprintf(filename, "C:/DoubleFusionList_%s.txt", textStd(gMapName) );
	file = fileOpen( filename, "wt" );

	if (!file)
	{
		addSystemChatMsg("Couldn't Open File for Writing Double Fusion", INFO_USER_ERROR, 0);
		return;
	}

	for( i = 0; i < eaSize(&s_eaDFObjectList); i++ )
	{
		DFObject *pObject = s_eaDFObjectList[i];
		doubleFusion_GetGotoPosition(pObject,file);
		// TODO: Ad model, texture names?
		fprintf(file, "\n" );
	}

	fclose(file);

}


