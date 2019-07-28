#include "edit_cmd_file.h"
#include "edit_cmd.h"
#include "win_init.h"
#include "edit_net.h"
#include "utils.h"
#include "sun.h"
#include <process.h>
#include "cmdgame.h"
#include "messageStore.h"
#include "language/langClientUtil.h"
#include "clientcomm.h"
#include "comm_game.h"
#include "file.h"
#include "tex.h"
#include "group.h"
#include "groupfilesave.h"
#include "MessageStoreUtil.h"
#include "GenericMesh.h"
#include "edit_select.h"
#include "anim.h"
#include "file.h"

extern char	map_loadsave_name[1024];

void editShowTitle()
{
	char		titleBuffer[1000];
	if(isDevelopmentMode())
	{
		char* mapName = game_state.world_name;
		
		if(strnicmp(mapName, "maps/", 5) == 0 || strnicmp(mapName, "maps\\", 5) == 0)
			mapName += 5;

		sprintf(titleBuffer, "City of Heroes : %s  PID: %d", mapName, _getpid());
		if (edit_state.isDirty)
			strcat(titleBuffer,"*");
	}
	else
	{
 		char mapName[1000];

 		if(game_state.map_instance_id > 1)
	 		sprintf(mapName, "%s_%d", game_state.world_name, game_state.map_instance_id);
	 	else
 			strcpy(mapName, game_state.world_name);

		msPrintf(menuMessages, SAFESTR(titleBuffer), mapName);
		
		if(!stricmp(titleBuffer, mapName))
			titleBuffer[0] = 0;
	}

	forwardSlashes(titleBuffer);
	winSetText(titleBuffer);
}

static int verifyWorldSaveable()
{
	int i;
	for (i = 0; i < group_info.ref_count; ++i) 
	{
		if (SAFE_MEMBER2(group_info.refs[i], def, name) &&
			strnicmp(group_info.refs[i]->def->name, "grp", 3) == 0)
		{
			return 1;
		}
	}

	winMsgAlert("A map must contain at least one custom group.");
	return 0;
}

void editCmdClientSave()
{
	char filename[256];

	edit_state.clientsave=0;

	if (!verifyWorldSaveable())
		return;

	if (strrchr(game_state.world_name,'.')==NULL)
		sprintf(filename,"%s.clientsave.txt",game_state.world_name);
	else
	{
		*strrchr(game_state.world_name,'.')=0;
		sprintf(filename,"%s.clientsave.txt",game_state.world_name);
		strcat(game_state.world_name,".txt");
	}
	groupSave(filename);
}

void editCmdSaveFile()
{
	char	*fname;

	if (!edit_state.savesel && !verifyWorldSaveable()) {
		edit_state.save=edit_state.saveas=edit_state.savesel=0;
		return;
	}

	if (game_state.remoteEdit && (edit_state.saveas || edit_state.savesel)) {
		edit_state.saveas = edit_state.savesel = 0;
		winMsgAlert("You cannot use Save As or Save Selection on a remote map.");
		return;
	}

	edit_state.autosavetime=clock();

	if (game_state.remoteEdit) {
		// Never give user a choice where to save in remote edit mode
		if (!game_state.world_name[0]) {
			edit_state.save = edit_state.saveas = edit_state.savesel = 0;
			winMsgAlert("The remote mapserver does not have a map loaded.");
			return;
		}

		fname = game_state.world_name;
	} else if (!game_state.world_name[0] || edit_state.saveas || edit_state.savesel)
		fname = winGetFileName("*.txt",map_loadsave_name,1);
	else
		fname = game_state.world_name;

	if (!fname) {			//otherwise if they hit cancel it will crash because fname will be NULL
		edit_state.save=edit_state.saveas=edit_state.savesel=0;
		return;
	}

	if (!strstr(fname,".txt")) {
		if (!fileExists(fname))
			strcat(fname,".txt");
	}
	if (fname)
	{
		if (edit_state.savesel)
			editSaveSelection(fname);
		else
		{
			strcpy(game_state.world_name,fname);
			editSave(fname);
			editShowTitle();

			if (game_state.remoteEdit) {
				// Also clientsave so that the files stay in sync
				groupSave(fname);
			}
		}
	}
	edit_state.savesel = edit_state.save = edit_state.saveas = 0;

	//at this point, we've saved the map.  We should store an image of it just in case it has changed.
	//minimap_saveHeader();
	//ImageCapture_WriteMapImages(fname, NULL, 1);
}

void editCmdAutosave(MenuEntry * me,ClickInfo * unused) {
	char buf[TEXT_DIALOG_MAX_STRLEN];
	char * time;
	int len;
	int i;

	if (me!=NULL)
		edit_state.autosave=!edit_state.autosave;

	if (edit_state.autosave && edit_state.autosavewait==0) {
		time = &buf[0];
		if( !winGetString("Enter number of minutes between autosaves.",time)  || !time[0] ) {
			edit_state.autosave=0;
			return;
		}
		len=strlen(time);
		for (i=0;i<len;i++)
			if (!(time[i]>='0' && time[i]<='9')) {
				edit_state.autosave=0;
			}
		if (edit_state.autosave) {
			edit_state.autosavewait=atoi(time)*CLK_TCK*60;
			if (edit_state.autosavewait==0)
				edit_state.autosave=0;
			else
				edit_state.autosavetime=clock();
		}
	}
	if (edit_state.autosave && !edit_state.isDirty)
		edit_state.autosavetime=clock();

	if (edit_state.autosave && clock()-edit_state.autosavetime>=edit_state.autosavewait && edit_state.isDirty) {
		Packet	*pak;
		char fname[256];
		if (!game_state.world_name[0])
			strcpy(fname,"noname.autosave.txt");
		else {
			char * pathend;
			strcpy(fname,game_state.world_name);
			pathend=strrchr(fname,'.');
			if (pathend!=NULL)
				strcpy(pathend,".autosave.txt");
			else
				strcat(fname,".autosave.txt");
		}
		pak = pktCreate();
		pktSendBitsPack(pak,1,CLIENT_WORLDCMD);
		pktSendString(pak,"autosave");
		pktSendString(pak,fname);
		commSendPkt(pak);
		edit_state.autosavetime=clock();
	}
	if (me!=NULL)
		if (edit_state.autosave) {
			estrPrintf(&me->name, "autosave ON: %dm", edit_state.autosavewait/(60*CLK_TCK));
		} else {
			estrPrintCharString(&me->name, "autosave OFF");
		}
}

void editCmdSceneFile()
{
	char	*s,buf[1000];

	if (!group_info.scenefile[0])
		strcpy(group_info.scenefile,"scenes/default_scene.txt");
	
	if (game_state.remoteEdit) {
		strcpy(buf,group_info.scenefile);
		if (winGetString("Enter the path to the scene file:", buf))
			s = buf;
		else
			s = 0;
	} else {
		sprintf(buf,"%s/%s",fileDataDir(),group_info.scenefile);
		backSlashes(buf);
		s = winGetFileName("*",buf,0);
	}
	forwardSlashes(buf);
	if (s)
	{
		s = strstri(buf,"/scenes/");
		s = s ? s++ : buf;

		if (s)
		{
			editSendSceneFile(s);
			sceneLoad(s);
			texCheckSwapList();
		}
	}
	edit_state.scenefile = 0;
}

void editCmdLoadScreen()
{
	char	*s,buf[1000];

	if (!group_info.loadscreen[0])
	{
		strcpy(group_info.loadscreen,map_loadsave_name);
		sprintf(buf,"%s",map_loadsave_name);
	}
	else if (game_state.remoteEdit)
		strcpy(buf,group_info.loadscreen);
	else
		sprintf(buf,"%s/%s",fileDataDir(),group_info.loadscreen);

	if (game_state.remoteEdit) {
		if (winGetString("Enter the path to the loading screen:", buf))
			s = buf;
		else
			s = 0;
	} else {
		backSlashes(buf);
		s = winGetFileName("*.txt",buf,0);
	}
	forwardSlashes(buf);
	if (s)
	{
		s = strstri(buf,"/maps/");
		s = s ? s++ : buf;
		if (s)
		{
			editSendLoadScreen(s);
		}
	}
	edit_state.loadscreen = 0;
}

static char **list_of_modelnames = 0;

static void freeStr(void *data)
{
	free(data);
}

#define roundVec3(dst, src) ((dst)[0]=round((src)[0]),(dst)[1]=round((src)[1]),(dst)[2]=round((src)[2]))
typedef enum {
	MOST_DETAILED = 0,
	LEAST_DETAILED = 1,
	ALL = 2,
} LodType;

void mergeModelIntoMesh(GMesh * mesh, Model * model, const Mat4 mat) 
{
	char modelname[MAX_PATH];
	U32 *tris;
	Vec3 *positions, *normals;
	Vec2 *texcoords;
	int i;

	sprintf(modelname, "%s/%s", model->filename, model->name);
	for (i = 0; i < eaSize(&list_of_modelnames); i++)
		if (stricmp(list_of_modelnames[i], modelname)==0)
			break;
	if (i == eaSize(&list_of_modelnames))
		eaPush(&list_of_modelnames, strdup(modelname));

	tris = calloc(model->tri_count, 3*sizeof(U32));
	positions = calloc(model->vert_count, sizeof(Vec3));
	normals = calloc(model->vert_count, sizeof(Vec3));
	texcoords = calloc(model->vert_count, sizeof(Vec2));

	modelGetTris(tris, model);
	modelGetVerts(positions, model);
	modelGetNorms(normals, model);
	modelGetSts(texcoords, model);

	for (i = 0; i < model->vert_count; i++)
	{
		Vec3 tvec;

		mulVecMat4(positions[i], mat, tvec);
		copyVec3(tvec, positions[i]);

		mulVecMat3(normals[i], mat, tvec);
		copyVec3(tvec, normals[i]);
	}

	for (i = 0; i < model->tri_count; i++)
	{
		int idx0 = tris[i*3], idx1 = tris[i*3+1], idx2 = tris[i*3+2];

		idx0 = gmeshAddVert(mesh, positions[idx0], normals[idx0], texcoords[idx0], NULL, NULL, NULL, 1);
		idx1 = gmeshAddVert(mesh, positions[idx1], normals[idx1], texcoords[idx1], NULL, NULL, NULL, 1);
		idx2 = gmeshAddVert(mesh, positions[idx2], normals[idx2], texcoords[idx2], NULL, NULL, NULL, 1);
		gmeshAddTri(mesh, idx0, idx1, idx2, 0, 0);
	}

	free(tris);
	free(positions);
	free(normals);
	free(texcoords);
}

static void mergeMostDetailedLodIntoMesh(GMesh *mesh, GroupDef *def, const Mat4 mat) 
{
	if (def->auto_lod_models)
		mergeModelIntoMesh(mesh, def->auto_lod_models[0], mat);
	else if (!def->model->trick || !def->model->trick->info || 
		def->model->trick->info->lod_near == 0.0f) {
			mergeModelIntoMesh(mesh, def->model, mat);
	}
}

static bool hasTextureCoordinates(Model *model) {
	int i;
	bool result = false;
	Vec2 *texcoords = calloc(model->vert_count, sizeof(Vec2));
	modelGetSts(texcoords, model);

	for (i = 0; i < model->vert_count; ++i) {
		if (texcoords[i][0] != 0.0f || texcoords[i][1] != 0.0f) {
			result = true;
			break;
		}
	}

	free(texcoords);
	return result;
}

static void mergeLeastDetailedTexturedLodIntoMesh(GMesh *mesh, GroupDef *def, const Mat4 mat) 
{
	if (def->auto_lod_models)
	{
		int i;
		for (i = eaSize(&def->auto_lod_models) - 1; i >= 0; --i) {
			if (hasTextureCoordinates(def->auto_lod_models[i])) {
				mergeModelIntoMesh(mesh, def->auto_lod_models[i], mat);
				return;
			}
		}
		// No models have texture coordinates, so just merge the least detailed model.
		mergeModelIntoMesh(mesh, def->auto_lod_models[eaSize(&def->auto_lod_models) - 1], mat);
	} else {
		// There doesn't seem to be any reliable way to determine if one model is an LOD of another.
		//  Checking lod_near against zero can identify highest-detail LODs, but I can't think of a
		//  good corresponding check for low LODs. Revisit if this becomes an issue.
		mergeModelIntoMesh(mesh, def->model, mat);
	}
}

static void mergeAllLodsIntoMesh(GMesh *mesh, GroupDef *def, const Mat4 mat) 
{
	if (def->auto_lod_models) {
		int i;
		for (i = 0; i < eaSize(&def->auto_lod_models); ++i)
			mergeModelIntoMesh(mesh, def->auto_lod_models[i], mat);
	} else
		mergeModelIntoMesh(mesh, def->model, mat);
}

static void mergeDefIntoMesh(GMesh *mesh, GroupDef *def, const Mat4 mat, int export_hidden, LodType lod_type, int snap_origins)
{
	int i;

	if(!def)
		return;

	if(!export_hidden && def->editor_visible_only)
		return;

	if(def->model)
	{
		switch (lod_type) {
			case MOST_DETAILED:
				mergeMostDetailedLodIntoMesh(mesh, def, mat);
				break;

			case LEAST_DETAILED:
				mergeLeastDetailedTexturedLodIntoMesh(mesh, def, mat);
				break;

			case ALL:
				mergeAllLodsIntoMesh(mesh, def, mat);
				break;
		}
	}

	for(i = 0; i < def->count; i++)
	{
		Mat4 m;
		mulMat4(mat, def->entries[i].mat, m);
		if(snap_origins)
			roundVec3(m[3], m[3]);
		mergeDefIntoMesh(mesh, def->entries[i].def, m, export_hidden, lod_type, snap_origins);
	}
}

void editCmdExportToVrml()
{
	int i, export_hidden = 0, snap_origins = 0;
	char *vrml_filename;
	char file_name[MAX_PATH] = "*.wrl";
	char modellist_filename[MAX_PATH];
	GMesh mesh = {0};
	FILE *f;
	Vec3 midbounds, minbounds={8e8,8e8,8e8}, maxbounds={-8e8,-8e8,-8e8};
	LodType lod_type;

	edit_state.exportsel = 0;

	vrml_filename = winGetFileName("Vrml Files (.wrl)", file_name, 1);
	if (!vrml_filename)
		return;

	assert(vrml_filename == file_name);

	if (!strEndsWith(vrml_filename, ".wrl"))
		strcat(file_name, ".wrl");

	changeFileExt(vrml_filename, ".txt", modellist_filename);

	export_hidden = winMsgYesNo("Export editor-only objects?");

	if (winMsgYesNo("Export highest LODs only?"))
		lod_type = MOST_DETAILED;
	else if (winMsgYesNo("For new-style LODs, export lowest textured LODs only?"))
		lod_type = LEAST_DETAILED;
	else
		lod_type = ALL;

	snap_origins = winMsgYesNo("Snap origins to integers?");

	gmeshSetUsageBits(&mesh, USE_POSITIONS|USE_NORMALS|USE_TEX1S);

	// merge models into generic mesh
	for (i = 0; i < sel_count; i++)
	{
		Mat4 mat;
		DefTracker *tracker = sel_list[i].def_tracker;
		if(edit_state.useObjectAxes)
			mulMat4(tracker->mat, edit_state.objectMatrixInverse, mat);
		else
			copyMat4(tracker->mat, mat);
		if(snap_origins)
			roundVec3(mat[3], mat[3]);
		mergeDefIntoMesh(&mesh, tracker->def, mat, export_hidden, lod_type, snap_origins);
	}

	gmeshUpdateGrid(&mesh, 1);

	if (!mesh.tri_count)
	{
		gmeshFreeData(&mesh);
		return;
	}

	// recenter mesh
	for (i = 0; i < mesh.vert_count; i++)
	{
		mesh.positions[i][2] = -mesh.positions[i][2]; // adjust to MAX's coordinates
		mesh.normals[i][2] = -mesh.normals[i][2];
		vec3RunningMinMax(mesh.positions[i], minbounds, maxbounds);
	}
	addVec3(minbounds, maxbounds, midbounds);
	scaleVec3(midbounds, 0.5f, midbounds);
	if (sel_count == 1)
	{
		copyVec3(sel_list[0].def_tracker->mat[3], midbounds);
		midbounds[2] = -midbounds[2]; // adjust to MAX's coordinates
	}
	for (i = 0; i < mesh.vert_count; i++)
		subVec3(mesh.positions[i], midbounds, mesh.positions[i]);

	// write vrml file
	f = fopen(vrml_filename, "wt");
	if (f)
	{
		fprintf(f, "\
#VRML V2.0 utf8\n\
\n\
DEF exported_mesh Transform {\n\
  translation 0 0 0\n\
  children [\n\
  DEF exported_mesh-TIMER TimeSensor { loop TRUE cycleInterval 3.33333 },\n\
    Shape {\n\
      appearance Appearance {\n\
        material Material {\n\
          diffuseColor 1.0 1.0 1.0\n\
          ambientIntensity 0.0\n\
          specularColor 0.0 0.0 0.0\n\
          shininess 0.0\n\
          transparency 0\n\
        }\n\
        texture ImageTexture {\n\
          url \"white.tga\"\n\
        }\n\
      }\n\
      geometry DEF exported_mesh-FACES IndexedFaceSet {\n\
        ccw TRUE\n\
        solid TRUE\n\
        coord DEF exported_mesh-COORD Coordinate { point [\n");
		for (i = 0; i < mesh.vert_count; i++)
			fprintf(f, "          %f %f %f,\n", mesh.positions[i][0], mesh.positions[i][1], mesh.positions[i][2]);
		fprintf(f, "          ]\n\
        }\n\
        normal Normal { vector [\n");
		for (i = 0; i < mesh.vert_count; i++)
			fprintf(f, "          %f %f %f,\n", mesh.normals[i][0], mesh.normals[i][1], mesh.normals[i][2]);
		fprintf(f, "          ]\n\
        }\n\
        normalPerVertex TRUE\n\
        texCoord DEF black_box-TEXCOORD TextureCoordinate { point [\n");
		for (i = 0; i < mesh.vert_count; i++)
			fprintf(f, "          %f %f,\n", mesh.tex1s[i][0], mesh.tex1s[i][1]);
		fprintf(f, "          ]\n\
        }\n\
        coordIndex [\n");
		for (i = 0; i < mesh.tri_count; i++)
			fprintf(f, "          %d, %d, %d, -1,\n", mesh.tris[i].idx[0], mesh.tris[i].idx[1], mesh.tris[i].idx[2]);
		fprintf(f, "          ]\n\
        texCoordIndex [\n");
		for (i = 0; i < mesh.tri_count; i++)
			fprintf(f, "          %d, %d, %d, -1,\n", mesh.tris[i].idx[0], mesh.tris[i].idx[1], mesh.tris[i].idx[2]);
		fprintf(f, "          ]\n\
        normalIndex [\n");
		for (i = 0; i < mesh.tri_count; i++)
			fprintf(f, "          %d, %d, %d, -1,\n", mesh.tris[i].idx[0], mesh.tris[i].idx[1], mesh.tris[i].idx[2]);
		fprintf(f, "          ]\n\
        }\n\
    }\n\
  ]\n\
}\n\n");

		fclose(f);
	}

	gmeshFreeData(&mesh);

	f = fopen(modellist_filename, "wt");
	if (f)
	{
		for (i = 0; i < eaSize(&list_of_modelnames); i++)
			fprintf(f, "%s\n", list_of_modelnames[i]);
		fclose(f);
	}
	eaDestroyEx(&list_of_modelnames, freeStr);
}


/*MW removed because nobody uses it, and I think it's broken
void editCmdCheckin()
{
	edit_state.save = 1;
	editCmdSaveFile();
	editCmd("checkin");
	edit_state.checkin = 0;
}*/
