#include "group.h"
#include "edit_select.h"
#include "utils.h"
#include "win_init.h"
#include "edit_cmd.h"
#include "edit_errcheck.h"
#include "StashTable.h"

int selIsLights(int checkall)
{
	GroupDef	*def;
	int			i;
	char		buf[1000];

	for (i=0;i<sel_count;i++)
	{
		def = sel_list[i].def_tracker->def;
		if (!def->entries || !(strstriConst(def->entries[0].def->name,"_omni") || strstriConst(def->entries[0].def->name,"_LightDir")) || (checkall && !def->has_light))
		{
			sprintf(buf,"%s isnt an omni, cant set lights.",def->name);
			winMsgAlert(buf);
			return 0;
		}
	}
	return 1;
}

int selIsCubemap()
{
	GroupDef	*def;
	char		buf[1000];

	if (sel_count != 1)
	{
		winMsgAlert("Only one cubemap can be adjusted at a time.");
		return 0;
	}

	def = sel_list[0].def_tracker->def;
	if (!def->entries || !strstriConst(def->entries[0].def->name,"_cubemapgen") || (!def->has_cubemap))
	{
		sprintf(buf,"%s is not a cubemap.",def->name);
		winMsgAlert(buf);
		return 0;
	}
	return 1;
}

int selIsSounds(int checkall)
{
GroupDef	*def;
int			i;
char		buf[1000];

	for (i=0;i<sel_count;i++)
	{
		def = sel_list[i].def_tracker->def;
		if (!def->entries || def->count==0 || !strstriConst(def->entries[0].def->name,"_sound") || (checkall && !def->has_sound))
		{
			sprintf(buf,"%s isnt a sound, cant set sounds.",def->name);
			winMsgAlert(buf);
			return 0;
		}
	}
	return 1;
}

int selIsFog(int checkall)
{
GroupDef	*def;
int			i;
char		buf[1000];

	for (i=0;i<sel_count;i++)
	{
		def = sel_list[i].def_tracker->def;
		if (!def->has_fog)
		{
			sprintf(buf,"%s isnt a fog controller, cant set fog.",def->name);
			winMsgAlert(buf);
			return 0;
		}
	}
	return 1;
}

int selIsBeacon()
{
GroupDef	*def;
int			i;
//char		buf[1000];
	
	for (i=0;i<sel_count;i++)
	{
		def = sel_list[i].def_tracker->def;
		if (!def->has_beacon && sel_list[i].def_tracker->parent)
		{
			// Probably the root-geo of a beacon, so go to parent.
			def = sel_list[i].def_tracker->parent->def;
			if (!def->has_beacon)
			{
				//sprintf(buf,"%s isn't a beacon, can't set beacon.",def->name);
				//winMsgAlert(buf);
				return 0;
			}
			else
			{
				sel_list[i].def_tracker = sel_list[i].def_tracker->parent;
			}
		}
	}
	return sel_count ? 1 : 0;
	
}

int isRootGeo(DefTracker *tracker)
{
	if (tracker->def->model && !tracker->def->is_tray)
	{
		winMsgAlert("You can't do that on a root-geo group");
		return 1;
	}
	return 0;
}

static int checkTracker(DefTracker *tracker,int warnlevel)
{
	DefTracker *t;
	GroupDef *def;

	if (!tracker)
		return 1;

	for(t = tracker; t; t = t->parent)
		if(stashFindPointer(t->def->properties, "SharedFrom", NULL))
		{
			if(!(warnlevel & WARN_OFF))
				winMsgAlert("You can't make changes to a shared layer.\n"
							"Remove the layer's SharedFrom property to make a local copy instead.");
			return 0;
		}

	def = tracker->def;
	if (!groupInLib(def))
		return 1;

	if (!edit_state.editorig)
	{
		if (def->lib_ungroupable || def->is_tray)
		{
			if (!tracker->parent || !groupInLib(tracker->parent->def))
				return 1;
		}
		if (!(warnlevel & WARN_OFF))
			winMsgAlert("You can only do this on a lib group with editlib or editorig- you probably "
						"want to make a container group and do this operation on it.");
		return 0;
	}
	else
	{
		if (def->lib_ungroupable && strstri(def->file->basename,"/omni") && edit_state.editorig==1)
		{
			winMsgAlert("You must use editlib to edit this, not editorig");
			return 0;
		}
		if (warnlevel & WARN_HIGH)
			return winMsgYesNo("Are you sure you want to do this operation on the library piece?");
		return 1;
	}
}

int isNonLibSelectSub(int warnlevel)
{
int			i;
DefTracker	*tracker;

	if (warnlevel & WARN_CHECKATTACH)
	{
		return checkTracker(sel.group_parent_def,warnlevel);
	}
	for(i=0;i<sel_count;i++)
	{
		tracker = sel_list[i].def_tracker;
		if (warnlevel & WARN_CHECKPARENT)
		{
			if (!tracker->parent)
				continue;
			tracker = tracker->parent;
		}
		if (checkTracker(tracker,warnlevel))
			continue;
		else
			return 0;
	}
	return 1;
}

static int *last_memp;

void editResetCmdChecker()
{
	if (last_memp && *last_memp==0)
		last_memp = 0;
}

static int checkCmdInit(int *memp)
{

	if (memp && memp == last_memp)
		return 1;
	last_memp = memp;
	return 0;
}

int isNonLibSelect(int *memp,int warnlevel)
{
	if (edit_state.noerrcheck)
		return 1;
	if (checkCmdInit(memp))
		return 1;
	return isNonLibSelectSub(warnlevel);
}

int isNonRootGeoSelect()
{
	if (sel_count != 1)
	{
		winMsgAlert("Only works with 1 group selected");
		return 0;
	}
	if (isRootGeo(sel_list[0].def_tracker))
		return 0;
	return 1;
}

int isNonLibSingleSelect(int *memp,int warnlevel)
{
	if (!isNonRootGeoSelect())
		return 0;
	if (edit_state.noerrcheck)
		return 1;
	if (checkCmdInit(memp))
		return 1;
	return isNonLibSelectSub(warnlevel);
}


