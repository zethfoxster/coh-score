#include "StashTable.h"
#include "group.h"
#include "utils.h"
#include <ctype.h>
#include "error.h"
#include <string.h>
#include "strings_opt.h"
#include "groupfileloadutil.h"
#include "assert.h"
#include "timing.h"

typedef struct NameList
{
	char		*newnames;
	StashTable	hashes;
	int			count;
	int			max_count;
	char		basename[1000];// maps/City_Zones/Trial_04_01 -or- Streets_Ruined/Ruined_Cars
} NameList;

//
void *groupRenameCreate(const char *fname)
{
	NameList	*rename = calloc(sizeof(NameList),1);
	rename->hashes = stashTableCreateWithStringKeys(5000,StashDeepCopyKeys);
	groupBaseName(fname,rename->basename);
	return rename;
}

void groupRenameFree(void *rename_void)
{
	NameList	*rename = rename_void;

	if (rename->hashes)
		stashTableDestroy(rename->hashes);
	rename->hashes = 0;
	if (rename->newnames)
		free(rename->newnames);
	free(rename);
}

//Renames and stretches the name to the full path.
void groupRename( void *rename_void, const char *oldName, char *newName, int typeToBeRenamed )
{
	int			idx;
	NameList	*rename = rename_void;
	int			dontRename = 0;
	//char		expandedOldName[400];

	PERFINFO_AUTO_START("groupRename",1);

	oldName = getFileName(oldName);
	//Debug : Refs, Defs and Entries must always be named grp, or object library.
	if( !( strnicmp( "grp", oldName, 3 ) || strnicmp( "object_library", oldName, 12 ) ) )
		assert(0);
	//End Debug
 
	//Group in a map:                         oldName ("grp1853") +  base ("maps/City_Zones/Trial_04_01")  =  expandedOldName ("maps/City_Zones/Trial_04_01/grp1853") //TO DO how object lib expand?
	//Group internal to object library piece: oldName ("grass_path_3way_03_1&") +  base ("Nature/grass_plates") = expandedOldName: ("Nature/grass_plates/grass_path_3way_03_1&")
	//Object in the object library:           oldName ("object_library/Omni/NAVCMBT") + base (Whatever) =  expandedOldName ("object_library/Omni/NAVCMBT")
	//Geometry ref in object library piece:   oldName ("_Door_City_direction_arrow") +  base ("Doors/City_Doors") = expandedOldName: ("Doors/City_Doors/_Door_City_direction_arrow")
	//(??Why _Door_City_direction_arrow not fulllpath like NAVCMBT?)
	//groupExpandName( oldName, rename->basename, expandedOldName );

	//dont rename lib piece references (or entries)
	if ( groupInLibSub( oldName ) )
	{ 
		strcpy( newName, oldName );
		PERFINFO_AUTO_STOP_CHECKED("groupRename");
		return;
	}

	//Debug : Refs, Defs and Entries must always be named grp, or object library.
	if( !( strnicmp( "grp", oldName, 3 ) || strnicmp( "maps", oldName, 4 ) ) )
		assert(0);
	if( strstriConst( oldName, "/map" ) )
		assert(0); //Why is this happening?
	if ( typeToBeRenamed != RENAME_DEF && !strstriConst( oldName, "/grp" ) ) //dont rename refs or group entries unless they have /grp in their name //TO DO what is this rule? 
		dontRename = 1;//Why whould this happen? Don't lib pieces take care of this?
	//Could happen if def is unused, shouldn't, though
	//End Debugging

	//Now we have all repeat defs + all groups and refs with "/grp" (leaving out all libpieces)

	//Has the name already been encountered in this file? 
	//If so, then figure out what we renamed it to, if anything, and use that.
	if ( stashFindInt( rename->hashes, oldName, &idx ) )
	{
		strcpy(newName, rename->newnames + idx );
	}
	else //This is the first time we've encountered this name in this file
	{
		if( !groupDefFind( oldName ) )  //If you've never seen this name before, just use it
		{
			strcpy( newName, oldName );
		}
		else  //If the def exists on this first hit, then you've got a conflict (only importing should make this happen!)
		{
			char namePrefix[256];
			char * s;

			printf("%s Needed to be renamed. Only should happen when you're importing a file or hand editing a mapfile.\n", oldName );

			strcpy( namePrefix, oldName ); //Why is this the whole path thing?
			assert(strnicmp(namePrefix,"grp",3)==0);
			s = namePrefix;
			if (s)
			{
				for( s += 4 ; *s ; s++ )
				{
					if (!isalpha((unsigned char)*s))
						break;
				}
				*s = 0;
			}
			groupMakeName( newName, namePrefix, oldName );
		}

		//In record that you saw it and what you did with it
		{ //This means most of the time, the name just points to itself
			char * s;
			s = dynArrayAdd( &rename->newnames, 1, &rename->count, &rename->max_count, strlen(newName) + 1 );
			strcpy( s, newName );
			stashAddInt( rename->hashes, oldName, s - rename->newnames ,false);
		}
	}

	PERFINFO_AUTO_STOP_CHECKED("groupRename");
}

#if SERVER
#include "cmdserver.h"
#include "groupdbmodify.h"
#endif

static int groupPruneDefsSub(int prune_libs)
{
	GroupDef	*def,*test;
	GroupFile	*file;
	char		defname[256];
	int			i,j,k,num_pruned=0;
	StashTable	valid_names;

	valid_names = stashTableCreateWithStringKeys(group_info.file_count*10,StashDeepCopyKeys);
	for(k=0;k<group_info.file_count;k++)
	{
		file = group_info.files[k];
		for(i=file->count-1;i>=0;i--)
		{
			def = file->defs[i];
			if (!def->in_use)
				continue;
			for(j=0;j<def->count;j++)
			{
				stashFindPointer( valid_names,def->entries[j].def->name, &test );
				if (!test)
					stashAddPointer(valid_names,def->entries[j].def->name, def->entries[j].def, false);
			}
		}
	}

	for(k=0;k<group_info.file_count;k++)
	{
		file = group_info.files[k];
		for(i=file->count-1;i>=0;i--)
		{
			def = file->defs[i];
			if (!def->in_use)
				continue;
			if (def->name[strlen(def->name)-1] != '&')
				continue;
			if (!prune_libs && groupInLib(def))
				continue;
			stashFindPointer( valid_names,def->name, &test );
			if (!test)
			{
				printf("pruning: %s\n",groupDefGetFullName(def,SAFESTR(defname)));
				groupDefFree(def,1);
				num_pruned++;
			}
		}
	}
	stashTableDestroy(valid_names);
	return num_pruned;
}

int groupPruneDefs(int prune_libs)
{
	int		i,num_pruned,total_pruned=0;

	//TO DO : check out map, since groupDefModify do it anymore.
	//checkoutMapFile(db_state.map_name); 

	for(i=1;;i++) 
	{
		num_pruned = groupPruneDefsSub(prune_libs);
		total_pruned += num_pruned;
		if (num_pruned)
			printf("%d defs removed in pruning pass %d\n",num_pruned,i);
		else
			break;
	}
	return total_pruned;
}

