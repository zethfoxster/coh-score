#include "seqtype.h"
#include "textparser.h"
#include "seq.h"
#include "utils.h"
#include "StashTable.h"
#include "earray.h"
#include "cmdcommon.h"
#include "splat.h"
#include "error.h"
#include "file.h"
#include "timing.h"

// All parser-used defaults must be integers
#define SEQ_DEFAULT_VISSPHERERADIUS 5
#define SEQ_DEFAULT_MAX_ALPHA 255
#define SEQ_DEFAULT_TICKS_TO_LINGER_AFTER_DEATH  (20*30)
#define SEQ_DEFAULT_TICKS_TO_FADE_AWAY_AFTER_DEATH  (90)
#define SEQ_DEFAULT_NEAR_FADE_DISTANCE 350

#define SEQ_DEFAULT_RETICLEWIDTHBIAS 1



#define SEQ_DEFAULT_RETICLEHEIGHTBIAS 0.5
#define SEQ_DEFAULT_RETICLEBASEBIAS 0.0
#define SEQ_DEFAULT_FADE_OUT 100
#define SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_X 3.0
#define SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_Y 6.0
#define SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_Z 3.0
#define SEQ_CRAZY_HIGH_LOD_DISTANCE 100000
#define SEQ_DEFAULT_MINIMUM_AMBIENT 0.15

#define SEQ_LOW_SHADOW_DEPTH	3.0		//for NPCS and other non jumpers
#define SEQ_MEDIUM_SHADOW_DEPTH 15.0	//for jumpers
#define SEQ_HIGH_SHADOW_DEPTH	30.0	//for fliers


typedef struct SeqTypeList {
	SeqType **seqTypes;
} SeqTypeList;

SeqTypeList seqTypeList;

StaticDefineInt	ParseShadowType[] =
{
	DEFINE_INT
	{ "Splat",		SEQ_SPLAT_SHADOW},
	{ "None",		SEQ_NO_SHADOW},
	DEFINE_END
};

StaticDefineInt	ParseShadowQuality[] =
{
	DEFINE_INT
	{ "Low",		SEQ_LOW_QUALITY_SHADOWS},
	{ "Medium",		SEQ_MEDIUM_QUALITY_SHADOWS},
	{ "High",		SEQ_HIGH_QUALITY_SHADOWS},
	DEFINE_END
};

StaticDefineInt	ParseSeqFlags[] =
{
	DEFINE_INT
	{ "NoShallowSplash",	SEQ_NO_SHALLOW_SPLASH},
	{ "NoDeepSplash",		SEQ_NO_DEEP_SPLASH | SEQ_NO_SHALLOW_SPLASH},
	{ "UseNormalTargeting",	SEQ_USE_NORMAL_TARGETING},
	{ "UseDynamicLibraryPiece",	SEQ_USE_DYNAMIC_LIBRARY_PIECE},
	{ "UseCapsuleForRangeFinding",	SEQ_USE_CAPSULE_FOR_RANGE_FINDING},
	DEFINE_END
};

StaticDefineInt ParseRejectContFX[] =
{
	DEFINE_INT
	{ "None",		SEQ_REJECTCONTFX_NONE },
	{ "All",		SEQ_REJECTCONTFX_ALL },
	{ "External",	SEQ_REJECTCONTFX_EXTERNAL },
	DEFINE_END
};


StaticDefineInt	ParseHealthFxFlags[] =
{
	DEFINE_INT
	{ "NoCollision",	HEALTHFX_NO_COLLISION },
	DEFINE_END
};

StaticDefineInt	ParseSeqPlacement[] =
{
	DEFINE_INT
	{ "DeadOn",			SEQ_PLACE_DEAD_ON},
	{ "SetBack",		SEQ_PLACE_SETBACK},
	DEFINE_END
};

StaticDefineInt	ParseSeqCollisionType[] =
{
	DEFINE_INT
	{ "Repulsion",		SEQ_ENTCOLL_REPULSION},
	{ "BounceUp",		SEQ_ENTCOLL_BOUNCEUP},
	{ "BounceFacing",	SEQ_ENTCOLL_BOUNCEFACING},
	{ "SteadyUp",		SEQ_ENTCOLL_STEADYUP},
	{ "SteadyFacing",	SEQ_ENTCOLL_STEADYFACING},
	{ "LaunchUp",		SEQ_ENTCOLL_LAUNCHUP},
	{ "LaunchFacing",	SEQ_ENTCOLL_LAUNCHFACING},
	{ "Door",			SEQ_ENTCOLL_DOOR},
	{ "LibraryPiece",	SEQ_ENTCOLL_LIBRARYPIECE},
	{ "Capsule",		SEQ_ENTCOLL_CAPSULE},
	{ "None",			SEQ_ENTCOLL_NONE},
	{ "BoneCapsules",	SEQ_ENTCOLL_BONECAPSULES},
	DEFINE_END
};

StaticDefineInt	ParseSeqSelectionType[] =
{
	DEFINE_INT
	{ "Capsule",		SEQ_SELECTION_CAPSULE},
	{ "LibraryPiece",	SEQ_SELECTION_LIBRARYPIECE},
	{ "Bones",			SEQ_SELECTION_BONES},
	{ "Door",			SEQ_SELECTION_DOOR},
	{ "BoneCapsules",	SEQ_SELECTION_BONECAPSULES},
	DEFINE_END
};

TokenizerParseInfo ParseHealthFx[] =
{
	{ "Range",				TOK_VEC2(HealthFx,range)		},
	{ "LibraryPiece",		TOK_STRINGARRAY(HealthFx,libraryPiece)	},
	{ "OneShotFx",			TOK_STRINGARRAY(HealthFx,oneShotFx)	},
	{ "ContinuingFx",		TOK_STRINGARRAY(HealthFx,continuingFx)	},
	{ "Flags",				TOK_FLAGS(HealthFx,flags,0),	ParseHealthFxFlags	},
	{ "End",				TOK_END,			0								},
	{ "", 0, 0 }
};

TokenizerParseInfo ParseSequencerVar[] =
{
	{ "",				TOK_STRUCTPARAM | TOK_STRING( SequencerVar, variableName, 0 )		},
	{ "",				TOK_STRUCTPARAM | TOK_STRING( SequencerVar, replacementName, 0 )	},
	{	"\n",			TOK_END,							0},
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBoneCapsule[] =
{
	{ "",	TOK_STRUCTPARAM | TOK_INT(BoneCapsule, bone, 0), BoneIdEnum	},
	{ "",	TOK_STRUCTPARAM | TOK_F32(BoneCapsule, radius, 0)			},
	{ "\n",	TOK_END														},
	{ "", 0 }
};

extern TokenizerParseInfo Vec3Default1[];	// from tricks.c, for geomscale

TokenizerParseInfo ParseSeqType[] =
{
	{ "Type",					TOK_IGNORE,		0}, // hack, for reloading
	{ "Name",					TOK_FIXEDSTR(SeqType,name)				},
	{ "FileName",				TOK_CURRENTFILE(SeqType,filename)			},
	{ "FileAge",				TOK_TIMESTAMP(SeqType,fileAge)	},
	{ "Sequencer",				TOK_FIXEDSTR(SeqType,seqname)			},
	{ "SequencerType",			TOK_FIXEDSTR(SeqType,seqTypeName)		},
	{ "Var",					TOK_STRUCT(SeqType, sequencerVars, ParseSequencerVar)	},
	{ "AnimScale",				TOK_F32(SeqType, animScale, 1)	},

	{ "Graphics",				TOK_STRING(SeqType,graphics,0)	},

	{ "LOD1_Dist",				TOK_F32(SeqType,loddist[1], 0)	},
	{ "LOD2_Dist",				TOK_F32(SeqType,loddist[2], 0)	},
	{ "LOD3_Dist",				TOK_F32(SeqType,loddist[3], 0)	},

	{ "VisSphereRadius",		TOK_F32(SeqType,vissphereradius,SEQ_DEFAULT_VISSPHERERADIUS)	},
	{ "MaxAlpha",				TOK_U8(SeqType,maxAlpha,SEQ_DEFAULT_MAX_ALPHA)		},
	{ "ReverseFadeOutDist",		TOK_F32(SeqType,reverseFadeOutDist, 0)		},

	{ "FadeOutStart",			TOK_F32(SeqType,fade[0],SEQ_DEFAULT_NEAR_FADE_DISTANCE)	},
	{ "FadeOutFinish",			TOK_F32(SeqType,fade[1],0)					},
	{ "TicksToLingerAfterDeath",TOK_INT(SeqType,ticksToLingerAfterDeath,SEQ_DEFAULT_TICKS_TO_LINGER_AFTER_DEATH)		},	
	{ "TicksToFadeAwayAfterDeath",TOK_INT(SeqType,ticksToFadeAwayAfterDeath,SEQ_DEFAULT_TICKS_TO_FADE_AWAY_AFTER_DEATH)	},	

	{ "ShadowType",				TOK_INT(SeqType,shadowType,SEQ_SPLAT_SHADOW),			ParseShadowType		},
	{ "ShadowTexture",			TOK_STRING(SeqType,shadowTexture, 0) },
	{ "ShadowQuality",			TOK_INT(SeqType,shadowQuality,SEQ_MEDIUM_QUALITY_SHADOWS),	ParseShadowQuality	},
	{ "ShadowSize",				TOK_VEC3(SeqType,shadowSize)		},
	{ "ShadowOffset",			TOK_VEC3(SeqType,shadowOffset)		},

	{ "Flags",					TOK_FLAGS(SeqType,flags,0),							ParseSeqFlags		},

	{ "LightAsDoorOutside",		TOK_INT(SeqType,lightAsDoorOutside, 0)},	
	{ "RejectContinuingFX",		TOK_INT(SeqType,rejectContinuingFX, 0),				ParseRejectContFX	},	

//	{ "GeomScale",				TOK_VEC3(SeqType,geomscale,1)	},  // MAK - Default of 1 for each component isn't supported any more
	{ "GeomScale",				TOK_EMBEDDEDSTRUCT(SeqType,geomscale,Vec3Default1) },
	{ "GeomScaleMax",			TOK_VEC3(SeqType,geomscalemax)		},
	{ "CapsuleSize",			TOK_VEC3(SeqType,capsuleSize)		},
	{ "CapsuleOffset",			TOK_VEC3(SeqType,capsuleOffset)		},
	{ "BoneCapsule",			TOK_STRUCT(SeqType,boneCapsules,ParseBoneCapsule)	},

	{ "HasRandomName",			TOK_INT(SeqType,hasRandomName, 0)		},
	{ "RandomNameFile",			TOK_STRING(SeqType,randomNameFile, 0)	},

	{ "BigMonster",				TOK_INT(SeqType,bigMonster, 0)		},

	{ "Fx",						TOK_STRINGARRAY(SeqType,fx)				},
	{ "OnClickFx",				TOK_STRING(SeqType,onClickFx, 0)			},
	
	{ "HealthFx",				TOK_STRUCT(SeqType, healthFx, ParseHealthFx)	},

	//{ "MinimumAmbient",			TOK_F32,			offsetof(SeqType,minimumambient),	SEQ_DEFAULT_MINIMUM_AMBIENT }, //floats dont work here
	{ "MinimumAmbient",			TOK_F32(SeqType,minimumambient,	0) },

	{ "BoneScaleFat",			TOK_STRING(SeqType,bonescalefat, 0)		},
	{ "BoneScaleSkinny",		TOK_STRING(SeqType,bonescaleskinny, 0)	},
	{ "RandomBoneScale",		TOK_INT(SeqType,randombonescale, 0)	},

	{ "NotSelectable",			TOK_INT(SeqType,notselectable,0)		},
	{ "CollisionType",			TOK_INT(SeqType,collisionType, SEQ_ENTCOLL_CAPSULE),	ParseSeqCollisionType	},
	{ "Bounciness",				TOK_F32(SeqType,bounciness, 0)		},
	{ "Placement",				TOK_INT(SeqType,placement, SEQ_PLACE_SETBACK),	ParseSeqPlacement},
	{ "Selection",				TOK_INT(SeqType,selection, SEQ_SELECTION_BONES),ParseSeqSelectionType		},

	{ "ConstantState",			TOK_STRINGARRAY(SeqType,constantStateStr)	},

	{ "ReticleHeightBias",		TOK_F32(SeqType,reticleHeightBias, 0)	}, // default is not an integer, set elsewhere
	{ "ReticleWidthBias",		TOK_F32(SeqType,reticleWidthBias, SEQ_DEFAULT_RETICLEWIDTHBIAS)	},
	{ "ReticleBaseBias",		TOK_F32(SeqType,reticleBaseBias, 0)	}, // default is not an integer, set elsewhere

	{ "ShoulderScaleSkinny",	TOK_VEC3(SeqType,shoulderScaleSkinny)},
	{ "ShoulderScaleFat",		TOK_VEC3(SeqType,shoulderScaleFat)	},
	{ "HipScale",				TOK_VEC3(SeqType,hipScale)			},
	{ "NeckScale",				TOK_VEC3(SeqType,neckScale)			},
	{ "LegScaleVec",			TOK_VEC3(SeqType,legScale)			},


	{ "HeadScaleRange",			TOK_F32(SeqType,headScaleRange, 0)	},
	{ "ShoulderScaleRange",		TOK_F32(SeqType,shoulderScaleRange, 0)},
	{ "ChestScaleRange",		TOK_F32(SeqType,chestScaleRange, 0),	},
	{ "WaistScaleRange",		TOK_F32(SeqType,waistScaleRange, 0)	},
	{ "HipScaleRange",			TOK_F32(SeqType,hipScaleRange, 0)		},
	{ "LegScaleMin",			TOK_F32(SeqType,legScaleMin, 0)		},
	{ "LegScaleMax",			TOK_F32(SeqType,legScaleMax, 0)		},
	{ "HeadScaleMin",			TOK_VEC3(SeqType,headScaleMin)		},
	{ "HeadScaleMax",			TOK_VEC3(SeqType,headScaleMax)		},

	{ "LegScaleRatio",			TOK_F32(SeqType,legScaleRatio, 0)		},

	{ "SpawnOffsetY",			TOK_F32(SeqType,spawnOffsetY, 0)		},

	{ "NoStrafe",				TOK_INT(SeqType,noStrafe, 0)			},
	{ "TurnSpeed",				TOK_F32(SeqType,turnSpeed,0)			},

	{ "CameraHeight",			TOK_F32(SeqType,cameraHeight, SEQ_DEFAULT_CAMERA_HEIGHT)	},

	{ "Pushable",				TOK_INT(SeqType,pushable, 0)			},
	{ "FullBodyPortrait",		TOK_INT(SeqType,fullBodyPortrait, 0)	},

	{ "StaticLighting",			TOK_INT(SeqType,useStaticLighting, 0)	},

	{ "End",					TOK_END												},
	{ "", 0, 0 }

};

TokenizerParseInfo ParseSeqTypeList[] = {
	{ "Type",		TOK_STRUCT(SeqTypeList,seqTypes,ParseSeqType) },
	{ "", 0, 0 }
};


void seqTypeCleanup(SeqType *seqtype)
{
	int i;
	// Get sequencer type name from file name
	strcpy(seqtype->name, seqtype->filename);
	forwardSlashes(seqtype->name);
	forwardSlashes(seqtype->graphics);
	if (strchr(seqtype->name, '/'))
		strcpy(seqtype->name, getFileName(seqtype->name));
	if (strchr(seqtype->name, '.'))
		*strchr(seqtype->name, '.')=0;

	//Clean up LOD distances
	if(!seqtype->loddist[3])         
		seqtype->loddist[3] = SEQ_CRAZY_HIGH_LOD_DISTANCE;
	if(!seqtype->loddist[2])
		seqtype->loddist[2] = seqtype->loddist[3];
	if(!seqtype->loddist[1])
		seqtype->loddist[1] = seqtype->loddist[2];

	//Set Default collision size
	if(!seqtype->capsuleSize[0])
		seqtype->capsuleSize[0] = SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_X;
	if(!seqtype->capsuleSize[1])
		seqtype->capsuleSize[1] = SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_Y;
	if(!seqtype->capsuleSize[2])
		seqtype->capsuleSize[2] = SEQ_DEFAULT_ENTTYPE_CAPSULESIZE_Z;

	//Fix TicksToLingerAfterDeath
	if( seqtype->ticksToLingerAfterDeath <= 1  ) //Kinda dumb hack, this one
		seqtype->ticksToLingerAfterDeath = 0;

	//Fix TicksToFadeAwayAfterDeath
	if( seqtype->ticksToFadeAwayAfterDeath <= 1 ) //Kinda dumb hack, this one
		seqtype->ticksToFadeAwayAfterDeath = 0;

	//Set Default MaxAlpha
	if (!seqtype->reticleHeightBias)
		seqtype->reticleHeightBias = SEQ_DEFAULT_RETICLEHEIGHTBIAS;

	//Set Default MaxAlpha
	if (!seqtype->reticleBaseBias)
		seqtype->reticleBaseBias = SEQ_DEFAULT_RETICLEBASEBIAS;

	//Set Default fade distances.
	if(!seqtype->fade[1])
		seqtype->fade[1] = seqtype->fade[0] + SEQ_DEFAULT_FADE_OUT;

	//x and z == SEQ_DEFAULT_SHADOW_SIZE, y depends on Quality
	if( seqtype->shadowSize[0] == 0 )
		seqtype->shadowSize[0] = SEQ_DEFAULT_SHADOW_SIZE;

	if( seqtype->shadowSize[2] == 0 )
		seqtype->shadowSize[2] = SEQ_DEFAULT_SHADOW_SIZE;

	if( seqtype->shadowSize[1] == 0 )
	{
		if( seqtype->shadowQuality == SEQ_LOW_QUALITY_SHADOWS )
			seqtype->shadowSize[1] = SEQ_LOW_SHADOW_DEPTH;
		else if( seqtype->shadowQuality == SEQ_MEDIUM_QUALITY_SHADOWS )
			seqtype->shadowSize[1] = SEQ_LOW_SHADOW_DEPTH; //SEQ_MEDIUM_SHADOW_DEPTH; //changed cuz it helps perf
		else if( seqtype->shadowQuality == SEQ_HIGH_QUALITY_SHADOWS )
			seqtype->shadowSize[1] = SEQ_HIGH_SHADOW_DEPTH;
	}

	
	if( seqtype->constantStateStr )
	{
		for( i = 0 ; i < eaSize( &seqtype->constantStateStr ) ; i++ )
		{
			seqSetStateFromString( seqtype->constantState, seqtype->constantStateStr[i] );
		}
	}

	//Because tokenizer parse info won't accept floats as default values. (In itself probably the casue of some bugs)
	if( seqtype->minimumambient == 0.0 )
		seqtype->minimumambient = SEQ_DEFAULT_MINIMUM_AMBIENT;

	// TODO: make this an earray of structs? //HealthFXMultiples
	for( i = 0 ; i < eaSize( &seqtype->healthFx ) ; i++ )
	{
		HealthFx * healthFx = seqtype->healthFx[i]; 
		healthFx->range[0] *= 0.01; //make a ratio
		healthFx->range[1] *= 0.01;
	}

	//Doors must always currently select and collide the same way
	if (seqtype->collisionType == SEQ_ENTCOLL_DOOR)
		seqtype->selection = SEQ_SELECTION_DOOR;
	//Never mind -- since all doors that are actual doors are not selectable

	//If no seqType name, use the sequencer's name
	if( 0 == stricmp( seqtype->seqTypeName, "" ) )
	{
		char buf[100];
		char * end;
		strcpy( buf, seqtype->seqname );
		end = strchr( buf, '.' );
		if( end )
			*end = 0;
		strcpy( seqtype->seqTypeName, buf );
	}

#ifdef CLIENT
	//TO DO: YAY, memory leak for reloadSeqs!
	{
		char fxNameBuf[1024];
		int size = eaSize( &seqtype->sequencerVars );
		if( size )
		{
			int i;
			seqtype->sequencerVarsStash = stashTableCreateWithStringKeys( (int)(0.75 * size), StashDefault );

			for( i = 0 ; i < size ; i++ )
			{
				SequencerVar * seqVar = seqtype->sequencerVars[i];
				if( !(0 == stricmp( seqVar->replacementName, "None" ) ) )
				{
					fxCleanFileName( fxNameBuf, seqVar->replacementName );
					strcpy( seqVar->replacementName, fxNameBuf );
				}
				stashAddPointer( seqtype->sequencerVarsStash, seqVar->variableName, seqVar, false );
			}
		}
	}
#endif

	// Verify data
	if (seqtype->collisionType == SEQ_ENTCOLL_DOOR) {
		if (!seqtype->graphics) {
			ErrorFilenamef(seqtype->filename, "Has CollisionType Door but has no Graphics line, change/remove CollisionType or add a Graphics line.");
		}
	}


}

StashTable htSeqTypes;
void seqTypeLoadFiles(void)
{
	int i;
	ParserLoadFiles("ent_types", ".txt", "ent_types.bin", 0, ParseSeqTypeList, &seqTypeList, NULL, NULL, NULL, NULL);
	htSeqTypes = stashTableCreateWithStringKeys(1024, StashDeepCopyKeys);
	PERFINFO_AUTO_START("seqTypeCleanup", 1);
		for (i=eaSize(&seqTypeList.seqTypes)-1; i>=0; i--) {
			SeqType *seqType = seqTypeList.seqTypes[i];
			seqTypeCleanup(seqType);
			stashAddPointer(htSeqTypes, seqType->name, seqType, false);
		}
	PERFINFO_AUTO_STOP();
}

// For development reloading
SeqType *seqTypeLoadSingleFile(const char *fname)
{
	SeqTypeList dummy_list={0};
	errorLogFileIsBeingReloaded(fname);
	if (ParserLoadFiles(NULL, (char*)fname, NULL, 0, ParseSeqTypeList, &dummy_list, NULL, NULL, NULL, NULL)) {
		if (eaSize(&dummy_list.seqTypes)!=1) {
			ErrorFilenamef(fname, "File with more or less than 1 ent_type defined!");
			ParserDestroyStruct(ParseSeqTypeList, &dummy_list);
		} else {
			SeqType *seqType = dummy_list.seqTypes[0];
			seqTypeCleanup(seqType);
			return seqType;
		}
	}
	return NULL;
}

SeqType *seqTypeFind(const char *type_name)
{
	SeqType *oldSeqType=NULL;
	SeqType *seqType;
	assert(htSeqTypes);
	stashFindPointer( htSeqTypes, type_name, &seqType );

	//always update if the file has changed
	if( seqType && !global_state.no_file_change_check && isDevelopmentMode() )
	{  
		if( fileLastChanged(seqType->filename) != seqType->fileAge ) {
			oldSeqType = seqType;
			seqType = NULL;
		}
	}

	if (!seqType) {
		char buf[128];
		sprintf(buf,"%s%s%s", "ent_types/", type_name, ".txt");
		seqType = seqTypeLoadSingleFile(buf);
		if (oldSeqType && seqType) {
			// Fill in the global list with the new data
			ParserDestroyStruct(ParseSeqType, oldSeqType);
			ParserCopyStruct(ParseSeqType, seqType, sizeof(*seqType), oldSeqType);
			seqType = oldSeqType;
		}
	}

	return seqType;
}
