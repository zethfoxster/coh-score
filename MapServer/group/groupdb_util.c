#include <assert.h>
#include <string.h>
#include "group.h"
#include "groupgrid.h"
#include "grouptrack.h"
#include "groupfilelib.h"
#include "groupfileload.h"
#include "groupdbmodify.h"
#include "mathutil.h"
#include "error.h"

extern int		world_modified;



int groupIsLibSub(GroupDef *def)
{
	if (!def || !def->name)
		return 0;
	if (def->name[strlen(def->name)-1] == '&')
		return 1;
	return 0;
}

static int groupPieceInThisLib(GroupDef *def,GroupFile *file,const char *libdir)
{
	int		len;

	len = strlen(libdir);
	if (strcmp(libdir,"grp")==0)
	{
		if (!groupInLib(def))
			return 1;
	}
	if (!groupIsLibSub(def))
		return 0;
	if (strnicmp(libdir,def->name,len)!=0)
		return 0;
	if (def->name[len] == '_')
		return 1;
	return 0;
}

static void patchDef(const char *libdir,GroupDef *group,GroupDef *old_def,GroupDef *new_def)
{
	int		i;

	if (!group->in_use || (!groupPieceInThisLib(group,group->file,libdir) && stricmp(group->name,libdir)!=0))
		return;
	for(i=0;i<group->count;i++)
	{
		if (group->entries[i].def == old_def)
			group->entries[i].def = new_def;
		patchDef(libdir,group->entries[i].def,old_def,new_def);
	}
}

static void makeUniqueName(const char *libname,char *name)
{
	char	*s;
	int		count = 0;

	for(;;)
	{
		strcpy(name,libname);
		s = strrchr(name,'&');
		if (s && strchr(name,'/')==NULL)	//must check that we didn't find an & somewhere
		{									//in the names of the folders this object is in
			s = strrchr(name,'_');
			if (s)
				*s = 0;
		}
		if (strcmp(libname,"grp")==0)
			groupMakeName(name,"grp", 0);
		else
			sprintf(name + strlen(name),"_%d&",++count);
		if (!groupDefFind(name))
			break;
	}
}

static void moveDefsToLibInternal(GroupDef *def,GroupFile *file,const char *libname,U32 last_access,GroupDef *parent)
{
	int			i;
	char		name[256];
	GroupDef	*old_def;

	if (!def || !def->in_use)
		return;
	if (groupInLib(def) && !groupIsLibSub(def))
	{
		file = def->file;
		libname = def->name;
	}
	else
	{
		if (!groupPieceInThisLib(def,file,libname))
		{
			makeUniqueName(libname,name);
			if (groupInLib(def))
			{
				assert(parent);
				printf("renaming %s to %s\n",def->name,name);
				old_def = def;
				def = groupDefDup(def,file);
				groupDefName(def,file->fullname,name);
				groupDefModify(def);
				patchDef(libname,parent,old_def,def);
				groupDefModify(parent);
			}
			else
			{
				old_def = def;
				def = groupDefDup(def,def->file);
				groupDefModify(def);
				groupDefName(def,file->fullname,name);
				if (parent)
					patchDef(libname,parent,old_def,def);
			}
		}
	}
	if (def->access_time == last_access)
		return;
	def->access_time = last_access;
	for(i=0;i<def->count;i++)
		moveDefsToLibInternal(def->entries[i].def,file,libname,last_access,def);
}

void moveDefsToLib(GroupDef *def)
{
	char	*s,buf[1000];

	if (groupInLib(def))
	{
		strcpy(buf,def->name);
		if (groupIsLibSub(def))
		{
			s = strrchr(buf,'_');
			*s = 0;
		}
	}
	else
		strcpy(buf,"grp");
	moveDefsToLibInternal(def,def->file,buf,++group_info.def_access_time,0);
}

void delLibSubs(GroupDef *def,char *name)
{
	int		i;
	char	*s;

	if (!def || !def->in_use)
		return;
	for(i=0;i<def->count;i++)
		delLibSubs(def->entries[i].def,name);
	s = strstr(def->name,name);
	if (s)
	{
		s += strlen(name);
		if (s[0] == '_' && s[strlen(s)-1] == '&')
			groupDefFree(def,1);
	}
}

static GroupDef *trayRenameRecur(GroupDef *def,char *selname)
{
	int				i;
	char			*s,tryname[1000],partname[1000];
	GroupFileEntry	*gf;
	GroupDef		*newdef;
	static			int counter;

	if (!groupInLib(def))
	{
		groupDefModify(def);

		for(i=0;i<def->count;i++)
		{
			newdef = trayRenameRecur(def->entries[i].def,selname);
			if (newdef)
			{
				def->entries[i].def = newdef;
			}
		}
		return 0;
	}

	strcpy(partname,def->name);
	s = strchr(partname,'_');
	if (!s++)
		return 0;

	sprintf(tryname,"%s_%s",selname,s);
	gf = objectLibraryEntryFromObj(tryname);
	if (!gf)
		return 0;
	if (!gf->loaded)
	{
		extern int world_modified;

		groupFileLoadFromName(tryname);
		world_modified = 1;
	}
	newdef = groupDefFind(tryname);
	if (newdef)
		printf("%s -> %s\n",def->name,tryname);
	return newdef;
}

void groupTraySwap(char *selname,DefTracker **trackers,int count)
{
	int			i,idx;
	GroupDef	*def;
	DefTracker	*rootref;

	printf("\nrename\n");
	for(i=0;i<count;i++)
	{
		rootref = trackerRoot(trackers[i]);
		groupRefModify(rootref);
		def = trayRenameRecur(trackers[i]->def,selname);
		if (def)
		{
			if (trackers[i]->parent)
			{
				idx = trackers[i] - trackers[i]->parent->entries;
				groupDefModify(trackers[i]->parent->def);
				trackers[i]->parent->def->entries[idx].def = def;
			}
			else
			{
				trackers[i]->def = def;
			}
		}
	}
	groupRefGridDel(rootref);
	groupRefActivate(rootref);
}
