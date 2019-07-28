#include "group.h"
#include "edit_errcheck.h"
#include "edit_net.h"
#include "edit_cmd.h"
#include "edit_select.h"
#include "win_init.h"
#include "utils.h"
#include "input.h"
#include "cmdGame.h"
#include "grouputil.h"
#include "groupfilelib.h"
#include "anim.h"
#include "edit_cmd_select.h"
#include "StashTable.h"
static char	obj_defnamedir[1024];

void editCmdUngroup()
{
	if (isNonLibSingleSelect(0,0))
		editUngroup(0);
	edit_state.ungroup = 0;
}
void editCmdGroup()
{
	if (isNonLibSelect(0,WARN_CHECKPARENT))
		editGroup(0);
	edit_state.group = 0;
}
void editCmdAttach()
{
	if (isNonLibSelect(0,WARN_CHECKATTACH))
		editGroup(1);
	edit_state.attach = 0;
}

void editCmdDetach() {
	edit_state.detach=0;
	if (sel_count==1)
		editDetach();
	else
		winMsgAlert("Only works with exactly one object selected.");
}

void editCmdReselectLast(int open)
{
	if(edit_state.last_selected_tracker)
	{
		DefTracker* tracker = edit_state.last_selected_tracker;
		
		if(!open || !tracker->edit || !tracker->count){
			while(tracker->parent && !tracker->parent->edit){
				tracker = tracker->parent;
			}
			
			selAdd(tracker, trackerRootID(tracker), 1,0);
		}
	}
}
void editCmdOpen()
{
int		i;

	for(i=0;i<sel_count;i++){
		if(!sel_list[i].def_tracker->def->has_beacon || edit_state.noerrcheck){
			trackerOpenEdit(sel_list[i].def_tracker);
		}
	}
	unSelect(1);
	edit_state.open = 0;
	
	editCmdReselectLast(1);
}
void editCmdClose()
{
	closeSelGroups();
	unSelect(1);
	sel_count = 0;
	updatePropertiesMenu = true;
	edit_state.close = edit_state.closeinst = 0;

	editCmdReselectLast(0);
}
void editCmdMouseWheelOpenClose()
{
	float wheelDelta = inpLevel(INP_MOUSEWHEEL);

	edit_state.mousewheelopenclose = 0;

	if(edit_state.pan)
		return;

 	if(game_state.ortho)
 	{
		game_state.ortho_zoom += wheelDelta * -30;
	}
	else
	{
		if(wheelDelta < 0)
			editCmdOpen();
		else
			editCmdClose();
	}
}
void editCmdUngroupAll()
{
	edit_state.ungroupall = 0;
	editCmd("ungroupall");
}

void editCmdGroupAll()
{
	edit_state.groupall = 0;
	editCmd("groupall");
}

void editCmdTraySwap()
{
	char	*name;
	static	char namebuf[TEXT_DIALOG_MAX_STRLEN];

	edit_state.trayswap = 0;
	if( winGetString("Swap",namebuf) )
	{
		name = strrchr(namebuf,'\\');
		if (!name++)
			name = namebuf;
		editTraySwap(name);
	}
}
void editCmdSetFxName()
{
	if (isNonLibSingleSelect(0,0))
	{
	GroupDef	*def;
	char		*s,*s2;

		def = sel_list[0].def_tracker->def;
		groupAllocString(&def->type_name);
		if (game_state.remoteEdit) {
			if (winGetString("Enter the path to the fx file:", def->type_name))
				s = def->type_name;
			else
				s = 0;
			backSlashes(def->type_name);
		} else {
			s = winGetFileName("*",def->type_name,0);
		}
		if (s)
		{
			if (edit_state.setfx)
			{
				s2 = strstri(s,"\\fx\\");
				if (s2)
					s2 += strlen("\\fx\\");
			}
			else
			{
				s2 = strrchr(s,'\\');
				if (!s2++)
					s2 = s;
			}
			strcpy(def->type_name,s2);
			editUpdateTracker(sel_list[0].def_tracker);
			//editUpdateDef(def);
			unSelect(2);
		}
	}
	edit_state.setfx = edit_state.settype = 0;
}

static void editNameDef(GroupDef *def,char *fullpath,char *objname)
{
	//groupDefName(def,def->file->fullname,objname);	// we could put fullpath here, but we like to let the server control loading new group files
	editUpdateDef(def,fullpath,objname);
	unSelect(2);
}

#if 0
	char		*s,name[1000];
	int			count=0;

	strcpy(name,namep);
	for(s=name;*s;s++)
	{
		if (*s == '\\')
			*s = '/';
	}
	groupDefName(def,def->file->fullname,name); // defnamefix
	editUpdateDef(def);
	unSelect(2);
}
#endif

#define MAX_CUSTOM_GROUP_NAME_LENGTH 20

extern char * firstCharAfterPrefix( char * name, const char * prefix );


void editName()
{
	GroupDef *def;
	char suggestedNewName[TEXT_DIALOG_MAX_STRLEN];
	char oldNamePrefix[128];
	char newName[256];
	char * s = 0; 
	extern GroupInfo * group_ptr;

	edit_state.name = 0;

	suggestedNewName[0] = 0;

	if (sel_count != 1)
	{
		winMsgAlert("Need one group only selected to rename it");
		return;
	}

	def = sel_list[0].def_tracker->def;

	if ( groupInLib( def ) || def->model || strstriConst( def->name, "&" ) )
	{
		winMsgAlert("Can't rename geo-groups, library pieces, or things inside a library piece");
		return;
	}

	{
		char prompt[128];
		sprintf( prompt, "Rename %s to grp_XXXXXX", def->name );
		if( !winGetString(prompt, suggestedNewName) )
			return; // user cancelled
		removeLeadingAndFollowingSpaces( suggestedNewName );
	}

  	if( !suggestedNewName[0] )
	{
		winMsgAlert("No new name given");	
		return;
	}

	if( strstri( suggestedNewName, " " ) || 
		strstri( suggestedNewName, "grp" ) || 
		strstri( suggestedNewName, "maps" ) || 
		strstri( suggestedNewName, "object_libary" ) ||
		strstri( suggestedNewName, "&" ) )
	{
		winMsgAlert("Just to be on the safe side, you can't have ' ', '&', 'Maps', 'grp', or 'object_library' anywhere in your name");	
		return;
	}

	if( strlen( suggestedNewName) > MAX_CUSTOM_GROUP_NAME_LENGTH )
	{
		char prompt[128];
		sprintf( prompt, "New name is %d characters max", MAX_CUSTOM_GROUP_NAME_LENGTH );
		winMsgAlert(prompt);	
		return;
	}

	//Preserve the prefix
	strcpy( oldNamePrefix, def->name );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grpsound"  );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grplite"   );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grpcubemap");
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grpvolume" );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grpfog"    );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grpa"      );
	if(!s) s = firstCharAfterPrefix( oldNamePrefix, "grp"       ); 
	if( !s )
	{
		winMsgAlert("Renaming '%s' Doesn't have a correct group name. I'm confused, and not doing it");
		return;
	}
	else
		*s = 0;

	//Autogrouped stuff needs to stop doing that
	if( 0 == stricmp( oldNamePrefix, "grpa" ) )
	{
		strcpy( oldNamePrefix, "grp" );
	}

	sprintf( newName, "%s_%s", oldNamePrefix, suggestedNewName );

	if (groupDefFind(newName) && groupDefRefCount(groupDefFind(newName)))
	{
		winMsgAlert("Name already used, try again");
		return;
	}
	stashRemovePointer(group_ptr->groupDefNameHashTable,def->name, NULL);
	editNameDef(def,def->file->fullname,newName);
}

//This function could be cleaner up a lot, since it used to be a generic renamer, and now only renames 
//things on the way to becoming a library piece.
void editNameNewLibraryPiece()
{
	char		*s,*objname;
	GroupDef	*def;
	char		defname[128],fullname[256];
	#define LIBNAMEFAIL(s) { winMsgAlert(s); return; }

	edit_state.makelibs = 0;

	if (sel_count == 1)
	{
		def = sel_list[0].def_tracker->def;
		strcpy(defname,def->file->fullname);
		getDirectoryName(defname);
		strcatf(defname,"/%s",def->name);
		if (sel_list[0].def_tracker->parent)
		{
			winMsgAlert("You can only makelibs on a root level group.");
			return;
		}
		if (def->model)
		{
			s = strrchr(defname,'/');
			if (!s++)
				s = defname;
			if (stricmp(s,def->model->name)==0)
				LIBNAMEFAIL("Can't rename geo-groups!");
		}
		if (!lastslash(defname)){
			sprintf(obj_defnamedir, "/maps/%s", defname);
			fileLocateWrite(obj_defnamedir, obj_defnamedir);
		}
		else if (strnicmp(defname,"maps",4) == 0)
		{
			if (!obj_defnamedir[0]) {
				fileLocateWrite(defname, obj_defnamedir);
			}
		}
		else
		{
			winMsgAlert("Warning! You are renaming an existing library piece! This can cause terrible things to happen! Make sure you know what you're doing!");
			sprintf(obj_defnamedir, "%s", defname);
			fileLocateWrite(obj_defnamedir, obj_defnamedir);
		}
		backSlashes(obj_defnamedir);
		if (obj_defnamedir[strlen(obj_defnamedir)-1] == '\\')
			strcat(obj_defnamedir,"pick_a_name");
		else if (strstr(obj_defnamedir,"pick_a_name")==NULL)
			strcat(obj_defnamedir,"\\pick_a_name");
		s = winGetFileName("*",obj_defnamedir,1);
		if (!s)
			return;
		forwardSlashes(obj_defnamedir);
		objname = strrchr(obj_defnamedir,'/');
		if (!objname)
			return;
		*objname++ = 0;
		if (strnicmp(objname,"grp",3)==0)
			LIBNAMEFAIL("Lib name can't start with \"grp\"");
		if (groupDefFind(objname) || objectLibraryEntryFromObj(objname))
			LIBNAMEFAIL("Name already used, try again");
		if (!(s = strstri(obj_defnamedir,"object_library")))
			LIBNAMEFAIL("Name must be under the object_library dir");
		sprintf(fullname,"%s/%s.txt",s,getFileName(s));
		editNameDef(def,fullname,objname);
	}
}

//Set active layer to nothing
void editCmdUnsetActiveLayer()
{
	if( sel.group_parent_def == sel.activeLayer )
	{
		sel.group_parent_def = 0;
		sel.group_parent_refid = 0;
	}
	sel.activeLayer = 0;
	sel.activeLayer_refid = 0;
}

//LAYER make active layer mean more than set parent!
void editCmdSetActiveLayer()
{
	int success = 1;
	if (!sel_count)
	{
		editCmdUnsetActiveLayer();
		success = 0;
	}
	else if (sel_count == 1 && sel_list[0].def_tracker )
	{
		char * layerName = groupDefFindPropertyValue( sel_list[0].def_tracker->def, "Layer");

		if( sel_list[0].def_tracker->tricks && sel_list[0].def_tracker->tricks->flags1 & TRICK_HIDE )
		{
			//Cant set hidden layers as active
			editCmdUnsetActiveLayer();
			success = 0;
		}
		else if( !layerName || !layerName[0] )
		{
			success = 0;
			winMsgAlert("Only layers can be set as the active layer!");
		}
		else
		{
			sel.activeLayer = sel_list[0].def_tracker;
			sel.activeLayer_refid = sel_list[0].id;

			//unSelect(1);
			if (!sel.activeLayer->def->count && !sel.activeLayer->def->is_tray)
			{
				success = 0;
				winMsgAlert("Not good idea to set this as an active layer!");
			}
		}
	}
	else
	{
		success = 0;
		winMsgAlert("Must only select one group for this operation");
	}

	edit_state.setactivelayer = 0;

	if( success )
	{
		sel.group_parent_def   = sel.activeLayer;
		sel.group_parent_refid = sel.activeLayer_refid;
	}
}

void editCmdSetParent()
{
	if (!sel_count)
	{
		sel.group_parent_def = sel.activeLayer;
		sel.group_parent_refid = sel.activeLayer_refid;
	}
	else if (sel_count == 1)
	{
		sel.group_parent_def = sel_list[0].def_tracker;
		sel.group_parent_refid = sel_list[0].id;
		unSelect(1);
		if (!sel.group_parent_def->def->count && !sel.group_parent_def->def->is_tray)
		{
			winMsgAlert("Not good idea to set this as a parent!");
		}
	}
	else
		winMsgAlert("Must only select one group for this operation");
	edit_state.setparent = 0;
}

